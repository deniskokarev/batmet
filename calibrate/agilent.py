#!/usr/bin/env python
import sys
import re
import time
from typing import Optional

import serial.tools.list_ports
from serial import PARITY_NONE


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
        return self

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
                            timeout=2.0,
                            parity=PARITY_NONE,
                            stopbits=2,
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
            ser.flush()
            time.sleep(.5)
            return ser
        except Exception as e:
            ser.close()
            raise e

    @staticmethod
    def get_err(ser: serial.SerialBase) -> None:
        ser.write(b"SYST:ERR?\r\n")
        ser.flush()
        return ser.read_until().decode('ascii').rstrip()

    @staticmethod
    def close_serial(ser: serial.SerialBase) -> None:
        print(f"err={Mult.get_err(ser)}")
        ser.write(b"SYSTEM:LOCAL\r\n")
        ser.write(b"*CLS\r\n")
        ser.flush()
        ser.close()

    def _read_float(self) -> float:
        result = self.ser.read_until().decode('ascii').rstrip()
        if len(re.findall(r"[+-][0-9][.][0-9]*[Ee][+-][0-9][0-9]", result)) >= 1:
            return float(result)
        else:
            raise Exception("Couldn't read a floating point number")

    def dc_current(self) -> float:
        self.ser.write(b":MEAS:CURR:DC?\r\n")
        return self._read_float()

    def dc_voltage(self) -> float:
        self.ser.write(b":MEAS:VOLT:DC?\r\n")
        return self._read_float()


def demo() -> int:
    try:
        with Mult() as mult:
            for i in range(3):
                print(f"I={mult.dc_current()}")
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return -1


if __name__ == '__main__':
    sys.exit(demo())
