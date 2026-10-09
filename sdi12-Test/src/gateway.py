import asyncio
import struct

import httpx
from bleak import BleakScanner

# Configuration
SERVER_URL = "http://10.12.1.191:5000/api/sensors-data/addData"
MY_DEVICES = [
    "f4:65:0b:c1:64:1e",
    "f4:65:0b:b5:6f:96",
    "f4:65:0b:c1:63:56",
]

# Dernier paquet reçu par adresse MAC, pour ignorer uniquement les annonces identiques.
last_payloads = {}


async def send_to_server(json_data):
    try:
        async with httpx.AsyncClient() as client:
            response = await client.post(SERVER_URL, json=json_data, timeout=5.0)
            print(f"[{json_data['device']}] Server Response: {response.status_code}")
    except Exception as error:
        print(f"Erreur lors de l'envoi POST: {error}")


def detection_callback(device, advertising_data):
    if device.address.lower() not in MY_DEVICES:
        return

    mfg_data = advertising_data.manufacturer_data.get(0xFFFF)
    if not mfg_data or len(mfg_data) < 20:
        print(f"Données fabricant absentes ou trop courtes : {mfg_data}")
        return

    # B = uint8, H = uint16, h = int16; little-endian 20-byte payload.
    data = struct.unpack("<BBBBHHhHhHhH", mfg_data)
    counters = data[0:4]

    if mfg_data == last_payloads.get(device.address):
        return

    last_payloads[device.address] = mfg_data
    payload = {
        "device": device.address,
        "counters": {
            "sen0603": counters[0],
            "sht3x": counters[1],
            "soil1": counters[2],
            "soil2": counters[3],
        },
        "measurements": {
            "ec": data[4] / 1000.0,
            "ph": data[5] / 10.0,
            "air_temp": data[6] / 100.0,
            "air_hum": data[7] / 100.0,
            "soil1_temp": data[8] / 100.0,
            "soil1_hum": data[9] / 100.0,
            "soil2_temp": data[10] / 100.0,
            "soil2_hum": data[11] / 100.0,
        },
    }

    print(f"\nNEW DATA {payload}")
    asyncio.create_task(send_to_server(payload))


async def main():
    print("Démarrage du scanner Gateway...")
    scanner = BleakScanner(
        detection_callback,
        scanning_mode="active",
        bluez={"interval": 1000, "window": 500},
    )
    await scanner.start()
    while True:
        await asyncio.sleep(1.0)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Arrêt de la Gateway.")

