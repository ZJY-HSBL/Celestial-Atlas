# Lunar Atlas / NASA LRO 真实月球影像

The Moon rendering in **v0.4.0** is a standalone globe, switchable from the
Earth view in the same desktop viewport. It loads NASA Goddard SVS's updated
2025 **CGI Moon Kit** LROC wide-angle camera color mosaic at **2048×1024**.

Official project: https://svs.gsfc.nasa.gov/4720/

The image is not AI-created. NASA explicitly states that it has been processed
for visualization, white-balanced, and composed from observational bands.
Missing far-polar parts were filled with a lower-resolution LRO/LOLA laser
altimeter albedo mosaic. The dataset is therefore NOT a raw observation for
quantitative photometry and should not be described as one.

## Modes

- **Earth:** observed NASA Blue Marble map, with approximate model sunlight.
- **Moon Survey:** actual NASA LROC global survey surface image. Camera begins
  looking at the near side. **The displayed Moon is a stand-alone map globe
  at unit scene scale, not at its real orbit distance from Earth.**
- Moon Survey uses **fixed studio light**, explicitly unrelated to real phase,
  current Sun/Earth orientation, or precise libration. The numerical phase,
  when a CSPICE-enabled executable has NAIF kernels installed, remains
  computed separately using DE442s Earth/Moon/Sun geometric ephemerides.
- A rigorous visually time-correct Moon requires loading an appropriate
  lunar orientation PCK + lunar frame kernel and converting directions
  from J2000 to the map's lunar body-fixed coordinate frame. These are
  NOT implemented in this milestone.

## Download behavior

The executable downloads the official JPEG on demand and stores it under the
application data directory. It validates dimensions and stores a SHA-256 with
the source's website and dataset processing caveats. Windows portable ZIP
does not bundle the NASA image. The offline developer/CI script is:

```bash
python -m pip install Pillow
python scripts/fetch_nasa_moon.py
```

GitHub Release includes genuine application-rendered Moon screenshot with
provenance, generated using NASA's JPEG and Qt/OpenGL under Linux Mesa.

NASA source credit is required when displaying screenshots in the README.
No lunar terrain displacement is applied in v0.4.0; surface relief is color
texture, not an actual DEM mesh.
