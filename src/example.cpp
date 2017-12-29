#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include "nmeaparser.h"

#define SIMU_DIR "../"
#define SIMU_FILE "NmeaFrames.nmeap"

#define LOG_NDEBUG 1

void gga_callback(std::string NmeaType, void * ggaStruct)
{
	nmeaparser_gga_sentence *gga = (nmeaparser_gga_sentence *)ggaStruct;
	if(gga != 0) {
		std::cout << "Time           = "  << gga->time << std::endl;
		std::cout << "Latitude       = "  << gga->latitude << std::endl;
		std::cout << "Longitude      = "  << gga->longitude << std::endl;
		std::cout << "Quality        = "  << gga->quality << std::endl;
		std::cout << "Satellites     = "  << gga->satellites << std::endl;
		std::cout << "Altitude       = "  << gga->altitude << std::endl;
		std::cout << "HDOP           = "  << gga->hdop << std::endl;
		std::cout << "GEOID          = "  << gga->geoid << std::endl;
	}
}

void rmc_callback(std::string NmeaType, void * rmcStruct)
{
	nmeap_rmc_sentence *rmc   = (nmeap_rmc_sentence *)rmcStruct;
	if(rmc != 0) {
		std::cout << "Time        = "  << rmc->time << std::endl;
		std::cout << "Latitude    = "  << rmc->latitude << std::endl;
		std::cout << "Longitude   = "  << rmc->longitude << std::endl;
		std::cout << "Date        = "  << rmc->date << std::endl;
		std::cout << "Warn        = "  << rmc->warn << std::endl;
		std::cout << "Speed       = "  << rmc->speed << std::endl;
		std::cout << "Course      = "  << rmc->course << std::endl;
		std::cout << "MDEV        = "  << rmc->magvar << std::endl;
	}
}

void gsa_callback(std::string NmeaType, void * gsaStruct)
{
	nmeaparser_gsa_sentence *gsa   = (nmeaparser_gsa_sentence *)gsaStruct;
	if(gsa != 0) {
		std::cout << "New GSA Struct" << std::endl;
	}
}

void gsv_callback(std::string NmeaType, void * gsvStruct)
{
	static nmeaparser_gsv_sentence *gsv   = (nmeaparser_gsv_sentence *)gsvStruct;
	if(gsv != 0) {
		std::cout << "New GSV Struct"        << std::endl;
		std::cout << "Nbre of Msg: "         << gsv->nomsg << std::endl;
		std::cout << "Msg Instance: "        << gsv->msgno << std::endl;
		std::cout << "Satellites In View : " << gsv->nosv << std::endl;
		for(int i=0; i < MAX_SATELLITES_IN_VIEW ; i++) {
			std::cout << "Sat PRN : " << gsv->satellite[i].prn << std::endl;
			std::cout << "Sat Elev: " << gsv->satellite[i].elevation << std::endl;
			std::cout << "Sat Azim: " << gsv->satellite[i].azimuth << std::endl;
			std::cout << "Sat SNR : " << gsv->satellite[i].snr << std::endl;
		}
	}
}


int main(int argc, char *argv[])
{
	
	if(!addNmea0183Parser(rmc_callback, "$GPRMC")){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}
	
	if(!addNmea0183Parser(gga_callback, "$GPGGA")){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}
	
	if(!addNmea0183Parser(gsa_callback, "$GPGSA")){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}
	
	if(!addNmea0183Parser(gsv_callback, "$GPGSV")){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}

	std::string GpsFrame;
	std::ifstream infile(std::string(SIMU_DIR)+std::string(SIMU_FILE));
	if(!infile.is_open()) {
		std::cerr << "couldn't find the Nmeap simulation file" << std::endl;
		return 1;
	}

	while (std::getline(infile, GpsFrame))
	{
		std::string GpsData;
		if(isValidSentenceChecksum(GpsFrame, GpsData)) {
#if(LOG_NDEBUG == 1)
			std::cout << "Correct NMEA0183 Sentence " << std::endl;
#endif
			ParseNmea0183Sentence(GpsData);
		}else
			std::cerr << "Bad NMEA0183 Sentence" << std::endl;
	}
	infile.close();
	return 0;
}
