# Verification / 测试与验证状态

## v0.2.0 planned GitHub Actions checks

- Linux: C++20 astro core builds, solar geometry/equinox/solstice tests, inertial-vs-geographic Earth lighting correspondence.
- Windows: Qt6 + MSVC builds both native GUI and tests, invokes windeployqt, checks qwindows.dll, creates a portable ZIP.
- Windows CI **attempts** a genuine OpenGL viewport screenshot using original NASA imagery. A GitHub-hosted runner might not offer a usable OpenGL 3.3 context; screenshot generation is best effort, optional, and never replaced with a fabricated image.
- A candidate screenshot is only considered evidence after examining the resulting rendered pixels and provenance. CI screenshot generation alone is not a substitute for manual runtime QA.

## Known limits

- No JPL CSPICE in v0.2.0; near-Earth Sun model is approximate, UTC substituted for UT1.
- NASA Blue Marble is an observed multi-date composite, not a live Earth view.
- Rendering of a Windows portable executable on a real desktop GPU has not yet been independently certified.
- No benchmark FPS, validated time-varying cloud fields, or planet-by-planet mission image coverage is claimed.

See README and WINDOWS-PORTABLE.md for use. Scientific sources and license information are kept separate from bundled executable dependencies.
