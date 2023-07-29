#!/usr/bin/env python3
"""
Collect calibration data from BatMet device using Agilent multimeter
"""

import sys
from agilent import Mult
from batmet import Batmet


def collect(mult: Mult, bm: Batmet) -> None:
    """
    Run the collection loop
    Go from d=0..4095 or until 2.5A current is met
    """
    for d in [i * 10 for i in range(410)]:
        bm.set_d(d)
        mult.display(f"D={d}")
        # reading each measurement few times
        cnt = 3
        vs = 0.0
        for _ in range(cnt):
            vs += mult.dc_current()
        va = vs / cnt
        print(f"{d}\t{va}", flush=True)
        if va > 2.5:
            break


def main() -> int:
    try:
        with Mult() as mult:
            with Batmet() as bm:
                collect(mult, bm)
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return -1


if __name__ == '__main__':
    sys.exit(main())
