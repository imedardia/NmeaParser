#ifndef __NMEAPRES_H__
#define __NMEAPRES_H__

#include "nmeaparser.h"

#define SIMU_DIR "../"
#define SIMU_FILE "NmeaFrames.nmeap"

#define LOG_NDEBUG 1

class NmeaPres : public NmeaParser
{
	public:
		NmeaPres(std::string NmeaPort, const unsigned long cuwBaudrate);
		~NmeaPres();
		bool NmeaParserInit;

	private:
		unsigned long ulBaudrate;
		std::string NmeaSerialPort;
	    int NmeaInitParsers();
		static void gga_callback(std::string NmeaType, void * ggaStruct);
		static void rmc_callback(std::string NmeaType, void * rmcStruct);
		static void gsa_callback(std::string NmeaType, void * gsaStruct);
		static void gsv_callback(std::string NmeaType, void * gsvStruct);	
};

#endif
