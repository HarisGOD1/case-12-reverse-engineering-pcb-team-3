#!/usr/bin/env python3
"""Build the attack4 Fritzing sketch from WIRING.md and the RP2040-Zero part.

The RP2040-Zero part is by vanepp on the Fritzing forum:
https://forum.fritzing.org/t/part-request-waveshare-rp2040-zero/16705
"""

import argparse
import hashlib
import io
import re
import urllib.request
import xml.etree.ElementTree as ET
import zipfile
from collections import defaultdict
from pathlib import Path


ATTACK = Path(__file__).resolve().parents[1]
PART_URL = "https://forum.fritzing.org/uploads/short-url/1WcSwVUPu6SFzwFR2UnXq3FZXqg.fzpz"
PART_SHA256 = "5d850fe40cff71dd5e8d4225c5ed36384bc596f2f29f3d605ff8612cd8243339"
SVG_NS = "http://www.w3.org/2000/svg"
PICO_ID = "waveshare-rp2040-zero-tht_1"
SAFE_ID = "attack4-safe-contacts"
VIEW_LAYERS = {"breadboardView": "breadboard", "schematicView": "schematic"}
PICO_INDEX, SAFE_INDEX, A_INDEX, B_INDEX = "1001", "1002", "1003", "1004"
RESISTOR_IMAGE = {
    "breadboardView": (38.625, 8.739, (0.9, 4.0), (37.73, 4.0)),
    "schematicView": (36.875, 7.74, (0.44, 3.87), (36.44, 3.87)),
}


def wiring():
    text = (ATTACK / "WIRING.md").read_text()
    rows = re.findall(r"^\| GP([2-5]) \| GPIO (\d+)(?:, (A|B|SW))? \|", text, re.M)
    pins = {int(gp): (gpio, signal or "SENSE") for gp, gpio, signal in rows}
    if len(pins) != 4 or not re.search(r"^\| GND \| GND \|", text, re.M):
        raise ValueError("WIRING.md must define GP2–GP5 and common ground")
    if any(not re.search(r"^\| GP%s \|[^\n]*150–330 Ом" % gp, text, re.M) for gp in (2, 4)):
        raise ValueError("The A and B lines must include the tested series resistor range")
    return pins


def load_pico_part(path):
    raw = Path(path).read_bytes() if path else urllib.request.urlopen(PART_URL, timeout=30).read()
    if PART_SHA256 and hashlib.sha256(raw).hexdigest() != PART_SHA256:
        raise ValueError("The upstream Fritzing part has changed")
    with zipfile.ZipFile(io.BytesIO(raw)) as part:
        assets = {name: part.read(name) for name in part.namelist()}
    descriptor = ET.fromstring(assets[f"part.{PICO_ID}.fzp"])
    schematic = ET.fromstring(assets[f"svg.schematic.{PICO_ID}_schematic.svg"])
    pin_ids = {}
    for node in schematic.iter():
        match = re.fullmatch(r"pin(\d+)label", node.get("id", ""))
        if match:
            pin_ids["".join(node.itertext()).strip()] = "connector" + match.group(1)
    if descriptor.get("moduleId") != PICO_ID or any(p not in pin_ids for p in ("2", "3", "4", "5", "GND")):
        raise ValueError("The RP2040-Zero part lacks a required labelled pin")
    return assets, pin_ids


def safe_part(pins):
    labels = [f"B GPIO{pins[2][0]}", f"SW GPIO{pins[3][0]}",
              f"A GPIO{pins[4][0]}", f"SENSE GPIO{pins[5][0]}", "GND"]
    root = ET.Element("module", {"moduleId": SAFE_ID, "fritzingVersion": "1.0.6"})
    for key, value in (("version", "1"), ("title", "Safe GPIO contacts"), ("label", "SAFE"),
                       ("description", "Known connection points, not a reconstructed safe PCB")):
        ET.SubElement(root, key).text = value
    views = ET.SubElement(root, "views")
    for view, layer in VIEW_LAYERS.items():
        layers = ET.SubElement(ET.SubElement(views, view), "layers", {"image": f"{layer}/{SAFE_ID}.svg"})
        ET.SubElement(layers, "layer", {"layerId": layer})
    connectors = ET.SubElement(root, "connectors")
    for number, name in enumerate(labels):
        connector = ET.SubElement(connectors, "connector", {"id": f"connector{number}", "name": name, "type": "female"})
        connector_views = ET.SubElement(connector, "views")
        for view, layer in VIEW_LAYERS.items():
            attrs = {"layer": layer, "svgId": f"connector{number}pin"}
            if view == "schematicView":
                attrs["terminalId"] = f"connector{number}terminal"
            ET.SubElement(ET.SubElement(connector_views, view), "p", attrs)
    assets = {f"part.{SAFE_ID}.fzp": ET.tostring(root, encoding="utf-8", xml_declaration=True)}
    ET.register_namespace("", SVG_NS)
    for view, layer in VIEW_LAYERS.items():
        svg = ET.Element(f"{{{SVG_NS}}}svg", {"width": "2in", "height": "1.5in", "viewBox": "0 0 200 150"})
        drawing = ET.SubElement(svg, f"{{{SVG_NS}}}g", {"id": layer})
        ET.SubElement(drawing, f"{{{SVG_NS}}}rect", {"x": "16", "y": "14", "width": "176", "height": "122",
                                                 "rx": "9", "fill": "#eef4fa", "stroke": "#31546f", "stroke-width": "2"})
        for y, size, content in ((32, 13, "SAFE GPIO TAP"), (126, 9, "Encoder remains connected")):
            ET.SubElement(drawing, f"{{{SVG_NS}}}text", {"x": "26", "y": str(y), "font-size": str(size),
                                                       "font-family": "sans-serif", "fill": "#17344b"}).text = content
        for number, (name, y) in enumerate(zip(labels, (40, 50, 60, 70, 110))):
            ET.SubElement(drawing, f"{{{SVG_NS}}}line", {"id": f"connector{number}pin", "x1": "0", "x2": "16",
                                                       "y1": str(y), "y2": str(y), "stroke": "#536370", "stroke-width": "2"})
            if layer == "schematic":
                ET.SubElement(drawing, f"{{{SVG_NS}}}rect", {"id": f"connector{number}terminal", "x": "0",
                                                           "y": str(y - 1), "width": "2", "height": "2", "fill": "none"})
            ET.SubElement(drawing, f"{{{SVG_NS}}}text", {"x": "26", "y": str(y + 4), "font-size": "11",
                                                       "font-family": "sans-serif", "fill": "#17344b"}).text = name
        assets[f"svg.{layer}.{SAFE_ID}.svg"] = ET.tostring(svg, encoding="utf-8", xml_declaration=True)
    return assets


