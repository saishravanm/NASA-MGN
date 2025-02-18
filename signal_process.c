#include "signal_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <iio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ncurses.h>
#include "data.h"
#include "decode.h"

//looks for 2.4 ghz signals 
#define TARGET_FREQUENCY 2400000000ULL
#define SAMPLE_COUNT 1024
#define SIGNAL_DETECTION_THRESHOLD 95
#define SARSAT_FREQUENCY 2406025000ULL
#define OUTPUT_SIZE 18

int check_frequency(struct iio_context *context, uint64_t frequency) {
    struct iio_device *receiver_device, *phy_device;
    struct iio_channel *i_channel, *q_channel, *lo_channel;
    struct iio_buffer *sample_buffer;
    ssize_t buffer_bytes;
    char *data_pointer, *data_end;
    ptrdiff_t step_size;

    // Find the RX device
    receiver_device = iio_context_find_device(context, "cf-ad9361-lpc");
    if (!receiver_device) {
        fprintf(stderr, "Unable to find RX device\n");
        return -1;  // Error finding RX device
    }

    // Find the PHY device to configure hardware settings
    phy_device = iio_context_find_device(context, "ad9361-phy");
    if (!phy_device) {
        fprintf(stderr, "Unable to find PHY device\n");
        return -2;  // Error finding PHY device
    }

    // Set the LO frequency to the desired frequency
    lo_channel = iio_device_find_channel(phy_device, "altvoltage0", true);
    if (!lo_channel) {
        fprintf(stderr, "Unable to find LO channel\n");
        return -3;  // Error finding LO channel
    }

    if (iio_channel_attr_write_longlong(lo_channel, "frequency", frequency) < 0) {
        fprintf(stderr, "Failed to set LO frequency to %llu Hz\n", frequency);
        return -4;  // Error setting LO frequency
    }

    // Enable the RX voltage channels for I (voltage0) and Q (voltage1)
    i_channel = iio_device_find_channel(receiver_device, "voltage0", false);
    q_channel = iio_device_find_channel(receiver_device, "voltage1", false);
    if (!i_channel || !q_channel) {
        fprintf(stderr, "Unable to find RX channels\n");
        return -5;  // Error finding RX channels
    }
    iio_channel_enable(i_channel);
    iio_channel_enable(q_channel);

    // Create a buffer to receive samples from the RX device
    sample_buffer = iio_device_create_buffer(receiver_device, SAMPLE_COUNT, false);
    if (!sample_buffer) {
        fprintf(stderr, "Unable to create RX buffer\n");
        return -6;  // Error creating buffer
    }

    // Fill the buffer with received samples
    buffer_bytes = iio_buffer_refill(sample_buffer);
    if (buffer_bytes < 0) {
        fprintf(stderr, "Error refilling RX buffer: %zd\n", buffer_bytes);
        iio_buffer_destroy(sample_buffer);
        return -7;  // Error refilling buffer
    }

    // Process the samples and calculate the magnitude
    step_size = iio_buffer_step(sample_buffer);
    data_end = iio_buffer_end(sample_buffer);
    double total_magnitude = 0;
    int16_t i_sample_value, q_sample_value;
    for (data_pointer = iio_buffer_first(sample_buffer, i_channel); data_pointer < data_end; data_pointer += step_size) {
        int16_t *sample_pointer = (int16_t*)data_pointer;
        i_sample_value = sample_pointer[0];  // I sample
        q_sample_value = sample_pointer[1];  // Q sample
        double sample_magnitude = sqrt((double)i_sample_value * i_sample_value + (double)q_sample_value * q_sample_value);
        total_magnitude += sample_magnitude;
    }

    // Calculate the average magnitude
    double average_magnitude = total_magnitude / SAMPLE_COUNT;
    printf("Average magnitude: %f\n", average_magnitude);

    // Determine if signal is detected based on the threshold
    if (average_magnitude > SIGNAL_DETECTION_THRESHOLD) {
        iio_buffer_destroy(sample_buffer);
        return 1;  // Signal detected
    }

    // Clean up
    iio_buffer_destroy(sample_buffer);
    return 0;  // No signal detected
}

