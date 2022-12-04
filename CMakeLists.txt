cmake_minimum_required(VERSION 3.24)
project(libespdisplay C)

set(CMAKE_C_STANDARD 11)

add_executable(libespdisplay src/lcd.c include/esp_display/raw_comm.h src/raw_comm.c src/main.c)
target_include_directories(libespdisplay PUBLIC include/)
