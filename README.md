# IR Bruteforcer QE 🦝📡

A Flipper Zero FAP for brute-forcing infrared signals across common device categories. Built for owned hardware research, toy hacking, and IR exploration.


---

## Features

- **Category-based smart scans** — pre-loaded address/command pairs tuned for each device type
- **Full Sweep** — brute forces all 256 addresses × 256 commands (65,536 total)
- **Custom mode** — pick your own protocol, address, and sweep all commands
- **Protocol support** — NEC, Samsung32, RC5, Sony SIRC
- **Recent Hits** — tracks last 5 logged hits, resend any of them instantly
- **SD card logging** — save hits to `/apps_data/ir_bruteforce/ir_hits.txt`
- **About screen** with double border because why not

---

## Menu Options

| Option | Description |
|---|---|
| **Full Sweep (NEC)** | Brute forces all 65,536 addr+cmd combos via NEC |
| **TV / Displays** | Common TV addresses across major brands including Sony-focused |
| **LED Strips** | 44-key and 24-key LED controller address/command combos |
| **Toys** | Generic Chinese IR toy addresses with power-of-2 command patterns |
| **RC Cars / Drones** | Channel-based RC addresses with directional command patterns |
| **Streaming** | Roku, Fire TV, Apple TV, and Android TV box common codes |
| **Custom...** | Pick protocol + address, sweep all 256 commands |
| **Recent Hits** | View, resend, or save last 5 logged signals |
| **About** | Credits |

---

## Controls

### Main Menu
- `UP / DOWN` — navigate
- `OK` — select

### Scan Screen
- `UP / DOWN` — move cursor between protocol selector and Start button
- `OK` on protocol — enter edit mode, cycle with UP/DOWN, OK to confirm
- `OK` on Start — start / pause / resume / restart scan
- `RIGHT` — log current addr+cmd as a hit (green blink = saved, red = failed)
- `BACK` — return to menu

### Custom Config
- `LEFT / RIGHT` — move cursor between Protocol / High Nibble / Low Nibble
- `OK` — enter edit mode for selected field
- `UP / DOWN` in edit — change value
- `OK` to confirm — on low nibble, launches scan

### Recent Hits
- `UP / DOWN` — select hit
- `OK` — resend selected hit
- `RIGHT` — save selected hit to SD card

---

## Protocols

| Protocol | Use Case |
|---|---|
| **NEC** | Most common — Chinese toys, LED strips, generic TVs |
| **Samsung32** | Samsung TVs |
| **RC5** | Older Philips / European devices |
| **Sony SIRC** | Sony TVs, Blu-ray, PlayStation accessories, Roku |

> **Tip:** If nothing responds on NEC, switch to Sony SIRC — especially for Sony TVs and Roku devices.

---

## Building

Requires [ufbt](https://github.com/flipperdevices/flipperzero-ufbt) and Unleashed firmware SDK.

```bash
# Install ufbt
pip install ufbt

# Pull Unleashed SDK
ufbt update --index-url https://up.unleashedflip.com/directory.json

# Build and flash
cd IR_Bruteforcer_QE
ufbt launch
```

---

## File Structure

```
IR_Bruteforcer_QE/
├── application.fam
├── ir_bruteforce.c
├── IR_icon.png
└── images/
    ├── arrow_updown_11x11.png
    ├── btn_ok_11x11.png
    └── btn_right_6x11.png
```

---

## Log File

Hits are saved to:
```
/apps_data/ir_bruteforce/ir_hits.txt
```

Format:
```
proto=NEC addr=0x01 cmd=0x15
proto=SIRC addr=0x01 cmd=0x15
```

---

## Disclaimer

For use on devices you own. IR brute forcing takes time — Full Sweep at 400ms per signal is ~7.3 hours for all 65,536 combinations. Use category scans first.

---

*Subscribe to [@RaccoonFacts](https://youtube.com/@RaccoonFacts) for weird inventions and Arduino projects 🦝*
