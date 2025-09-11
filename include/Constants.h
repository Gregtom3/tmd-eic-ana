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

#endif // TMD_CONSTANTS_H
