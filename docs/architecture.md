# Architecture / 技术架构

- **astronomy**: C++20 numerical core, including geometry-only Moon phase tests and optional official CSPICE SPK/LSK adapter. The implemented solar model is an approximation, not a precision ephemeris.
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

## v0.4.0 two-globe rendering

The same OpenGL sphere and GLSL shader pipeline are reused for Earth and the standalone Moon Survey globe, with separate GPU textures and file caches. The Moon map is a real NASA SVS/LRO processed global survey map with a fixed nonphysical studio light. The program does NOT yet draw a spatial Earth–Moon orbital system or high-accuracy lunar body-fixed libration transforms. The numerical CSPICE panel remains a separate scientifically grounded calculation where built and loaded.

## v0.5 lunar frame layer

The NASA NAIF CSPICE adapter is now optionally provisioned with lunar DE440
binary PCK and matching ME frame kernel. `LunarBodyFrame` holds the Moon-fixed
directions to the Earth and Sun; its construction uses NAIF kernel-frame
transformations, not hand-coded epicycles. The Qt viewport consumes this frame
to aim the camera toward the Earth and shade a real LROC Moon globe from the
correct geometric Sun direction. Motion is updated from the shared simulated
UTC. Without kernels, the viewer remains a survey globe and explicitly says so.
A true shared-space Earth–Moon orbital renderer is future work.

## v0.6 shared world coordinates

The common renderer uses an Earth-centered J2000 inertial scene, with x/y/z Qt coordinate permutation (x,z,y) for the NAIF axes. Earth mean radius 6371 km defines one render unit. Moon center and Sun directions originate from the same JPL DE442s epoch, while Moon sphere attitude is transformed from the NASA DE440 Moon_ME frame using pxform_c. The Moon mesh radius is fixed at 1737.4/6371 Earth-radius units. Only the Earth–Moon center vector is optionally divided by twelve for presentation; truthful 1:1 coordinates are always available. The camera can focus either body. Missing CSPICE fails closed, so no invented orbital path is shown.

## Windows portable NAIF resolution v0.7

The source supports `ATLAS_ENABLE_CSPICE=ON` on both Linux and Windows. On Windows, CI consumes NAIF's official N0067 Visual C 64-bit `cspice.lib` and packages original JPL ephemeris kernels under `<portable-root>/kernels/`. `MainWindow` chooses the explicit `ATLAS_SPICE_KERNEL_DIR` environment setting first, otherwise finds packaged official kernels relative to the executable, and finally checks the user's application-data kernels folder. The executable `--spice-self-test` validates a real 2024 eclipse-epoch JPL lunar state and MOON_ME frame orientation without creating a 3D window, allowing Windows deployment to be validated before Release publication.
