# ESP32 Environmental Data Logger

Firmware for a Denky32 (ESP32) that reads two environmental sensors, reports the readings over BLE advertising, and lets a Python gateway forward them to an HTTP API.

## System Overview

| Sensor | Interface | Measurements |
| --- | --- | --- |
| TBSMP02 | SDI-12 | Humidity and temperature |
| SEN0603 | RS-485 Modbus RTU | Electrical conductivity (EC) and pH |

The ESP32 reads both sensors automatically, approximately every four seconds. It prints the readings and publishes a BLE manufacturer-data advertisement only when both reads succeed. The gateway filters advertisements by BLE MAC address and posts changed packets to the configured server endpoint.

```text
TBSMP02 -- SDI-12 --+
                    +--> Denky32 / ESP32 -- BLE advertising --> gateway.py -- HTTP --> API
SEN0603 -- RS-485 -+
```

## Hardware Connections

The pin assignments below are defined in the source. Connect the sensors through suitable SDI-12 and RS-485 transceivers; do not connect an RS-485 differential bus directly to ESP32 GPIO pins.

| Bus signal | ESP32 GPIO | Source |
| --- | ---: | --- |
| SDI-12 TX | 22 | `src/sdi12_sensor.cpp` |
| SDI-12 RX | 23 | `src/sdi12_sensor.cpp` |
| SDI-12 TX enable | 21 | `src/sdi12_sensor.cpp` |
| SDI-12 break control | 19 | `src/sdi12_sensor.cpp` |
| RS-485 UART RX | 4 | `src/rs485_sensor.cpp` |
| RS-485 UART TX | 2 | `src/rs485_sensor.cpp` |
| RS-485 direction control (RTS) | 13 | `src/rs485_sensor.cpp` |

Current serial settings:

- SDI-12: 1200 baud, 7E1, sensor address `0`.
- RS-485: 9600 baud, 8N1, Modbus device address `1`.
- Modbus: reads four holding registers starting at `0x0000`; EC and pH are taken from response registers 2 and 3.

Check sensor and transceiver documentation before wiring power, ground, or bus lines. The firmware pin assignments do not define the power-supply requirements.

## Firmware Setup

Install PlatformIO Core or use the PlatformIO extension in VS Code. The project configuration selects the `denky32` board and Arduino framework; `ModbusMaster` is declared as a project dependency.

From the project root:

```sh
pio run
pio run -t upload
pio device monitor
```

The serial monitor runs at `115200` baud. At startup, the firmware prints the board's BLE address. Add that address in lowercase to `MY_DEVICES` in `src/gateway.py` so the gateway accepts advertisements from the board.

## BLE Data Format

The firmware advertises manufacturer data with company identifier `0xFFFF`. The 20-byte payload is little-endian and matches the Python format `<BBBBHHhHhHhH` in `src/gateway.py`.

| Payload values | Encoding | Scale | Current source |
| --- | --- | --- | --- |
| Four sensor counters | `uint8` each | 1 | Reserved; currently zero |
| EC | `uint16` | value / 1000 | SEN0603 |
| pH | `uint16` | value / 10 | SEN0603 |
| Temperature | `int16` | value / 100 | TBSMP02, mapped to `air_temp` by the gateway |
| Humidity | `uint16` | value / 100 | TBSMP02, mapped to `air_hum` by the gateway |
| Soil 1 and soil 2 temperature/humidity | `int16` / `uint16` | value / 100 | Reserved; currently zero |

Reserved fields are placeholders and should not be interpreted as valid measurements. The current firmware broadcasts only after successful readings from both sensors.

## Python Gateway

The gateway requires Python 3 and the `bleak` and `httpx` packages:

```sh
python -m pip install bleak httpx
python src/gateway.py
```

Before running it, configure these values near the top of `src/gateway.py`:

- `MY_DEVICES`: lowercase BLE MAC addresses to accept.
- `SERVER_URL`: HTTP endpoint that accepts the gateway's JSON payload.

The scanner currently passes BlueZ-specific options, so run it on a Linux host using BlueZ. The gateway sends a POST when an accepted device's manufacturer payload changes. Its JSON contains the device MAC, four counters, and the decoded measurements.

## Source Layout

| File | Responsibility |
| --- | --- |
| `src/main.cpp` | Initializes both sensor interfaces and coordinates periodic readings and BLE publishing. |
| `src/sdi12_sensor.cpp` | TBSMP02 SDI-12 communication and humidity/temperature parsing. |
| `src/rs485_sensor.cpp` | SEN0603 Modbus RTU communication and EC/pH parsing. |
| `src/network.cpp` | BLE manufacturer-data encoding and advertising. |
| `src/gateway.py` | BLE scanning, payload decoding, MAC filtering, and HTTP forwarding. |

## Troubleshooting

- **No data reaches the server:** confirm the ESP32's printed BLE address is in `MY_DEVICES`, the gateway is running, and `SERVER_URL` is reachable from the gateway host.
- **A sensor is unavailable:** check its wiring, transceiver, power, configured address, and serial settings. BLE publishing is skipped if either sensor read fails.
- **Measurements appear as zero:** verify that the sensor register mapping and scaling match the connected sensor. Reserved counter and soil fields are expected to remain zero with the current firmware.