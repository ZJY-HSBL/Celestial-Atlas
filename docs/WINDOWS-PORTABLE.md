# Celestial Atlas v0.6.0 — Windows portable preview
# Celestial Atlas v0.6.0 — Windows 便携预览版

1. Extract the **entire** ZIP archive, not just CelestialAtlas.exe.
2. Open the extracted CelestialAtlas folder and launch CelestialAtlas.exe.
3. Choose Earth or Moon Survey from the top scene selector. Click **下载 NASA 地球真实影像** for Blue Marble, or **下载 NASA LRO 全球月面影像** for NASA's 2025 2048x1024 Moon global image mosaic. These are separate verified image datasets. Internet access is needed once for the original image download.
4. Drag with the left mouse button to orbit the camera; use the mouse wheel to zoom. Use the UTC/time-rate controls to inspect Earth's rotation and approximate solar terminator.
5. Use **保存真实渲染截图** (OpenGL viewport) or **保存完整工作台截图** (the actual full desktop application) after downloading NASA imagery to save an unmodified application viewport PNG plus a provenance JSON. If texture download fails or the OpenGL renderer is unavailable, the export will not pretend the placeholder is a real Earth image.

**Requirements:** Windows x64, GPU/driver supporting OpenGL 3.3 core profile, and current Microsoft Visual C++ x64 Redistributable. Qt libraries are bundled. No Python installation is required. Install the official Visual C++ Redistributable if necessary: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

**Scientific limitations:** Earth's surface is an observed NASA Blue Marble multi-date composite; clouds and lighting are not a real-time view. Sun position uses a low-precision approximation (UTC substituted for UT1), **not** JPL SPICE or a high-precision ephemeris. This preview does not yet implement Moon orbits, other planets, Gaia stars or interstellar navigation.

**No installer / 不是安装程序:** This is a portable ZIP assembled with Qt's official windeployqt. It is a preview without formal graphical or hardware certification. If a window fails to display, update the GPU driver and verify OpenGL support.

**Source:** https://github.com/ZJY-HSBL/Celestial-Atlas
**NASA imagery:** https://svs.gsfc.nasa.gov/2915/

**Moon mode on Windows:** This portable build is compiled WITHOUT CSPICE, so Moon mode is a real LROC survey-textured globe under fixed presentation light; it is not a real lunar phase or libration. On the optional Linux CSPICE build, with four official NAIF kernels, the camera and lunar lighting follow authentic geometric sub-Earth and sub-Sun directions. For quantitative lunar science see the separate CSPICE calculations and the NASA reference data notes in docs/MOON-ATLAS.md.

## Earth–Moon spatial scene / 地月共享空间

The optional Linux CSPICE build adds a third Earth–Moon mode driven by JPL DE442s geometric J2000 lunar position and DE440 Moon_ME orientation. It has a real center-separation setting (1:1) and an explicitly labeled presentation mode with only center separation compressed 12x; body radius ratio is conserved. The Windows portable ZIP remains CSPICE-disabled: its shared scene reports that precise orbit data are unavailable and does not render a fictional Moon position. Use the right sidebar to focus Earth/Moon and switch separation. For independent current results, see the JSON metrics attached to the v0.6.0 Release.
