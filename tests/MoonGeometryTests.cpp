#include "astronomy/MoonGeometry.hpp"
#include "astronomy/SpiceEphemeris.hpp"
#include <cmath>
#include <exception>
#include <iostream>
int main(){
 using namespace atlas::astro;
 int failed=0;
 const auto check=[&](bool condition,const char* message){
  std::cout<<(condition?"PASS ":"FAIL ")<<message<<'\n';
  if(!condition)++failed;
 };
 const auto newMoon=computeMoonGeometry({150000000.0,0,0},{384400.0,0,0});
 check(newMoon.illuminatedFraction<1e-12,"new Moon zero illuminated fraction");
 check(newMoon.elongationDeg<1e-12,"new Moon elongation zero");
 const auto fullMoon=computeMoonGeometry({150000000.0,0,0},{-384400.0,0,0});
 check(fullMoon.illuminatedFraction>1.0-1e-12,"full Moon all illuminated");
 check(std::abs(fullMoon.elongationDeg-180)<1e-10,"full Moon elongation 180 degrees");
 const auto quarter=computeMoonGeometry({150000000.0,0,0},{0,384400.0,0});
 check(std::abs(quarter.illuminatedFraction-0.5)<0.005,"quarter Moon near 50 percent");
 check(std::abs(quarter.moonDistanceKm-384400)<1e-8,"Earth-Moon distance preserved");
 bool zeroRejected=false;
 try{(void)computeMoonGeometry({0,0,0},{1,0,0});}
 catch(const std::exception&){zeroRejected=true;}
 check(zeroRejected,"invalid vectors rejected");
 std::string error;
 SpiceEphemeris spice;
 SolarLunarSample invalid;
 check(!spice.sample(std::chrono::system_clock::now(),invalid,error) && !error.empty(),
   "without explicit kernels no fabricated ephemeris");
 return failed?1:0;
}
