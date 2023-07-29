import sys
import re
import time
from typing import Optional

import serial.tools.list_ports
from serial import PARITY_NONE, STOPBITS_ONE


class Mult:
    """
    open Agilent 34401A multimeter
    using:
    - null-modem cable: https://www.amazon.com/dp/B00006B8BJ
    - Prolific PL2303 Chip, DB9 Adapter: https://www.amazon.com/dp/B00GRP8EZU
    Make sure to set the RS232 interface with 9600 baud 8n1 mode
    """
    PID = 0x2303
    VID = 0x067b
    MODEL = "34401A"

    def __init__(self, port: Optional[str] = None):
        self.ser = Mult.open_serial(port)

    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_val, exc_tb):
        Mult.close_serial(self.ser)

    @classmethod
    def open_serial(cls, port: Optional[str] = None) -> serial.SerialBase:
        if not port:
            ports = [pi for pi in serial.tools.list_ports.comports() if pi.pid == cls.PID and pi.vid == cls.VID]
            if len(ports) < 1:
                raise Exception("no serial devices detected")
            elif len(ports) > 1:
                raise Exception("more than one serial device detected, please specify port")
            port = ports[0]
        ser = serial.Serial(port=port.device,
                            baudrate=9600,
                            timeout=1.0,
                            parity=PARITY_NONE,
                            stopbits=STOPBITS_ONE,
                            )
        try:
            ser.write(b"*IDN?\r\n")
            ser.flush()
            name = ser.read_until().decode('ascii').rstrip()
            if not re.search(cls.MODEL, name):
                raise Exception(f"There is no {cls.MODEL} multimeter connected to {port}")
            ser.write(b"*CLS\r\n")
            ser.write(b"SYSTEM:REMOTE\r\n")
            ser.write(b'DISPLAY:TEXT "EXEC..."\r\n')
            return ser
        except Exception as e:
            ser.close()
            raise e

    @staticmethod
    def close_serial(ser: serial.SerialBase) -> None:
        ser.write(b"SYSTEM:LOCAL\r\n")
        ser.write(b"*CLS\r\n")
        ser.flush()
        ser.close()


def demo() -> int:
    try:
        with Mult() as mult:
            print("ok")
            time.sleep(3)
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return -1


if __name__ == '__main__':
    sys.exit(demo())
