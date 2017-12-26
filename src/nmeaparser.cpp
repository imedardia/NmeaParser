#include <boost/tokenizer.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>

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


int ConvertHexStrToInt(std::string csval)
{
	int iRes = 0;
	std::stringstream cs;
	cs << std::hex << csval;
	cs >> iRes;
	return iRes;
}

int ConvertStrToInt(std::string csval)
{
	int iRes = 0;
	std::stringstream cs(csval);
	cs >> iRes;
	return iRes;
}

double ConvertStrToDouble(std::string csval)
{
	double iRes = 0;
	std::stringstream cs(csval);
	cs >> iRes;
	return iRes;
}

int CalculateXoRCheckSum(const char * Sentence, int size)
{
	int iXorCS = 0;
	for(int i = 0; i < size; i++)
		iXorCS ^= Sentence[i];
	return iXorCS;
}

void ParseNmea0183Sentence(std::string GpsFrameData)
{
	boost::char_separator<char> DecSep{","};
	tokenizer GpsContent{GpsFrameData, DecSep};
	auto parser = GpsContent.begin();
	std::string NmeaType = *parser;
	if(NmeaType == Nmea0183FramesHdr[NMEAPARSER_GPGGA_ID])
		std::cout << "GGA Frame Found" << std::endl;

	/*for (tokenizer::iterator tok_iter = GpsContent.begin();
		tok_iter != GpsContent.end(); ++tok_iter) {
		std::cout << *tok_iter << std::endl;
	}*/
}

bool isValidHeaderWeCare(std::string s)
{
	boost::char_separator<char> sep{","};
	tokenizer tokens{s, sep};
	auto parser = tokens.begin();
	std::string SentType = *parser;
	for(int i = 0; i < NMEAPARSER_MAX_SENTENCES; i++) {
		if(SentType == Nmea0183FramesHdr[i])
			return true;
	}
	return false;
}

bool isValidSentenceChecksum(std::string s, std::string &GpsData)
{
	boost::char_separator<char> sep{"*"};
	tokenizer tokens{s, sep};
	auto parser = tokens.begin();
	GpsData = *parser;
	std::advance(parser, 1);
	unsigned int ulCS = static_cast<unsigned int>(ConvertHexStrToInt(*parser));
	unsigned int iCalcCS = static_cast<unsigned int>(CalculateXoRCheckSum(&(GpsData.c_str())[1], GpsData.size()));

	if(ulCS == iCalcCS)
		return true;
	else
		return false;
}

int main(int argc, char *argv[])
{
	std::string s("$GPGGA,064036.289,4836.5375,N,00740.9373,E,1,04,3.2,200.2,M,,,,0000*0E");
	if(!isValidHeaderWeCare(s))
	return 1;
	std::string GpsFrame;
	if(isValidSentenceChecksum(s, GpsFrame)) {
	std::cout << "Correct NMEA0183 Sentence" << std::endl;
	ParseNmea0183Sentence(GpsFrame);
	}else
	std::cout << "Bad NMEA0183 Sentence" << std::endl;

	return 0;
}
