# Verification / 测试状态

Baseline date: 2026-10-17.

- Reference local Linux environment: GNU C++ 14.2, CMake 3.31, Ninja.
- The original standalone astronomy core built successfully with ATLAS_BUILD_GUI=OFF.
- One CTest suite passed, covering Julian date, approximate GMST, solar declination at equinox/solstice, subsolar and antipodal solar altitude, and angle normalization.
- Qt6 developer headers were not present in that environment: the desktop GUI was **not compiled or launched** there.
- NASA source page was identified but actual image download, image mapping and GPU rendering were **not verified** in that environment.
- No application screenshot or FPS result is claimed.
- GitHub Actions additionally attempts Linux core and Windows Qt builds for committed sources; its status must be reviewed separately.

Before a runnable binary release: successful Windows compile, actual GUI launch, measured rendering results, verified Earth texture orientation, checked daylight terminator across several UTC dates, documentation of downloadable NASA image processing, and provenance metadata.
