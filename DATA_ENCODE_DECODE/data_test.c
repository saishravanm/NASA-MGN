#include "data.h"

int main(int argc, char** argv) {

	if(argc<2) {
		fprintf(stderr, "Specify file");
	}

	FILE* test;
	test=fopen(argv[1], "rb");

	char buffer[HEX_PACK_SIZE];
	DATA data;
	fread(buffer, sizeof(buffer), sizeof(buffer)/sizeof(buffer[0]), test);
	
	data_memcpy(&data, buffer);

	COORD coord=read_coordinates(&data);

	printf("Synchronization & Error Correction Data (Currently Unused:\n"
	       "Bit & Frame Synchronization: %d\n"
	       "BCH1: %d\n"
	       "BCH2: %d\n"
	       "Format Flag: %d\n"
	       "Protocol Flag: %d\n"
	       "Country Code: %d\n"
	       "Identification Type (PC): %d\n"
	       "Identification Info: %d\n"
	       "Location: %c-%f, %c-%f\n"
	       "Location Delta: %02d:%02d, %02d:%02d\n" 
	       "Supplementary Data %d\n",
	       data.frame, data.bch1, data.bch2, data.format_f, data.protocol_f, data.country_code, data.pc, data.id_data.init_buffer, coord.ns, coord.lat_deg, coord.ew, coord.long_deg, coord.lat_delta_min, coord.lat_delta_sec, coord.long_delta_min, coord.long_delta_sec, data.supp_data);

}
