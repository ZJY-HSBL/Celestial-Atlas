# Moon Atlas — NASA LRO Survey & JPL Geocentric Libration

**v0.5.1 scientific contract.** This project uses the real NASA 2025 CGI Moon Kit
[LROC WAC global map](https://svs.gsfc.nasa.gov/4720/) for the lunar surface.
The 2048×1024 JPEG is based on lunar orbiter observations; it is a
visualization-processed composite, not a single raw photo. Some high-latitude
gaps are supplemented by LOLA albedo observations. No artificial continents,
AI image synthesis or implied live satellite photographs.

**When all optional NASA kernel files are available**, native CSPICE performs:

1. Convert selected UTC to NAIF ephemeris time (ET/TDB) using `naif0012.tls`.
2. Compute geometric Moon→Earth and Moon→Sun J2000/ICRF vectors with
   `spkezr_c`, targets `EARTH` and `SUN`, observer `MOON`, corrections `NONE`,
   using JPL `de442s.bsp`.
3. Transform both directions into the `MOON_ME` frame through
   `pxform_c("J2000","MOON_ME",et,...)`, backed by
   `moon_pa_de440_200625.bpc` and `moon_de440_250416.tf`.
4. Express ME +X,+Y,+Z in the viewport's +X,+Z,+Y axes. Move the camera
   along the lunar ME Earth direction; illuminate from the lunar ME Sun
   direction. This creates time-dependent **geocentric lunar libration and
   geometric lunar phase** directly on the real LROC image.

A normal drag releases sub-Earth camera tracking; reset returns to the actual
sub-Earth viewing direction. The numerical phase fraction is cross-checked
against the two computed unit directions.

**Fallback:** The Windows portable binary in v0.5.1 does *not* ship a compiled
CSPICE library. Without compiled CSPICE and all official FK/PCK/LSK/SPK kernels,
the UI explicitly displays Moon Survey mode with fixed studio light. Do not
interpret that mode as actual Moon phases or positions. The underlying Moon
map remains a real observation-based NASA image in both modes.

**Important boundaries:** The model is a fixed-radius **standalone Moon
globe**, not yet an Earth–Moon spatial orbit. The view is geocentric, not a
topocentric observatory view and does not apply light-time, parallax, local
refraction or surface elevation. DE440 lunar orientation and DE442s positions
are used separately; their model versions are documented instead of obscured.
Earth's day/night terminator still uses a separate approximate model.

**Reference imagery:** NASA SVS [CGI Moon Kit](https://svs.gsfc.nasa.gov/4720/).
Official lunar frame definitions: [NAIF DE440 Moon FK](https://naif.jpl.nasa.gov/pub/naif/generic_kernels/fk/satellites/moon_de440_250416.tf)
and [lunar binary PCK](https://naif.jpl.nasa.gov/pub/naif/generic_kernels/pck/moon_pa_de440_200625.cmt).

The release screenshots compare full and new Moon at distinct actual UTC
epochs with fixed UTC through the application's `--utc` CLI flag; they are
not conceptual/mockup images.
