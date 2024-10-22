#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define HEX_PACK_SIZE 18

typedef unsigned int MMSI;
typedef unsigned int BNO;

#pragma pack(push, 1)

typedef struct COUNTRY_CODE {
	int digits;
	char code[4];
} COUNTRY_CODE;

COUNTRY_CODE country_codes[] = {
    {4, "AFG"}, {8, "ALB"}, {12, "DZA"}, {20, "AND"}, {24, "AGO"}, 
    {32, "ARG"}, {36, "AUS"}, {40, "AUT"}, {48, "BHR"}, {50, "BGD"}, 
    {56, "BEL"}, {64, "BTN"}, {68, "BOL"}, {72, "BWA"}, {76, "BRA"}, 
    {84, "BLZ"}, {100, "BGR"}, {108, "BDI"}, {116, "KHM"}, {120, "CMR"}, 
    {124, "CAN"}, {132, "CPV"}, {144, "LKA"}, {152, "CHL"}, {156, "CHN"}, 
    {170, "COL"}, {178, "COG"}, {180, "COD"}, {188, "CRI"}, {191, "HRV"}, 
    {196, "CYP"}, {203, "CZE"}, {208, "DNK"}, {214, "DOM"}, {218, "ECU"}, 
    {222, "SLV"}, {231, "ETH"}, {233, "EST"}, {242, "FJI"}, {246, "FIN"}, 
    {250, "FRA"}, {268, "GEO"}, {270, "GMB"}, {276, "DEU"}, {288, "GHA"}, 
    {300, "GRC"}, {320, "GTM"}, {324, "GIN"}, {328, "GUY"}, {332, "HTI"}, 
    {340, "HND"}, {348, "HUN"}, {356, "IND"}, {360, "IDN"}, {364, "IRN"}, 
    {368, "IRQ"}, {372, "IRL"}, {376, "ISR"}, {380, "ITA"}, {392, "JPN"}, 
    {400, "JOR"}, {404, "KEN"}, {408, "PRK"}, {410, "KOR"}, {414, "KWT"}, 
    {417, "KGZ"}, {418, "LAO"}, {422, "LBN"}, {426, "LSO"}, {428, "LVA"}, 
    {430, "LBR"}, {434, "LBY"}, {440, "LTU"}, {442, "LUX"}, {450, "MDG"}, 
    {454, "MWI"}, {458, "MYS"}, {462, "MDV"}, {466, "MLI"}, {470, "MLT"}, 
    {478, "MRT"}, {480, "MUS"}, {484, "MEX"}, {496, "MNG"}, {498, "MDA"}, 
    {504, "MAR"}, {508, "MOZ"}, {512, "OMN"}, {516, "NAM"}, {524, "NPL"}, 
    {528, "NLD"}, {533, "ABW"}, {540, "NCL"}, {554, "NZL"}, {558, "NIC"}, 
    {566, "NGA"}, {578, "NOR"}, {586, "PAK"}, {591, "PAN"}, {598, "PNG"}, 
    {600, "PRY"}, {604, "PER"}, {608, "PHL"}, {616, "POL"}, {620, "PRT"}, 
    {634, "QAT"}, {642, "ROU"}, {643, "RUS"}, {646, "RWA"}, {662, "LCA"}, 
    {670, "VCT"}, {682, "SAU"}, {688, "SRB"}, {690, "SYC"}, {694, "SLE"}, 
    {702, "SGP"}, {704, "VNM"}, {710, "ZAF"}, {724, "ESP"}, {752, "SWE"}, 
    {756, "CHE"}, {764, "THA"}, {780, "TTO"}, {788, "TUN"}, {800, "UGA"}, 
    {804, "UKR"}, {807, "MKD"}, {818, "EGY"}, {826, "GBR"}, {834, "TZA"}, 
    {840, "USA"}, {850, "VIR"}, {854, "BFA"}, {858, "URY"}, {860, "UZB"}, 
    {862, "VEN"}, {887, "YEM"}, {894, "ZMB"}
};

