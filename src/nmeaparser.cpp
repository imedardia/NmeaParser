#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <cassert>
#include "nmeaparser.h"
#include "nmeautils.h"


void ParseGGASentence(std::vector<std::string> &GgaValidData, nmeaparser_gga_sentence &gga_data)
{
	//$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
	static int i = 1;
	gga_data.time = ConvertStrToInt(GgaValidData[i]);

	i++;
	gga_data.latitude =  ConvertStrToLat(GgaValidData[i],GgaValidData[i+1]);
	
	i+=2;
	gga_data.longitude = ConvertStrToLon(GgaValidData[i],GgaValidData[i+1]);

	i+=2;
	gga_data.quality = ConvertStrToInt(GgaValidData[i]);

	i++;
	gga_data.satellites = ConvertStrToInt(GgaValidData[i]);

	i++;
	gga_data.hdop = ConvertStrToDouble(GgaValidData[i]);

	i++;
	gga_data.altitude = ConvertDistUnits(GgaValidData[i], GgaValidData[i+1]);

	i+=2;
	gga_data.geoid = ConvertDistUnits(GgaValidData[i], GgaValidData[i+1]);
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
		ParseGGASentence(tokens, gga_data);
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
