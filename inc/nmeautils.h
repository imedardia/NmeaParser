#ifndef __NMEA_UTILS_H__
#define __NMEA_UTILS_H__

#define FeetsInMeter 3.2808399

enum ConvertField {
	CONV_LAT = 0,
	CONV_LON,
	CONV_TOT
};

class NmeaUtils
{
	public:
		NmeaUtils();
		~NmeaUtils();
		int ConvertHexStrToInt(std::string csval);
		int ConvertStrToInt(std::string csval);
		double ConvertStrToDouble(std::string csval);
		int CalculateXoRCheckSum(const char * Sentence, int size);
		double ConvertDoubleDistUnits(std::string Value, std::string Units);
		double ConvertStrToLat(std::string Value, std::string Dir);
		double ConvertStrToLon(std::string Value, std::string Dir);
		double ConvertStrToMagnDev(std::string Value, std::string Dir);
	private:
		template <ConvertField Conv>
		double ConvertStrToLatLon(std::string Value, std::string Dir);
};

#endif
