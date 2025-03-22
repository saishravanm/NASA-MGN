//DO NOT RUN THIS PROGRAM YOU WILL GO DIE AND JAIL

// ADALM-PLUTO SAR (Search and Rescue) Signal Generator
// This program generates signals that simulate those from an Orion capsule's emergency beacons
// It creates two types of signals:
// 1. A 121.65 MHz swept-tone signal (standard aviation distress frequency)
// 2. A 406.025 MHz COSPAS-SARSAT message (satellite-based SAR system)
//
// Compilation: gcc -o generate_signals generate_signals.c -liio -lm
// Execution: ./generate_signals
//
// This program requires the ADALM-PLUTO Software Defined Radio (SDR) connected via USB or network

#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Memory allocation, random numbers, etc.
#include <string.h>     // String manipulation functions
#include <unistd.h>     // POSIX operating system API (sleep, etc.)
#include <stdbool.h>    // Boolean type and values
#include <math.h>       // Mathematical functions

// Define PI if not already defined by math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <signal.h>     // Signal handling (for catching Ctrl+C)
#include <iio.h>        // Industrial I/O library - for interfacing with the ADALM-PLUTO
#include <time.h>       // Time-related functions

// ===== ADALM-PLUTO Overview =====
// The ADALM-PLUTO (PlutoSDR) is a Software Defined Radio platform from Analog Devices
// It features:
// - AD9363 RF transceiver (transmitter and receiver)
// - Tunable frequencies from 325 MHz to 3.8 GHz (with unofficial support down to ~70 MHz)
// - Up to 20 MHz bandwidth
// - 12-bit ADC and DAC
// - USB or Ethernet connectivity
// - ARM Cortex-A9 processor running Linux
//
// The device processes I/Q data (In-phase and Quadrature) for RF signal generation and reception
// This allows software to create and manipulate complex RF signals directly

// ===== Signal Configuration =====
// Sample rate defines how many I/Q samples per second are processed
// Higher rates allow higher bandwidth signals but require more processing power
#define SAMPLE_RATE 2000000  // 2 MSPS (Million Samples Per Second)

// Buffer size for I/Q data
// Larger buffers reduce CPU overhead but increase latency
#define BUFFER_SIZE 1024 * 64  // 65536 samples per buffer

// ===== Emergency Beacon Frequencies =====
// 121.65 MHz: Standard aviation emergency frequency used by ELTs (Emergency Locator Transmitters)
#define FREQ_121_65_MHZ 121650000  // 121.65 MHz in Hz

// 406.025 MHz: COSPAS-SARSAT satellite system emergency frequency 
// COSPAS-SARSAT is an international satellite-based search and rescue distress alert system
#define FREQ_406_025_MHZ 406025000 // 406.025 MHz in Hz

// ===== Transmit Power Levels =====
// Power levels in milliwatts - these would be adjusted for actual emergency beacons
// Real ELTs typically transmit 100-200 mW at 121.5 MHz and 5W at 406 MHz
#define POWER_121_65_MW 100  // 100 mW for swept-tone signal
#define POWER_406_025_W 5    // 5 W for SARSAT beacon signal

// ===== Signal Timing Parameters =====
// Real 406 MHz beacons transmit a short burst every 50 seconds (±2.5 sec)
#define MSG_INTERVAL_SECONDS 53  // Send 406 MHz burst every 53 seconds
#define TOTAL_DURATION_MINUTES 7 // Total transmission duration: 7 minutes

// ===== Global Variables for IIO Library =====
// These are used to interact with the ADALM-PLUTO hardware
static struct iio_context *ctx = NULL;        // IIO context - main connection to the device
static struct iio_device *tx = NULL;          // Transmitter device
static struct iio_channel *tx0_i = NULL;      // I (In-phase) channel for transmitter 
static struct iio_channel *tx0_q = NULL;      // Q (Quadrature) channel for transmitter
static struct iio_buffer *tx_buffer = NULL;   // Buffer for transmit data
static bool stop = false;                     // Flag to control program termination

// Signal handler for Ctrl+C
// This ensures clean shutdown when the user terminates the program
void handle_sig(int sig)
{
    printf("Caught signal %d\n", sig);
    stop = true;  // Set flag to exit main loop
}

