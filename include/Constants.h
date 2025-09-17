// Constants and label mappings for plotting
#ifndef TMD_CONSTANTS_H
#define TMD_CONSTANTS_H

#include <string>
#include <map>

inline const std::map<std::string, std::string> VarToLabel = {
	{"X", "x_{B}"},
	{"Q", "Q [GeV]"},
	{"Z", "z"},
	{"PhPerp", "p_{T} [GeV]"},
	{"PhiH", "#phi_{h} [rad]"},
	{"PhiS", "#phi_{S} [rad]"},
	{"Y", "y"}
};


inline const std::map<std::string, double> IntegratedLuminosities = {
	{"5x41", 2.86}, // in fb^-1
	{"10x100", 51.3}, // in fb^-1
	{"18x275", 10.0}, // in fb^-1
};

#endif // TMD_CONSTANTS_H
