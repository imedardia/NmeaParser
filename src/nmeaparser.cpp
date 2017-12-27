#include <boost/tokenizer.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>

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

void ParseGGASentence(std::vector<std::string> &GgaValidData)
{
	//$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
	static int i = 1;
	std::string time = GgaValidData[i];
	std::cout << "Time : " << time << std::endl;
	i++;
	std::string latitude =  GgaValidData[i];
	std::cout << "Latitude : " << latitude << std::endl;
	i++;
	std::string latitudeDir =  GgaValidData[i];
	std::cout << "LatitudeDir : " << latitudeDir << std::endl;
	i++;
	std::string longitude = GgaValidData[i];
	std::cout << "Longitude : " << longitude << std::endl;
	i++;
	std::string longitudeDir = GgaValidData[i];
	std::cout << "LongitudeDir : " << longitudeDir << std::endl;
	i++;
	int GgaFix = ConvertStrToInt(GgaValidData[i]);
	std::cout << "Fix : " << GgaFix << std::endl;
	i++;
	int GgaSatNb = ConvertStrToInt(GgaValidData[i]);
	std::cout << "Nb of Satellites : " << GgaSatNb << std::endl;
	i++;
	double hdop = ConvertStrToDouble(GgaValidData[i]);
	std::cout << "hdop : " << hdop << std::endl;
	i++;
	double altitude = ConvertStrToDouble(GgaValidData[i]);
	std::cout << "Altitude : " << altitude << std::endl;
	i++;
	std::string AltUnit = GgaValidData[i];
	std::cout << "AltUnit : " << AltUnit << std::endl;
	i++;
	double Geoid = ConvertStrToDouble(GgaValidData[i]);
	std::cout << "Geoid : " << Geoid << std::endl;
	i++;
	std::string geoidUnit = GgaValidData[i];
	std::cout << "AltUnit : " << geoidUnit << std::endl;
}

void ParseRMCSentence(std::vector<std::string> &GgaValidData)
{
	//$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
			//123519       Fix taken at 12:35:19 UTC
			//A            Status A=active or V=Void.
			//4807.038,N   Latitude 48 deg 07.038' N
			//01131.000,E  Longitude 11 deg 31.000' E
			//022.4        Speed over the ground in knots
			//084.4        Track angle in degrees True
			//230394       Date - 23rd of March 1994
			//003.1,W      Magnetic Variation
}

void ParseNmea0183Sentence(std::string GpsFrameData)
{
	boost::char_separator<char> DecSep{","};
	std::vector<std::string> tokens;
	tokenizer GpsContent{GpsFrameData, DecSep};
	for (tokenizer::iterator tok_iter = GpsContent.begin();
		tok_iter != GpsContent.end(); ++tok_iter) {
		tokens.push_back(*tok_iter);
	}

	if(tokens[0] == Nmea0183FramesHdr[NMEAPARSER_GPGGA_ID]) {
		std::cout << "GGA Frame Found" << std::endl;
		ParseGGASentence(tokens);
	}
	if(tokens[0] == Nmea0183FramesHdr[NMEAPARSER_GPRMC_ID]) {
		std::cout << "RMC Frame Found" << std::endl;
		ParseRMCSentence(tokens);
	}
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
	std::string s("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
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
