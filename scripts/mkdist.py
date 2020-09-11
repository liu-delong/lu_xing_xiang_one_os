#
# File      : mkdir.py
# This file is part of CMCC IOT OS
# COPYRIGHT (C) 2012-2020, CMCC IOT
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

import os
import shutil

from shutil import ignore_patterns

def do_copy_file(src, dst):
    # check source file
    if not os.path.exists(src):
        return

    path = os.path.dirname(dst)
    # mkdir if path not exist
    if not os.path.exists(path):
        os.makedirs(path)

    shutil.copy2(src, dst)

def do_copy_folder(src_dir, dst_dir, ignore=None):
    import shutil
    # check source directory
    if not os.path.exists(src_dir):
        return

    try:
        if os.path.exists(dst_dir):
            shutil.rmtree(dst_dir)
    except:
        print('Deletes folder: %s failed.' % dst_dir)
        return
        
    shutil.copytree(src_dir, dst_dir, ignore = ignore)


source_ext = ['c', 'h', 's', 'S', 'cpp', 'xpm']
source_list = []


def walk_children(child):
    global source_list
    global source_ext

    # print child
    full_path = child.rfile().abspath
    file_type = full_path.rsplit('.', 1)[1]
    # print file_type
    if file_type in source_ext:
        if full_path not in source_list:
            source_list.append(full_path)

    children = child.all_children()
    if children != []:
        for item in children:
            walk_children(item)


def walk_kconfig(OS_ROOT, source_list):
    for parent, dirnames, filenames in os.walk(OS_ROOT):
        if 'bsp' in parent:
            continue
        if '.git' in parent:
            continue
        if 'scripts' in parent:
            continue

        if 'Kconfig' in filenames:
            pathfile = os.path.join(parent, 'Kconfig')
            source_list.append(pathfile)
        if 'KConfig' in filenames:
            pathfile = os.path.join(parent, 'KConfig')
            source_list.append(pathfile)


def bsp_copy_files(bsp_root, dist_dir):
    # copy BSP files
    do_copy_folder(os.path.join(bsp_root), dist_dir,
        ignore_patterns('build', 'customized-proj', 'customized-proj-strip', '*.pyc', '*.old', '*.map', 'oneos.bin', '.sconsign.dblite', '*.elf', '*.axf', 'cconfig.h'))


def bsp_update_sconstruct(dist_dir):
    with open(os.path.join(dist_dir, 'SConstruct'), 'r') as f:
        data = f.readlines()
    with open(os.path.join(dist_dir, 'SConstruct'), 'w') as f:
        for line in data:
            if line.find('OS_ROOT') != -1:
                if line.find('sys.path') != -1:
                    f.write('# set OS_ROOT\n')
                    f.write(
                        "if not os.getenv('OS_ROOT'): \n    OS_ROOT= os.path.normpath(os.getcwd() + '/oneos')\n\n")
            f.write(line)


def bsp_update_kconfig(dist_dir):
    # change OS_ROOT in Kconfig
    if not os.path.isfile(os.path.join(dist_dir, 'Kconfig')):
        return

    with open(os.path.join(dist_dir, 'Kconfig'), 'r') as f:
        data = f.readlines()
    with open(os.path.join(dist_dir, 'Kconfig'), 'w') as f:
        found = 0
        for line in data:
            if line.find('OS_ROOT') != -1:
                found = 1
            if line.find('default') != -1 and found:
                position = line.find('default')
                line = line[0:position] + 'default "oneos"\n'
                found = 0
            f.write(line)


def bsp_update_kconfig_library(dist_dir):
    # change OS_ROOT in Kconfig
    if not os.path.isfile(os.path.join(dist_dir, 'Kconfig')):
        return

    with open(os.path.join(dist_dir, 'Kconfig'), 'r') as f:
        data = f.readlines()
    with open(os.path.join(dist_dir, 'Kconfig'), 'w') as f:
        found = 0
        for line in data:
            if line.find('OS_ROOT') != -1:
                found = 1
            if line.find('../..') != -1 and found:
                line = line.replace('../..', '$RTT_DIR')
            f.write(line)


