cmake_minimum_required(VERSION 3.20.0)
# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(BOARD stm32f3_batmet)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(batmet)

# common src
set(SRC src/main.c src/opamp.c src/adc.c src/dac.c src/led.c src/cmd.c)

if (CONFIG_SHELL)
    # if shell is enabled adding our commands
    list(APPEND SRC src/cli.c)
endif()

target_sources(app PRIVATE ${SRC})
