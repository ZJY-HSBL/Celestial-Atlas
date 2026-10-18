#pragma once
#include "astronomy/MoonGeometry.hpp"
#include <chrono>
#include <string>
namespace atlas::astro {
struct SolarLunarSample {
 MoonGeometry lunar;
 Vec3 earthToSunJ2000Km{};
 Vec3 earthToMoonJ2000Km{};
 double ephemerisTimeTdbSeconds=0.0;
};
// Only reports real JPL SPK samples when built with ATLAS_ENABLE_CSPICE and
// after valid user-provided LSK/SPK kernels are explicitly loaded.
class SpiceEphemeris final {
public:
 SpiceEphemeris();
 ~SpiceEphemeris();
 SpiceEphemeris(const SpiceEphemeris&)=delete;
 SpiceEphemeris& operator=(const SpiceEphemeris&)=delete;
 static bool builtWithSpice();
 bool loadKernels(const std::string& lsk,const std::string& spk,std::string& error);
 bool sample(std::chrono::system_clock::time_point utc,SolarLunarSample& value,std::string& error) const;
 bool ready()const{return ready_;}
private:
 bool ready_=false;
};
} // namespace atlas::astro