char* detect_sarsat_signal(struct iio_context *context, uint64_t frequency) {
    struct iio_device *receiver_device, *phy_device;
    struct iio_channel *i_channel, *q_channel, *lo_channel;
    struct iio_buffer *sample_buffer;
    ssize_t buffer_bytes;
    static char output[OUTPUT_SIZE];

    // Find the RX device
    receiver_device = iio_context_find_device(context, "cf-ad9361-lpc");
    if (!receiver_device) {
        fprintf(stderr, "Unable to find RX device\n");
        strncpy(output, "Error: RX device", OUTPUT_SIZE);
        return output;
    }

    // Find the PHY device to configure hardware settings
    phy_device = iio_context_find_device(context, "ad9361-phy");
    if (!phy_device) {
        fprintf(stderr, "Unable to find PHY device\n");
        strncpy(output, "Error: PHY device", OUTPUT_SIZE);
        return output;
    }

    // Set the LO frequency to the desired SARSAT frequency
    lo_channel = iio_device_find_channel(phy_device, "altvoltage0", true);
    if (!lo_channel) {
        fprintf(stderr, "Unable to find LO channel\n");
        strncpy(output, "Error: LO channel", OUTPUT_SIZE);
        return output;
    }

    if (iio_channel_attr_write_longlong(lo_channel, "frequency", frequency) < 0) {
        fprintf(stderr, "Failed to set LO frequency to %llu Hz\n", frequency);
        strncpy(output, "Error: Set LO freq", OUTPUT_SIZE);
        return output;
    }

    // Enable the RX voltage channels for I (voltage0) and Q (voltage1)
    i_channel = iio_device_find_channel(receiver_device, "voltage0", false);
    q_channel = iio_device_find_channel(receiver_device, "voltage1", false);
    if (!i_channel || !q_channel) {
        fprintf(stderr, "Unable to find RX channels\n");
        strncpy(output, "Error: RX channels", OUTPUT_SIZE);
        return output;
    }
    iio_channel_enable(i_channel);
    iio_channel_enable(q_channel);

    // Create a buffer to receive samples from the RX device
    sample_buffer = iio_device_create_buffer(receiver_device, SAMPLE_COUNT, false);
    if (!sample_buffer) {
        fprintf(stderr, "Unable to create RX buffer\n");
        strncpy(output, "Error: Create buffer", OUTPUT_SIZE);
        return output;
    }

    // Fill the buffer with received samples
    buffer_bytes = iio_buffer_refill(sample_buffer);
    if (buffer_bytes < 0) {
        fprintf(stderr, "Error refilling RX buffer: %zd\n", buffer_bytes);
        iio_buffer_destroy(sample_buffer);
        strncpy(output, "Error: Refill buffer", OUTPUT_SIZE);
        return output;
    }

    // Check if buffer is empty (no valid data)
    if (buffer_bytes == 0) {
        fprintf(stderr, "Empty buffer received\n");
        iio_buffer_destroy(sample_buffer);
        strncpy(output, "Error: Empty buffer", OUTPUT_SIZE);
        return output;
    }

    // Example: Return raw signal data as "RAW_SIGNAL_XXXX" placeholder
    strncpy(output, "RAW_SIGNAL_DATA", OUTPUT_SIZE);  // Replace with actual signal data

    // Clean up
    iio_buffer_destroy(sample_buffer);
    return output;
}

void beacon_search(void)
{
    // Create the IIO context using USB connection
    struct iio_context *context = iio_create_context_from_uri("usb:");
    if (!context) {
        printw("Unable to create IIO context\n");
        return;
    }

    int beacon_check = check_frequency(context, TARGET_FREQUENCY);
    if (beacon_check == 1) {
        printw("Signal detected at %llu Hz\n", TARGET_FREQUENCY);
    } 
    else if (beacon_check == 0) {
        printw("No signal detected at %llu Hz\n", TARGET_FREQUENCY);
    } 
    else {
        // Print the error code returned from check_frequency
        printw("Error: code %d\n", beacon_check);
    }

    char *sarsat_result = detect_sarsat_signal(context, SARSAT_FREQUENCY);
    if (strcmp(sarsat_result, "RAW_SIGNAL_DATA") != 0) {
        printw("Sarsat signal detected: %s\n", sarsat_result);

        // Prepare the DATA structure to hold the decoded SARSAT signal
        DATA data;
        data_memcpy(&data, sarsat_result);

        // Extract necessary fields from the DATA structure
        COUNTRY_CODE cc = read_country_code(&data); 
        COORD coords     = read_coordinates(&data);
        time_t current_time = time(NULL);

        // Send the short data burst with the decoded information
        // (From decode.h)
        short_data_burst(&cc, NULL, &coords, current_time);
    } 
    else {
        printw("No SARSAT signal detected.\n");
    }

    iio_context_destroy(context);
}