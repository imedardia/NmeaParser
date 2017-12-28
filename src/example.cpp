#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include "nmeaparser.h"

#define SIMU_DIR "../"
#define SIMU_FILE "NmeaFrames.nmeap"

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
	if(!addNmea0183Parser(gga_callback, "$GPGGA")){
		std::cout << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}

	if(!addNmea0183Parser(rmc_callback, "$GPRMC")){
		std::cout << "Cannot add a new NMEAP parser" << std::endl;
		return 1;
	}

	std::ifstream infile(std::string(SIMU_DIR)+std::string(SIMU_FILE));
	std::string GpsFrame;
	std::string GpsData;
	while (std::getline(infile, GpsFrame))
	{
		if(isValidSentenceChecksum(GpsFrame, GpsData)) {
			std::cout << "Correct NMEA0183 Sentence" << std::endl;
			ParseNmea0183Sentence(GpsData);
		}else
			std::cout << "Bad NMEA0183 Sentence" << std::endl;
	}
	return 0;
}
