import collections
import re
import unittest
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path


ATTACK = Path(__file__).resolve().parents[1]
ARCHIVE = ATTACK / "out" / "attack4-wiring.fzz"


class FritzingWiringTests(unittest.TestCase):
    def test_project_matches_tested_wiring(self):
        wiring = (ATTACK / "WIRING.md").read_text()
        connections = re.findall(r"^\| GP(\d) \| GPIO (\d+)[^|]*\|", wiring, re.M)
        self.assertEqual({("4", "29"), ("2", "27"), ("3", "28"), ("5", "10")},
                         set(connections))

        with zipfile.ZipFile(ARCHIVE) as archive:
            self.assertIsNone(archive.testzip())
            sketch = ET.fromstring(archive.read("attack4-wiring.fz"))
            instances = sketch.findall("./instances/instance")
            by_index = {part.get("modelIndex"): part for part in instances}
            pico = next(part for part in instances if "waveshare-rp2040-zero" in part.get("moduleIdRef", ""))
            safe = next(part for part in instances if part.get("moduleIdRef") == "attack4-safe-contacts")
            resistors = [part for part in instances if part.get("moduleIdRef") == "ResistorModuleID"]
            self.assertEqual(2, len(resistors))
            self.assertEqual({"220"}, {part.find("./property[@name='resistance']").get("value")
                                       for part in resistors})

            schematic = ET.fromstring(archive.read("svg.schematic.waveshare-rp2040-zero-tht_1_schematic.svg"))
            labels = {node.get("id").replace("pin", "connector").replace("label", ""):
                      "".join(node.itertext()).strip()
                      for node in schematic.iter() if re.fullmatch(r"pin\d+label", node.get("id", ""))}
            contact_part = ET.fromstring(archive.read("part.attack4-safe-contacts.fzp"))
            safe_labels = {node.get("id"): node.get("name") for node in contact_part.findall("./connectors/connector")}
            for contact in contact_part.findall("./connectors/connector"):
                for view in ("breadboardView", "schematicView"):
                    marker = contact.find(f"./views/{view}/p")
                    self.assertIsNotNone(marker)
                    self.assertEqual(contact.get("id") + "pin", marker.get("svgId"))
                    if view == "schematicView":
                        self.assertEqual(contact.get("id") + "terminal", marker.get("terminalId"))

            graph = collections.defaultdict(set)
            for resistor in resistors:
                zero = (resistor.get("modelIndex"), "connector0")
                one = (resistor.get("modelIndex"), "connector1")
                graph[zero].add(one)
                graph[one].add(zero)
            wires = [part for part in instances if part.get("moduleIdRef") == "WireModuleID"]
            self.assertEqual(11, len(wires))
            for wire in wires:
                zero = (wire.get("modelIndex"), "connector0")
                one = (wire.get("modelIndex"), "connector1")
                graph[zero].add(one)
                graph[one].add(zero)
                view = wire.find("./views/schematicView")
                ends = view.findall("./connectors/connector")
                self.assertEqual(2, len(ends))
                endpoints = []
                for end in ends:
                    refs = end.findall("./connects/connect")
                    self.assertEqual(1, len(refs))
                    ref = refs[0]
                    self.assertIn(ref.get("modelIndex"), by_index)
                    endpoints.append((ref.get("modelIndex"), ref.get("connectorId")))
                left, right = endpoints
                graph[left].add(right)
                graph[right].add(left)

            def path_to(source, destination, without=frozenset()):
                seen = {source}
                frontier = [source]
                while frontier:
                    vertex = frontier.pop()
                    if vertex == destination:
                        return seen
                    for adjacent in graph[vertex] - seen - without:
                        seen.add(adjacent)
                        frontier.append(adjacent)
                return None

            for gp, gpio in connections:
                start = (pico.get("modelIndex"), next(pin for pin, label in labels.items() if label == gp))
                end = (safe.get("modelIndex"), next(pin for pin, label in safe_labels.items() if "GPIO" + gpio in label))
                network = path_to(start, end)
                self.assertIsNotNone(network)
                present = {part.get("modelIndex") for part in resistors if any(node[0] == part.get("modelIndex") for node in network)}
                self.assertEqual(1 if gp in {"2", "4"} else 0, len(present))
                if present:
                    blocked = {(part, connector) for part in present for connector in ("connector0", "connector1")}
                    self.assertIsNone(path_to(start, end, without=blocked))

            gnd = (pico.get("modelIndex"), next(pin for pin, label in labels.items() if label == "GND"))
            safe_gnd = (safe.get("modelIndex"), next(pin for pin, label in safe_labels.items() if label == "GND"))
            self.assertIsNotNone(path_to(gnd, safe_gnd))
            self.assertNotIn("VBUS", safe_labels.values())
            self.assertNotIn("3V3", safe_labels.values())


if __name__ == "__main__":
    unittest.main()
