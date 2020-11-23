import os
import platform

# toolchains options
ARCH = 'xtensa'
CPU = 'esp32'
CROSS_TOOL = 'gcc'
BUILD = 'debug'

ESP_CROSSTOOL_PATH = os.path.abspath('../../thirdparty/esp32/xtensa-esp32-elf/bin')
ESP_TOOL = os.path.abspath('./esptool.py')
ESP_INCLUDE = os.path.abspath('../../thirdparty/esp32/include')
ESP_LIB_PATH = os.path.abspath(f'../../thirdparty/esp32/prebuilts/lib/{BUILD}')
ESP_LIBS = ' '
ESP_IDF_PATH = os.path.abspath('../../thirdparty/esp32/esp-idf-port/esp-idf')
# cross_tool provides the cross compiler
# COMPILER_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR
if CROSS_TOOL == 'gcc':
    COMPILER    = 'gcc'
    if(platform.system() == 'Windows'):
        COMPILER_PATH = ESP_CROSSTOOL_PATH
    elif(platform.system() == 'Linux'):
        print('Error: error compile platform, only support windows')
    else:
        print('Error: error compile platform, only support windows')
else:
    print("Error: Not support <%s> cross tool" % CROSS_TOOL)
    exit(-1)


if not os.path.exists(COMPILER_PATH):
    print("Error: tool <%s> not exist" % COMPILER_PATH)
    exit(-1)


if COMPILER == 'gcc':
    # toolchains
    PREFIX = 'xtensa-esp32-elf-'
    CC = PREFIX + 'gcc'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    CXX = PREFIX + 'g++'
    LINK = PREFIX + 'g++'
    RESULT_SUFFIX = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = ' '
    CFLAGS = DEVICE + '-Os -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -Wall -Wno-address '
    AFLAGS = ' -c' + DEVICE + f' -x assembler-with-cpp -MMD -MP -I{ESP_IDF_PATH}/components/freertos/include/freertos -I{ESP_IDF_PATH}/components/esp32/include -I{ESP_IDF_PATH}/components/soc/esp32/include -I.'
    LFLAGS = DEVICE + f' -u call_user_start_cpu0 -u __cxa_guard_dummy -Wl,--undefined=uxTopUsedPriority,--gc-sections,-Map=oneos-esp32.map,-cref,-static -nostdlib -T esp32_out.ld -T esp32.common.ld -T {ESP_IDF_PATH}/components/esp32/ld/esp32.rom.ld -T {ESP_IDF_PATH}/components/esp32/ld/esp32.peripherals.ld -T {ESP_IDF_PATH}/components/esp32/ld/esp32.rom.spiflash.ld'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -ggdb'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O2'

    CXXFLAGS = CFLAGS + ' -std=gnu++11 -fno-exceptions -fno-rtti'
    CFLAGS   = CFLAGS + ' -std=gnu99'

    POST_ACTION = f'cp ../../thirdparty/esp32/prebuilts/*.bin  ../../out/esp32 \n'
    POST_ACTION += f'python {ESP_TOOL} --chip esp32 elf2image --flash_mode "dio" --flash_freq "40m" --flash_size "4MB" -o oneos_esp32.bin oneos_esp32.elf \n'
    POST_ACTION += SIZE + ' $TARGET \n'

