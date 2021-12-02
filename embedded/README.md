# Audiyour ESP32 Embedded Code

## Prerequisites
- You must have ESP-ADF and ESP-IDF installed
    https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html
- This code has been tested on and developed for the ESP32-LyraT devboard


## Build, Flash, and Montitor UART Output
- Assuming that the ESP32 board is connected to /dev/ttyUSB0
    -   'idf.py -p /dev/ttyUSB0 fullclean flash monitor'