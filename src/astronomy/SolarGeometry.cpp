#include "astronomy/SolarGeometry.hpp"
#include <algorithm>
#include <cmath>
namespace atlas::astro {
namespace {
constexpr double pi=3.1415926535897932384626433832795;
constexpr double d2r=pi/180.0, r2d=180.0/pi;
double sinDeg(double x){return std::sin(x*d2r);}
double cosDeg(double x){return std::cos(x*d2r);}
}
double normalize360(double v){v=std::fmod(v,360.0);return v<0?v+360.0:v;}
double normalize180(double v){return normalize360(v+180.0)-180.0;}
SolarGeometry solarGeometry(std::chrono::system_clock::time_point utc){
 const double seconds=std::chrono::duration<double>(utc.time_since_epoch()).count();
 const double jd=seconds/86400.0+2440587.5;
 const double T=(jd-2451545.0)/36525.0;
 const double L0=normalize360(280.46646+T*(36000.76983+0.0003032*T));
 const double M=normalize360(357.52911+T*(35999.05029-0.0001537*T));
 const double C=(1.914602-T*(0.004817+0.000014*T))*sinDeg(M)
    +(0.019993-0.000101*T)*sinDeg(2*M)+0.000289*sinDeg(3*M);
 const double omega=125.04-1934.136*T;
 const double lambda=L0+C-0.00569-0.00478*sinDeg(omega);
 const double epsilon0=23.0+(26.0+(21.448-T*(46.815+T*(0.00059-T*0.001813)))/60.0)/60.0;
 const double epsilon=epsilon0+0.00256*cosDeg(omega);
 const double ra=normalize360(std::atan2(cosDeg(epsilon)*sinDeg(lambda),cosDeg(lambda))*r2d);
 const double dec=std::asin(sinDeg(epsilon)*sinDeg(lambda))*r2d;
 // Approximation substitutes UTC for the rigorously required UT1.
 const double gmst=normalize360(280.46061837+360.98564736629*(jd-2451545.0)
                                 +0.000387933*T*T-(T*T*T)/38710000.0);
 return {jd,ra,dec,gmst,dec,normalize180(ra-gmst)};
}
double solarAltitudeDeg(const SolarGeometry& sun,double latitude,double longitude){
 double s=sinDeg(latitude)*sinDeg(sun.subsolarLatitudeDeg)
   +cosDeg(latitude)*cosDeg(sun.subsolarLatitudeDeg)*cosDeg(longitude-sun.subsolarLongitudeDeg);
 return std::asin(std::clamp(s,-1.0,1.0))*r2d;
}
} // namespace atlas::astro
