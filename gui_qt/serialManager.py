import serial
import struct
import math

class SerialManager:
    def __init__(self, port: str = None):
        self.start_byte = b'\xFF'
        self.ser = serial.Serial()
        self.ser.port = port
        self.ser.baudrate = 921600
        self.ser.bytesize = 8
        self.ser.parity = 'N'
        self.ser.timeout = 0.01
        self.ack_byte = b'A'
        
    def set_port(self, port: str):
        is_open = self.ser.isOpen()
        if is_open:
            self.ser.close()
        self.ser.port = port
        if is_open:
            self.ser.open()
        
    def open(self):
        if not self.ser.isOpen():
            self.ser.open()
        
    def close(self):
        if self.ser.isOpen():
            self.ser.close()

    def check_conn(self) -> bool:
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()
            
        # flush the pipe, sync CPU
        for i in range(0, 6):
            self.ser.write(b'\x00')
        
        command = b'\xFF'
        self.ser.write(self.start_byte)
        self.ser.write(command)
        ack = self.ser.read(size=1) # wait for CPU to send ACK
        
        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()
            
        return ack == self.ack_byte
    
    def get_eq_gains(self) -> list:
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()

        command = (30).to_bytes(1, byteorder='little')
        self.ser.write(self.start_byte)
        self.ser.write(command)
        ratios = struct.unpack(f'>{"f" * 10}', self.ser.read(size=4 * 10))
        gains = [round(20 * math.log10(ratio), 1) for ratio in ratios]
        self.ser.read(size=1) # wait for CPU to send ACK

        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()

        return gains
    
    def get_eq_status(self) -> bool:
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()
        
        command = (40).to_bytes(1, byteorder='little')
        self.ser.write(self.start_byte)
        self.ser.write(command)

        status = self.ser.read(size=1)
        self.ser.read(size=1) # wait for CPU to send ACK
            
        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()
            
        return len(status) == 1 and status[0] >= 1
    
    def get_diff_surround_ms(self) -> float:
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()
            
        command = (50).to_bytes(1, byteorder='little')
        self.ser.write(self.start_byte)
        self.ser.write(command)
        [diff_surround_ms] = struct.unpack('>f', self.ser.read(size=4))
        self.ser.read(size=1) # wait for CPU to send ACK
            
        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()
            
        return diff_surround_ms

    def set_eq_band(self, band_num: int, dB: int) -> bool:            
        # exit early if the argument is invalid
        if band_num < 0 or band_num > 9:
            return False
        
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()
        
        command = band_num.to_bytes(1, byteorder='little')
        packed = struct.pack('>f', 10**(dB/20))
        self.ser.write(self.start_byte)
        self.ser.write(command)
        self.ser.write(packed)
        ack = self.ser.read(size=1) # wait for CPU to send ACK
        
        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()
        
        return ack == self.ack_byte

    def set_diff_surround_ms(self, time_ms: float) -> bool:
        # exit early if the argument is invalid
        if time_ms < 0 or time_ms > 50:
            return False
        
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()
        
        command = (10).to_bytes(1, byteorder='little')
        packed = struct.pack('>f', time_ms)
        self.ser.write(self.start_byte)
        self.ser.write(command)
        self.ser.write(packed)
        ack = self.ser.read(size=1) # wait for CPU to send ACK
        
        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()
            
        return ack == self.ack_byte

    def set_eq_en(self, enable: bool) -> bool:
        is_open = self.ser.isOpen()
        if not is_open:
            self.ser.open()
            
        command = (20).to_bytes(1, byteorder='little')
        packed = enable.to_bytes(1, byteorder='little')
        self.ser.write(self.start_byte)
        self.ser.write(command)
        self.ser.write(packed)
        ack = self.ser.read(size=1) # wait for CPU to send ACK
        
        # if port was opened by this function, then close it
        if not is_open:
            self.ser.close()
            
        return ack == self.ack_byte
    
    @staticmethod
    def get_available_ports() -> list:
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]