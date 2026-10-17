# Celestial Atlas / 天穹图鉴

**A science-first desktop astronomy visualization prototype.**  
**以真实天文资料为基础的原生桌面宇宙探索系统。**

> Version: 0.1.0 (source prototype). Built around C++20, Qt6 Widgets and OpenGL 3.3. The current version visualizes Earth's rotation and approximate solar illumination. It is **not** a complete star atlas, JPL-accurate ephemeris engine or interstellar flight simulator yet.

## Scope / 当前功能

- Interactive 3D Earth sphere; drag to rotate camera, scroll to zoom.
- UTC time selection, pause, realtime, 60× and 3600× simulation.
- A documented **low-precision** solar direction and day/night model, suitable for an initial visualization but not high-precision astronomical claims.
- On-demand NASA Blue Marble satellite composite downloaded from the original source, checked for dimensions and recorded with SHA-256 provenance.
- Chinese/English UI labels and a clear separation between implemented and planned features.
- Independent C++ astronomy-core tests that can compile without Qt6.

**Scientific integrity / 科学真实性：** Observed imagery is different from real-time imagery. NASA Blue Marble is a multi-date satellite mosaic. There is no fake exoplanet surface or generated planetary texture. Bodies without measured surface imagery must show the real observations or "data unavailable".

## Build / 构建

Requires CMake 3.21+, C++20, Qt 6.4+ (Core, Widgets, Gui, Network, OpenGLWidgets), and an OpenGL 3.3 core-compatible GPU and driver.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

For the independently testable astronomy core without a Qt6 installation:

```bash
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DATLAS_BUILD_GUI=OFF
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

The Earth image is not included in Git history. In the GUI, choose “下载 NASA 真实影像” to fetch it. Alternatively:

```bash
python scripts/fetch_nasa_earth.py
```

Python is a **developer utility only**, not a runtime requirement for the C++ desktop executable. The asset URL is NASA SVS: https://svs.gsfc.nasa.gov/2915/.

## Verification / 验证状态

- The C++ astronomy core was compiled on Linux and one CTest suite passed on 2026-10-17.
- Qt6 GUI and GPU rendering **were not compiled or launched** in that environment (Qt6 SDK missing).
- NASA image download and real image mapping **were not verified** there due network restrictions.
- No genuine application screenshot, performance benchmark, JPL SPICE results or prebuilt binary is available yet.

See [scientific integrity](docs/scientific-integrity.md), [verification](docs/verification.md), and [architecture](docs/architecture.md).

## Roadmap / 路线

1. Verify the Qt/Windows GUI with real NASA imagery and capture unmodified application screenshots.
2. Replace the solar approximation with NASA NAIF CSPICE and test against JPL Horizons.
3. Add geocentric ground-observer views, Moon motion/phases, and planetary imagery with provenance.
4. Add measured star catalogs, catalog-aware 3D navigation and scalable spatial indexing.
5. Explore FITS/HiPS surveys, exoplanet archives and accurate time/Earth orientation data.

## Resources / 参考

- NASA Blue Marble: https://svs.gsfc.nasa.gov/2915/
- NASA NAIF SPICE: https://naif.jpl.nasa.gov/
- ESA Gaia Archive: https://gea.esac.esa.int/archive/
- SIMBAD: https://simbad.cds.unistra.fr/simbad/
- NASA Exoplanet Archive: https://exoplanetarchive.ipac.caltech.edu/

License: MIT for original source code; third-party astronomical image/data usage is governed separately by the applicable source policies.
