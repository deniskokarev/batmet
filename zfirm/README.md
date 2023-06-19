# TODO
Fix Settings NVS

# Zephyr
The project doesn't include Zephyr distribution. You should be installing it yourself as per
[Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
It is recommended to use default location and python `venv` method.

Install ARM toll-chain as well. Recommending [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/toolchains/zephyr_sdk.html).
*(You need only `arm` target to compile this project)*

# Build
If Zephyr was installed at the default location the `zephyr.env` will include it correctly.

Setup environment
```
source ./zephyr.env
cmake -B build .
alias b="cmake --build build -- -j16"
alias f="cmake --build build --target flash"
```

Rebuild using `b`, flash using `f`

Likewise `west build` and `west flash` will work as well

# CLion
Open CLion project - it should automatically recognize all targets and should be able to build.
Only it will be using `cmake-build-debug` directory instead of just `build`.

NB!: If you accidentally damage the dir or remove it CLion will not be happy. In order to re-create
the `cmake-build-debug` dir right-click on `CMakeList.txt` and do Reload CMake Project.

Due to the OpenOCD problem with stm32f3 board the flashing is not as straightforward. Don't use OpenOCD target
for this project, use `st-util` instead: `brew install st-util`

Then

Tools -> Run -> New Embedded Configuration

- select st-link
- target `zephyr`
- executable binary `zephyr-final`

Clicking on Debug button should initiate flashing

# Flashing from CLI
Flashing is configured using ST-Link V2

### Use CubeMX programmer
This is a default method. Install [CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
and Zephyr will automatically discover it and will use for flashing.

It'll be invoked upon `f` alias defined above

### Use `st-util`
If you don't feel like installing CubeMX you can use more lightweight method.
```
brew install stlink
```

Use the following command to flash:
```
alias f="st-flash --connect-under-reset write build/zephyr/zephyr.bin 0x8000000"
```

# Serial Flash

Serial flashing is also available if you don't want to use a programmer.
```
brew install stm32flash
```

Set `boot0` jumper to pull it up and reboot the board. It should turn into DFU mode and will be barely glowing the red LED.
Then run serial flashing

```
west flash -r stm32flash --device /dev/cu.usbserial-D30DTADJ
```
or invoke stm32flash directly

```
stm32flash -b 57600 -m 8e1 -S 0x8000000 -w ./build/zephyr/zephyr.bin /dev/cu.usbserial-D30DTADJ
```
*(use your /dev/... serial device name, of course)*

When serial flashing is over remove the boot0 jumper and reboot the board.

# Run
```
screen /dev/cu.usbserial-D30DTADJ 115200
```
*Exit from screen Ctrl-A + Ctrl-K*


opens the Zephyr Shell
```
batmet:~$ bm
bm - Battery meter commands
Subcommands:
  d          :Set DAC value [0..4095]
  v          :Set desired voltage
  dac_cycle  :Start/Stop Dac cycle
```
