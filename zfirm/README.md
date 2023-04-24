# TODO
Time goes twice slower. The prescribed baud rate is 115200, but the client must use 57600 to receive data. The k_sleep(K_MSEC(1000)) also take twice longer.

The Shell doesn't react. Probably inbound UART doesn't work

Remove Settings from prj.conf to save program size

# Flash
```
. ~/bin/zephyr

cmake -B cmake-build-debug .

cmake --build cmake-build-debug

st-flash write cmake-build-debug/zephyr/zephyr.bin 0x8000000
```

# Run
```
screen /dev/cu.usbserial-D30DTADJ 57600
```
