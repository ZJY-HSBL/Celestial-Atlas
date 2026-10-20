#pragma once
#include "astronomy/MoonGeometry.hpp"
#include <chrono>
#include <string>
namespace atlas::astro {
// Orthogonal Moon_ME body frame directions at a chosen ET/TDB instant.
struct LunarBodyFrame {
 Vec3 moonToEarthMe{}; // SPICE axes +X 0E, +Y 90E, +Z north
 Vec3 moonToSunMe{};
 // 3x3 row-major rotation: MOON_ME local vector -> J2000 inertial vector.
 // Uses an official lunar orientation binary PCK and compatible FK.
 std::array<double,9> meToJ2000{};
 double subEarthLongitudeDeg=0.0;
 double subEarthLatitudeDeg=0.0;
 double subSolarLongitudeDeg=0.0;
 double subSolarLatitudeDeg=0.0;
 double ephemerisTimeTdbSeconds=0.0;
};
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
 bool loadMoonOrientation(const std::string& frameKernel,const std::string& lunarPck,std::string& error);
 bool sampleMoonBodyFrame(std::chrono::system_clock::time_point utc,LunarBodyFrame& value,std::string& error) const;
 bool lunarOrientationReady()const{return moonOrientationReady_;}
 bool ready()const{return ready_;}
private:
 bool ready_=false;
 bool moonOrientationReady_=false;
};
} // namespace atlas::astro
