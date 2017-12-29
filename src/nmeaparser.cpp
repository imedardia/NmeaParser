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

static void ParseGGASentence(std::vector<std::string> &GgaValidData, NmeaParserCallback * GgaCallback)
{
	//$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
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
	gga_data.altitude = ConvertDoubleDistUnits(GgaValidData[i], GgaValidData[i+1]);

	i+=2;
	gga_data.geoid = ConvertDoubleDistUnits(GgaValidData[i], GgaValidData[i+1]);
	if(GgaCallback->NmeapCb != nullptr)
		GgaCallback->NmeapCb(GgaCallback->NmeaType, &gga_data);
}

static void ParseRMCSentence(std::vector<std::string> &RmcValidData, NmeaParserCallback * RmcCallback)
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
	rmc_data.magvar = ConvertStrToMagnDev(RmcValidData[i], RmcValidData[i+1]);
	
	if(RmcCallback->NmeapCb != nullptr)
		RmcCallback->NmeapCb(RmcCallback->NmeaType, &rmc_data);
}

static void ParseGSASentence(std::vector<std::string> &GsaValidData, NmeaParserCallback * GsaCallback)
{
	nmeaparser_gsa_sentence gsa_data;
	
	int i = 1;
	i++;
	if(GsaCallback->NmeapCb != nullptr)
		GsaCallback->NmeapCb(GsaCallback->NmeaType, &gsa_data);
}

static void ParseGSVSentence(std::vector<std::string> &GsvValidData, NmeaParserCallback * GsvCallback)
{
	//$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
	/*Where:
      GSV          Satellites in view
      2            Number of sentences for full data
      1            sentence 1 of 2
      08           Number of satellites in view

      01           Satellite PRN number
      40           Elevation, degrees
      083          Azimuth, degrees
      46           SNR - higher is better
                   for up to 4 satellites per sentence */
	nmeaparser_gsv_sentence gsv_data;
	int i = 1, k = 0;
	gsv_data.nomsg = ConvertStrToInt(GsvValidData[i]);
	i++;
	gsv_data.msgno = ConvertStrToInt(GsvValidData[i]);
	i++;
	gsv_data.nosv  = ConvertStrToInt(GsvValidData[i]);
	i++;
	//As we have up to 4 satellites per GSV Sentence
	//We should calculate the satellites instances in current sentence
	//Index starts always from 1 (not 0)
	if (gsv_data.nomsg == gsv_data.msgno) // Last index
		k = (gsv_data.nosv - (( gsv_data.nomsg - 1) * 4))*4;
	else
		k = 4*4;
	if (gsv_data.nomsg == 1) {
		for (int j = 0;j< MAX_SATELLITES_IN_VIEW;j++) {
			gsv_data.satellite[j].prn        = 0;
			gsv_data.satellite[j].elevation = 0;
			gsv_data.satellite[j].azimuth   = 0;
			gsv_data.satellite[j].snr       = 0;
		}
	}
	for (int j = 0;j < k;j+=4) {
		int prn        	                    = ConvertStrToInt(GsvValidData[i+j]);
		gsv_data.satellite[prn].prn        	= ConvertStrToInt(GsvValidData[i+1+j]);
		gsv_data.satellite[prn].elevation  	= ConvertStrToInt(GsvValidData[i+2+j]);
		gsv_data.satellite[prn].azimuth 	= ConvertStrToInt(GsvValidData[i+3+j]);
		gsv_data.satellite[prn].snr      	= ConvertStrToInt(GsvValidData[i+4+j]);
	}
	if(GsvCallback->NmeapCb != nullptr)
		GsvCallback->NmeapCb(GsvCallback->NmeaType, &gsv_data);
}

void ParseNmea0183Sentence(std::string GpsFrameData)
{
	std::cout << GpsFrameData << std::endl;
	boost::char_separator<char> DecSep{","};
	tokenizer GpsContent{GpsFrameData, DecSep};
	std::vector<std::string> tokens;
	for (tokenizer::iterator tok_iter = GpsContent.begin();
		tok_iter != GpsContent.end(); ++tok_iter) {
		tokens.push_back(*tok_iter);
	}
	
	for(std::vector<NmeaParserCallback>::iterator it = NmeaParsers.begin(); it != NmeaParsers.end(); ++it) {
		if(tokens[0] == it->NmeaType) {
			if(it->NmeaType == Nmea0183FramesHdr[NMEAPARSER_GPGGA_ID]) {
				std::cout << "New GGA Sentence " << std::endl;
				ParseGGASentence(tokens, &NmeaParsers[it - NmeaParsers.begin()]);
			}
			if(tokens[0] == Nmea0183FramesHdr[NMEAPARSER_GPRMC_ID]) {
				std::cout << "New RMC Sentence " << std::endl;
				ParseRMCSentence(tokens, &NmeaParsers[it - NmeaParsers.begin()]);
			}
			if(tokens[0] == Nmea0183FramesHdr[NMEAPARSER_GPGSV_ID]) {
				std::cout << "New GSV Sentence " << std::endl;
				ParseGSVSentence(tokens, &NmeaParsers[it - NmeaParsers.begin()]);
			}
			if(tokens[0] == Nmea0183FramesHdr[NMEAPARSER_GPGSA_ID]) {
				ParseGSASentence(tokens, &NmeaParsers[it - NmeaParsers.begin()]);
				std::cout << "New GSA Sentence " << std::endl;
			}
			break;
		}
	}
}

bool isValidSentenceChecksum(std::string s, std::string &GpsData)
{
	boost::char_separator<char> sep{"*"};
	tokenizer tokenTmp{s, sep};
	auto parser = tokenTmp.begin();
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
