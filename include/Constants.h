// Constants and label mappings for plotting
#ifndef TMD_CONSTANTS_H
#define TMD_CONSTANTS_H

#include <string>
#include <map>

inline const std::map<std::string, std::string> VarToLabel = {
	{"X", "x_{B}"},
	{"Q", "Q [GeV]"},
	{"Z", "z"},
	{"PhPerp", "p_{T} [GeV]"}
};

// Integrated luminosities (units unspecified in file; treated as given constants)
// Values provided by user: 5x41 -> 0.44, 10x100 -> 7.9, 18x275 -> 1.54
inline const std::map<std::string, double> IntegratedLuminosities = {
	{"5x41", 0.44},
	{"10x100", 7.9},
	{"18x275", 1.54},
};

#endif // TMD_CONSTANTS_H
