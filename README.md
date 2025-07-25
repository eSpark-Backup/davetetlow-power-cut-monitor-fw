# davetetlow-PowerCutMonitor-fw
This is a Power Cutoff Detection Device for refrigerators.

## Controller:
ESP32-C3

## GPIO Pin
GPIO 5: Used for power cutoff detection.

ADC_CHANNEL_4: Used for batter percentage detection

## Features:
Wi-Fi/BLE Connectivity: Connects to Wi-Fi or switches to BLE mode.

Battery Monitoring: Measures and calculates battery percentage.

Power Cutoff Detection: Wakes up from sleep on GPIO power cutoff.

Deep Sleep: Enters deep sleep for 30 minutes to conserve power after each cycle. Also awake on power cutoff.

Cloud Reporting: Sends battery data (if less than 20% false else true) and power cutoff detection (power cutoff = true else false) including device mac in JSON format to a server.

## Wifi-Provisioning:

BLE with device uuid ```0x180``` and write characteristic uuid ```0xCDAD``` turns on if wifi credentials are not found in nvs or wifi did not connnected in 10 tries.
Data packet should be sent to provide wifi credentials in below format:
```{"ssid": "Wifi_ssid","password": "Wifi_password"}```
