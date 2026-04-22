# Combine bootloader, partition table and application into a final binary.
from __future__ import print_function

import argparse
import datetime
import subprocess
import os, sys

import gen_esp32part

OFFSET_BOOTLOADER_DEFAULT = 0x0
OFFSET_PARTITIONS_DEFAULT = 0x8000

def load_partition_table(filename):
    with open(filename, "rb") as f:
        return gen_esp32part.PartitionTable.from_binary(f.read())

def littlefs_bin_park(size):
    os.system(f"partition/mklittlefs.exe -c partition/file_system  -s {size} lfs2.bin")
            
build_date = datetime.date.today().strftime("%Y-%m-%d")
bin_name = "mPython_v3_v1.0_{}.bin".format(build_date)
uf2_name = "mPython_v3_v1.0_{}.uf2".format(build_date)
print(bin_name)

littlefs_bin_park(0x400000)

arg_sdkconfig = "sdkconfig"
arg_bootloader_bin = "partition/bootloader.bin"
arg_partitions_bin = "partition/partition-table.bin"
arg_application_bin = "partition/micropython.bin"
arg_vfs_bin = "partition/lfs2.bin"
arg_font_bin = "partition/lv_font_siyuan_heiti_medium_20.bin"
arg_voice_data_bin = "partition/esp_tts_voice_data_xiaoxin.dat"
arg_sr_bin = "partition/srmodels.bin"
arg_output_bin = bin_name
arg_output_uf2= uf2_name

offset_partitions = OFFSET_PARTITIONS_DEFAULT
offset_bootloader = OFFSET_BOOTLOADER_DEFAULT
# Load the partition table.
partition_table = load_partition_table(arg_partitions_bin)

max_size_bootloader = offset_partitions - offset_bootloader
max_size_partitions = 0
offset_application = 0
max_size_application = 0
offset_font = 0
max_size_font = 0
offset_vfs = 0
max_size_vfs = 0
offset_voice_data = 0
max_size_voice_data = 0
offset_sr = 0
max_size_sr = 0

# partition table
# Name,       Type, SubType,   Offset,    Size,      Flags
# -------------------------------------------------------
# bootloader, app,  boot,      0x0,       0x7000,
# partitions, data, partition, 0x8000,    0
# nvs,        data, nvs,       0x9000,    0x6000,
# phy_init,   data, phy,       0xf000,    0x1000,
# factory,    app,  factory,   0x10000,   0x3D0000,
# lfs2,       data, spiffs,    0x3E0000,  0x400000,
# font,       data, 0x40,      0x7E0000,  0x120000,
# voice_data, data, fat,       0x900000,  0x3BD000,
# model,      data, spiffs,    0xCBD000,  0x31F000,
# fr,         data,   ,        0xFDC000,  0x20000,
# user_nvs,   data, nvs,       0xFFC000,  0x4000,

# Inspect the partition table to find offsets and maximum sizes.
for part in partition_table:
    if part.name == "nvs":
        max_size_partitions = part.offset - offset_partitions # partition table size
    elif part.type == gen_esp32part.APP_TYPE and offset_application == 0:
        offset_application = part.offset
        max_size_application = part.size
    elif part.name == "lfs2":
        offset_vfs = part.offset
        max_size_vfs = part.size
    elif part.name == "font":
        offset_font = part.offset
        max_size_font = part.size
    elif part.name == "voice_data":
        offset_voice_data = part.offset
        max_size_voice_data = part.size
    elif part.name == "model":
        offset_sr = part.offset
        max_size_sr = part.size

# Define the input files, their location and maximum size.
files_in = [
    ("bootloader", offset_bootloader, max_size_bootloader, arg_bootloader_bin),
    ("partitions", offset_partitions, max_size_partitions, arg_partitions_bin),
    ("application", offset_application, max_size_application, arg_application_bin),
    ("lfs2", offset_vfs, max_size_vfs, arg_vfs_bin),
    ("font", offset_font, max_size_font, arg_font_bin),
    ("voice_data", offset_voice_data, max_size_voice_data, arg_voice_data_bin),
    ("model", offset_sr, max_size_sr, arg_sr_bin),
]
file_out = arg_output_bin

# Write output file with combined firmware.
cur_offset = offset_bootloader
with open(file_out, "wb") as fout:
    for name, offset, max_size, file_in in files_in:
        assert offset >= cur_offset
        fout.write(b"\xff" * (offset - cur_offset))
        cur_offset = offset
        with open(file_in, "rb") as fin:
            data = fin.read()
            fout.write(data)
            cur_offset += len(data)
            print(
                "%-12s@0x%06x % 8d  (% 8d remaining)"
                % (name, offset, len(data), max_size - len(data))
            )
            if len(data) > max_size:
                print(
                    "ERROR: %s overflows allocated space of %d bytes by %d bytes"
                    % (name, max_size, len(data) - max_size)
                )
                sys.exit(1)
    print("%-22s% 8d" % ("total", cur_offset))

