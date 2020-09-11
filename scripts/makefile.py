import os
import sys

from utils import *
from utils import _make_path_relative
import osconfig

makefile = '''phony := all
all:

include config.mk

ifneq ($(MAKE_LIB),1)
TARGET := oneos.elf
include src.mk
endif

$(if $(strip $(OS_ROOT)),,$(error OS_ROOT not defined))

include $(OS_ROOT)/scripts/oneos.mk
'''

def TargetMakefile(env):
    project = ProjectInfo(env)

    BSP_ROOT = os.path.abspath(env['BSP_ROOT'])
    OS_ROOT = os.path.abspath(env['OS_ROOT'])

    match_bsp = False
    if BSP_ROOT.startswith(OS_ROOT): 
        match_bsp = True

    make = open('config.mk', 'w')

    make.write('BSP_ROOT ?= %s\n' % BSP_ROOT.replace('\\', '/'))
    make.write('OS_ROOT ?= %s\n' % OS_ROOT.replace('\\', '/'))
    make.write('\n')

    cross = os.path.abspath(osconfig.COMPILER_PATH)
    cross = os.path.join(cross, osconfig.PREFIX)
    make.write('CROSS_COMPILE ?=%s' % cross.replace('\\', '\\\\'))
    make.write('\n')
    make.write('\n')

    make.write('CFLAGS :=%s' % (osconfig.CFLAGS))
    make.write('\n')
    make.write('AFLAGS :=%s' % (osconfig.AFLAGS))
    make.write('\n')
    make.write('LFLAGS :=%s' % (osconfig.LFLAGS))
    make.write('\n')
    if 'CXXFLAGS' in dir(osconfig):
        make.write('CXXFLAGS :=%s' % (osconfig.CXXFLAGS))
        make.write('\n')

    make.write('\n')

    Files   = project['FILES']
    Headers = project['HEADERS']
    CPPDEFINES = project['CPPDEFINES']

    paths = [os.path.normpath(i) for i in project['CPPPATH']]
    CPPPATH = []
    for path in paths:
        fn = os.path.normpath(path)
        if match_bsp:
            if fn.startswith(BSP_ROOT):
                fn = '$(BSP_ROOT)' + fn.replace(BSP_ROOT, '')
            elif fn.startswith(OS_ROOT):
                fn = '$(OS_ROOT)' + fn.replace(OS_ROOT, '')
        else:
            if fn.startswith(OS_ROOT):
                fn = '$(OS_ROOT)' + fn.replace(OS_ROOT, '')
            elif fn.startswith(BSP_ROOT):
                fn = '$(BSP_ROOT)' + fn.replace(BSP_ROOT, '')

        CPPPATH.append(fn)

    path = ''
    paths = CPPPATH
    for item in paths:
        path += '\t-I%s \\\n' % item

    make.write('CPPPATHS :=')
    if path[0] == '\t': path = path[1:]
    length = len(path)
    if path[length - 2] == '\\': path = path[:length - 2]
    make.write(path)
    make.write('\n')
    make.write('\n')

    defines = ''
    for item in project['CPPDEFINES']:
        defines += ' -D%s' % item
    make.write('DEFINES :=')
    make.write(defines)
    make.write('\n')

    files = Files
    Files = []
    for file in files:
        fn = os.path.normpath(file)
        if match_bsp:
            if fn.startswith(BSP_ROOT):
                fn = '$(BSP_ROOT)' + fn.replace(BSP_ROOT, '')
            elif fn.startswith(OS_ROOT):
                fn = '$(OS_ROOT)' + fn.replace(OS_ROOT, '')
        else:
            if fn.startswith(OS_ROOT):
                fn = '$(OS_ROOT)' + fn.replace(OS_ROOT, '')
            elif fn.startswith(BSP_ROOT):
                fn = '$(BSP_ROOT)' + fn.replace(BSP_ROOT, '')

        Files.append(fn)
        # print(fn)

    src = open('src.mk', 'w')
    files = Files
    src.write('SRC_FILES :=\n')
    for item in files:
        src.write('SRC_FILES +=%s\n' % item.replace('\\', '/'))

    make = open('Makefile', 'w')
    make.write(makefile)
    make.close()

    return
