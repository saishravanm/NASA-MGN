#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define HEX_PACK_SIZE 18

typedef unsigned int MMSI;
typedef unsigned int BNO;

#pragma pack(push, 1)

struct MMSI_BNO {
	
	MMSI  mmsi: 20;
	BNO bno: 4; 
};

struct AIR_ADDRESS {

	unsigned int air_oper: 15;
	unsigned int serial_no: 9;

};

struct AIR_ADDRESS2 {
	
	unsigned int csta_no: 10;
	unsigned int serial_no: 14;

};

union ID_DATA {

	unsigned int init_buffer: 24;
	struct MMSI_BNO mmsi_bno;
	struct AIR_ADDRESS air_addr;
	struct AIR_ADDRESS2 air_addr2;
};

typedef struct DATA {

	unsigned int frame: 24;
	
	unsigned int format_f: 1;
	unsigned int protocol_f: 1;
	unsigned int country_code: 10;
	unsigned int pc: 4;
	



	union ID_DATA id_data;
			
	
	unsigned int ns: 1;
	unsigned int lat_deg: 9;
	unsigned int ew: 1;
	unsigned int long_deg: 10;


	unsigned int bch1: 21;

	unsigned int supp_data: 6;
		
	unsigned int lat_sign: 1;
	unsigned int lat_minutes: 5;
	unsigned int lat_seconds: 4;
	
	unsigned int long_sign: 1;
	unsigned int long_minutes: 5;
	unsigned int long_seconds: 4;

	unsigned int bch2: 12;
	
} DATA;

#pragma pack(pop)


typedef struct COORD {
	
	char ns;
	char ew;
	float lat_deg;
	float long_deg;
	int lat_delta_min;
	int lat_delta_sec;
	int long_delta_min;
	int long_delta_sec;

} COORD;

int read_country_code(DATA *data) {
	
	return (int)data->country_code;	

}

COORD read_coordinates(DATA *data) {

	COORD coord;

	coord.ns=data->ns?'N':'S';
	coord.ew=data->ew?'E':'W';

	coord.lat_deg=(90.0/512.0)*data->lat_deg;
	coord.long_deg=(180.0/1024.0)*data->long_deg;

	coord.lat_delta_min=(data->lat_sign?-1:1)*(data->lat_minutes);
	coord.lat_delta_sec=data->lat_seconds;	

	coord.long_delta_min=(data->long_sign?-1:1)*(data->long_minutes);
	coord.long_delta_sec=data->long_seconds;

	return coord;
}

enum PC { ELT_SERIAL, EPIRB_SERIAL, PLB, TEST };


void data_memcpy(DATA* data, char* buffer) {

	//Check if buffer is HEX_PACK_SIZE
	//Layer add error codes to return
	data->frame=(buffer[0]<<16 | buffer[1]<<8 | buffer[2]);
	data->format_f=(buffer[3]&0x80)>>7;
	data->protocol_f=(buffer[3]&0x40)>>6;
	data->country_code=((buffer[3]&0x3f)<<4 | (buffer[4]&0xf0)>>4);
	data->pc=buffer[4]&0x0f;
	data->id_data.init_buffer=(buffer[5]<<16 | buffer[6]<<8 | buffer[7]);
	data->ns=(buffer[8]&0x80)>>7;
	data->lat_deg=((buffer[8]&0x7f)<<2 | (buffer[9]&0xc0)>>6);
	data->ew=(buffer[9]&0x20)>>5;
	data->long_deg=((buffer[9]&0x1f)<<5 | (buffer[10]&0xf8)>>3);
	data->bch1=0;
	data->supp_data=buffer[13]&0x3f;
	data->lat_sign=(buffer[14]&0x80)>>7;
	data->lat_minutes=(buffer[14]&0x7c)>>2;
	data->lat_seconds=(buffer[14]&0x02)<<2 | (buffer[15]&0xc0)>>6;
	data->long_sign=(buffer[15]&0x20)>>5;
	data->long_minutes=buffer[15]&0x1f;
	data->long_seconds=(buffer[16]&0xf0)>>4;
	data->bch2=0;

}	

/*
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

*/
