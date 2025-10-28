// Constants and label mappings for plotting
#ifndef TMD_CONSTANTS_H
#define TMD_CONSTANTS_H

#include <iostream>
#include <string>
#include <map>
#include <tuple>

inline const std::map<std::string, std::string> VarToLabel = {
	{"X", "x_{B}"},
	{"Q", "Q [GeV]"},
	{"Z", "z"},
	{"PhPerp", "p_{T} [GeV]"},
	{"PhiH", "#phi_{h} [rad]"},
	{"PhiS", "#phi_{S} [rad]"},
	{"Y", "y"}
};


enum class channel_type {
    Hadron,
    Dihadron
};

enum class eic_timeline_type {
    EarlyScience,
    Full
};

enum class target_type {
    Proton,
    Helium3
};


struct LuminosityKey {
    std::string energy;
    eic_timeline_type timeline;
    target_type target;

    bool operator<(const LuminosityKey& other) const noexcept {
        return std::tie(energy, timeline, target) <
               std::tie(other.energy, other.timeline, other.target);
    }
};

inline std::ostream& operator<<(std::ostream& os, const LuminosityKey& k) {
    os << "(" << k.energy
       << ", " << (k.timeline == eic_timeline_type::Full ? "Full" : "EarlyScience")
       << ", " << (k.target == target_type::Proton ? "Proton" : "Helium3")
       << ")";
    return os;
}

inline const std::map<LuminosityKey, double> IntegratedLuminosities = {
    {{"5x41",   eic_timeline_type::Full,        	target_type::Proton},   2.86},
    {{"10x100", eic_timeline_type::Full,        	target_type::Proton},   51.3},
    {{"18x275", eic_timeline_type::Full,        	target_type::Proton},   10.0},
    {{"10x100", eic_timeline_type::EarlyScience,	target_type::Proton},   10.0},
    {{"10x166", eic_timeline_type::EarlyScience,	target_type::Helium3},  8.65},
};


#endif // TMD_CONSTANTS_H
