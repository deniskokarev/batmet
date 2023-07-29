import sys
import re
import time
from typing import Optional

import serial.tools.list_ports
from serial import PARITY_NONE, STOPBITS_ONE


def open_multimeter(port: Optional[str] = None) -> serial.SerialBase:
    """
    open Agilent 34401A multimeter port
    using:
    - null-modem cable: https://www.amazon.com/dp/B00006B8BJ
    - Prolific PL2303 Chip, DB9 Adapter: https://www.amazon.com/dp/B00GRP8EZU
    Make sure to set the RS232 interface with 9600 baud 8n1 mode
    """
    if not port:
        ports = [pi for pi in serial.tools.list_ports.comports() if pi.pid == 0x2303 and pi.vid == 0x067b]
        if len(ports) < 1:
            raise Exception("no usb serial devices detected")
        elif len(ports) > 1:
            raise Exception("more than one device detected, please specify port")
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
        if not re.search(r"34401A", name):
            raise Exception("This is not 34401A multimeter")
        ser.write(b"*CLS\r\n")
        ser.write(b"SYSTEM:REMOTE\r\n")
        ser.write(b'DISPLAY:TEXT "EXEC: 0"\r\n')
        return ser
    except Exception as e:
        ser.close()
        raise e


def demo() -> int:
    try:
        with open_multimeter() as ser:
            print("ok")
            time.sleep(3)
            ser.write(b"SYSTEM:LOCAL\r\n")
            ser.write(b"*CLS\r\n")
            ser.flush()
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return -1


if __name__ == '__main__':
    sys.exit(demo())
