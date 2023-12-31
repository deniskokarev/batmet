#!/usr/bin/env python
import sys
import re
import time
from typing import Optional

import serial.tools.list_ports


class Batmet:
    """
    open Battery meter electronic load device
    """
    PID = 0x6015
    VID = 0x0403

    def __init__(self, port: Optional[str] = None):
        self.ser = Batmet.open_serial(port)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close_serial()

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
                            baudrate=115200,
                            timeout=2.0,
                            )
        try:
            ser.write(b"help\r\n")
            ser.flush()
            for _ in range(2):
                lout = ser.read_until().decode("ascii").rstrip()
                if re.search("Supported commands", lout):
                    break
            else:
                raise Exception(f"There is no BatMet device on port {port}")
            ser.read(1024)
            return ser
        except Exception as e:
            ser.close()
            raise e

    def close_serial(self) -> None:
        self.set_d(0)
        self.ser.close()

    def set_d(self, d: int) -> None:
        self.ser.write(f"d {d}\r\n".encode())
        self.ser.flush()
        self.ser.read_until()


def demo() -> int:
    try:
        with Batmet() as bm:
            for d in [400, 500, 600, 0]:
                print(f"setting d={d}", file=sys.stderr)
                bm.set_d(d)
                time.sleep(5)
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return -1


if __name__ == '__main__':
    sys.exit(demo())
