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
    Go from d=[450..4090] or until 3A current is met
    """
    for d in range(450, 4100, 10):
        bm.set_d(d)
        mult.display(f"D={d}")
        # reading each measurement few times
        cnt = 3
        vs = 0.0
        for _ in range(cnt):
            vs += mult.dc_current()
        va = vs / cnt
        print(f"{d}\t{va}", flush=True)
        if va > 3:
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