struct MMSI_BNO {
	
	MMSI  mmsi: 20;
	BNO bno: 4; 
};

struct AIR_ADDRESS {
	unsigned int air_addr: 24;
};

struct AIR_OP {

	unsigned int air_oper: 15;
	unsigned int serial_no: 9;

};

struct CSTA_NO {
	
	unsigned int csta_no: 10;
	unsigned int serial_no: 14;

};

union ID_DATA {

	unsigned int init_buffer: 24;
	struct MMSI_BNO mmsi_bno;
	struct AIR_ADDRESS air_addr;
	struct AIR_OP air_op;
	struct CSTA_NO csta;
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

COUNTRY_CODE read_country_code(DATA *data) {
	
	for(int i=0; i<(sizeof(country_codes)/sizeof(COUNTRY_CODE)); i++) {
		if(country_codes[i].digits==data->country_code)
			return country_codes[i];
	}
	
	COUNTRY_CODE cc={ -1, "UNK" };

	return cc;	

}

COORD read_coordinates(DATA *data) {

	COORD coord;

	coord.ns=data->ns?'N':'S';
	coord.ew=data->ew?'E':'W';

	coord.lat_deg=0.25*data->lat_deg;
	coord.long_deg=0.25*data->long_deg;

	coord.lat_delta_min=(data->lat_sign?-1:1)*(data->lat_minutes);
	coord.lat_delta_sec=data->lat_seconds;	

	coord.long_delta_min=(data->long_sign?-1:1)*(data->long_minutes);
	coord.long_delta_sec=data->long_seconds;

	return coord;
}

enum IDENTIFICATION_T { MMSI_BNO=2, AIRCRAFT_ADDR=3, AIRCRAFT_OP=5, ELT_SERIAL=4, EPIRB_SERIAL=6, PLB=7, MMSI_FIXED=12, TEST=14, UNKNOWN=0 };

typedef struct IDENTIFICATION {

	enum IDENTIFICATION_T type;
	union ID_DATA data;

} IDENTIFICATION;

IDENTIFICATION read_identification(DATA *data) {

	IDENTIFICATION id;

	switch(data->pc) {

		case 0b0010:
			id.type=MMSI_BNO;
			break;
		case 0b0011:
			id.type=AIRCRAFT_ADDR;
			break;
		case 0b0101:
			id.type=AIRCRAFT_OP;
			break;
		case 0b0100:
			id.type=ELT_SERIAL;
			break;
		case 0b0110:
			id.type=EPIRB_SERIAL;
			break;
		case 0b0111:
			id.type=PLB;
			break;
		case 0b1100:
			id.type=MMSI_FIXED;
			break;
		case 0b1110:
			id.type=TEST;
			break;
		default:
			id.type=UNKNOWN;
	}

	id.data=data->id_data;
	return id;

}

/*

void print_id(IDENTIFICATION *id) {

	printf("Identification Type: ");

	switch(id->type) {

		case MMSI_BNO:
			printf("MMSI, BNO\n");
			printf("Maritime Mobile Service Identity (Last 6 Digits): %d\nB. Number: %d\n", id->data.mmsi_bno.mmsi, id->data.mmsi_bno.bno);

			break;
		case AIRCRAFT_ADDR:
			printf("AIRCRAFT ADDRESS\n");
			printf("Aircraft Address: %d\n", id->data.air_addr.air_addr);
			break;
		case AIRCRAFT_OP:
			printf("AIRCRAFT OPER DESIGNATOR, SERIAL #\n");
			printf("")
			break;
		case ELT_SERIAL:
			break;
		case EPIRB_SERIAL:
			break;
		case PLB:
			break;
		case MMSI_FIXED:
			break;
		case TEST:
			break;
		default:
	}

}

*/

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
