# TODO
The Shell doesn't react. Probably inbound UART doesn't work

Remove Settings from prj.conf to save program size

# Build
Install Zephyr as per [Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)

Setup environment
```
. ./zephyr.env
cmake -B cmake-build-debug .
alias b="(cmake --build cmake-build-debug -- -j16)"
```

Develop and rebuild using
```
b
```

# Flash

Use `st-util`
```
brew install stlink
```

Setup environment
```
alias f="st-flash --connect-under-reset write cmake-build-debug/zephyr/zephyr.bin 0x8000000"
```

Flash after build with
```
f
```

# Run
```
screen /dev/cu.usbserial-D30DTADJ 115200
```