// Initialize Pluto SDR
// This function establishes connection to the device and configures the transmitter
bool init_pluto(void)
{
    // Create IIO context for Pluto device
    // First try network connection (default IP for ADALM-PLUTO)
    ctx = iio_create_context_from_uri("ip:192.168.2.1");
    if (!ctx) {
        // If network connection fails, try USB connection
        // "usb:0" identifies the first ADALM-PLUTO device connected via USB
        ctx = iio_create_context_from_uri("usb:0"); 
        if (!ctx) {
            fprintf(stderr, "Unable to create IIO context\n");
            return false;
        }
    }

    // Find transmitter device
    // "cf-ad9361-dds-core-lpc" is the IIO device name for the transmitter
    // The naming convention comes from:
    // cf = FPGA carrier for
    // ad9361 = The RF transceiver chip (technically AD9363 in ADALM-PLUTO)
    // dds = Direct Digital Synthesis
    // lpc = Low Pin Count interface
    tx = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (!tx) {
        fprintf(stderr, "No transmitter found\n");
        return false;
    }

    // Configure TX channels
    // In SDR, signals are represented as I/Q (In-phase and Quadrature) components
    // This allows representation of both amplitude and phase of the signal
    // I and Q together form a complex signal that can be easily modulated to RF
    tx0_i = iio_device_find_channel(tx, "voltage0", true);  // I channel (true = output channel)
    tx0_q = iio_device_find_channel(tx, "voltage1", true);  // Q channel (true = output channel)
    
    if (!tx0_i || !tx0_q) {
        fprintf(stderr, "TX channels not found\n");
        return false;
    }

    // Enable channels - this activates them for use
    iio_channel_enable(tx0_i);
    iio_channel_enable(tx0_q);

    // Set sample rate
    // ad9361-phy represents the physical layer functions of the AD9361/AD9363 chip
    struct iio_device *phy = iio_context_find_device(ctx, "ad9361-phy");
    if (!phy) {
        fprintf(stderr, "No ad9361-phy device found\n");
        return false;
    }
    
    // Find the sample rate control channel
    struct iio_channel *tx_samp = iio_device_find_channel(phy, "voltage0", true);
    if (!tx_samp) {
        fprintf(stderr, "TX sample rate channel not found\n");
        return false;
    }
    
    // Set the sample rate to 2 MSPS (defined above)
    iio_channel_attr_write_longlong(tx_samp, "sampling_frequency", SAMPLE_RATE);
    
    // Create buffer for transmit data
    // The buffer is used to transfer I/Q samples to the hardware
    // false = don't enable cyclic mode (which would repeat the buffer content automatically)
    tx_buffer = iio_device_create_buffer(tx, BUFFER_SIZE, false);
    if (!tx_buffer) {
        fprintf(stderr, "Could not create TX buffer\n");
        return false;
    }

    return true;
}

// Clean up and close Pluto SDR
// This properly releases resources when the program exits
void cleanup_pluto(void)
{
    // Free the transmit buffer
    if (tx_buffer) {
        iio_buffer_destroy(tx_buffer);
    }
    
    // Destroy the IIO context (disconnects from device)
    if (ctx) {
        iio_context_destroy(ctx);
    }
}

// Set TX frequency
// This configures the ADALM-PLUTO's local oscillator (LO) to the desired frequency
bool set_tx_frequency(unsigned long long frequency)
{
    struct iio_device *phy = iio_context_find_device(ctx, "ad9361-phy");
    if (!phy) {
        fprintf(stderr, "No ad9361-phy device found\n");
        return false;
    }
    
    // altvoltage1 represents the TX Local Oscillator (LO)
    // In RF systems, the LO is mixed with the baseband signal to produce the RF output
    struct iio_channel *tx_lo = iio_device_find_channel(phy, "altvoltage1", true);
    if (!tx_lo) {
        fprintf(stderr, "TX LO channel not found\n");
        return false;
    }
    
    // Set the frequency in Hz
    return iio_channel_attr_write_longlong(tx_lo, "frequency", frequency) == 0;
}

// Set TX power (attenuation in reverse - lower number means more power)
// ADALM-PLUTO controls output power through an attenuator
bool set_tx_power(double power_mw) 
{
    // Convert mW to dBm (decibels relative to 1 milliwatt)
    // dBm = 10 * log10(power_in_mW)
    double power_dbm = 10 * log10(power_mw);
    
    // Max power for Pluto is about 7 dBm at maximum gain
    // The ADALM-PLUTO controls power via an attenuator (0 to -89.75 dB)
    // Where 0 dB attenuation = max power, -89.75 dB = min power
    // So we need to calculate how much to attenuate from the max power
    double attenuation = -1 * (power_dbm - 7.0);  // Convert desired power to attenuation value
    
    // Clamp values to valid range
    if (attenuation < 0) attenuation = 0;               // Max power
    if (attenuation > 89.75) attenuation = 89.75;       // Min power
    
    // Find the physical device
    struct iio_device *phy = iio_context_find_device(ctx, "ad9361-phy");
    if (!phy) {
        fprintf(stderr, "No ad9361-phy device found\n");
        return false;
    }
    
    // Find the TX attenuation control channel
    struct iio_channel *tx_att = iio_device_find_channel(phy, "voltage0", true);
    if (!tx_att) {
        fprintf(stderr, "TX attenuation channel not found\n");
        return false;
    }
    
    // Convert attenuation to string and set it
    // "hardwaregain" is actually an attenuation value, so negative values reduce power
    char attenuation_str[10];
    snprintf(attenuation_str, sizeof(attenuation_str), "%.2f", attenuation);
    return iio_channel_attr_write(tx_att, "hardwaregain", attenuation_str) == 0;
}

