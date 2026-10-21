#include "astronomy/SpiceEphemeris.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>
using namespace std::chrono;
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: spice_kernel_tests LSK SPK\n";return 2;}
 atlas::astro::SpiceEphemeris spice;
 std::string error;
 if(!spice.loadKernels(argv[1],argv[2],error)){std::cerr<<error<<'\n';return 3;}
 auto sample=[&](int year,unsigned month,unsigned day,int hour){
  atlas::astro::SolarLunarSample out;
  auto utc=sys_days{year_month_day{std::chrono::year{year},std::chrono::month{month},std::chrono::day{day}}}+hours(hour);
  if(!spice.sample(utc,out,error))throw std::runtime_error(error);
  return out;
 };
 try{
  // Total solar eclipse 2024-04-08 around 18:00 UTC, necessarily near new Moon.
  const auto eclipse=sample(2024,4,8,18);
  if(!(eclipse.lunar.illuminatedFraction<0.01))throw std::runtime_error("eclipse epoch is not near new Moon");
  if(!(eclipse.lunar.moonDistanceKm>340000&&eclipse.lunar.moonDistanceKm<410000))throw std::runtime_error("moon distance unphysical");
  if(!(eclipse.lunar.sunDistanceKm>145000000&&eclipse.lunar.sunDistanceKm<155000000))throw std::runtime_error("sun distance unphysical");
  // Total lunar eclipse 2024-03-25 (penumbral) occurs near full Moon.
  const auto full=sample(2024,3,25,7);
  if(!(full.lunar.illuminatedFraction>0.99))throw std::runtime_error("lunar eclipse epoch not near full Moon");
  const auto later=sample(2024,4,15,18);
  if(!(later.lunar.illuminatedFraction>0.3&&later.lunar.illuminatedFraction<0.7))throw std::runtime_error("quarter Moon illumination outside broad range");
  std::string moonFK,moonPCK;
  if(const char* p=std::getenv("ATLAS_SPICE_MOON_FK"))moonFK=p;
  if(const char* p=std::getenv("ATLAS_SPICE_MOON_PCK"))moonPCK=p;
  if(!moonFK.empty()&&!moonPCK.empty()){
   if(!spice.loadMoonOrientation(moonFK,moonPCK,error))throw std::runtime_error(error);
   atlas::astro::LunarBodyFrame frame;
   const auto utc=sys_days{year_month_day{std::chrono::year{2024},std::chrono::month{4},std::chrono::day{8}}}+hours(18);
   if(!spice.sampleMoonBodyFrame(utc,frame,error))throw std::runtime_error(error);
   const auto norm=[](const atlas::astro::Vec3& v){return std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);};
   const auto dot=[](const atlas::astro::Vec3& a,const atlas::astro::Vec3& b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];};
   if(std::abs(norm(frame.moonToEarthMe)-1.0)>1e-10||std::abs(norm(frame.moonToSunMe)-1.0)>1e-10)
    throw std::runtime_error("MOON_ME vectors not unit length");
   if(std::abs(frame.subEarthLongitudeDeg)>12||std::abs(frame.subEarthLatitudeDeg)>12)
    throw std::runtime_error("Unexpected lunar libration outside broad geometric bounds");
   // Both data paths share geometric Earth-Moon-Sun angle despite the frame transform.
   const double illumination=(1.0+dot(frame.moonToEarthMe,frame.moonToSunMe))/2.0;
   if(std::abs(illumination-eclipse.lunar.illuminatedFraction)>1e-9)
    throw std::runtime_error("MOON_ME orientation inconsistent with J2000 phase angle");
   atlas::astro::LunarBodyFrame late;
   const auto laterUtc=utc+hours(24*8);
   if(!spice.sampleMoonBodyFrame(laterUtc,late,error))throw std::runtime_error(error);
   if(std::abs(frame.subEarthLongitudeDeg-late.subEarthLongitudeDeg)<0.01&&
      std::abs(frame.subEarthLatitudeDeg-late.subEarthLatitudeDeg)<0.01)
    throw std::runtime_error("Lunar orientation did not change over eight days");
   std::cout<<"MOON_ME frame PASS sub-Earth lon="<<frame.subEarthLongitudeDeg
    <<", lat="<<frame.subEarthLatitudeDeg<<"; 8 days later lon="<<late.subEarthLongitudeDeg
    <<", lat="<<late.subEarthLatitudeDeg<<"\\n";
  }
  std::cout<<"SPICE integration PASS, 2024-04-08 illumination="<<eclipse.lunar.illuminatedFraction
   <<", Moon distance km="<<eclipse.lunar.moonDistanceKm
   <<", Sun distance km="<<eclipse.lunar.sunDistanceKm<<"\n";
  return 0;
 }catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 4;}
}
