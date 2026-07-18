import serial
import struct
from dataclasses import dataclass
import numpy as np

MAX_PAYLOAD = 256

PID_CONTROL = 1
PID_IMU_DATA = 2
PID_ANGLE_ESTIMATE = 3
PID_MSG = 4
gyro = None
accel = None
comp = None

def init_worker(gyro_q,accel_q,comp_q):
    global gyro,accel,comp 
    gyro,accel,comp = gyro_q,accel_q,comp_q
def handle_msg(data,header):
    print(data.decode("utf-8"))

@dataclass
class angle_estimate_payload:
    theta_a : int
    theta_g : int
    theta : int
    FORMAT = "<iii"
    SIZE = struct.calcsize(FORMAT)
    def convert_to_float(self):
        self.theta_a/=(1 << 16)
        self.theta_g = (self.theta_g/(1 << 16)+180)%360 - 180
        #print("theta_a:",self.theta_a,self.theta_g)
        self.theta /= (1 << 16)
    @classmethod
    def from_bytes(cls,data):
        ret = cls(*struct.unpack(cls.FORMAT,data))
        ret.convert_to_float()
        return ret

def handle_angle_estimate(data,header):
    global comp,accel,gyro
    angle = angle_estimate_payload.from_bytes(data)
    comp.append(angle.theta)
    accel.append(angle.theta_a)
    gyro.append(angle.theta_g)
    ##print(angle)

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
offx , offy , offz = 129.53181818181818, 130.2590909090909, 64.65

def handle_imu(data,header):
    global offx,offy,offz,c
    imu_data = imu_payload.from_bytes(data)
    """imu_data.gyro_x-= offx
    imu_data.gyro_y-= offy
    imu_data.gyro_z-= offz"""
    imu_data.gyro_z /= 131
    imu_data.gyro_y /= 131
    imu_data.gyro_x /= 131

    imu_data.accel_x /= 16384
    imu_data.accel_y /= 16384
    imu_data.accel_z /= 16384

    imu_data.temp = imu_data.temp/340 + 36.53
    #print(imu_data)

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
    handlers = {PID_MSG:handle_msg, PID_IMU_DATA:handle_imu, PID_ANGLE_ESTIMATE:handle_angle_estimate}
    handler = handlers.get(header.pid)
    if handler:
        handler(data,header)

buf = bytearray() 

ser = serial.Serial("/dev/ttyACM0",baudrate=115200,parity=serial.PARITY_ODD,timeout=0)
def uart_worker():
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