// ===== Signal Generation Functions =====

// Generate swept tone for 121.65 MHz
// This mimics the audio swept tone used in aviation emergency beacons (ELTs)
// Traditional ELTs sweep from ~400 Hz to ~1200 Hz at a rate of ~2-4 Hz
void generate_swept_tone(int duration_seconds)
{
    printf("Generating 121.65 MHz swept tone for %d seconds\n", duration_seconds);
    
    // Set the transmit frequency to 121.65 MHz
    if (!set_tx_frequency(FREQ_121_65_MHZ)) {
        fprintf(stderr, "Failed to set frequency to 121.65 MHz\n");
        return;
    }
    
    // Set the transmit power to 100 mW
    if (!set_tx_power(POWER_121_65_MW)) {
        fprintf(stderr, "Failed to set power to 100 mW\n");
        return;
    }
    
    // Parameters for sweeping tone
    // Real ELTs sweep from ~400 Hz to ~1200 Hz at a rate of ~2-4 Hz
    float sweep_min = 400;  // 400 Hz - start of sweep 
    float sweep_max = 1200; // 1200 Hz - end of sweep
    float sweep_rate = 2;   // 2 Hz = one complete sweep every 0.5 second
    
    // Record start time for duration tracking
    time_t start_time = time(NULL);
    time_t current_time;
    
    // Main loop for tone generation
    while (!stop) {
        current_time = time(NULL);
        if (current_time - start_time >= duration_seconds) {
            break;  // Exit after specified duration
        }
        
        // Get buffer pointers
        // The buffer contains interleaved I/Q samples (I,Q,I,Q,...)
        int16_t *buf = (int16_t *)iio_buffer_start(tx_buffer);
        
        // Time offset within this buffer
        float time_offset = (float)(current_time - start_time);
        
        // Fill buffer with swept tone
        for (int i = 0; i < BUFFER_SIZE; i++) {
            // Calculate time for this sample
            float t = time_offset + ((float)i / SAMPLE_RATE);
            
            // Calculate instantaneous frequency using sinusoidal sweep function
            // This creates a smooth back-and-forth sweep between min and max frequencies
            // The 0.5 * (1 + sin(...)) maps the -1 to 1 sine output to 0 to 1 range
            float freq = sweep_min + (sweep_max - sweep_min) * 
                        (0.5f * (1.0f + sinf(2.0f * M_PI * sweep_rate * t)));
            
            // Calculate phase increment for this frequency
            // Phase accumulates over time to create the continuous waveform
            static float phase = 0;
            phase += 2.0f * M_PI * freq / SAMPLE_RATE;  // Phase change per sample
            
            // Keep phase in range [0, 2π] to prevent floating point precision issues
            if (phase > 2.0f * M_PI) {
                phase -= 2.0f * M_PI;
            }
            
            // Generate I/Q samples (amplitude scaled to near max)
            // I = real part = cosine of phase
            // Q = imaginary part = sine of phase
            // Together they create a complex exponential e^(jωt) that represents a pure tone
            float amplitude = 30000.0f;  // Scale to almost full 16-bit range (-32768 to 32767)
            buf[i*2] = (int16_t)(amplitude * cosf(phase));      // I channel
            buf[i*2 + 1] = (int16_t)(amplitude * sinf(phase));  // Q channel
        }
        
        // Send buffer to the hardware
        ssize_t nbytes = iio_buffer_push(tx_buffer);
        if (nbytes < 0) {
            fprintf(stderr, "Error pushing buffer: %zd\n", nbytes);
            break;
        }
    }
}

