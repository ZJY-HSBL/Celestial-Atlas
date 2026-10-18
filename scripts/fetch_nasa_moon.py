#!/usr/bin/env python3
"""Fetch the real NASA SVS LRO global color mosaic used for the 3D Moon atlas.

NASA SVS CGI Moon Kit, 2025 update (processed LROC WAC color composite,
polar gaps supplemented using LRO LOLA albedo). Visualization resource,
not raw calibrated scientific pixel data.
"""
import argparse
import hashlib
import json
import os
import urllib.request
from pathlib import Path
from PIL import Image

SOURCE_PAGE = "https://svs.gsfc.nasa.gov/4720/"
IMAGE_URL = "https://svs.gsfc.nasa.gov/vis/a000000/a004700/a004720/lroc_color_2k.jpg"

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--output",type=Path,default=Path("assets/nasa/lroc_color_2k.jpg"))
    args=parser.parse_args()
    req=urllib.request.Request(IMAGE_URL,headers={"User-Agent":"CelestialAtlas/0.4.0 NASA survey viewer"})
    with urllib.request.urlopen(req,timeout=90) as stream:
        data=stream.read(16*1024*1024+1)
    if not data.startswith(b"\xff\xd8\xff") or not 10000<len(data)<16*1024*1024:
        raise RuntimeError("NASA resource is not a plausible JPEG")
    import io
    with Image.open(io.BytesIO(data)) as image:
        if image.size!=(2048,1024) or image.format!="JPEG":
            raise RuntimeError(f"Unexpected NASA Moon texture: {image.format}, {image.size}")
        image.verify()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    temp=args.output.with_suffix(".download")
    temp.write_bytes(data)
    os.replace(temp,args.output)
    sha=hashlib.sha256(data).hexdigest()
    info={
      "id":"moon",
      "source_page":SOURCE_PAGE,
      "image_url":IMAGE_URL,
      "credit":"NASA/Goddard Scientific Visualization Studio, Lunar Reconnaissance Orbiter",
      "kind":"Real LROC WAC survey mosaic processed for 3D visualization; polar regions filled using LOLA albedo, not raw image data or real-time Moon",
      "projection":"Global equirectangular, longitude 0 at center",
      "dimensions":[2048,1024],
      "sha256":sha
    }
    args.output.with_suffix(".provenance.json").write_text(json.dumps(info,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(f"NASA Moon LROC observed composite: {len(data)} bytes, SHA-256 {sha}")
if __name__=="__main__":
    main()
