#pragma once
#include <chrono>
namespace atlas::astro {
// Approximate solar model for visualization; NOT JPL CSPICE or scientific ephemerides.
// RA/equator of date (approx), GMST and longitude east positive in [-180,+180].
struct UnitVector { double x, y, z; };
UnitVector celestialUnitVector(double rightAscensionDeg, double declinationDeg);
UnitVector surfaceNormalInertial(double latitudeDeg, double longitudeDeg, double greenwichSiderealDeg);
double dot(const UnitVector& a, const UnitVector& b);
// Coordinate axes: +X = zero RA, +Z = RA 90 east, +Y = north.
struct SolarGeometry {
 double julianDateUtc;
 double rightAscensionDeg;
 double declinationDeg;
 double greenwichMeanSiderealDeg;
 double subsolarLatitudeDeg;
 double subsolarLongitudeDeg;
};
SolarGeometry solarGeometry(std::chrono::system_clock::time_point utc);
double solarAltitudeDeg(const SolarGeometry& solar, double latitudeDeg, double longitudeDeg);
double normalize360(double degrees);
double normalize180(double degrees);
} // namespace atlas::astro
