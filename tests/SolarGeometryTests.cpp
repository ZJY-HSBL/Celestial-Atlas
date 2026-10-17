#include "astronomy/SolarGeometry.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace atlas::astro;
using namespace std::chrono;
namespace {
int failures=0;
void expect(bool b,const char* message){
 std::cout<<(b?"PASS: ":"FAIL: ")<<message<<'\n';
 if(!b)++failures;
}
system_clock::time_point utc(int year,unsigned m,unsigned d,int h=0){
 return sys_days{year_month_day{std::chrono::year{year},std::chrono::month{m},std::chrono::day{d}}}+hours(h);
}
}
int main(){
 expect(normalize360(-15)==345,"normalize360");
 expect(normalize180(190)==-170,"normalize180");
 auto j=solarGeometry(utc(2000,1,1,12));
 expect(std::abs(j.julianDateUtc-2451545.0)<1e-9,"Julian date");
 expect(std::abs(j.greenwichMeanSiderealDeg-280.46061837)<1e-4,"approximate GMST");
 auto mar=solarGeometry(utc(2026,3,20,15));
 expect(std::abs(mar.declinationDeg)<0.5,"March equinox declination");
 auto jun=solarGeometry(utc(2026,6,21,12));
 auto dec=solarGeometry(utc(2026,12,21,12));
 expect(jun.declinationDeg>23.3&&jun.declinationDeg<23.6,"June solstice");
 expect(dec.declinationDeg< -23.3&&dec.declinationDeg> -23.6,"December solstice");
 expect(std::abs(solarAltitudeDeg(jun,jun.subsolarLatitudeDeg,jun.subsolarLongitudeDeg)-90.0)<1e-5,"subsolar altitude");
 expect(std::abs(solarAltitudeDeg(jun,-jun.subsolarLatitudeDeg,normalize180(jun.subsolarLongitudeDeg+180))+90.0)<1e-5,"antipodal altitude");
 auto equator=surfaceNormalInertial(0.0,0.0,0.0);
 expect(std::abs(equator.x-1.0)<1e-12 && std::abs(equator.y)<1e-12 && std::abs(equator.z)<1e-12,
        "Greenwich equator normal");
 auto north=surfaceNormalInertial(90.0,0.0,90.0);
 expect(std::abs(north.y-1.0)<1e-12,"north pole normal independent of sidereal rotation");
 auto east=surfaceNormalInertial(0.0,90.0,0.0);
 expect(std::abs(east.z-1.0)<1e-12,"east longitude matches RA 90 positive Z");
 auto sun=celestialUnitVector(jun.rightAscensionDeg,jun.declinationDeg);
 auto surface=surfaceNormalInertial(jun.subsolarLatitudeDeg,jun.subsolarLongitudeDeg,jun.greenwichMeanSiderealDeg);
 expect(std::abs(dot(sun,surface)-1.0)<1e-10,"solar model and Earth rotation agree at subsolar point");
 auto antipode=surfaceNormalInertial(-jun.subsolarLatitudeDeg,
     normalize180(jun.subsolarLongitudeDeg+180),jun.greenwichMeanSiderealDeg);
 expect(std::abs(dot(sun,antipode)+1.0)<1e-10,"opposite side of Earth is in darkness");
 for(int lat=-75;lat<=75;lat+=25){
  for(int lon=-180;lon<=180;lon+=30){
   const auto p=surfaceNormalInertial(lat,lon,mar.greenwichMeanSiderealDeg);
   const auto d=celestialUnitVector(mar.rightAscensionDeg,mar.declinationDeg);
   const double a=solarAltitudeDeg(mar,lat,lon);
   expect(std::abs(dot(p,d)-std::sin(a*3.14159265358979323846/180.0))<1e-12,
          "geographic sun altitude matches inertial 3D lighting");
  }
 }
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
