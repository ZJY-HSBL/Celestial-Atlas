#include "astronomy/EarthMoonSystem.hpp"
#include <cmath>
#include <exception>
#include <iostream>
using namespace atlas::astro;
int main(){
 int failures=0;
 const auto test=[&](bool ok,const char* msg){
  std::cout<<(ok?"PASS ":"FAIL ")<<msg<<'\n';if(!ok)++failures;
 };
 const Vec3 measured{384400.0,0,0};
 auto trueScale=earthMoonLayout(measured,1.0);
 auto compressed=earthMoonLayout(measured,12.0);
 test(std::abs(trueScale.trueDistanceKm-384400)<1e-9,"physical JPL distance in km");
 test(std::abs(trueScale.shownDistanceEarthRadii-384400.0/6371.0)<1e-12,
  "true separation is approximately 60.3 Earth radii");
 test(std::abs(compressed.moonCenterJ2000EarthRadii[0]*12-
          trueScale.moonCenterJ2000EarthRadii[0])<1e-12,
  "only center separation is compressed by documented factor");
 test(std::abs(compressed.moonRadiusEarthRadii-1737.4/6371.0)<1e-12,
  "Moon physical radius stays unchanged in compressed mode");
 test(std::abs(compressed.trueDistanceKm-trueScale.trueDistanceKm)<1e-12,
  "compression never changes reported physical distance");
 test(earthMoonLayout({0,384400,0},1).moonCenterJ2000EarthRadii[1]>60,
  "Moon orientation represented in same J2000 frame");
 bool rejected=false;
 try{(void)earthMoonLayout({0,0,0},12);}catch(const std::exception&){rejected=true;}
 test(rejected,"no SPICE sample means no invented orbit");
 rejected=false;
 try{(void)earthMoonLayout(measured,0.5);}catch(const std::exception&){rejected=true;}
 test(rejected,"unreported scale enlargement rejected");
 return failures!=0;
}
