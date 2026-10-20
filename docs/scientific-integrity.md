# Scientific integrity / 科学真实性规范

1. Separate **observed imagery**, **measurement-constrained visualization** and **artistic illustration**. Artistic illustrations must never masquerade as observation.
2. For every data asset, record original source URL, mission or archive, processing description, time if known, scientific references when applicable, and SHA-256.
3. NASA Blue Marble is a processed composite of multiple satellite observations. It is neither a single raw frame nor a live map of Earth.
4. The current Sun position and Earth rotation use a *low-precision approximation*. The computation substitutes UTC where high-precision Earth orientation would need UT1, TT/TDB, polar motion, precession/nutation and IERS data.
5. No fictional exoplanet surfaces. Without measured resolved imaging, present a point detection and known parameters, or mark data unavailable.
6. Treat a single spacecraft view of Jupiter or Saturn as observed imagery, **not** as a measured global equirectangular texture.
7. Do not quietly add random data to fill gaps in imagery or catalog coordinates. Flag missing and uncertain measurements.
8. User-selected visual time may run at accelerated rates; the camera's faster-than-light navigation is a UI animation, not physical travel.

## Planned precision migration

Use NAIF CSPICE kernels with explicit kernel manifests (LSK, SPK, PCK, Earth orientation kernels as appropriate) and state whether a computed position is geometric/apparent, specify the frame and corrections, and validate specific times and targets against JPL Horizons. Maintain time-scale conversions and coordinate-frame provenance.

## v0.3 scientific precision boundary

JPL DE442s + NASA NAIF N0067 power Moon/Sun *geometric J2000 states* where the official toolkit is compiled and kernels loaded. The Moon illuminated fraction uses Sun–Moon–Earth geometry from those vectors, not a synthetic phase. The **Earth surface day/night terminator is still a distinct approximate model**. Do not label it CSPICE-grade until rigorous Earth orientation and time-frame conversion are implemented and validated. Windows default build does not ship CSPICE: no made-up lunar phase values when unavailable.

## v0.5 lunar phase / libration

The Moon Survey becomes a geocentric geometric lunar visualization only when
the **DE440 binary lunar orientation PCK and companion MOON_ME FK** and
**DE442s planetary ephemeris SPK**, together with an NAIF LSK and compiled
CSPICE, are present. The project reports the distinct data sources.
Topocentric parallax, light-time/aberration, DEM relief, Earth-orientation
accuracy, and eclipses are not yet modeled in the rendered globe. When lunar
orientation kernels are absent, the fixed studio light is explicit.

## v0.6 geometry disclosure

Real-scale and presentation-scale Earth–Moon views are different. Presentation only compresses distance between centers by 12, not the sphere radii or JPL-reported true physical separation. Neither setting is a complete tidal/shape model: both bodies remain spheres; Earth rotation is an explicitly approximate GMST frame, while lunar frame orientation is from a measured JPL DE440 PCK. The implementation refuses to substitute a circular orbit if true CSPICE data are unavailable.
