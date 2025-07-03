# davetetlow-PowerCutMonitor-fw
This is a Power Cutoff Detection Device for refrigerators.

# ESP32 Power Cutoff Detection & Battery Monitoring

This project uses an ESP32-C3 microcontroller to monitor battery voltage and detect power cutoff. When external power is present, the system enters deep sleep to conserve battery and periodically wakes to recheck power if it lost then update on cloud.

---

## 🔋 Battery Monitoring

**Functionality:**

- Continuously reads analog voltage from a battery through the ADC (connected to GPIO4).
- Converts the voltage to battery percentage using a predefined voltage-to-capacity lookup table.
- Uses linear interpolation for accurate percentage estimation between defined voltage points.
- Averages multiple ADC readings to filter out noise and ensure stable output.
- If the battery voltage is 20% it will update on cloud.

**Battery Voltage Range:**

- Expected voltage input: **1.8V to 3.0V**
- Higher or lower values are clamped within this range for consistency.

---

## 🔌 Power Cutoff Detection

**Functionality:**

- Monitors a digital GPIO pin (GPIO5) to detect the presence of external power.
- When **power is available**, the system move to timer base sleep mode.
- When **power is lost**, the system:
  - Sets a timer to **wake periodically** (e.g., every 30 minutes) and check for power restoration and Update on cloud.

**Wake-Up Behavior:**

- The system wakes up after the timer interval .
- Resumes normal battery monitoring and power status checks.

Certainly! Here's an additional section specifically about **Sleep Mode**, formatted consistently with the rest of your documentation:


---




