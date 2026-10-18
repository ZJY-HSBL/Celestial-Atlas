# Verification / 测试与验证状态

## v0.3.0 planned GitHub Actions checks

- Linux: C++20 astro core builds, solar geometry/equinox/solstice tests, inertial-vs-geographic Earth lighting correspondence.
- Windows: Qt6 + MSVC builds both native GUI and tests, invokes windeployqt, checks qwindows.dll, creates a portable ZIP.
- - **Linux GUI render:** installs Qt6, Xvfb and Mesa; downloads NASA Blue Marble; launches the actual Qt/OpenGL application, forces the sunlit hemisphere into view, links NASA CSPICE N0067 and JPL DE442s, validates eclipse-era Moon phase/distance values and saves real framebuffer and complete desktop captures, validates size/brightness/ocean-blue pixels of both, and uploads PNGs plus scientific metadata. Installs Noto CJK font for readable Chinese labels. This is a CI-gated requirement for v0.3.0. 
- Windows hosted runners do not provide a reliable OpenGL GUI context for this application. Windows compilation and portable packaging are independently checked, but Windows interactive pixels require testing on a real user's GPU.
- A candidate screenshot is only considered evidence after examining the resulting rendered pixels and provenance. CI screenshot generation alone is not a substitute for manual runtime QA.

## Known limits

- No JPL CSPICE in v0.3.0; near-Earth Sun model is approximate, UTC substituted for UT1.
- NASA Blue Marble is an observed multi-date composite, not a live Earth view.
- Rendering of a Windows portable executable on a real desktop GPU has not yet been independently certified.
- No benchmark FPS, validated time-varying cloud fields, or planet-by-planet mission image coverage is claimed.

See README and WINDOWS-PORTABLE.md for use. Scientific sources and license information are kept separate from bundled executable dependencies.

## Explicit model separation

- Precision Sun/Moon J2000 **geometric ephemeris** is computed by CSPICE when enabled and official kernels loaded.
- Earth UV texture lighting **still uses a low-precision solar orientation model**, not DE442s+IERS.
- Windows portable binary is built without official CSPICE until newer MSVC compatibility is validated; missing lunar precision is visible in the app.
