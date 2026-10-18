#pragma once
#include <array>
namespace atlas::astro {
using Vec3 = std::array<double,3>;
struct MoonGeometry {
 double illuminatedFraction = 0.0;
 double phaseAngleDeg = 0.0; // Sun-Moon-Earth angle; 0 = full Moon
 double elongationDeg = 0.0; // Sun-Earth-Moon angle; 0 = new Moon
 double moonDistanceKm = 0.0;
 double sunDistanceKm = 0.0;
};
// Inputs: geocentric SUN and MOON geometric J2000 position vectors, both in km
// at the same ephemeris epoch and without aberration corrections.
MoonGeometry computeMoonGeometry(const Vec3& earthToSunKm,const Vec3& earthToMoonKm);
}
