#!/usr/bin/env python3
import argparse
import logging
import numpy as np
from numpy.typing import ArrayLike
import sys

from matplotlib import pyplot
from scipy.optimize import curve_fit


def objective(x, a, b, c):
    """
    There is a tiny bend in (x, y) dependency.
    Let's approximate our curve with quadratic function.
    """
    return a * x * x + b * x + c


def fit_i2dac(x: ArrayLike, y: ArrayLike) -> None:
    """
    :param x: Desired Current
    :param y: DAC value to get it
    Print the target function and domain
    """
    (a, b, c), _ = curve_fit(objective, x, y)

    print(f"""
Calibration complete. You can now open BatMet serial console
\t% picocom -b 115200 /dev/cu.usbserial-D*
and update calibration values
\t$ csink 0 4 {a:.6f} {b:.6f} {c:.6f}
    """)
    # define a sequence of inputs between the smallest and largest known inputs
    x_line = np.arange(min(x), max(x), .01)
    # calculate the output for the range
    y_line = objective(x_line, a, b, c)
    # create a line plot for the mapping function
    pyplot.plot(x_line, y_line, '--', color='blue')
    pyplot.scatter(x, y, color='blue')
    pyplot.show()


def main() -> int:
    parser = argparse.ArgumentParser(
        prog='Calibrate Current -> DAC values for batmet device',
        description='Reads calibration data from collection stage and compute calibration coefficients',
        epilog='')
    parser.add_argument('-c', '--calibration', type=str, required=True)
    args = parser.parse_args()
    try:
        with open(args.calibration, encoding="utf-8") as f:
            mat = np.loadtxt(f, delimiter="\t", skiprows=0)
            idx = np.argmax(mat[:, 1] > 1e-4)
            dac, amp = mat[idx:, 0], mat[idx:, 1]
            fit_i2dac(amp, dac)
    except IOError as ex:
        logging.error(f"Cannot open {args.calibration}")
        return -1


if __name__ == '__main__':
    sys.exit(main())
