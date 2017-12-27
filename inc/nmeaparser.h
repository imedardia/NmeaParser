#ifndef __NMEA_PARSER_H__
#define __NMEA_PARSER_H__

#include <boost/tokenizer.hpp>

#define NMEAPARSER_MAX_SENTENCE_LENGTH 255 //MAX Sentence Length in bytes
#define NMEAPARSER_MAX_TOKENS          24  //Max number of data fields in one GPS Sentence

typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

enum Nmea0183Frames {
	NMEAPARSER_GPGGA_ID = 0,
	NMEAPARSER_GPRMC_ID,
	NMEAPARSER_GPGSV_ID,
	NMEAPARSER_GPTXT_ID,
	NMEAPARSER_GPVTG_ID,
	NMEAPARSER_GPGSA_ID,
	NMEAPARSER_GPGLL_ID,
	NMEAPARSER_GPZDA_ID,
	NMEAPARSER_MAX_SENTENCES
};

enum GgaFixQuality {
	GGA_FIX_INV = 0,
	GGA_FIX_SPS,
	GGA_FIX_DGPS,
	GGA_FIX_PPS,
	GGA_FIX_RTK,
	GGA_FIX_FRTK,
	GGA_FIX_EST,
	GGA_FIX_MAN,
	Gga_Fix_SIM,
	Gga_FIX_TOT
};

const char * Nmea0183FramesHdr[NMEAPARSER_MAX_SENTENCES]={
	"$GPGGA",
	"$GPRMC",
	"$GPGSV",
	"$GPTXT",
	"$GPVTG",
	"$GPGSA",
	"$GPGLL",
	"$GPZDA",
};

/** GGA Sentence structure */
typedef struct {
	double        latitude;
	double        longitude;
	double        altitude;
	unsigned long time;
	int           satellites;
	int           quality;
	double        hdop;
	double        geoid;
}nmeaparser_gga_sentence;

nmeaparser_gga_sentence gga_data;

/** RMC Sentence structure */
typedef struct {
	unsigned long time;
	char          warn;
	double        latitude;
	double        longitude;
	double        speed;
	double        course;
	unsigned long date;
	double        magvar;
}nmeap_rmc_sentence;

/** GSV Sentence structure */
typedef struct {
	unsigned int nomsg;
	unsigned int msgno;
	unsigned int  nosv;
	struct {
		int 	sv;
		int     elevation;
		int     azimuth;
		int 	cno;
	}satellite[64];
}nmeaparser_gsv_sentence;

/** TXT Sentence structure */
typedef struct {
	int 	number;
	struct {
		int		total;
		int		number;
		int		severity;
		char	message[255];
	}id[16];
}nmeaparser_txt_sentence;

/** VTG Sentence structure */
typedef struct {
	double        course;
	double        speedkn;
	double        speedkm;
}nmeaparser_vtg_sentence;

/** GSA Sentence structure */
typedef struct {
	char 	smode;
	int 	fs;
	int 	sv[12];
	float	pdop;
	float hdop;
	float vdop;
}nmeaparser_gsa_sentence;

#endif
