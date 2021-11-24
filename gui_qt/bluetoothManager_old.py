import gatt
import binascii
import numpy as np
import copy
import time
from datetime import datetime, timedelta

AUDIYOUR_CONFIG_SERVICE = '000000ff-0000-1000-8000-00805f9b34fb'
EQUALIZER_GAINS_CHARACTERISTIC = '0000ff01-0000-1000-8000-00805f9b34fb'

manager = gatt.DeviceManager(adapter_name='hci0')

class AudiyourDevice(gatt.Device):
    def __init__(self, mac_address, manager, managed=True):
        super().__init__(mac_address, manager, managed)

        self.eq_gains = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.eq_gains_local = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.manager = manager
        self.eq_gails_characteristic = None

        self._last_call = None
        self._wait = timedelta(microseconds=1)

    def services_resolved(self):
        super().services_resolved()

        eq_config_service = next(
            s for s in self.services
            if s.uuid == AUDIYOUR_CONFIG_SERVICE)

        self.eq_gails_characteristic = next(
            c for c in eq_config_service.characteristics
            if c.uuid == EQUALIZER_GAINS_CHARACTERISTIC)

        self.eq_gails_characteristic.read_value()

    def characteristic_value_updated(self, characteristic, value):
        print(value)
        if characteristic.uuid == EQUALIZER_GAINS_CHARACTERISTIC:
            for i in range(0, 10):
                self.eq_gains[i] = int.from_bytes([value[i]], "little", signed=True)
                self.eq_gains_local[i] = int.from_bytes([value[i]], "little", signed=True)
        self.manager.stop()

    def characteristic_write_value_succeeded(self, characteristic):
        if characteristic.uuid == EQUALIZER_GAINS_CHARACTERISTIC:
            self.eq_gains = copy.deepcopy(self.eq_gains_local)
        self.manager.stop()

    def characteristic_write_value_failed(self, characteristic, error):
        if characteristic.uuid == EQUALIZER_GAINS_CHARACTERISTIC:
            self.eq_gains_local = copy.deepcopy(self.eq_gains)
        self.manager.stop()

    def write_gains(self, new_gains):
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

        print(new_gains_bytes)

        self.eq_gails_characteristic.write_value(new_gains_bytes)


    def write_gain_index(self, index, new_gain):

        self.eq_gains_local[index] = new_gain

        this_call = datetime.now()

        if self._last_call is not None:
            time_since_last_call = this_call - self._last_call
            if time_since_last_call < self._wait:
                return    

        self.write_gains(self.eq_gains_local)


# device = AudiyourDevice(mac_address='7c:87:ce:cc:f0:12', manager=manager)
# device.connect()

# manager.run()

# # device.write_gains([15,-20,-10,0,0,0,0,10,10,10])

# while True: 
#     device.write_gains([15,15,0,0,0,0,0,0,0,15])
#     device.write_gains([15,-20,-10,0,0,0,0,10,10,10])

# # device.write_gains([0,0,0,0,0,0,0,0,0,0])

