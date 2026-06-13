import serial
import struct
from collections import deque
from dataclasses import dataclass 

MAX_PAYLOAD = 256

PID_DEBUG = 3

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

buf = bytearray() 

ser = serial.Serial("/dev/ttyUSB0",baudrate=115200,parity=serial.PARITY_ODD,timeout=0)
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
                data = buf[:header.length+header.SIZE]
                handle_telemetry(header.pid,data)
                del buf[:header.length+header.SIZE]
                continue 
        del buf[:1]
def handle_telemetry(pid,data):
    handlers = {PID_DEBUG:handle_debug}
    handler = handlers.get(pid)
    if handler:
        handler(data)

def handle_debug(data):
    print(data[header.SIZE:].decode("utf-8"))

