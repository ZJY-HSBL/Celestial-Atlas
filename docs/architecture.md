# Architecture / 技术架构

- **astronomy**: dependency-free C++20 numerical core. The implemented solar model is an approximation, not a precision ephemeris.
- **render**: Qt QOpenGLWidget owns 3D Earth mesh, camera, buffers, GLSL shaders, texture. OpenGL resources are modified in the owning GUI context only.
- **app**: Qt desktop workspace, simulated UTC clock, target panel and asynchronous NASA satellite-image downloader.
- **scripts**: Python used offline for provenance-aware downloads and validation; not required to launch the native executable.

## Frames and models / 坐标约定

- Mesh: +X = longitude 0 / latitude 0, +Z = longitude +90° east, +Y = north.
- Visual equatorial inertial frame: +X = right ascension 0, +Z = RA 90°, +Y = north celestial pole.
- Earth: model is rotated about +Y by negative *approximate* GMST.
- Sun direction: (cos δ cos α, sin δ, cos δ sin α). Its RA and Dec are computed by the low-precision solar algorithm.
- NASA image: equirectangular global composite, longitudinal U mapping and vertical flip when sent to OpenGL.
- v0.1 depicts Earth at unit radius, not a rigorously scaled heliocentric solar system.

## Planned interfaces

Introduce UniverseState (time/observer/camera/selection), EphemerisProvider (CSPICE), CatalogService (Gaia/SIMBAD), AstronomicalImageService (HiPS/FITS and planetary imagery), and camera-relative coordinate management only when the corresponding work starts. Avoid placeholder classes or compatibility layers.

## Limitations

No Moon orbit, high-precision JPL ephemeris, Gaia rendering or distant-galaxy 3D reconstructions yet. The atmosphere and shading are visual approximations. Large imagery and mission kernels remain outside Git history. Intended initial target is Windows/Qt6/OpenGL 3.3.
