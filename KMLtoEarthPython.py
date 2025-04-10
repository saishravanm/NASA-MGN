import os
import re
import subprocess
import webbrowser

# List all .kml files in the current directory
kml_files = [f for f in os.listdir('.') if f.lower().endswith('.kml')]
if not kml_files:
    print("No KML files found in the current directory.")
    exit(1)  # Exit if no KML files are present

# Display the list of KML files for the user to choose from
print("KML files in current directory:")
for idx, filename in enumerate(kml_files, start=1):
    print(f"{idx}. {filename}")
print()  # blank line for readability

# Prompt the user to select a file by number
selected_index = None
while selected_index is None:
    choice = input("Enter the number of the KML file to select (or 'q' to quit): ").strip()
    if choice.lower() == 'q':
        print("Exiting without selecting a file.")
        exit(0)
    if choice.isdigit():
        num = int(choice)
        if 1 <= num <= len(kml_files):
            selected_index = num - 1
        else:
            print(f"Invalid selection. Please enter a number between 1 and {len(kml_files)}.")
    else:
        print("Please enter a valid number.")

selected_file = kml_files[selected_index]
file_path = os.path.abspath(selected_file)  # full path of the selected KML file
print(f"\nSelected file: {selected_file}\n")

# Define a helper to open a URL in Chrome (or default browser if Chrome not found)
def open_url_in_browser(url: str):
    """Open the given URL in Chrome via Windows; fall back to default browser if needed."""
    # Standard locations for Chrome on Windows (64-bit and 32-bit)
    chrome_paths = [
        "/mnt/c/Program Files/Google/Chrome/Application/chrome.exe",
        "/mnt/c/Program Files (x86)/Google/Chrome/Application/chrome.exe"
    ]
    for chrome in chrome_paths:
        if os.path.isfile(chrome):
            # Launch Chrome with the URL (no waiting for it to close)
            try:
                subprocess.Popen([chrome, url], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                return
            except Exception as e:
                print(f"Error launching Chrome: {e}")
                break
    # If we reach here, Chrome was not found or failed to launch
    print("Google Chrome not found or could not be launched. Opening URL with the default browser...")
    # Use Windows "start" command via cmd to open the URL in the default browser
    try:
        subprocess.Popen(["cmd.exe", "/c", "start", "", url], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"Failed to open the URL in default browser: {e}")

# Present the four options to the user
print("Choose an option for the selected KML file:")
print("1. Open the containing folder in Windows Explorer")
print("2. Open the KML file in Google Earth (via Chrome browser)")
print("3. Extract coordinates and open in Google Maps (Chrome)")
print("4. Extract coordinates, open in Google Maps, and save the link to GoogleMapsLinks.txt")
print()

# Prompt the user to select an option
option = None
while option is None:
    choice = input("Enter option number (1-4, or 'q' to quit): ").strip()
    if choice.lower() == 'q':
        print("Exiting without performing any action.")
        exit(0)
    if choice.isdigit():
        num = int(choice)
        if 1 <= num <= 4:
            option = num
        else:
            print("Invalid option. Please enter a number between 1 and 4.")
    else:
        print("Please enter a valid option number.")

# Execute the chosen option
if option == 1:
    # Option 1: Open the directory containing the KML file in Windows Explorer
    folder_path = os.path.dirname(file_path) or os.getcwd()
    try:
        # Convert WSL path to Windows path before calling explorer.exe
        win_path = subprocess.check_output(["wslpath", "-w", folder_path]).decode().strip()
    except Exception as e:
        print(f"Error converting path for Explorer: {e}")
        win_path = folder_path  # fallback to original (may fail if used)
    print(f"Opening folder in Windows Explorer: {win_path}")
    try:
        # Launch Explorer on the folder (opens in Windows File Explorer)
        subprocess.Popen(["explorer.exe", win_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except Exception as e:
        print(f"Failed to open Explorer: {e}")

elif option == 2:
    # Option 2: Open the KML file in Google Earth via Chrome
    # Google Earth Web cannot directly load local KML via URL parameters as of now,
    # so we open the Earth web application and the user may import the KML manually.
    earth_url = "https://earth.google.com/web"
    print("Opening Google Earth web in Chrome... (you may need to load the KML file manually in the Earth interface)")
    open_url_in_browser(earth_url)

elif option == 3 or option == 4:
    # Option 3 or 4: Extract coordinates from KML and open in Google Maps
    # Read the KML file content to find coordinates
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            kml_text = f.read()
    except Exception as e:
        print(f"Error reading the KML file: {e}")
        exit(1)

    # Attempt to find the first occurrence of coordinates in the KML
    coords_match = re.search(r'<coordinates>\s*([\d\.\-]+),([\d\.\-]+)', kml_text)
    # The regex above captures two numbers (lon, lat) at the start of a <coordinates> tag.
    lat_val = None
    lon_val = None
    if coords_match:
        lon_val = coords_match.group(1).strip()
        lat_val = coords_match.group(2).strip()
    if not coords_match or not lat_val or not lon_val:
        print("No coordinates found in the KML file or unable to parse coordinates.")
        exit(1)

    # Build Google Maps URL with the extracted latitude and longitude.
    # Note: Google Maps expects the format as latitude,longitude.
    maps_url = f"https://www.google.com/maps?q={lat_val},{lon_val}"
    print(f"Opening Google Maps at coordinates: {lat_val}, {lon_val}")
    open_url_in_browser(maps_url)

    # If option 4, also save the link to GoogleMapsLinks.txt
    if option == 4:
        try:
            with open("GoogleMapsLinks.txt", "a") as link_file:
                link_file.write(maps_url + "\n")
            print("Saved Google Maps link to GoogleMapsLinks.txt")
        except Exception as e:
            print(f"Could not write to GoogleMapsLinks.txt: {e}")
