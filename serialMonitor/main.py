import serial
import struct
from collections import deque
from dataclasses import dataclass 

MAX_PAYLOAD = 256

PID_CONTROL = 1
PID_IMU_DATA = 2
PID_FAST = 3
PID_MSG = 4

def handle_msg(data,header):
    print(data.decode("utf-8"))

@dataclass
class imu_payload:
    gyro_z : int
    gyro_y : int
    gyro_x : int
    temp : int
    accel_z : int
    accel_y : int
    accel_x : int
    FORMAT = "<hhhhhhh"
    SIZE = struct.calcsize(FORMAT)
    @classmethod
    def from_bytes(cls,data):
        return cls(*struct.unpack(cls.FORMAT,data))
def handle_imu(data,header):
    imu_data = imu_payload.from_bytes(data)
    print(imu_data)

@dataclass
class packet_header:
    sync : int
    pid : int
    length : int
    timestamp : int

    FORMAT = "<HHHI"
    SIZE = struct.calcsize(FORMAT)
    @classmethod
    def from_bytes(cls,data):
        return cls(*struct.unpack(cls.FORMAT,data))

def process_telemetry(data,header):
    handlers = {PID_MSG:handle_msg, PID_IMU_DATA:handle_imu}
    handler = handlers.get(header.pid)
    if handler:
        handler(data,header)

buf = bytearray() 

ser = serial.Serial("/dev/ttyACM0",baudrate=115200,parity=serial.PARITY_ODD,timeout=0)
while True:
    buf.extend(ser.read(1024))
    while len(buf)>=packet_header.SIZE:
        if buf[0] == 0xaa:
            if buf[1] == 0x55:
                header = packet_header.from_bytes(buf[:packet_header.SIZE])
                if header.length>MAX_PAYLOAD:
                    del buf[:2]
                    continue
                if len(buf)<(header.length+header.SIZE):
                    break #try again next iteration
                data = buf[header.SIZE:header.length+header.SIZE]
                process_telemetry(data,header)
                del buf[:header.length+header.SIZE]
                continue 
        del buf[:1]

