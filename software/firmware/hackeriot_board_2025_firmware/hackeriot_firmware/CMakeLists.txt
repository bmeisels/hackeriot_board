# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hackeriot_firmware)

target_sources(app PRIVATE src/buttons.c)
target_sources(app PRIVATE src/kc.c)
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/maze.c)
target_sources(app PRIVATE src/screen.c)
target_sources(app PRIVATE src/simon.c)
target_sources(app PRIVATE src/snake.c)