// Create COSPAS-SARSAT 406 MHz message
// COSPAS-SARSAT is the international satellite-based search and rescue system
// 406 MHz beacons transmit digital messages containing identification and location data
void generate_406_message(uint8_t *message)
{
    // Example Hex Packet from the challenge document:
    // FFFE2F970E00800127299B1E21F600657969
    
    // In a real application, this function would:
    // 1. Create a properly formatted 406 MHz beacon message according to specs
    // 2. Include proper identification, position data, country code, etc.
    // 3. Calculate and append BCH error-correction codes
    
    // For testing, we'll use the example message from the PDF
    // A real 406 MHz message is 144 bits (18 bytes) structured as:
    // - Frame sync (48 bits)
    // - Format flag (1 bit)
    // - Protocol flag (3 bits)
    // - Country code (10 bits)
    // - Beacon identification data (~60 bits)
    // - Position data (if available)
    // - BCH error correction (48 bits)
    uint8_t example_message[] = {
        0xFF, 0xFE, 0x2F, 0x97, 0x0E, 0x00, 0x80, 0x01, 
        0x27, 0x29, 0x9B, 0x1E, 0x21, 0xF6, 0x00, 0x65, 0x79, 0x69
    };
    
    // Copy the example message to the output buffer
    memcpy(message, example_message, sizeof(example_message));
}

// Transmit 406.025 MHz SARSAT message
// This simulates the short digital burst sent by COSPAS-SARSAT beacons
void transmit_406_message(void)
{
    printf("Transmitting 406.025 MHz SARSAT message\n");
    
    // Set the frequency to 406.025 MHz (COSPAS-SARSAT frequency)
    if (!set_tx_frequency(FREQ_406_025_MHZ)) {
        fprintf(stderr, "Failed to set frequency to 406.025 MHz\n");
        return;
    }
    
    // Set power to 5 W (5000 mW) - typical for real 406 MHz beacons
    if (!set_tx_power(POWER_406_025_W * 1000)) { // Convert W to mW
        fprintf(stderr, "Failed to set power to 5 W\n");
        return;
    }
    
    // Prepare message payload
    uint8_t message[18]; // 144 bits = 18 bytes
    generate_406_message(message);
    
    // ===== Manchester Encoding =====
    // Manchester encoding is a self-clocking binary encoding where:
    // - Bit 1 is encoded as a transition from high to low (10)
    // - Bit 0 is encoded as a transition from low to high (01)
    // This ensures frequent transitions for clock recovery and no DC bias
    
    // Allocate buffer for the short data burst (approx. 500 ms duration)
    // 406 MHz beacons transmit a ~500 ms burst containing 144 bits at 400 bps
    const int burst_samples = SAMPLE_RATE / 2; // 500 ms of samples at 2 MHz sample rate
    float *burst_i = malloc(burst_samples * sizeof(float));  // I samples
    float *burst_q = malloc(burst_samples * sizeof(float));  // Q samples
    
    if (!burst_i || !burst_q) {
        fprintf(stderr, "Memory allocation failed\n");
        free(burst_i);
        free(burst_q);
        return;
    }
    
    // Fill burst buffer with encoded message
    int bit_duration = SAMPLE_RATE / 400; // 400 bps -> samples per bit at our sample rate
    int sample_index = 0;
    
    // First, add a preamble of unmodulated carrier
    // This helps receivers detect the signal and establish a reference level
    for (int j = 0; j < 5000; j++) { // ~12.5 ms of unmodulated carrier
        burst_i[sample_index] = 1.0f;  // Constant amplitude
        burst_q[sample_index] = 0.0f;  // No phase shift (pure real signal)
        sample_index++;
        if (sample_index >= burst_samples) break;
    }
    
    // Transmit each bit of the message using Manchester encoding
    for (int byte_idx = 0; byte_idx < 18 && sample_index < burst_samples; byte_idx++) {
        for (int bit_idx = 7; bit_idx >= 0 && sample_index < burst_samples; bit_idx--) {
            // Extract the current bit (MSB first)
            int bit = (message[byte_idx] >> bit_idx) & 0x01;
            
            // Manchester encoding: 1 -> 10, 0 -> 01
            // First half of the bit period
            for (int i = 0; i < bit_duration/2 && sample_index < burst_samples; i++) {
                // For bit 1: First half is high (1.0)
                // For bit 0: First half is low (-1.0)
                burst_i[sample_index] = bit ? 1.0f : -1.0f;
                burst_q[sample_index] = 0.0f;  // No phase component (phase modulation would use this)
                sample_index++;
            }
            
            // Second half of the bit period
            for (int i = 0; i < bit_duration/2 && sample_index < burst_samples; i++) {
                // For bit 1: Second half is low (-1.0)
                // For bit 0: Second half is high (1.0)
                burst_i[sample_index] = bit ? -1.0f : 1.0f;
                burst_q[sample_index] = 0.0f;
                sample_index++;
            }
        }
    }
    
    // Fill the rest with unmodulated carrier
    // This ensures smooth transition back to the carrier
    for (; sample_index < burst_samples; sample_index++) {
        burst_i[sample_index] = 1.0f;
        burst_q[sample_index] = 0.0f;
    }
    
    // Transmit the burst in chunks that fit our buffer
    // We need to break up the transmission into chunks because our TX buffer
    // might be smaller than the entire burst
    for (int offset = 0; offset < burst_samples && !stop; offset += BUFFER_SIZE/2) {
        int16_t *buf = (int16_t *)iio_buffer_start(tx_buffer);
        
        // Calculate size of this chunk (handle the last partial chunk properly)
        int chunk_size = (offset + BUFFER_SIZE/2 <= burst_samples) ? 
                          BUFFER_SIZE/2 : (burst_samples - offset);
                          
        // Copy data from our burst buffer to the TX buffer
        for (int i = 0; i < chunk_size; i++) {
            // Scale to near maximum amplitude for 16-bit samples
            buf[i*2] = (int16_t)(30000.0f * burst_i[offset + i]);       // I channel
            buf[i*2 + 1] = (int16_t)(30000.0f * burst_q[offset + i]);   // Q channel
        }
        
        // Fill remaining buffer with zeros if needed (for the last chunk)
        for (int i = chunk_size; i < BUFFER_SIZE/2; i++) {
            buf[i*2] = 0;
            buf[i*2 + 1] = 0;
        }
        
        // Send buffer to hardware
        ssize_t nbytes = iio_buffer_push(tx_buffer);
        if (nbytes < 0) {
            fprintf(stderr, "Error pushing buffer: %zd\n", nbytes);
            break;
        }
    }
    
    // Free the temporary buffers
    free(burst_i);
    free(burst_q);
}

