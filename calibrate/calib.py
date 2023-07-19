#!/usr/bin/env python3

import sys
import io
import serial.tools.list_ports


def main() -> int:
    ports = [pi for pi in serial.tools.list_ports.comports() if pi.pid == 24597 and pi.vid == 1027]
    if len(ports) < 1:
        print("no devices detected", file=sys.stderr)
    elif len(ports) > 1:
        print("more than one device detected, please specify port", file=sys.stderr)
    port = ports[0]
    print(port)
    #with serial.Serial(port.device, 115200, timeout=0, parity=serial.PARITY_NONE, rtscts=1) as ser:
    with serial.Serial(port.device, 115200) as ser:
        ser.write(b"help\r\n")
        ser.flush()
        for _ in range(3):
            l = ser.read_until()
            print(l)


if __name__ == '__main__':
    sys.exit(main())
