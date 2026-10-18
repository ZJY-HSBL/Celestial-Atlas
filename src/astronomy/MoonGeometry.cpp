#include "astronomy/MoonGeometry.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace atlas::astro {
namespace {
constexpr double radToDeg=57.2957795130823208768;
double dot3(const Vec3& a,const Vec3& b){
 return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}
double norm(const Vec3& v){return std::sqrt(dot3(v,v));}
double angle(const Vec3& a,const Vec3& b){
 const double na=norm(a),nb=norm(b);
 if(!(na>0.0&&nb>0.0&&std::isfinite(na)&&std::isfinite(nb)))
  throw std::invalid_argument("Non-finite or zero-length astronomical position");
 return std::acos(std::clamp(dot3(a,b)/(na*nb),-1.0,1.0));
}
}
MoonGeometry computeMoonGeometry(const Vec3& sun,const Vec3& moon){
 const double sunDistance=norm(sun),moonDistance=norm(moon);
 if(!(sunDistance>0&&moonDistance>0))
  throw std::invalid_argument("Sun and Moon vectors must have positive length");
 const Vec3 moonToSun{sun[0]-moon[0],sun[1]-moon[1],sun[2]-moon[2]};
 const Vec3 moonToEarth{-moon[0],-moon[1],-moon[2]};
 const double phase=angle(moonToEarth,moonToSun);
 const double elongation=angle(sun,moon);
 return {std::clamp((1.0+std::cos(phase))*0.5,0.0,1.0),
   phase*radToDeg,elongation*radToDeg,moonDistance,sunDistance};
}
} // namespace atlas::astro
