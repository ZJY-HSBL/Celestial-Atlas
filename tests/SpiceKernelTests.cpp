#include "astronomy/SpiceEphemeris.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
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
  std::cout<<"SPICE integration PASS, 2024-04-08 illumination="<<eclipse.lunar.illuminatedFraction
   <<", Moon distance km="<<eclipse.lunar.moonDistanceKm
   <<", Sun distance km="<<eclipse.lunar.sunDistanceKm<<"\n";
  return 0;
 }catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 4;}
}
