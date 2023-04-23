# Flash
```
. ~/bin/zephyr

cmake -B cmake-build-debug .

cmake --build cmake-build-debug

st-flash write cmake-build-debug/zephyr/zephyr.bin 0x8000000
```
