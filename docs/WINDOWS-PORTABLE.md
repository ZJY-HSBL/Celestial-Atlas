# Celestial Atlas v0.3.0 — Windows portable preview
# Celestial Atlas v0.3.0 — Windows 便携预览版

1. Extract the **entire** ZIP archive, not just CelestialAtlas.exe.
2. Open the extracted CelestialAtlas folder and launch CelestialAtlas.exe.
3. Click **下载 NASA 真实影像** to download NASA's 2048x1024 Blue Marble mosaic on demand. Internet access is needed once for the original image download.
4. Drag with the left mouse button to orbit the camera; use the mouse wheel to zoom. Use the UTC/time-rate controls to inspect Earth's rotation and approximate solar terminator.
5. Use **保存真实渲染截图** (OpenGL viewport) or **保存完整工作台截图** (the actual full desktop application) after downloading NASA imagery to save an unmodified application viewport PNG plus a provenance JSON. If texture download fails or the OpenGL renderer is unavailable, the export will not pretend the placeholder is a real Earth image.

**Requirements:** Windows x64, GPU/driver supporting OpenGL 3.3 core profile, and current Microsoft Visual C++ x64 Redistributable. Qt libraries are bundled. No Python installation is required. Install the official Visual C++ Redistributable if necessary: https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist

**Scientific limitations:** Earth's surface is an observed NASA Blue Marble multi-date composite; clouds and lighting are not a real-time view. Sun position uses a low-precision approximation (UTC substituted for UT1), **not** JPL SPICE or a high-precision ephemeris. This preview does not yet implement Moon orbits, other planets, Gaia stars or interstellar navigation.

**No installer / 不是安装程序:** This is a portable ZIP assembled with Qt's official windeployqt. It is a preview without formal graphical or hardware certification. If a window fails to display, update the GPU driver and verify OpenGL support.

**Source:** https://github.com/ZJY-HSBL/Celestial-Atlas
**NASA imagery:** https://svs.gsfc.nasa.gov/2915/
