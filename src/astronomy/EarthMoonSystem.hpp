#pragma once
#include "astronomy/MoonGeometry.hpp"
namespace atlas::astro {
// One render unit equals 6371 km, Earth's approximate mean radius.
// The Earth is the scene origin and the real Moon radius is conserved.
// Only Moon center separation may be compressed, always disclosed.
inline constexpr double earthMeanRadiusKm=6371.0;
inline constexpr double moonMeanRadiusKm=1737.4;
inline constexpr double moonRadiusEarthUnits=moonMeanRadiusKm/earthMeanRadiusKm;
struct EarthMoonLayout {
 Vec3 moonCenterJ2000EarthRadii{};
 double trueDistanceKm=0;
 double shownDistanceEarthRadii=0;
 double centerSeparationCompression=1;
 double moonRadiusEarthRadii=moonRadiusEarthUnits;
};
EarthMoonLayout earthMoonLayout(const Vec3& earthToMoonJ2000Km,double distanceCompression);
}
