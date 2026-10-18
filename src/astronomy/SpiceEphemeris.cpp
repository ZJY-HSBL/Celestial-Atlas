#include "astronomy/SpiceEphemeris.hpp"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <string>
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
 std::snprintf(result,sizeof(result),"%04d-%02u-%02uT%02lld:%02lld:%02lld UTC",
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
 kclear_c();ready_=false;
 furnsh_c(lsk.c_str());
 if(failed_c()){error=spiceError();kclear_c();return false;}
 furnsh_c(spk.c_str());
 if(failed_c()){error=spiceError();kclear_c();return false;}
 ready_=true;return true;
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
} // namespace atlas::astro
