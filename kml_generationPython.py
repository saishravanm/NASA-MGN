from datetime import datetime, timezone

def generate_kml(latitude, longitude, country_code, beacon_id, timestamp):
    """
    Generate a KML file with the given latitude, longitude, country code, beacon ID,
    and timestamp. The timestamp should be a datetime object, which will be formatted
    to ISO 8601 string (UTC) inside this function.
    """
    # Format the timestamp as an ISO 8601 string (e.g. "2025-04-09T23:18:00Z")
    timestamp_str = timestamp.strftime("%Y-%m-%dT%H:%M:%SZ")
    
    # Open (or create) the output KML file and write the XML structure
    with open("beacon_locations.kml", "w") as file:
        # XML declaration and opening <kml> tag with namespace
        file.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        file.write('<kml xmlns="http://www.opengis.net/kml/2.2">\n')
        # Placemark entry
        file.write('  <Placemark>\n')
        # Name of the placemark: using the beacon ID
        file.write(f'    <name>{beacon_id}</name>\n')
        # Description section with country code, beacon ID, and timestamp (on separate lines)
        file.write(f"    <description>Country code: {country_code}\n"
                   f"    Beacon ID: {beacon_id}\n"
                   f"    Timestamp: {timestamp_str}</description>\n")
        # Point coordinates (note: KML expects "longitude,latitude")
        file.write('    <Point>\n')
        file.write(f'      <coordinates>{longitude},{latitude}</coordinates>\n')
        file.write('    </Point>\n')
        # Close Placemark and kml tags
        file.write('  </Placemark>\n')
        file.write('</kml>\n')

# Example usage of the generate_kml function
if __name__ == "__main__":
    # Define sample input values
    sample_latitude = 37.422    # e.g., latitude of a location
    sample_longitude = -122.084 # e.g., longitude of a location
    sample_country_code = "US"  # example country code
    sample_beacon_id = "BEACON-12345"  # example beacon identifier
    
    # Get the current time (UTC) as a datetime object
    current_time = datetime.now(timezone.utc)

    # Call the generate_kml function with the sample data
    generate_kml(sample_latitude, sample_longitude, sample_country_code, sample_beacon_id, current_time)
    
    print("KML file 'beacon_locations.kml' has been created successfully.")
