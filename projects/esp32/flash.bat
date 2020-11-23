python ./esptool.py --chip esp32 --port "COM4" --baud 115200 --before "default_reset" --after "hard_reset" write_flash -z --flash_mode "dio" --flash_freq "40m" --flash_size detect 0x1000 ../../out/esp32/bootloader.bin 0x10000 ../../out/esp32/oneos_esp32.bin 0x8000 ../../out/esp32/partition_table.bin

pause