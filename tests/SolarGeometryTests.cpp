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
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
