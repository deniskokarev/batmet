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


def fit(x: ArrayLike, y: ArrayLike) -> None:
    """
    :param x: Desired Voltage
    :param y: DAC value to get it
    Print the target function and domain
    """
    (a, b, c), _ = curve_fit(objective, x, y)

    print(f"static int v2dac(float v) {{ return {a} * v * v + {b} * v + {c}; }}")
    # define a sequence of inputs between the smallest and largest known inputs

    x_line = np.arange(min(x), max(x), .01)
    # calculate the output for the range
    y_line = objective(x_line, a, b, c)
    # create a line plot for the mapping function
    pyplot.plot(x_line, y_line, '--', color='red')

    pyplot.scatter(x, y)
    pyplot.show()


def main() -> int:
    parser = argparse.ArgumentParser(
        prog='Calibrate batmet device',
        description='Reads calibration data from CSV file and outputs the calibration coefficients',
        epilog='')
    parser.add_argument('-c', '--csv', type=str, required=True)
    args = parser.parse_args()
    try:
        with open(args.csv, encoding="utf-8") as f:
            mat = np.loadtxt(f, delimiter="\t", skiprows=0, usecols=(0, 3))
            dac, vsens = mat[:, 0], mat[:, 1]
            fit(vsens, dac)
    except IOError as ex:
        logging.error(f"Cannot open {args.csv}")
        return -1


if __name__ == '__main__':
    sys.exit(main())
