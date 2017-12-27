#ifndef __NMEA_UTILS_H__
#define __NMEA_UTILS_H__

enum ConvertField {
	CONV_LAT = 0,
	CONV_LON,
	CONV_TOT
};

int ConvertHexStrToInt(std::string csval);
int ConvertStrToInt(std::string csval);
double ConvertStrToDouble(std::string csval);
int CalculateXoRCheckSum(const char * Sentence, int size);
double ConvertDistUnits(std::string Value, std::string Units);
double ConvertStrToLat(std::string Value, std::string Dir);
double ConvertStrToLon(std::string Value, std::string Dir);

#endif
