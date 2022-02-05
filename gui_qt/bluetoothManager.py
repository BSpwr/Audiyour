import struct
from typing import List
import qasync
from bleak import BleakClient, BleakScanner
import bleak
from datetime import datetime, timedelta


class BluetoothManager:
    def __init__(self):
        self.AUDIYOUR_CONFIG_SERVICE = '000000ff-0000-1000-8000-00805f9b34fb'
        self.EQUALIZER_GAINS_CHARACTERISTIC = '0000ff01-0000-1000-8000-00805f9b34fb'
        self.EQUALIZER_ENABLE_CHARACTERISTIC = '0000ff05-0000-1000-8000-00805f9b34fb'
        self.MIXER_GAINS_CHARACTERISTIC = '0000ff02-0000-1000-8000-00805f9b34fb' 
        self.MIXER_LINE_IN_ENABLE_CHARACTERISTIC = '0000ff03-0000-1000-8000-00805f9b34fb' 
        self.MIXER_WIRELESS_ENABLE_CHARACTERISTIC = '0000ff04-0000-1000-8000-00805f9b34fb'

        self.eq_gains: list[float] = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.eq_enable = False
        self.mix_gains = [0, 0]
        self.mac_address = None

        self.client = None
        self.scanner = BleakScanner()

        self._last_call = None
        self._wait = timedelta(microseconds=1)


    def set_address(self, address):
        print(address)
        self.mac_address = address


    async def connect(self):
        if self.mac_address is None:
            return

        if self.client is None:
            self.client = BleakClient(self.mac_address)
        elif self.client.address != self.mac_address:
            if self.client.is_connected:
                await self.client.disconnect()
            self.client = BleakClient(self.mac_address)

        if not self.client.is_connected:
            await self.client.connect()


    def is_connected(self):
        if self.client is not None:
            return self.client.is_connected
        return False


    async def disconnect(self):
        if self.client is not None and self.client.is_connected:
            await self.client.disconnect()


    async def discover_availiable_devices(self) -> List[bleak.backends.device.BLEDevice]:
        return await self.scanner.discover(timeout=4)


    async def read_eq_gains(self):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        value = await self.client.read_gatt_char(self.EQUALIZER_GAINS_CHARACTERISTIC)
        for i in range(0, 10):
            self.eq_gains[i] = struct.unpack('<f', value[i*4:i*4+4])[0]


    async def read_eq_enable(self):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        value = await self.client.read_gatt_char(self.EQUALIZER_ENABLE_CHARACTERISTIC)
        self.eq_enable = bool(int.from_bytes([value[0]], "little", signed=True))


    async def read_mix_gains(self):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        value = await self.client.read_gatt_char(self.MIXER_GAINS_CHARACTERISTIC)
        for i in range(0, 2):
            self.mix_gains[i] = int.from_bytes([value[i]], "little", signed=True)


    async def read_mix_line_in_en(self):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        value = await self.client.read_gatt_char(self.MIXER_LINE_IN_ENABLE_CHARACTERISTIC)
        self.mix_line_in_en = bool(int.from_bytes([value[0]], "little", signed=True))

    async def read_mix_wireless_in_en(self):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        value = await self.client.read_gatt_char(self.MIXER_WIRELESS_ENABLE_CHARACTERISTIC)
        self.mix_wireless_in_en = bool(int.from_bytes([value[0]], "little", signed=True))


    async def write_eq_gains(self, new_gains):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        self._last_call = this_call

        print(new_gains)

        new_gains_bytes = b''
        for i in new_gains:
            new_gains_bytes = new_gains_bytes + bytearray(struct.pack("<f", i))
            # i.to_bytes(1, "little", signed=True)

        self.eq_gains = new_gains

        k = await self.client.write_gatt_char(self.EQUALIZER_GAINS_CHARACTERISTIC, new_gains_bytes, response=True)
        # self.eq_gails_characteristic.write_value(new_gains_bytes)

    async def write_eq_enable(self, new_status: bool):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        self._last_call = this_call

        print(new_status)

        if new_status:
            new_status = 1
        else:
            new_status = 0

        new_status_bytes = b''
        new_status_bytes = new_status.to_bytes(1, "little", signed=True)

        print(new_status_bytes)

        k = await self.client.write_gatt_char(self.EQUALIZER_ENABLE_CHARACTERISTIC, new_status_bytes, response=True)


    async def write_mix_gains(self, new_gains):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        self._last_call = this_call

        print(new_gains)

        new_gains_bytes = b''
        for i in new_gains:
            new_gains_bytes = new_gains_bytes + i.to_bytes(1, "little", signed=True)

        self.mix_gains = new_gains

        k = await self.client.write_gatt_char(self.MIXER_GAINS_CHARACTERISTIC, new_gains_bytes, response=True)
        # self.eq_gails_characteristic.write_value(new_gains_bytes)

    async def write_mix_line_in_en(self, new_status: bool):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        self._last_call = this_call

        print(new_status)

        if new_status:
            new_status = 1
        else:
            new_status = 0

        new_status_bytes = b''
        new_status_bytes = new_status.to_bytes(1, "little", signed=True)

        k = await self.client.write_gatt_char(self.MIXER_LINE_IN_ENABLE_CHARACTERISTIC, new_status_bytes, response=True)

    
    async def write_mix_wireless_in_en(self, new_status: bool):
        await self.connect()
        if self.client is None or not self.client.is_connected:
            return

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        self._last_call = this_call

        print(new_status)

        if new_status:
            new_status = 1
        else:
            new_status = 0

        new_status_bytes = b''
        new_status_bytes = new_status.to_bytes(1, "little", signed=True)

        k = await self.client.write_gatt_char(self.MIXER_WIRELESS_ENABLE_CHARACTERISTIC, new_status_bytes, response=True)



    async def write_eq_gain_index(self, index, new_gain):
        self.eq_gains[index] = new_gain

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        await self.write_eq_gains(self.eq_gains)


    async def write_mix_gain_index(self, index, new_gain):
        self.mix_gains[index] = new_gain

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return

        await self.write_mix_gains(self.mix_gains)

# async def main(address):
#     async with BleakClient(address) as client:
#         model_number = await client.read_gatt_char(EQUALIZER_GAINS_CHARACTERISTIC)
#         print("Model Number", model_number)

# asyncio.run(main(address))
