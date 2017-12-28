#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <cassert>
#include "nmeaparser.h"
#include "nmeautils.h"

std::vector<NmeaParserCallback> NmeaParsers;

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

static void ParseGGASentence(std::vector<std::string> &GgaValidData, NmeaParserCallback * ggaCallback)
{
	//$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
	std::cout << "New GGA Frame Found" << std::endl;
	nmeaparser_gga_sentence gga_data;
	int i = 1;
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
	//memcpy(ggaCallback->NmeaDataStruct,(void*)&gga_data, sizeof(gga_data));
	ggaCallback->NmeapCb(ggaCallback->NmeaType, &gga_data);
}

static void ParseRMCSentence(std::vector<std::string> &RmcValidData, NmeaParserCallback * RmcCallback)
{
	std::cout << "New RMC Frame Found" << std::endl;
	//$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
			//123519       Fix taken at 12:35:19 UTC
			//A            Status A=active or V=Void.
			//4807.038,N   Latitude 48 deg 07.038' N
			//01131.000,E  Longitude 11 deg 31.000' E
			//022.4        Speed over the ground in knots
			//084.4        Track angle in degrees True
			//230394       Date - 23rd of March 1994
			//003.1,W      Magnetic Variation
	nmeap_rmc_sentence rmc_data;
	int i = 1;
	rmc_data.time = ConvertStrToInt(RmcValidData[i]);

	i++;
	rmc_data.warn =  (RmcValidData[i].c_str())[0];
	
	i++;
	rmc_data.latitude = ConvertStrToLat(RmcValidData[i],RmcValidData[i+1]);

	i+=2;
	rmc_data.longitude = ConvertStrToLon(RmcValidData[i], RmcValidData[i+1]);

	i+=2;;
	rmc_data.speed = ConvertStrToDouble(RmcValidData[i]);

	i++;
	rmc_data.course = ConvertStrToDouble(RmcValidData[i]);

	i++;
	rmc_data.date = ConvertStrToInt(RmcValidData[i]);

	i++;
	rmc_data.magvar = ConvertStrToLon(RmcValidData[i], RmcValidData[i+1]);

	//memcpy(RmcCallback->NmeaDataStruct,(void*)&rmc_data, sizeof(rmc_data));
	RmcCallback->NmeapCb(RmcCallback->NmeaType, &rmc_data);


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
	
	for(std::vector<NmeaParserCallback>::iterator it = NmeaParsers.begin(); it != NmeaParsers.end(); ++it) {
		if(tokens[0] == it->NmeaType) {
			if(it->NmeaType == Nmea0183FramesHdr[NMEAPARSER_GPGGA_ID]) {
				ParseGGASentence(tokens, &NmeaParsers[it - NmeaParsers.begin()]);
			}
			if(tokens[0] == Nmea0183FramesHdr[NMEAPARSER_GPRMC_ID]) {
				ParseRMCSentence(tokens, &NmeaParsers[it - NmeaParsers.begin()]);
			}
			break;
		}
	}
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

static bool isValidNmeapParser(std::string NmeaType)
{
	for(int i = 0; i < NMEAPARSER_MAX_SENTENCES; i++) {
		if(NmeaType == Nmea0183FramesHdr[i]) {
			std::cout << "valid nmeaparser found :" << NmeaType << std::endl;
			return true;
		}
	}
	return false;
}

bool addNmea0183Parser(nmea_callback ParserCb, std::string NmeaType)
{
	NmeaParserCallback NmeaTmpParser;
	if((NmeaType.size() == 0) || !isValidNmeapParser(NmeaType) || ParserCb == nullptr)
		return false;
		
	NmeaTmpParser.NmeaType = NmeaType;
	NmeaTmpParser.NmeapCb = ParserCb;
	NmeaParsers.push_back(NmeaTmpParser);
	return true;
}
