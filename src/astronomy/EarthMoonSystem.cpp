#include "astronomy/EarthMoonSystem.hpp"
#include <cmath>
#include <stdexcept>
namespace atlas::astro {
EarthMoonLayout earthMoonLayout(const Vec3& moon,double factor){
 const double distance=std::hypot(moon[0],moon[1],moon[2]);
 if(!(distance>0&&std::isfinite(distance)&&std::isfinite(factor)
    &&factor>=1.0&&factor<=32.0))
  throw std::invalid_argument("Earth-Moon position missing, or invalid distance compression");
 EarthMoonLayout layout;
 layout.trueDistanceKm=distance;
 layout.centerSeparationCompression=factor;
 layout.shownDistanceEarthRadii=distance/(earthMeanRadiusKm*factor);
 for(int axis=0;axis<3;++axis)
  layout.moonCenterJ2000EarthRadii[axis]=moon[axis]/(earthMeanRadiusKm*factor);
 return layout;
}
} // namespace atlas::astro
