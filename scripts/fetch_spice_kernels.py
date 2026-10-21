#!/usr/bin/env python3
"""Fetch official NASA NAIF JPL DE442s and leap-second kernels.
Optional offline maintenance script. Do not treat these files as bundled
source, a commercial image license, or a live science feed.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
from urllib.request import Request, urlopen

KERNELS = {
    "naif0012.tls": "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/lsk/naif0012.tls",
    "de442s.bsp": "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de442s.bsp",
    "moon_pa_de440_200625.bpc": "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/pck/moon_pa_de440_200625.bpc",
    "moon_de440_250416.tf": "https://naif.jpl.nasa.gov/pub/naif/generic_kernels/fk/satellites/moon_de440_250416.tf",
}
def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=Path("data/kernels"))
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    for filename, url in KERNELS.items():
        print(f"Requesting {filename} from NASA NAIF...")
        req = Request(url, headers={"User-Agent": "CelestialAtlas/0.7.0 scientific research"})
        with urlopen(req, timeout=180) as response:
            blob = response.read(80 * 1024 * 1024 + 1)
        if filename.endswith(".tls") or filename.endswith(".tf"):
            prefix = b"KPL/LSK" if filename.endswith(".tls") else b"KPL/FK"
            if not blob.startswith(prefix) or len(blob) < 1000:
                raise RuntimeError(f"Unexpected NAIF text kernel: {filename}")
        else:
            prefix = b"DAF/SPK" if filename.endswith(".bsp") else b"DAF/PCK"
            if len(blob) < 1_000_000 or blob[:7] != prefix:
                raise RuntimeError(f"Unexpected NAIF binary kernel: {filename}")
        if len(blob) > 80 * 1024 * 1024:
            raise RuntimeError(f"Unexpected kernel size for {filename}")
        digest = hashlib.sha256(blob).hexdigest()
        dest = args.output / filename
        temp = dest.with_name(dest.name + ".download")
        temp.write_bytes(blob)
        os.replace(temp, dest)
        provenance = {
            "source": url, "source_organization": "NASA/JPL NAIF",
            "kind": filename.rsplit(".",1)[-1].upper(),
            "sha256": digest, "bytes": len(blob),
            "retrieved_name": filename,
            "usage_note": "Unmodified official kernel. The Windows portable app may include a copy with attribution under NASA NAIF rules."
        }
        (args.output / f"{filename}.provenance.json").write_text(
            json.dumps(provenance, indent=2) + "\n", encoding="utf-8")
        print(f"{filename}: {len(blob)} bytes; SHA256 {digest}")
if __name__ == "__main__":
    main()