def bs_update_ide_project(bsp_root, os_root):
    import subprocess
    # default update the projects which have template file
    tgt_dict = {'mdk4': ('keil', 'armcc'),
                'mdk5': ('keil', 'armcc'),
                'iar': ('iar', 'iar'),
                'vs': ('msvc', 'cl'),
                'vs2012': ('msvc', 'cl'),
                'cdk': ('gcc', 'gcc')}

    scons_env = os.environ.copy()
    scons_env['OS_ROOT'] = os_root

    for item in tgt_dict:
        child = subprocess.Popen('scons --target=' + item, cwd=bsp_root,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        stdout, stderr = child.communicate()
        if child.returncode == 0:
            print('update %s project' % item)


def zip_dist(dist_dir, dist_name):
    import zipfile

    zip_filename = os.path.join(dist_dir)
    zip = zipfile.ZipFile(zip_filename + '.zip', 'w')
    pre_len = len(os.path.dirname(dist_dir))

    for parent, dirnames, filenames in os.walk(dist_dir):
        for filename in filenames:
            pathfile = os.path.join(parent, filename)
            arcname = pathfile[pre_len:].strip(os.path.sep)
            zip.write(pathfile, arcname)

    zip.close()


def MkDist_Strip(program, BSP_ROOT, OS_ROOT, Env):
    global source_list

    print('Generating new code project and strip useless files....')

    dist_name = os.path.basename(BSP_ROOT)
    dist_dir = os.path.join(BSP_ROOT, 'customized-proj-strip', dist_name)
    target_path = os.path.join(dist_dir, 'oneos')

    print('=> %s' % os.path.basename(BSP_ROOT))
    bsp_copy_files(BSP_ROOT, dist_dir)

    # copy stm32 bsp libiary files
    if os.path.basename(os.path.dirname(BSP_ROOT)) == 'stm32':
        print("=> copy stm32 bsp library")
        library_path = os.path.join(os.path.dirname(BSP_ROOT), 'libraries')
        library_dir = os.path.join(dist_dir, 'libraries')
        bsp_copy_files(os.path.join(library_path, 'HAL_Drivers'),
                       os.path.join(library_dir, 'HAL_Drivers'))
        bsp_copy_files(os.path.join(library_path, Env['bsp_lib_type']), os.path.join(
            library_dir, Env['bsp_lib_type']))
        shutil.copyfile(os.path.join(library_path, 'Kconfig'),
                        os.path.join(library_dir, 'Kconfig'))

    # do bsp special dist handle
    if 'dist_handle' in Env:
        print("=> start dist handle")
        dist_handle = Env['dist_handle']
        dist_handle(BSP_ROOT)

    # get all source files from program
    for item in program:
        walk_children(item)
    source_list.sort()

    # copy the source files without libcpu and components/libc in CMCC IOT
    target_list = []
    libcpu_dir = os.path.join(OS_ROOT, 'libcpu').lower()
    libc_dir = os.path.join(OS_ROOT, 'components',
                            'libc', 'compilers').lower()
    sal_dir = os.path.join(OS_ROOT, 'components', 'net', 'sal_socket').lower()
    sources_include_sal = False
    for src in source_list:
        if src.lower().startswith(BSP_ROOT.lower()):
            continue

        # skip libc and libcpu dir
        if src.lower().startswith(libcpu_dir):
            continue
        if src.lower().startswith(libc_dir):
            continue
        if src.lower().startswith(sal_dir):
            sources_include_sal = True
            continue

        if src.lower().startswith(OS_ROOT.lower()):
            target_list.append(src)
    source_list = target_list

    # get source directory
    src_dir = []
    for src in source_list:
        src = src.replace(OS_ROOT, '')
        if src[0] == os.sep or src[0] == '/':
            src = src[1:]

        path = os.path.dirname(src)
        sub_path = path.split(os.sep)
        full_path = OS_ROOT
        for item in sub_path:
            full_path = os.path.join(full_path, item)
            if full_path not in src_dir:
                src_dir.append(full_path)

    # add all of SConscript files
    for item in src_dir:
        source_list.append(os.path.join(item, 'SConscript'))

    # add all of Kconfig files
    walk_kconfig(OS_ROOT, source_list)

    # copy all files to target directory
    source_list.sort()
    for src in source_list:
        dst = src.replace(OS_ROOT, '')
        if dst[0] == os.sep or dst[0] == '/':
            dst = dst[1:]

        print('=> %s' % dst)
        dst = os.path.join(target_path, dst)
        do_copy_file(src, dst)

    # copy scripts directory
    print('=> scripts')
    do_copy_folder(os.path.join(OS_ROOT, 'scripts'), os.path.join(
        target_path, 'scripts'), ignore_patterns('*.pyc'))
    do_copy_file(os.path.join(OS_ROOT, 'Kconfig'),
                 os.path.join(target_path, 'Kconfig'))
    do_copy_file(os.path.join(OS_ROOT, 'AUTHORS'),
                 os.path.join(target_path, 'AUTHORS'))
    do_copy_file(os.path.join(OS_ROOT, 'COPYING'),
                 os.path.join(target_path, 'COPYING'))
    do_copy_file(os.path.join(OS_ROOT, 'README.md'),
                 os.path.join(target_path, 'README.md'))
    do_copy_file(os.path.join(OS_ROOT, 'README_zh.md'),
                 os.path.join(target_path, 'README_zh.md'))

    print('=> %s' % os.path.join('components', 'libc', 'compilers'))
    do_copy_folder(os.path.join(OS_ROOT, 'components', 'libc', 'compilers'),
                   os.path.join(target_path, 'components', 'libc', 'compilers'))

    if sources_include_sal:
        print('=> %s' % os.path.join('components', 'net', 'sal_socket'))
        do_copy_folder(os.path.join(OS_ROOT, 'components', 'net', 'sal_socket'), os.path.join(
            target_path, 'components', 'net', 'sal_socket'))

    # copy all libcpu/ARCH directory
    import osconfig
    print('=> %s' % (os.path.join('libcpu', osconfig.ARCH, osconfig.CPU)))
    do_copy_folder(os.path.join(OS_ROOT, 'libcpu', osconfig.ARCH, osconfig.CPU),
                   os.path.join(target_path, 'libcpu', osconfig.ARCH, osconfig.CPU))
    if os.path.exists(os.path.join(OS_ROOT, 'libcpu', osconfig.ARCH, 'common')):
        print('=> %s' % (os.path.join('libcpu', osconfig.ARCH, 'common')))
        do_copy_folder(os.path.join(OS_ROOT, 'libcpu', osconfig.ARCH, 'common'), os.path.join(
            target_path, 'libcpu', osconfig.ARCH, 'common'))
    do_copy_file(os.path.join(OS_ROOT, 'libcpu', 'Kconfig'),
                 os.path.join(target_path, 'libcpu', 'Kconfig'))
    do_copy_file(os.path.join(OS_ROOT, 'libcpu', 'SConscript'),
                 os.path.join(target_path, 'libcpu', 'SConscript'))

    # change OS_ROOT in SConstruct
    bsp_update_sconstruct(dist_dir)
    # change OS_ROOT in Kconfig
    bsp_update_kconfig(dist_dir)
    bsp_update_kconfig_library(dist_dir)
    # update all project files
    bs_update_ide_project(dist_dir, target_path)

    # make zip package
    zip_dist(dist_dir, dist_name)

    print("\nNew code project is generated at: \n%s\n" %dist_dir)


def MkDist(program, BSP_ROOT, OS_ROOT, Env):
    print('Generating new code project ....')

    dist_name = os.path.basename(BSP_ROOT)
    dist_dir = os.path.join(BSP_ROOT, 'customized-proj', dist_name)
    
    target_path = os.path.join(dist_dir, 'oneos')

    # copy BSP files
    print('=> %s' % os.path.basename(BSP_ROOT))
    bsp_copy_files(BSP_ROOT, dist_dir)

    # copy stm32 driver files
    print("=> copy stm32 drivers")
    library_path = os.path.join(OS_ROOT, 'drivers')
    library_dir = os.path.join(target_path, 'drivers')
    bsp_copy_files(os.path.join(library_path, 'stm32'),
                   os.path.join(library_dir, 'stm32'))
    bsp_copy_files(os.path.join(library_path, 'virtual'),
                   os.path.join(library_dir, 'virtual'))

    # copy stm32 lib files
    print("=> copy stm32 lib")
    library_path = os.path.join(OS_ROOT, 'lib')
    library_dir = os.path.join(target_path, 'lib')
    do_copy_file(os.path.join(library_path, 'Kconfig'),
                 os.path.join(library_dir, 'Kconfig'))
    do_copy_file(os.path.join(library_path, 'SConscript'),
                 os.path.join(library_dir, 'SConscript'))
    library_path = os.path.join(library_path, 'stm32')
    library_dir = os.path.join(library_dir, 'stm32')
    do_copy_file(os.path.join(library_path, 'Kconfig'),
                 os.path.join(library_dir, 'Kconfig'))
    if Env['bsp_lib_type']:
        bsp_copy_files(os.path.join(library_path, Env['bsp_lib_type']), 
                       os.path.join(library_dir, Env['bsp_lib_type']))

    # do bsp special dist handle
    if 'dist_handle' in Env:
        print("=> start dist handle")
        dist_handle = Env['dist_handle']
        dist_handle(BSP_ROOT)

    # copy components directory
    print('=> components')
    do_copy_folder(os.path.join(OS_ROOT, 'components'),
                   os.path.join(target_path, 'components'))

    # copy thirdparty directory
    print('=> thirdparty')
    do_copy_folder(os.path.join(OS_ROOT, 'thirdparty'),
                   os.path.join(target_path, 'thirdparty'))

    # skip documentation directory
    # skip examples

    # copy include directory
    print('=> include')
    do_copy_folder(os.path.join(OS_ROOT, 'include'),
                   os.path.join(target_path, 'include'))

    # copy all arch directory
    print('=> arch')
    import osconfig
    do_copy_folder(os.path.join(OS_ROOT, 'arch', osconfig.ARCH),
                   os.path.join(target_path, 'arch', osconfig.ARCH))
    do_copy_file(os.path.join(OS_ROOT, 'arch', 'Kconfig'),
                 os.path.join(target_path, 'arch', 'Kconfig'))
    do_copy_file(os.path.join(OS_ROOT, 'arch', 'SConscript'),
                 os.path.join(target_path, 'arch', 'SConscript'))

    # copy src directory
    print('=> kernel')
    do_copy_folder(os.path.join(OS_ROOT, 'kernel'),
                   os.path.join(target_path, 'kernel'))

    # copy scripts directory
    print('=> scripts')
    do_copy_folder(os.path.join(OS_ROOT, 'scripts'), os.path.join(
        target_path, 'scripts'), ignore_patterns('*.pyc'))

    do_copy_file(os.path.join(OS_ROOT, 'Kconfig'),
                 os.path.join(target_path, 'Kconfig'))
    do_copy_file(os.path.join(OS_ROOT, 'AUTHORS'),
                 os.path.join(target_path, 'AUTHORS'))
    do_copy_file(os.path.join(OS_ROOT, 'COPYING'),
                 os.path.join(target_path, 'COPYING'))
    do_copy_file(os.path.join(OS_ROOT, 'README.md'),
                 os.path.join(target_path, 'README.md'))
    do_copy_file(os.path.join(OS_ROOT, 'README_zh.md'),
                 os.path.join(target_path, 'README_zh.md'))

    # change OS_ROOT in SConstruct
    bsp_update_sconstruct(dist_dir)
    # change OS_ROOT in Kconfig
    bsp_update_kconfig(dist_dir)
    bsp_update_kconfig_library(dist_dir)
    # update all project files
    bs_update_ide_project(dist_dir, target_path)

    # make zip package
    zip_dist(dist_dir, dist_name)

    print("\nNew code project is generated at: \n%s\n" %dist_dir)
