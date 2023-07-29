# Calibration

The calibration code does both collection and evaluation steps.

Set your virtual env and Install dependencies

```
pip install matplotlib scipy pyserial
```

# Collect

Connect BatMet to Agilent E3601A lab power supply limited to 3A max in series with Agilent 34401A multimeter configured for upto 3A current measurement.

Connect multimeter via USB Serial adapter and BatMet just to USB port.

agilent.py configures PID and VID values for a given Serial adapter.

Run collection loop - it'll tabulate DAC values from 0..4090 in the increments by 10.

The first column is DAC value, second column is Current measured by multimeter at that DAC value.

Run it like so:

```
./collect.py | tee calibration_d_i.txt
```

# Evaluate

Process tab-delimited calibration file, where column 1 contains DAC values and column 4 desired voltage

Column 5 contains ADC sensing value

```
./main.py --csv calibration.csv
```

Copy-paste the resulting function into your code

TODO: use persistent Settings to allow calibration update from device command line
