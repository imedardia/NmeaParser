#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cstring>
#include "nmeaPres.h"

NmeaPres::NmeaPres(std::string NmeaPort, const unsigned long culBaudrate):
ulBaudrate(culBaudrate),
NmeaSerialPort(NmeaPort)
{
	std::cout << "NmeaPres CTOR" << std::endl;
	std::cout << "NmeaPort : " << NmeaSerialPort
	          << " "<< "Baudrate : " << ulBaudrate << std::endl;
	memset(&mGlobalGps, 0, sizeof(mGlobalGps));
}
NmeaPres::~NmeaPres()
{
	std::cout << "NmeaPres DTOR" << std::endl;
}

bool NmeaPres::NmeaInitParsers()
{
	if(!addNmea0183Parser(rmc_callback, "$GPRMC", &mGlobalGps)){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return false;
	}
	
	if(!addNmea0183Parser(gga_callback, "$GPGGA", &mGlobalGps)){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return false;
	}
	
	if(!addNmea0183Parser(gsa_callback, "$GPGSA", &mGlobalGps)){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return false;
	}
	
	if(!addNmea0183Parser(gsv_callback, "$GPGSV", &mGlobalGps)){
		std::cerr << "Cannot add a new NMEAP parser" << std::endl;
		return false;
	}
	return true;
}

void NmeaPres::gga_callback(std::string NmeaType, void * ggaStruct, void * context)
{
	static nmeaparser_gga_sentence *gga = (nmeaparser_gga_sentence *)ggaStruct;
	if(gga != 0) {
		((GlobalGpsStruct *)(context))->fAltitude = gga->altitude;
	}
}

void NmeaPres::rmc_callback(std::string NmeaType, void * rmcStruct, void * context)
{
	static nmeap_rmc_sentence *rmc   = (nmeap_rmc_sentence *)rmcStruct;
	if(rmc != 0) {
		((GlobalGpsStruct *)(context))->fLatitude = rmc->latitude;
		((GlobalGpsStruct *)(context))->fLongitude = rmc->longitude;
		((GlobalGpsStruct *)(context))->ulTime    = rmc->time;
		((GlobalGpsStruct *)(context))->ulDate    = rmc->date;
		((GlobalGpsStruct *)(context))->fSpeed    = rmc->speed;
		((GlobalGpsStruct *)(context))->fCourse   = rmc->course;
	}
}

void NmeaPres::gsa_callback(std::string NmeaType, void * gsaStruct, void * context)
{
	static nmeaparser_gsa_sentence *gsa   = (nmeaparser_gsa_sentence *)gsaStruct;
	if(gsa != 0) {
		((GlobalGpsStruct *)(context))->CeGpsFixMode  = gsa->fs;
		((GlobalGpsStruct *)(context))->dilution.hdop = gsa->hdop;
		((GlobalGpsStruct *)(context))->dilution.pdop = gsa->pdop;
		((GlobalGpsStruct *)(context))->dilution.vdop = gsa->vdop;
	}
}

void NmeaPres::gsv_callback(std::string NmeaType, void * gsvStruct, void * context)
{
	static nmeaparser_gsv_sentence *gsv   = (nmeaparser_gsv_sentence *)gsvStruct;
	if(gsv != 0) {
		if(gsv->msgno == gsv->nomsg) {
			int snr = 0;
			for(int i=0; i < MAX_SATELLITES_IN_VIEW ; i++) {
				if(gsv->satellite[i].prn != 0)
				   snr += gsv->satellite[i].snr;
			}
			((GlobalGpsStruct *)(context))->fGpsSNR = (double)((double)(snr * 1.0)/gsv->nosv);
		}
	}
}

void NmeaPres::getGpsGlobalStruct(GlobalGpsStruct &GlobalGps)
{
	GlobalGps = mGlobalGps;
}


int main(int argc, char *argv[])
{
	NmeaPres * NewParser = new NmeaPres("/dev/ttyO0", 115200);
    if(NewParser->NmeaInitParsers() == false)
		delete NewParser;

	std::string GpsFrame;
	std::ifstream infile(std::string(SIMU_DIR)+std::string(SIMU_FILE));
	if(!infile.is_open()) {
		std::cerr << "couldn't find the Nmeap simulation file" << std::endl;
		return 1;
	}

	while (std::getline(infile, GpsFrame))
	{
		std::string GpsData;
		if(NewParser->isValidSentenceChecksum(GpsFrame, GpsData)) {
#if(LOG_NDEBUG == 1)
			std::cout << "Correct NMEA0183 Sentence " << std::endl;
#endif
			NewParser->ParseNmea0183Sentence(GpsData);
		}else
			std::cerr << "Bad NMEA0183 Sentence" << std::endl;
	}
	infile.close();
	GlobalGpsStruct GlobalGps;
	NewParser->getGpsGlobalStruct(GlobalGps);
	std::cout << "Latitude: "    << GlobalGps.fLatitude     << std::endl;
	std::cout << "Longitude: "   << GlobalGps.fLongitude    << std::endl;
	std::cout << "Altitude: "    << GlobalGps.fAltitude     << std::endl;
	std::cout << "Time: "        << GlobalGps.ulTime        << std::endl;
	std::cout << "Date: "        << GlobalGps.ulDate        << std::endl;
	std::cout << "Speed: "       << GlobalGps.fSpeed        << std::endl;
	std::cout << "Course: "      << GlobalGps.fCourse       << std::endl;
	std::cout << "Fix Mode: "    << GlobalGps.CeGpsFixMode  << std::endl;
	std::cout << "SNR: "         << GlobalGps.fGpsSNR       << std::endl;
	std::cout << "PDOP: "        << GlobalGps.dilution.pdop << std::endl;
	std::cout << "HDOP: "        << GlobalGps.dilution.hdop << std::endl;
	std::cout << "VDOP: "        << GlobalGps.dilution.vdop << std::endl;

	delete NewParser;
	return 0;
}