// Main program
int main(int argc, char *argv[])
{
    // Set up signal handler for Ctrl+C to allow clean shutdown
    signal(SIGINT, handle_sig);
    
    printf("SAFE-T: Orion Capsule SAR Signal Generator\n");
    printf("------------------------------------------\n");
    
    // Initialize the ADALM-PLUTO SDR
    if (!init_pluto()) {
        fprintf(stderr, "Failed to initialize ADALM-PLUTO\n");
        return -1;
    }
    
    printf("ADALM-PLUTO initialized successfully\n");
    
    // Calculate the total test duration
    int total_minutes = TOTAL_DURATION_MINUTES;
    int total_seconds = total_minutes * 60;
    
    // Calculate number of 406 MHz bursts to send based on interval
    int burst_count = (total_seconds / MSG_INTERVAL_SECONDS) + 1;
    
    printf("Beginning %d-minute test transmission\n", total_minutes);
    printf("Will transmit %d 406 MHz bursts at %d second intervals\n", burst_count, MSG_INTERVAL_SECONDS);
    
    // Record start time for scheduling
    time_t start_time = time(NULL);
    time_t current_time;
    time_t next_burst_time = start_time;  // Schedule first burst immediately
    int bursts_sent = 0;
    
    // Main loop for test duration
    printf("Starting 121.65 MHz swept-tone transmission...\n");
    
    bool tone_active = true;  // Flag to control swept tone generation
    
    while (!stop) {
        current_time = time(NULL);
        
        // Check if we've reached the total test duration
        if (current_time - start_time >= total_seconds) {
            printf("Test complete after %d minutes\n", total_minutes);
            break;
        }
        
        // Time to send a 406 MHz burst?
        if (current_time >= next_burst_time && bursts_sent < burst_count) {
            // Pause the swept tone
            tone_active = false;
            
            // Send the 406 MHz message
            transmit_406_message();
            
            // Update counters and schedule next burst
            bursts_sent++;
            next_burst_time = start_time + (bursts_sent * MSG_INTERVAL_SECONDS);
            
            printf("Sent 406 MHz burst %d of %d\n", bursts_sent, burst_count);
            
            // Resume the swept tone
            tone_active = true;
        }
        
        // Continue generating the swept tone when active
        if (tone_active) {
            // Generate swept tone for a short duration, then check if it's time for a burst
            // This allows us to interleave the continuous tone with periodic bursts
            generate_swept_tone(1);  // Generate tone for 1 second at a time
        }
    }
    
    // Clean up resources before exiting
    cleanup_pluto();
    printf("ADALM-PLUTO cleanup complete\n");
    
    return 0;
}