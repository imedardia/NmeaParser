#ifndef __NMEA_PARSER_H__
#define __NMEA_PARSER_H__

#include <boost/tokenizer.hpp>
#include "nmeautils.h"

#define NMEAPARSER_MAX_SENTENCE_LENGTH 255 //MAX Sentence Length in bytes
#define NMEAPARSER_MAX_TOKENS          24  //Max number of data fields in one GPS Sentence
#define MAX_SATELLITES_IN_VIEW         64  //Max Number of Satellites In view
#define MAX_SATELLITES_USED_FIX        12  //Max Number of Satellites Used For Fix
#define MAX_TXT_MSG_NUMBER             16  //Max Number of GPTXT Messages

typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
typedef void (*nmea_callback)(std::string NmeaType, void * NmeaStruct, void * context);
typedef struct {
	std::string NmeaType;
	nmea_callback NmeapCb;
	void * context;
}NmeaParserCallback;

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

enum RmcDataValid {
	RMC_DATA_ACTIVE = 'A',
	RMC_DATA_VOID   = 'V'
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
		int 	prn;
		int     elevation;
		int     azimuth;
		int 	snr;
	}satellite[MAX_SATELLITES_IN_VIEW];
}nmeaparser_gsv_sentence;

/** TXT Sentence structure */
typedef struct {
	int 	number;
	struct {
		int		total;
		int		number;
		int		severity;
		char	message[NMEAPARSER_MAX_SENTENCE_LENGTH];
	}id[MAX_TXT_MSG_NUMBER];
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
	int 	sv[MAX_SATELLITES_USED_FIX];
	float	pdop;
	float   hdop;
	float   vdop;
}nmeaparser_gsa_sentence;


class NmeaParser: public NmeaUtils
{
	public:
		NmeaParser();
		~NmeaParser();
		bool addNmea0183Parser(nmea_callback ParserCb, std::string NmeaType, void * tmpcontext);
		void ParseNmea0183Sentence(std::string GpsFrameData);
		bool isValidSentenceChecksum(std::string s, std::string &GpsData);

	private:
		std::vector<NmeaParserCallback> NmeaParsers;
		void ParseGGASentence(std::vector<std::string> &GgaValidData, NmeaParserCallback * GgaCallback);
		void ParseRMCSentence(std::vector<std::string> &RmcValidData, NmeaParserCallback * RmcCallback);
		void ParseGSASentence(std::vector<std::string> &GsaValidData, NmeaParserCallback * GsaCallback);
		void ParseGSVSentence(std::vector<std::string> &GsvValidData, NmeaParserCallback * GsvCallback);
		bool isValidNmeapParser(std::string NmeaType);
	
};

#endif
