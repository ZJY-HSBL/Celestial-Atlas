# Celestial Atlas v0.6.0 verification contract

**Hard-gated GitHub CI:**
- Linux C++20 core without Qt6 or SPICE: solar and Moon geometry tests.
- Windows/MSVC/Qt6 GUI compilation, tests, portable ZIP assembled using `windeployqt`.
- Linux Qt6 + official NAIF CSPICE N0067 with real `de442s.bsp`,
  `naif0012.tls`, `moon_pa_de440_200625.bpc` and `moon_de440_250416.tf`.
- Numeric integration tests check 2024 eclipse/full-moon geometry, distances,
  lunar ME-body frame unit vectors, physically plausible sub-Earth angles,
  evolving libration across eight days, and agreement of frame lighting
  directions with lunar illuminated fraction.
- Actual Mesa/Xvfb Qt/OpenGL renders of Earth, near-full Moon (2024-03-25),
  and near-new Moon (2024-04-08), with NASA observational imagery, provenance
  JSON, and a numeric pixel-brightness comparison ensuring **real rendered
  phase changes**, not just a number shown in the UI.

**Scientific caveats:** Geometric lunar phase is geocentric and neglects
light-time, aberration and observer parallax. Lunar orientation comes from
DE440; Sun/Moon positions from DE442s. The Moon remains a standalone sphere,
not an Earth–Moon orbit overview. Earth's Sun model is still approximate.
The Windows package does not ship CSPICE. A passing Windows build does not
certify the software on every GPU. No FPS claim without instrumentation.

## v0.6.0 Moon render metrics

The release emits `lunar-render-metrics.json` calculated by Pillow from the unmodified CI Moon OpenGL renders. Mean RGB value of the central 56% × 60% viewport crop for a near-full and near-new Moon is reported, with a fail threshold of full > new + 12. This metric is a regression test rather than absolute luminance calibration. v0.6.0 also removes Earth-science wording from Moon mode in the app's GUI.

## v0.6 shared Earth–Moon acceptance

The independent EarthMoonSystem tests enforce real kilometer-to-Earth-radius transforms, true center separation, unchanged real Moon radius under 12x center separation compression, and failure on absent ephemeris. Linux CSPICE tests verify lunar ME→J2000 rotation. OpenGL integration renders authentic two-body scenes at two fixed UTC epochs and in physical vs 12x compressed separation, checking distinct real JPL vectors (not procedural orbit motion) and NASA blue pixels; a JSON file of measured Earth–Moon data is published in Releases. The Earth orientation remains approximate GMST, unlike the DE440 Moon orientation.
