# Calibration

The calibration code does both collection and evaluation steps. It required a lab power supply
and a digital multimeter suitable of measuring DC current upto 3A using SCPI commands over serial
interface.

The calibration result is coefficients that has to be entered into BatMet device.


Don't forget to activate your virtual env before using calibration scripts. Somthing like:
```
source ~/.sci/bin/activate
```
install dependencies
```
pip install matplotlib scipy pyserial
```

# Collect


Connect BatMet to a lab power supply set to 4V and limited to 3A max in series with Agilent 34401A
multimeter configured for upto 3A current measurement.

Connect multimeter via Null-Modem cable and USB/RS232 Serial adapter. BatMet just to USB port.
*(agilent.py configures PID and VID values for a given Serial adapter)*
- null-modem cable: https://www.amazon.com/dp/B00006B8BJ
- Serial DB9 Adapter on Prolific PL2303 chip: https://www.amazon.com/dp/B00GRP8EZU

Run collection loop - it'll tabulate DAC values from 0..4090 in the increments of 10 and will obtain
the actual current reading from Multimeter.

The first column is DAC value, second column is measured Current.

Run it like so:
```
% ./collect.py | tee calibration_d_i.txt
450	5.1925633333333336e-05
...
630	0.14030770666666667
640	0.15371349666666667
650	0.16676064
660	0.17984219
670	0.19308439666666669
...
2790	2.9714523666666666
2800	2.9835309666666667
2810	2.9965467666666665
2820	3.0102106
```

The collection script has rudimentary fault-tolerance and will properly shutdown the current sink upon
termination. While collection runs, the Multimeter will be displaying current DAC value.

If you encounter Multimeter serial port errors, like below, just power-cycle it.
```
Error: There is no 34401A multimeter connected to /dev/cu.usbserial-14D120 - USB-Serial Controller
```

# Compute Calibration Coefficients

Run the evaluation of the collected calibration data to produce resulting coefficients:

```
./cal_sink.py -c calibration_d_i.txt
```
it'll plot the measurements and will give you the calibration parameters to save into BatMet.
The command will be similar to:
```
csink 0 4 0.209938 762.136354 522.857571
```
this will enable `sink <amp>` command on BatMet. The resulting accuracy is about 2mA on the entire range from 0..4A
