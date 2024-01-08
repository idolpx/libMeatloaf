# Part of ESPEasy build toolchain.
#
# Combines separate bin files with their respective offsets into a single file
# This single file must then be flashed to an ESP32 node with 0 offset.
#
# Original implementation: Bartłomiej Zimoń (@uzi18)
# Maintainer: Gijs Noorlander (@TD-er)
#
# Special thanks to @Jason2866 (Tasmota) for helping debug flashing to >4MB flash
# Thanks @jesserockz (esphome) for adapting to use esptool.py with merge_bin
#
# Typical layout of the generated file:
#    Offset | File
# -  0x1000 | ~\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dout_40m.bin
# -  0x8000 | ~\ESPEasy\.pio\build\<env name>\partitions.bin
# -  0xe000 | ~\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin
# - 0x10000 | ~\ESPEasy\.pio\build\<env name>/<built binary>.bin

Import("env")

platform = env.PioPlatform()

import os, stat, shutil


# Create the 'bin' folder if it doesn't exist
if not os.path.exists('bin'):
    os.makedirs('bin')

def export_bin(source, target, env):
    program = env.subst("$BUILD_DIR/${PROGNAME}")
    new_file_name = f"bin/libMeatloaf"
    if (os.path.exists(new_file_name)):
        os.remove(new_file_name)
    print(f"Exporting [{program}] to [{new_file_name}]")
    shutil.copyfile(program, new_file_name)
    os.chmod(new_file_name, stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    os.system(new_file_name)

env.AddPostAction("$BUILD_DIR/${PROGNAME}", export_bin)
