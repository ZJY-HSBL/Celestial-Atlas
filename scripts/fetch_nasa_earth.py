#!/usr/bin/env python3
"""Download the real NASA Blue Marble 2048×1024 PNG and save provenance.
This is an optional offline developer tool; Python is not needed to run the Qt app.
"""
import argparse
import hashlib
import json
import struct
import urllib.request
from pathlib import Path

SOURCE_URL = "https://svs.gsfc.nasa.gov/vis/a000000/a002900/a002915/bluemarble-2048.png"
SOURCE_PAGE = "https://svs.gsfc.nasa.gov/2915/"

def validate_png(data: bytes) -> tuple[int, int]:
    if len(data) < 24 or data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("Not a valid PNG header")
    if data[12:16] != b"IHDR":
        raise ValueError("Missing IHDR")
    dimensions = struct.unpack(">II", data[16:24])
    if dimensions != (2048, 1024):
        raise ValueError(f"Unexpected image dimensions: {dimensions}")
    return dimensions

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--output", type=Path, default=Path("assets/nasa/bluemarble-2048.png"))
    args = p.parse_args()
    req = urllib.request.Request(SOURCE_URL, headers={"User-Agent": "CelestialAtlas/0.1.0"})
    with urllib.request.urlopen(req, timeout=60) as response:
        if response.status != 200:
            raise RuntimeError(f"HTTP {response.status}")
        data = response.read(16 * 1024 * 1024 + 1)
    validate_png(data)
    sha = hashlib.sha256(data).hexdigest()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    tmp = args.output.with_suffix(".png.download")
    tmp.write_bytes(data)
    tmp.replace(args.output)
    info = {
        "source_page": SOURCE_PAGE,
        "source_url": SOURCE_URL,
        "credit": "NASA/Goddard Space Flight Center Scientific Visualization Studio",
        "processing": "Multi-date satellite composite, not a live Earth photograph",
        "dimensions": [2048, 1024],
        "sha256": sha
    }
    args.output.with_suffix(".provenance.json").write_text(
        json.dumps(info, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Downloaded {args.output}: {len(data)} bytes, SHA256 {sha}")

if __name__ == "__main__":
    main()
