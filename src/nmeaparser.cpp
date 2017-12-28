#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <cassert>
#include "nmeaparser.h"
#include "nmeautils.h"


void ParseGGASentence(std::vector<std::string> &GgaValidData, NmeaParserCallback * ggaCallback)
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

void ParseRMCSentence(std::vector<std::string> &RmcValidData, NmeaParserCallback * RmcCallback)
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

bool isValidNmeapParser(std::string NmeaType)
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


void gga_callback(std::string NmeaType, void * ggaStruct)
{
	nmeaparser_gga_sentence *gga = (nmeaparser_gga_sentence *)ggaStruct;
	std::cout << "Time        = "  << gga->time << std::endl;
	std::cout << "Latitude    = "  << gga->latitude << std::endl;
	std::cout << "Longitude   = "  << gga->longitude << std::endl;
	std::cout << "Quality     = "  << gga->quality << std::endl;
	std::cout << "Satellites  = "  << gga->satellites << std::endl;
	std::cout << "Altitude    = "  << gga->altitude << std::endl;
	std::cout << "HDOP        = "  << gga->hdop << std::endl;
	std::cout << "GEOID       = "  << gga->geoid << std::endl;
}

void rmc_callback(std::string NmeaType, void * rmcStruct)
{
	nmeap_rmc_sentence *rmc = (nmeap_rmc_sentence *)rmcStruct;
	std::cout << "Time        = "  << rmc->time << std::endl;
	std::cout << "Latitude    = "  << rmc->latitude << std::endl;
	std::cout << "Longitude   = "  << rmc->longitude << std::endl;
	std::cout << "Date        = "  << rmc->date << std::endl;
	std::cout << "Warn        = "  << rmc->warn << std::endl;
	std::cout << "Speed       = "  << rmc->speed << std::endl;
	std::cout << "Course      = "  << rmc->course << std::endl;
	std::cout << "MDEV        = "  << rmc->magvar << std::endl;
}

int main(int argc, char *argv[])
{
	std::string s("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A");
	if(!addNmea0183Parser(gga_callback, "$GPGGA")){
		std::cout << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}

	if(!addNmea0183Parser(rmc_callback, "$GPRMC")){
		std::cout << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}
	
	std::string GpsFrame;
	if(isValidSentenceChecksum(s, GpsFrame)) {
		std::cout << "Correct NMEA0183 Sentence" << std::endl;
		ParseNmea0183Sentence(GpsFrame);
	}else
		std::cout << "Bad NMEA0183 Sentence" << std::endl;

	return 0;
}
