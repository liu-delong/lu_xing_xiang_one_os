# -*- coding: utf-8 -*-
'''
用途：
    遍历某目录的下级目录，并查找指定类型文件，复制到上层文件夹
'''
import shutil,os

def get_dir(path,fileType):
    '''
    :param path: 路径
    :param fileType: 需要复制的文件类型（.a，前面需要加.）
    :return:null
    '''

    allfilelist = os.listdir(path)

    for file in allfilelist:
        filepath = os.path.join(path, file)
        if os.path.isdir(filepath):
            allfilelist2 = os.listdir(filepath)
            for file2 in allfilelist2:
                filepath3 = os.path.join(filepath, file2)
                if filepath3.endswith(fileType) and file2 != 'libmain.a':
                    print('copy：'+ file + fileType)
                    shutil.copy(filepath3, distPath)
        else:
            print('not a dir, keep searching')

ESP_IDF_PATH = 'E:/esp32_idf'
LIBS_FIX = []
LIBS_FIX += [ESP_IDF_PATH + '/components/xtensa/esp32/libhal.a']
LIBS_FIX += [ESP_IDF_PATH + '/components/esp_wifi/lib/esp32/libpp.a']
LIBS_FIX += [ESP_IDF_PATH + '/components/esp_wifi/lib/esp32/libphy.a']
LIBS_FIX += [ESP_IDF_PATH + '/components/esp_wifi/lib/esp32/librtc.a']

LD_FILES = []
LD_FILES += [ESP_IDF_PATH + '/examples/get-started/hello_world/build/esp-idf/esp32/esp32_out.ld']
LD_FILES += [ESP_IDF_PATH + '/examples/get-started/hello_world/build/esp-idf/esp32/ld/esp32.project.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp32/ld/esp32.peripherals.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp_rom/esp32/ld/esp32.rom.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp_rom/esp32/ld/esp32.rom.libgcc.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp_rom/esp32/ld/esp32.rom.newlib-time.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp_rom/esp32/ld/esp32.rom.newlib-data.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp_rom/esp32/ld/esp32.rom.newlib-funcs.ld']
LD_FILES += [ESP_IDF_PATH + '/components/esp_rom/esp32/ld/esp32.rom.syscalls.ld']

BIN_FILES = []
BIN_FILES += [ESP_IDF_PATH + '/examples/get-started/hello_world/build/bootloader/bootloader.bin']
BIN_FILES += [ESP_IDF_PATH + '/examples/get-started/hello_world/build/partition_table/partition-table.bin']

if __name__ == '__main__':
    print ('start copy prebuilts')
    path = os.path.abspath(ESP_IDF_PATH + '/examples/get-started/hello_world/build/esp-idf')
    distPath = os.path.abspath('../../thirdparty/esp32/prebuilts/lib/debug')
    get_dir(path,'.a')

    for lib in LIBS_FIX:
        print('copy：' + lib)
        shutil.copy(lib, distPath)

    distPath = os.path.abspath('./ld')
    for ld_file in LD_FILES:
        print('copy：' + ld_file)
        shutil.copy(ld_file, distPath)

    distPath = os.path.abspath('../../thirdparty/esp32/prebuilts')
    for bin_file in BIN_FILES:
        print('copy：' + bin_file)
        shutil.copy(bin_file, distPath)