def pico_point(assets, connector, view, origin):
    layer = VIEW_LAYERS[view]
    suffix = "terminal" if layer == "schematic" else "pin"
    svg = ET.fromstring(assets[f"svg.{layer}.{PICO_ID}_{layer}.svg"])
    width = float(svg.get("width").removesuffix("in")) * 90
    x0, y0, vw, _ = map(float, svg.get("viewBox").split())
    marker = next(node for node in svg.iter() if node.get("id") == connector + suffix)
    if marker.tag.endswith("circle"):
        x, y = float(marker.get("cx")), float(marker.get("cy"))
    else:
        x = float(marker.get("x")) + float(marker.get("width", "0")) / 2
        y = float(marker.get("y")) + float(marker.get("height", "0")) / 2
    scale = width / vw
    return (origin[0] + (x - x0) * scale, origin[1] + (y - y0) * scale)


def sketch(assets, pin_ids, pins):
    root = ET.Element("module", {"fritzingVersion": "1.0.6", "icon": ".png"})
    views = ET.SubElement(root, "views")
    for view in (*VIEW_LAYERS, "pcbView"):
        ET.SubElement(views, "view", {"name": view, "backgroundColor": "#ffffff",
                                            "gridSize": "0.1in", "showGrid": "1", "alignToGrid": "0"})
    instances = ET.SubElement(root, "instances")
    origins = {
        "schematicView": {PICO_INDEX: (100, 100), SAFE_INDEX: (500, 100), A_INDEX: (330, 150), B_INDEX: (330, 132)},
        "breadboardView": {PICO_INDEX: (100, 100), SAFE_INDEX: (420, 100), A_INDEX: (280, 143), B_INDEX: (280, 125)},
    }
    parts = {
        PICO_INDEX: (PICO_ID, "RP2040-Zero", f"contrib/{PICO_ID}.fzp"),
        SAFE_INDEX: (SAFE_ID, "Safe contacts", f"contrib/{SAFE_ID}.fzp"),
        A_INDEX: ("ResistorModuleID", "R_A", ":/resources/parts/core/resistor.fzp"),
        B_INDEX: ("ResistorModuleID", "R_B", ":/resources/parts/core/resistor.fzp"),
    }
    entries = {}
    incoming = defaultdict(list)
    for index, (module, title, path) in parts.items():
        inst = ET.SubElement(instances, "instance", {"moduleIdRef": module, "modelIndex": index, "path": path})
        if module == "ResistorModuleID":
            ET.SubElement(inst, "property", {"name": "resistance", "value": "220"})
        ET.SubElement(inst, "title").text = title
        part_views = ET.SubElement(inst, "views")
        for view, layer in VIEW_LAYERS.items():
            node = ET.SubElement(part_views, view, {"layer": layer})
            pos = origins[view][index]
            ET.SubElement(node, "geometry", {"x": str(pos[0]), "y": str(pos[1]), "z": "2.5"})
            ET.SubElement(node, "connectors")
            entries[index, view] = node

    routes = []
    colors = {2: "#2073b9", 3: "#1c965a", 4: "#ef8e29", 5: "#9e55b0"}
    safe_pin = {2: "connector0", 3: "connector1", 4: "connector2", 5: "connector3"}
    for gp in (2, 3, 4, 5):
        first = (PICO_INDEX, pin_ids[str(gp)])
        last = (SAFE_INDEX, safe_pin[gp])
        if gp in (2, 4):
            resistor = B_INDEX if gp == 2 else A_INDEX
            routes.extend(((first, (resistor, "connector0"), colors[gp]),
                           ((resistor, "connector1"), last, colors[gp])))
        else:
            routes.append((first, last, colors[gp]))
    for segment in range(5):
        left = (PICO_INDEX, pin_ids["GND"]) if segment == 0 else (str(2006 + segment), "connector1")
        right = (SAFE_INDEX, "connector4") if segment == 4 else (str(2008 + segment), "connector0")
        routes.append((left, right, "#505968"))

    for number, (left, right, color) in enumerate(routes, 1):
        index = str(2000 + number)
        wire = ET.SubElement(instances, "instance", {"moduleIdRef": "WireModuleID", "modelIndex": index,
                                                      "path": ":/resources/parts/core/wire.fzp"})
        ET.SubElement(wire, "title").text = f"Wire{number}"
        wire_views = ET.SubElement(wire, "views")
        for view, layer in VIEW_LAYERS.items():
            def position(endpoint):
                component, connector = endpoint
                origin = origins[view][component]
                if component == PICO_INDEX:
                    return pico_point(assets, connector, view, origin)
                if component == SAFE_INDEX:
                    row = int(connector.removeprefix("connector"))
                    return (origin[0], origin[1] + (40, 50, 60, 70, 110)[row] * 0.9)
                _, _, c0, c1 = RESISTOR_IMAGE[view]
                point = c0 if connector == "connector0" else c1
                return (origin[0] + point[0], origin[1] + point[1])

            if number <= 6:
                start, end = position(left), position(right)
            else:
                safe_ground = position((SAFE_INDEX, "connector4"))
                pico_ground = position((PICO_INDEX, pin_ids["GND"]))
                left_route = 62 if view == "schematicView" else 65
                right_route = 458 if view == "schematicView" else 375
                floor = 239 if view == "schematicView" else 235
                points = (pico_ground, (left_route, pico_ground[1]), (left_route, floor),
                          (right_route, floor), (right_route, safe_ground[1]), safe_ground)
                start, end = points[number - 7:number - 5]
            wire_layer = "breadboardWire" if view == "breadboardView" else "schematicTrace"
            wire_view = ET.SubElement(wire_views, view, {"layer": wire_layer})
            ET.SubElement(wire_view, "geometry", {"x": str(start[0]), "y": str(start[1]),
                                                       "x1": "0", "y1": "0", "x2": str(end[0] - start[0]),
                                                       "y2": str(end[1] - start[1]), "z": "4.5",
                                                       "wireFlags": "128" if number > 6 else "64"})
            ET.SubElement(wire_view, "wireExtras", {"mils": "22.2222", "color": color,
                                                        "opacity": "1", "banded": "0"})
            connectors = ET.SubElement(wire_view, "connectors")
            for slot, endpoint in enumerate((left, right)):
                port = ET.SubElement(connectors, "connector", {"connectorId": f"connector{slot}", "layer": wire_layer})
                ET.SubElement(port, "geometry", {"x": "0", "y": "0"})
                ET.SubElement(ET.SubElement(port, "connects"), "connect", {
                    "modelIndex": endpoint[0], "connectorId": endpoint[1],
                    "layer": VIEW_LAYERS[view] if endpoint[0] in parts else wire_layer})
                if endpoint[0] in parts:
                    incoming[endpoint[0], endpoint[1], view].append((index, slot, wire_layer))

    for (index, connector, view), wire_refs in incoming.items():
        ports = entries[index, view].find("connectors")
        port = ET.SubElement(ports, "connector", {"connectorId": connector, "layer": VIEW_LAYERS[view]})
        ET.SubElement(port, "geometry", {"x": "0", "y": "0"})
        connects = ET.SubElement(port, "connects")
        for wire_index, slot, wire_layer in wire_refs:
            ET.SubElement(connects, "connect", {"connectorId": f"connector{slot}",
                                                   "modelIndex": wire_index, "layer": wire_layer})
    return ET.tostring(root, encoding="utf-8", xml_declaration=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--part", type=Path, help="Use a local copy of the linked RP2040-Zero Fritzing part")
    parser.add_argument("--output", type=Path, default=ATTACK / "out" / "attack4-wiring.fzz")
    args = parser.parse_args()
    pins = wiring()
    assets, pin_ids = load_pico_part(args.part)
    assets.update(safe_part(pins))
    assets["attack4-wiring.fz"] = sketch(assets, pin_ids, pins)
    with io.BytesIO() as buffer:
        with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as output:
            for name, data in sorted(assets.items()):
                entry = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
                entry.compress_type = zipfile.ZIP_DEFLATED
                output.writestr(entry, data)
        content = buffer.getvalue()
    if args.output.exists():
        if args.output.read_bytes() != content:
            raise FileExistsError(f"Output differs and was not replaced: {args.output}")
    else:
        with args.output.open("xb") as output:
            output.write(content)
    print(f"Saved: {args.output} ({len(content)} bytes, GP2–GP5 and GND connected)")


if __name__ == "__main__":
    main()
