# NASA / JPL NAIF attribution and distribution notice

Celestial Atlas links to the **NASA/JPL NAIF SPICE Toolkit (CSPICE N0067)** when precision mode is enabled. The official Windows Visual C 64-bit static library is downloaded from:

https://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Windows_VisualC_64bit/packages/cspice.zip

**NAIF SPICE Rules (including terms of use, attribution, redistribution and disclaimers):**
https://naif.jpl.nasa.gov/naif/rules.html

This is a NASA/Caltech/JPL toolkit with conditions separate from Celestial Atlas's MIT license. NAIF's rules permit using Toolkit modules within a distinct SPICE-based application while restricting redistribution of the standalone unmodified Toolkit. Users seeking the original NAIF Toolkit should obtain it directly from the official NASA site.

**Data included in the v0.7.0 Windows portable ZIP:**

- `kernels/naif0012.tls` — NASA NAIF leap-second kernel
- `kernels/de442s.bsp` — JPL DE442s planetary and lunar geometric ephemeris SPK
- `kernels/moon_pa_de440_200625.bpc` — JPL lunar DE440 binary orientation PCK
- `kernels/moon_de440_250416.tf` — NASA NAIF lunar Mean Earth / principal-axis frame definitions

These four kernel files are downloaded from NASA NAIF **without modification** and retain separate `*.provenance.json` records with source URL, file size and SHA-256. NAIF allows use/redistribution of the kernels it distributes as unmodified files under its rules.

NASA/JPL/Caltech neither endorses nor provides support for this third-party application. CSPICE and official data are provided **as-is**, without NASA warranties. Celestial Atlas's own source code and its MIT license do not change the applicable rights/conditions for NASA/NAIF data and libraries.

**Scientific boundaries:** JPL DE442s provides geometric J2000 positions; lunar frame rotation is from DE440 PCK/FK. Earth orientation in the current viewer remains approximate GMST rather than precision UT1/EOP/ITRF. NASA Blue Marble Earth and LROC Moon imagery are processed observations, not live captures or AI-created substitutes.
