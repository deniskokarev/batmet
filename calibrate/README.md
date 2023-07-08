# Calibration

The calibration file contains DAC value to Sensing Voltage mapping.

Install dependencies

```
pip install matplotlib scipy
```

Process tab-delimited calibration file, where column 1 contains DAC values and column 4 desired voltage

```
./main.py --csv calibration.csv
```

Copy-paste the resulting function into your code

TODO: use persistent Settings to allow calibration update from device command line
