# SPDX-License-Identifier: Apache-2.0
#
# The following flashing methods supported:
# - CubeMX Programmer over ST-Link v2 (default)
# - stm32flash serial flasher doesn't need any programmer, just use boot0 jumper
# - JLink works fine too - validated with `west flash -r jlink`
# - Lightweight `st-flash --connect-under-reset write cmake-build-debug/zephyr/zephyr.bin 0x8000000`
# - OpenOCD - DOES NOT WORK!!!

# Automatically finds CubeMX Programmer at the default location and works fine!
# https://www.st.com/en/development-tools/stm32cubemx.html
board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")
include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)

# Serial flasher `stm32flash` works fine too! `brew install stm32flash`
# just use boot0 jumper and reset the board.
# Use your own device name, of course
board_runner_args(stm32flash "--device=/dev/cu.usbserial-D30DTADJ" "--start-addr=0x8000000")
include(${ZEPHYR_BASE}/boards/common/stm32flash.board.cmake)

# JLink flashing works fine
board_runner_args(jlink "--device=STM32F334K8" "--speed=4000")
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)

# OpenOCD ST-Link v2 should have worked, but it does not
# :-( fails with core.c:96 flash_driver_write(): error writing to flash at address 0x08000000 at offset 0x00000000
#board_runner_args(openocd --cmd-pre-init "source [find interface/stlink-v2.cfg]" --cmd-pre-init "source [find target/stm32f3x.cfg]")
#include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
