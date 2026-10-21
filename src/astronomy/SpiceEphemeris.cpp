#include "astronomy/SpiceEphemeris.hpp"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#ifndef ATLAS_HAS_CSPICE
#define ATLAS_HAS_CSPICE 0
#endif
#if ATLAS_HAS_CSPICE
#include <SpiceUsr.h>
#endif
namespace atlas::astro {
namespace {
#if ATLAS_HAS_CSPICE
std::mutex& spiceMutex(){static std::mutex mutex;return mutex;}
std::string spiceError(){
 if(!failed_c())return {};
 char message[1841]{};
 getmsg_c("LONG",1840,message);
 reset_c();
 return std::string(message);
}
std::string isoUtc(std::chrono::system_clock::time_point instant){
 using namespace std::chrono;
 auto seconds=floor<std::chrono::seconds>(instant);
 auto day=floor<days>(seconds);
 const year_month_day date{day};
 const hh_mm_ss clock{seconds-day};
 char result[80];
 std::snprintf(result,sizeof(result),"%04d-%02u-%02u %02lld:%02lld:%02lld UTC",
  int(date.year()),unsigned(date.month()),unsigned(date.day()),
  static_cast<long long>(clock.hours().count()),
  static_cast<long long>(clock.minutes().count()),
  static_cast<long long>(clock.seconds().count()));
 return result;
}
#endif
}
SpiceEphemeris::SpiceEphemeris()=default;
SpiceEphemeris::~SpiceEphemeris(){
#if ATLAS_HAS_CSPICE
 std::lock_guard guard(spiceMutex());
 if(ready_)kclear_c();
#endif
}
bool SpiceEphemeris::builtWithSpice(){return ATLAS_HAS_CSPICE!=0;}
bool SpiceEphemeris::loadKernels(const std::string& lsk,const std::string& spk,std::string& error){
 error.clear();
#if ATLAS_HAS_CSPICE
 std::lock_guard guard(spiceMutex());
 if(!std::filesystem::is_regular_file(lsk)||!std::filesystem::is_regular_file(spk)){
  error="SPICE LSK/SPK kernel files do not exist";ready_=false;return false;
 }
 // CSPICE maintains a global kernel pool. Our application deliberately owns
 // it through this single provider instance; do not use SPICE concurrently.
 char action[]="RETURN";
 erract_c("SET",0,action);
 kclear_c();ready_=false;moonOrientationReady_=false;
 furnsh_c(lsk.c_str());
 if(failed_c()){error=spiceError();kclear_c();return false;}
 furnsh_c(spk.c_str());
 if(failed_c()){error=spiceError();kclear_c();return false;}
 ready_=true;moonOrientationReady_=false;return true;
#else
 (void)lsk;(void)spk;
 error="CSPICE is not enabled in this build; ephemerides unavailable";
 return false;
#endif
}
bool SpiceEphemeris::sample(std::chrono::system_clock::time_point utc,
 SolarLunarSample& value,std::string& error)const{
 error.clear();
#if ATLAS_HAS_CSPICE
 std::lock_guard guard(spiceMutex());
 if(!ready_){error="No valid SPICE kernels loaded";return false;}
 double et=0.0;
 const std::string timestamp=isoUtc(utc);
 str2et_c(timestamp.c_str(),&et);
 if(failed_c()){error=spiceError();return false;}
 SpiceDouble sun[6],moon[6],lightTime=0.0;
 // Geometric states at the same epoch. No LT/S aberration correction.
 spkezr_c("SUN",et,"J2000","NONE","EARTH",sun,&lightTime);
 if(failed_c()){error=spiceError();return false;}
 spkezr_c("MOON",et,"J2000","NONE","EARTH",moon,&lightTime);
 if(failed_c()){error=spiceError();return false;}
 SolarLunarSample computed;
 computed.ephemerisTimeTdbSeconds=et;
 for(int i=0;i<3;++i){computed.earthToSunJ2000Km[i]=sun[i];computed.earthToMoonJ2000Km[i]=moon[i];}
 try{computed.lunar=computeMoonGeometry(computed.earthToSunJ2000Km,computed.earthToMoonJ2000Km);}
 catch(const std::exception& ex){error=ex.what();return false;}
 value=computed;return true;
#else
 (void)utc;(void)value;
 error="CSPICE is not enabled in this build; lunar ephemeris unavailable";
 return false;
#endif
}
bool SpiceEphemeris::loadMoonOrientation(const std::string& frameKernel,const std::string& lunarPck,
 std::string& error){
 error.clear();
#if ATLAS_HAS_CSPICE
 std::lock_guard guard(spiceMutex());
 if(!ready_){error="Load a valid NAIF LSK and JPL SPK first";return false;}
 if(!std::filesystem::is_regular_file(frameKernel)||!std::filesystem::is_regular_file(lunarPck)){
  error="Required lunar MOON_ME frame and binary PCK files are missing";return false;
 }
 moonOrientationReady_=false;
 furnsh_c(frameKernel.c_str());
 if(failed_c()){error=spiceError();return false;}
 furnsh_c(lunarPck.c_str());
 if(failed_c()){error=spiceError();return false;}
 // Fail closed if a valid MOON_ME transform is not obtainable at J2000 epoch.
 // Runtime time coverage is separately validated on each sample.
 SpiceDouble rot[3][3];
 pxform_c("J2000","MOON_ME",0.0,rot);
 if(failed_c()){error=spiceError();return false;}
 moonOrientationReady_=true;
 return true;
#else
 (void)frameKernel;(void)lunarPck;
 error="CSPICE not enabled; lunar body-fixed orientation is unavailable";
 return false;
#endif
}
bool SpiceEphemeris::sampleMoonBodyFrame(std::chrono::system_clock::time_point utc,
 LunarBodyFrame& value,std::string& error)const{
 error.clear();
#if ATLAS_HAS_CSPICE
 std::lock_guard guard(spiceMutex());
 if(!ready_||!moonOrientationReady_){
  error="Precise MOON_ME orientation unavailable; load FK and binary lunar PCK";
  return false;
 }
 const auto timeString=isoUtc(utc);
 SpiceDouble et=0.0;
 str2et_c(timeString.c_str(),&et);
 if(failed_c()){error=spiceError();return false;}
 SpiceDouble sunState[6],earthState[6],lt=0,rot[3][3];
 spkezr_c("SUN",et,"J2000","NONE","MOON",sunState,&lt);
 if(failed_c()){error=spiceError();return false;}
 spkezr_c("EARTH",et,"J2000","NONE","MOON",earthState,&lt);
 if(failed_c()){error=spiceError();return false;}
 pxform_c("J2000","MOON_ME",et,rot);
 if(failed_c()){error=spiceError();return false;}
 const auto transform=[&](const SpiceDouble state[6]){
  Vec3 direction{};
  for(int row=0;row<3;++row){
   for(int col=0;col<3;++col)direction[row]+=rot[row][col]*state[col];
  }
  const double norm=std::hypot(direction[0],direction[1],direction[2]);
  if(!(norm>0.0&&std::isfinite(norm)))return Vec3{};
  for(auto& coordinate:direction)coordinate/=norm;
  return direction;
 };
 LunarBodyFrame result;
 result.moonToEarthMe=transform(earthState);
 result.moonToSunMe=transform(sunState);
 SpiceDouble lunarRotation[3][3];
 pxform_c("MOON_ME","J2000",et,lunarRotation);
 if(failed_c()){error=spiceError();return false;}
 for(int row=0;row<3;++row)
  for(int col=0;col<3;++col)
   result.meToJ2000[row*3+col]=lunarRotation[row][col];
 auto angles=[](const Vec3& direction){
  constexpr double radToDeg=180.0/3.14159265358979323846;
  return std::pair{std::atan2(direction[1],direction[0])*radToDeg,
   std::asin(std::clamp(direction[2],-1.0,1.0))*radToDeg};
 };
 const auto earth=angles(result.moonToEarthMe);
 const auto sun=angles(result.moonToSunMe);
 result.subEarthLongitudeDeg=earth.first;
 result.subEarthLatitudeDeg=earth.second;
 result.subSolarLongitudeDeg=sun.first;
 result.subSolarLatitudeDeg=sun.second;
 result.ephemerisTimeTdbSeconds=et;
 if(!std::isfinite(result.subEarthLongitudeDeg)||!std::isfinite(result.subSolarLongitudeDeg)){
  error="Non-finite MOON_ME lunar orientation";return false;
 }
 value=result;
 return true;
#else
 (void)utc;(void)value;
 error="CSPICE not enabled; lunar orientation unavailable";
 return false;
#endif
}
} // namespace atlas::astro
