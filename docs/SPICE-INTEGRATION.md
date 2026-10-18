# JPL CSPICE precise ephemeris integration / 精密星历

**Scientific computation, not an illustration.**

Celestial Atlas v0.3.0 contains an optional NASA NAIF CSPICE adapter that computes
geometric J2000 Earth→Sun and Earth→Moon position vectors from the **DE442s SPK**
and the **naif0012.tls leap-second kernel**. The lunar illuminated fraction is
computed from the Sun–Moon–Earth phase angle; the astronomical distances are
given in kilometres. No value is invented if kernels or toolkit are missing.

The *Earth sunlight shader is still the prior low-precision approximation*
and is **not claimed to use the precise SPICE Sun vector**. This requires
rigorous Earth orientation (ITRF/EOP/UT1) integration before we can claim
sub-degree precision for the geodetic subsolar point.

## Required official files

- Toolkit: https://naif.jpl.nasa.gov/naif/toolkit.html
- DE442s SPK: https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de442s.bsp
- LSK: https://naif.jpl.nasa.gov/pub/naif/generic_kernels/lsk/naif0012.tls
- Rules and attribution: https://naif.jpl.nasa.gov/naif/rules.html

NASA toolkit code and kernel files are intentionally **not checked into Git**.

## Linux build with NAIF N0067

Download the official Linux C toolkit archive (N0067) from:
https://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Linux_GCC_64bit/packages/cspice.tar.Z

Unpack it so that the headers and libraries exist at
`/path/to/cspice/include/SpiceUsr.h` and `/path/to/cspice/lib/cspice.a`.

Download the two data kernels with:

```bash
python3 scripts/fetch_spice_kernels.py --output data/kernels

cmake -S . -B build-spice -DATLAS_ENABLE_CSPICE=ON \
  -DCSPICE_ROOT=/path/to/cspice \
  -DATLAS_SPICE_LSK="$PWD/data/kernels/naif0012.tls" \
  -DATLAS_SPICE_SPK="$PWD/data/kernels/de442s.bsp"
cmake --build build-spice --parallel
ctest --test-dir build-spice --output-on-failure
ATLAS_SPICE_KERNEL_DIR="$PWD/data/kernels" ./build-spice/CelestialAtlas
```

Use `-DATLAS_BUILD_GUI=OFF` to compile the science library without Qt.

**Windows:** the official Windows CSPICE static library can require a rebuild
with your newer Visual Studio, per NAIF's compatibility note. The standard
Windows portable preview is compiled without CSPICE and therefore explicitly
marks lunar ephemerides unavailable. It does *not* silently substitute a
hand-drawn phase estimate.

## Scientific definitions

- Source model: DE442s (JPL planetary ephemeris), with NAIF N0067 Toolkit.
- Clock: input UTC, converted by CSPICE to ET/TDB via NAIF LSK.
- Frame: `J2000`; observer: `EARTH`; targets: `SUN`, `MOON`.
- Corrections: `NONE` (geometric, no light-time or stellar aberration).
- Output: Earth-relative positions in kilometres.
- Lunar phase angle: angle at the Moon between directions to Earth and Sun.
- Illuminated fraction: `k = (1 + cos(phase_angle))/2`.
- Validity: only within the loaded ephemeris coverage; outside coverage
  CSPICE reports an error and the app must not invent a numerical value.
- C-SPICE uses global mutable error and kernel state, protected by one mutex
  and owned by one `SpiceEphemeris` object in this initial architecture.

Tests cross-check eclipse epochs (2024-04-08 and 2024-03-25), magnitude of
Sun/Earth/Moon distances and synthetic phase geometry. Further high-precision
comparison against published Horizons reference vectors is still planned.

## Images / 真实天体照片

Click the Moon image button in the desktop app to open a **NASA LRO observed
Moon photomosaic**: https://science.nasa.gov/resource/moon-mosaic/.
This is not a global measured 3D Moon surface texture; 3D lunar terrain/imagery
requires a separate properly registered lunar map from PDS.
