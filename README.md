
## BMW Isetta 300 EV Retro-Gauge Cluster

A custom, round digital gauge cluster for an electrified BMW Isetta 300. The project uses an ESP32 microcontroller to translate 48V Curtis 1268 controller telemetry into a vintage-accurate VDO speedometer display based on the oem gauge

---

## Display Layout Blueprint

The user interface replicates the mid-century aesthetic of original oem gauge while subtly converting its elements to display critical electric vehicle (EV) metrics.

```
            _______________________
          /        [  6 0  ]        \
         /      0              60    \
        /   [0][8][4]%                \
       |  10     \                 50  |
       |          \  (Needle)          |
       |  20       O               40  |
        \                              /
         \      [BATT TEMP: 32°C]     /
          \________  3 0  ___________/
                   \_____/

```

### UI Visual Mapping

* **Dial Background:** Hex color `#F4EFE3` (Aged, vintage cream paper finish).
* **Typography:** Mid-century geometric serif typeface, colored in charcoal black.
* **The Needle:** A custom-rendered white tapered pointer rotating around the absolute center point ($X: 240, Y: 240$ on a $480 \times 480$ display).
* **Speed Scale (0–60 MPH):** Retains the full sweeping arc from the original gauge. Since top speed is $\sim 15\text{ MPH}$, the needle will sweep dynamically between the `0`, `10`, and `20` markers.
* **Odometer Windows $\rightarrow$ Battery State of Charge (SoC):** The rolling mechanical odometer slot right under the top text is repurposed into a digital block displaying the battery percentage (e.g., `[0][8][4]%`).
* **Perimeter Red Markings $\rightarrow$ Thermal Alerts:** The vintage red accent arches on the dial face are static under normal conditions, but flash bright red if motor or battery temperatures exceed safety thresholds.

---

## Project Directory Structure

```text
Isetta_RetroGauge/
├── Isetta_RetroGauge.ino    # Main entry point (Setup & FreeRTOS Core Loops)
├── curtis_serial.h          # Handles inverted TTL data parsing from the 1268
├── curtis_serial.cpp        # Serial implementation & hex-string parsing logic
├── gui_engine.h             # UI initialization, LVGL/TFT_eSPI setup
├── gui_engine.cpp           # Needle rendering, background canvas drawing
└── assets/                  # Graphics and fonts compiled to C arrays
    ├── vdo_background.h     # Raw bitmap array of the cream dial face background
    └── vintage_font.h       # Converted TTF font file to font bitmaps

```

---

## System Architecture & Logic Flow

To ensure the display is completely smooth and does not stutter while driving, the ESP32 utilizes **FreeRTOS** to split tasks across its dual cores.

```
  [ Curtis 1268 Controller ]
             │  (Inverted 9600 Baud Serial)
             ▼
 ┌───────────────────────────────────────┐
 │               CORE 0                  │
 │  • Reads Serial Streams               │
 │  • Handles Inverted Logic Interfere   │  
 │  • Math: Converts RPM to MPH          │
 └───────────┬───────────────────────────┘
             │  (Shared Variables / Safe Thread)
             ▼
 ┌───────────────────────────────────────┐
 │               CORE 1                  │
 │  • Runs LVGL / TFT_eSPI Engine        │
 │  • Updates Needle Position            │
 │  • Animates Odometer SoC Rollers     │
 └───────────┬───────────────────────────┘
             │  (SPI Display Interface)
             ▼
     [ Round LCD Screen ]

```

### 1. Data Processing Task (Core 0)

* Communicates with the Curtis 1268 programming port via an isolated, hardware inverted serial pipeline.
* Extracts raw motor RPM and converts it to current speed using the formula:

$$\text{MPH} = \frac{\text{RPM} \times \text{Tire Circumference}}{\text{Gear Ratio} \times 1056}$$


* Updates core global variables (`live_mph`, `battery_soc`, `motor_temp`).

### 2. Graphics Rendering Task (Core 1)

* Dedicated entirely to drawing pixels via `TFT_eSPI` with DMA (Direct Memory Access).
* Monitors `live_mph` and computes the exact rotation angle for the digital needle sprint.
* Refreshes the screen at a locked $30\text{ FPS}$ for smooth animation.

---

## Hardware Bill of Materials (BOM)

1. **Brain:** ESP32 Development Board (NodeMCU or similar).
2. **Display:** 2.1-inch or 2.8-inch Round IPS LCD Module ($480 \times 480$, SPI Interface, ST7701S or GC9A01 driver chip).
3. **Signal Isolation:** ADUM1201 Magnetic Digital Isolator (Prevents high-voltage ground loops between the Curtis controller and the ESP32).
4. **Signal Inverter:** 74HC14 Hex Inverter (or custom software logic configuration to read the inverted Curtis logic levels).
5. **Power Step-Down:** Isolated 12V-to-5V DC-DC buck converter tapped from the vehicle accessory circuit.
