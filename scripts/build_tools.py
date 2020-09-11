#
# File      : build_tools.py
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
#

import os
import sys
import string
import utils

from SCons.Script import *
from utils import _make_path_relative
from mkdist import do_copy_file

BuildOptions = {}
Projects = []
Os_Root = ''
Env = None
out_dir = ''

# SCons PreProcessor patch
def start_handling_includes(self, t=None):
    """
    Causes the PreProcessor object to start processing #import,
    #include and #include_next lines.

    This method will be called when a #if, #ifdef, #ifndef or #elif
    evaluates True, or when we reach the #else in a #if, #ifdef,
    #ifndef or #elif block where a condition already evaluated
    False.

    """
    d = self.dispatch_table
    p = self.stack[-1] if self.stack else self.default_table

    for k in ('import', 'include', 'include_next', 'define'):
        d[k] = p[k]

def stop_handling_includes(self, t=None):
    """
    Causes the PreProcessor object to stop processing #import,
    #include and #include_next lines.

    This method will be called when a #if, #ifdef, #ifndef or #elif
    evaluates False, or when we reach the #else in a #if, #ifdef,
    #ifndef or #elif block where a condition already evaluated True.
    """
    d = self.dispatch_table
    d['import'] = self.do_nothing
    d['include'] =  self.do_nothing
    d['include_next'] =  self.do_nothing
    d['define'] =  self.do_nothing

PatchedPreProcessor = SCons.cpp.PreProcessor
PatchedPreProcessor.start_handling_includes = start_handling_includes
PatchedPreProcessor.stop_handling_includes = stop_handling_includes

class Win32Spawn:
    def spawn(self, sh, escape, cmd, args, env):
        # deal with the cmd build-in commands which cannot be used in
        # subprocess.Popen
        if cmd == 'del':
            for f in args[1:]:
                try:
                    os.remove(f)
                except Exception as e:
                    print ('Error removing file: ' + e)
                    return -1
            return 0

        import subprocess

        newargs = ' '.join(args[1:])
        cmdline = cmd + " " + newargs

        # Make sure the env is constructed by strings
        _e = dict([(k, str(v)) for k, v in env.items()])

        # Windows(tm) CreateProcess does not use the env passed to it to find
        # the executables. So we have to modify our own PATH to make Popen
        # work.
        old_path = os.environ['PATH']
        os.environ['PATH'] = _e['PATH']

        try:
            proc = subprocess.Popen(cmdline, env=_e, shell=False)
        except Exception as e:
            print ('Error in calling command:' + cmdline.split(' ')[0])
            print ('Exception: ' + os.strerror(e.errno))
            if (os.strerror(e.errno) == "No such file or directory"):
                print ("\nPlease check Toolchains PATH setting.\n")

            return e.errno
        finally:
            os.environ['PATH'] = old_path

        return proc.wait()

# generate cconfig.h file
def GenCconfigFile(env, BuildOptions):
    import osconfig

    if osconfig.COMPILER == 'gcc':
        contents = ''
        if not os.path.isfile('cconfig.h'):
            import gcc
            gcc.GenerateGCCConfig(osconfig)

        # try again
        if os.path.isfile('cconfig.h'):
            f = open('cconfig.h', 'r')
            if f:
                contents = f.read()
                f.close()

                prep = PatchedPreProcessor()
                prep.process_contents(contents)
                options = prep.cpp_namespace

                BuildOptions.update(options)

                # add HAVE_CCONFIG_H definition
                env.AppendUnique(CPPDEFINES = ['HAVE_CCONFIG_H'])

def SetupCompile(env, root_directory, has_libcpu=False, remove_components = [],SDK_LIB=None,SDK_DRIVER=None):
    import osconfig

    global BuildOptions
    global Projects
    global Env
    global Os_Root

    # ===== Add option to SCons =====
    AddOption('--customized-proj',
                      dest = 'customized-proj',
                      action = 'store_true',
                      default = False,
                      help = 'Generate a new code project for current board type, code for other bsps are removed. ' + \
                      'the new code project can be build independently.')
    AddOption('--customized-proj-strip',
                      dest = 'customized-proj-strip',
                      action = 'store_true',
                      default = False,
                      help = 'Generate a new code project for current board type, code for other bsps are removed. ' + \
                      'the new code project can be build independently. useless files are stripped.')
    '''    
    AddOption('--cscope',
                      dest = 'cscope',
                      action = 'store_true',
                      default = False,
                      help = 'Build Cscope cross reference database. Requires cscope installed.')
                      
    AddOption('--clang-analyzer',
                      dest = 'clang-analyzer',
                      action = 'store_true',
                      default = False,
                      help = 'Perform static analyze with Clang-analyzer. ' + \
                           'Requires Clang installed.\n' + \
                           'It is recommended to use with scan-build like this:\n' + \
                           '`scan-build scons --clang-analyzer`\n' + \
                           'If things goes well, scan-build will instruct you to invoke scan-view.')
    
    AddOption('--buildlib',
                      dest = 'buildlib',
                      type = 'string',
                      help = 'building library of a component')
    AddOption('--cleanlib',
                      dest = 'cleanlib',
                      action = 'store_true',
                      default = False,
                      help = 'clean up the library by --buildlib')
    '''
    AddOption('--ide',
                      dest = 'ide',
                      type = 'string',
 #                     help = 'set target project: mdk/mdk4/mdk5/iar/vs/vsc/ua/cdk/ses/makefile/eclipse')
					  help = 'Build a project file for IDEs: mdk/mdk4/mdk5.')
    AddOption('--restore-config',
                dest = 'restore-config',
                action = 'store_true',
                default = False,
                help = 'Generate .config from oneos_config.h.')
    '''
    AddOption('--useconfig',
                dest = 'useconfig',
                type = 'string',
                help = 'make oneos_config.h from config file.')
    '''
    AddOption('--verbose',
                dest = 'verbose',
                action = 'store_true',
                default = False,
                help = 'Print more detailed information during build process.')

    Env = env
    # Os_Root = os.path.abspath(root_directory)
    Os_Root = root_directory

    # make an absolute root directory
    OS_ROOT = Os_Root
    Export('OS_ROOT')

    # set OS_ROOT in ENV
    Env['OS_ROOT'] = Os_Root
    # set BSP_ROOT in ENV
    Env['BSP_ROOT'] = Dir('#').abspath

    sys.path = sys.path + [os.path.join(Os_Root, 'scripts')]

    # {target_name:(CROSS_TOOL, PLATFORM)}
    tgt_dict = {'mdk':('keil', 'armcc'),
                'mdk4':('keil', 'armcc'),
                'mdk5':('keil', 'armcc'),
                'iar':('iar', 'iar'),
                'vs':('msvc', 'cl'),
                'vs2012':('msvc', 'cl'),
                'vsc' : ('gcc', 'gcc'),
                'cb':('keil', 'armcc'),
                'ua':('gcc', 'gcc'),
                'cdk':('gcc', 'gcc'),
                'makefile':('gcc', 'gcc'),
                'eclipse':('gcc', 'gcc'),
                'ses' : ('gcc', 'gcc')}
    tgt_name = GetOption('ide')
    
    if tgt_name:
        # --target will change the toolchain settings which clang-analyzer is
        # depend on
        '''
        if GetOption('clang-analyzer'):
            print ('--clang-analyzer cannot be used with --target')
            sys.exit(1)
        '''
        SetOption('no_exec', 1)
        try:
            osconfig.CROSS_TOOL, osconfig.COMPILER = tgt_dict[tgt_name]
            # replace the 'OS_CC' to 'CROSS_TOOL'
            os.environ['OS_CC'] = osconfig.CROSS_TOOL
            utils.ReloadModule(osconfig)
        except KeyError:
            print ('Unknow target: '+ tgt_name+'. Avaible targets: ' +', '.join(tgt_dict.keys()))
            sys.exit(1)
    '''
    elif (IsDefined('OS_USING_NEWLIB') == False and IsDefined('OS_USING_NOLIBC') == False) \
        and osconfig.COMPILER == 'gcc':
        AddDefined('OS_USING_MINILIBC')
    '''

    # auto change the 'OS_EXEC_PATH' when 'osconfig.COMPILER_PATH' get failed
    if not os.path.exists(osconfig.COMPILER_PATH):
        if 'OS_EXEC_PATH' in os.environ:
            # del the 'OS_EXEC_PATH' and using the 'COMPILER_PATH' setting on osconfig.py
            del os.environ['OS_EXEC_PATH']
            utils.ReloadModule(osconfig)

    # add compability with Keil MDK 4.6 which changes the directory of armcc.exe
    if osconfig.COMPILER == 'armcc':
        if not os.path.isfile(os.path.join(osconfig.COMPILER_PATH, 'armcc.exe')):
            if osconfig.COMPILER_PATH.find('bin40') > 0:
                osconfig.COMPILER_PATH = osconfig.COMPILER_PATH.replace('bin40', 'armcc/bin')
                Env['LINKFLAGS'] = Env['LINKFLAGS'].replace('RV31', 'armcc')

        # reset AR command flags
        env['ARCOM'] = '$AR --create $TARGET $SOURCES'
        env['LIBPREFIX'] = ''
        env['LIBSUFFIX'] = '.lib'
        env['LIBLINKPREFIX'] = ''
        env['LIBLINKSUFFIX'] = '.lib'
        env['LIBDIRPREFIX'] = '--userlibpath '

    elif osconfig.COMPILER == 'iar':
        env['LIBPREFIX'] = ''
        env['LIBSUFFIX'] = '.a'
        env['LIBLINKPREFIX'] = ''
        env['LIBLINKSUFFIX'] = '.a'
        env['LIBDIRPREFIX'] = '--search '

    # patch for win32 spawn
    if env['PLATFORM'] == 'win32':
        win32_spawn = Win32Spawn()
        win32_spawn.env = env
        env['SPAWN'] = win32_spawn.spawn

    if env['PLATFORM'] == 'win32':
        os.environ['PATH'] = osconfig.COMPILER_PATH + ";" + os.environ['PATH']
    else:
        os.environ['PATH'] = osconfig.COMPILER_PATH + ":" + os.environ['PATH']

    if os.getenv('EXEC_BIN_PATH'):
        env.PrependENVPath('PATH', os.getenv('EXEC_BIN_PATH'))
        
    # add program path
    env.PrependENVPath('PATH', osconfig.COMPILER_PATH)
    
    # add oneos_config.h/BSP path into Kernel group
    AddCodeGroup("Kernel", [], [], CPPPATH=[str(Dir('#').abspath)])

    # add library build action
    act = SCons.Action.Action(BuildLibInstallAction, 'Install compiled library... $TARGET')
    bld = Builder(action = act)
    Env.Append(BUILDERS = {'BuildLib': bld})

    # parse oneos_config.h to get used component
    PreProcessor = PatchedPreProcessor()
    f = open('oneos_config.h', 'r')
    contents = f.read()
    f.close()
    PreProcessor.process_contents(contents)
    BuildOptions = PreProcessor.cpp_namespace
    '''
    if GetOption('clang-analyzer'):
        # perform what scan-build does
        env.Replace(
                CC   = 'ccc-analyzer',
                CXX  = 'c++-analyzer',
                # skip as and link
                LINK = 'true',
                AS   = 'true',)
        env["ENV"].update(x for x in os.environ.items() if x[0].startswith("CCC_"))
        # only check, don't compile. ccc-analyzer use CCC_CC as the CC.
        # fsyntax-only will give us some additional warning messages
        env['ENV']['CCC_CC']  = 'clang'
        env.Append(CFLAGS=['-fsyntax-only', '-Wall', '-Wno-invalid-source-encoding'])
        env['ENV']['CCC_CXX'] = 'clang++'
        env.Append(CXXFLAGS=['-fsyntax-only', '-Wall', '-Wno-invalid-source-encoding'])
        # remove the POST_ACTION as it will cause meaningless errors(file not
        # found or something like that).
        osconfig.POST_ACTION = ''
    '''
    # generate cconfig.h file
    GenCconfigFile(env, BuildOptions)

    # auto append '_REENT_SMALL' when using newlib 'nano.specs' option
    if osconfig.COMPILER == 'gcc' and str(env['LINKFLAGS']).find('nano.specs') != -1:
        env.AppendUnique(CPPDEFINES = ['_REENT_SMALL'])

    if GetOption('restore-config'):
        from genconf import genconfig
        genconfig()
        exit(0)

    if env['PLATFORM'] != 'win32':
        AddOption('--menuconfig',
                    dest = 'menuconfig',
                    action = 'store_true',
                    default = False,
                    help = 'make menuconfig for CMCC IOT BSP')
        if GetOption('menuconfig'):
            menuconfig_file_path = os.path.join(OS_ROOT, 'scripts', 'linux-menuconfig', 'menuconfig.py')

            sys.path = sys.path + [os.path.join(OS_ROOT, 'scripts', 'linux-menuconfig')]
            os.system("python %s" % (menuconfig_file_path))
            exit(0)

    '''
    AddOption('--pyconfig',
                dest = 'pyconfig',
                action = 'store_true',
                default = False,
                help = 'make menuconfig for CMCC IOT BSP')
    AddOption('--pyconfig-silent',
                dest = 'pyconfig_silent',
                action = 'store_true',
                default = False,
                help = 'Don`t show pyconfig window')

    if GetOption('pyconfig_silent'):    
        from menuconfig import pyconfig_silent

        pyconfig_silent(Os_Root)
        exit(0)
    elif GetOption('pyconfig'):
        from menuconfig import pyconfig

        pyconfig(Os_Root)
        exit(0)
    '''
    
    '''
    configfn = GetOption('useconfig')
    if configfn:
        from menuconfig import mk_rtconfig
        mk_rtconfig(configfn)
        exit(0)
    '''


    if not GetOption('verbose'):
        # override the default verbose command string
        env.Replace(
            ARCOMSTR = 'AR $TARGET',
            ASCOMSTR = 'AS $TARGET',
            ASPPCOMSTR = 'AS $TARGET',
            CCCOMSTR = 'CC $TARGET',
            CXXCOMSTR = 'CXX $TARGET',
            LINKCOMSTR = 'LINK $TARGET'
        )

    # fix the linker for C++
    if IsDefined('OS_USING_CPLUSPLUS'):
        if env['LINK'].find('gcc') != -1:
            env['LINK'] = env['LINK'].replace('gcc', 'g++')

    # we need to seperate the variant_dir for BSPs and the kernels. BSPs could
    # have their own components etc. If they point to the same folder, SCons
    # would find the wrong source code to compile.
    global out_dir
    out_dir = OS_ROOT+'/out/'+os.path.basename(PresentDir())
    os.makedirs(out_dir, exist_ok=True)
    build_vdir = 'build'
    kernel_vdir = 'build/kernel'
    
    Export('build_vdir')
    Export('kernel_vdir')
    Export('remove_components')
  
    # board build script
    objs = SConscript('SConscript', variant_dir=build_vdir + '/bsp', duplicate=0)

    # else build script
    objs.extend(SConscript(os.path.join(Os_Root, 'SConscript'), variant_dir=build_vdir, duplicate=0))

    # sort group by name
    Projects = sorted(Projects, key = lambda group:group["name"])

    return objs

def PrepareModuleBuilding(env, root_directory, bsp_directory):
    import osconfig

    global BuildOptions
    global Env
    global Os_Root

    # patch for win32 spawn
    if env['PLATFORM'] == 'win32':
        win32_spawn = Win32Spawn()
        win32_spawn.env = env
        env['SPAWN'] = win32_spawn.spawn

    Env = env
    Os_Root = root_directory

    # parse bsp oneos_config.h to get used component
    PreProcessor = PatchedPreProcessor()
    f = open(bsp_directory + '/oneos_config.h', 'r')
    contents = f.read()
    f.close()
    PreProcessor.process_contents(contents)
    BuildOptions = PreProcessor.cpp_namespace

    # add build/clean library option for library checking
    AddOption('--buildlib',
              dest='buildlib',
              type='string',
              help='building library of a component')
    AddOption('--cleanlib',
              dest='cleanlib',
              action='store_true',
              default=False,
              help='clean up the library by --buildlib')

    # add program path
    env.PrependENVPath('PATH', osconfig.COMPILER_PATH)

def GetConfigValue(name):
    assert type(name) == str, 'GetConfigValue: only string parameter is valid'
    try:
        return BuildOptions[name]
    except:
        return ''

def IsDefined(depend):
    building = True
    if type(depend) == type('str'):
        if not depend in BuildOptions or BuildOptions[depend] == 0:
            building = False
        elif BuildOptions[depend] != '':
            return BuildOptions[depend]

        return building

    # for list type depend
    for item in depend:
        if item != '':
            if not item in BuildOptions or BuildOptions[item] == 0:
                building = False

    return building

def LocalOptions(config_filename):
    from SCons.Script import SCons

    # parse wiced_config.h to get used component
    PreProcessor = SCons.cpp.PreProcessor()

    f = open(config_filename, 'r')
    contents = f.read()
    f.close()

    PreProcessor.process_contents(contents)
    local_options = PreProcessor.cpp_namespace

    return local_options

def IsLocalDefined(options, depend):
    building = True
    if type(depend) == type('str'):
        if not depend in options or options[depend] == 0:
            building = False
        elif options[depend] != '':
            return options[depend]

        return building

    # for list type depend
    for item in depend:
        if item != '':
            if not item in options or options[item] == 0:
                building = False

    return building

def AddDefined(option):
    BuildOptions[option] = 1

def MergeGroup(src_group, group):
    src_group['src'] = src_group['src'] + group['src']
    if 'CCFLAGS' in group:
        if 'CCFLAGS' in src_group:
            src_group['CCFLAGS'] = src_group['CCFLAGS'] + group['CCFLAGS']
        else:
            src_group['CCFLAGS'] = group['CCFLAGS']
    if 'CPPPATH' in group:
        if 'CPPPATH' in src_group:
            src_group['CPPPATH'] = src_group['CPPPATH'] + group['CPPPATH']
        else:
            src_group['CPPPATH'] = group['CPPPATH']
    if 'CPPDEFINES' in group:
        if 'CPPDEFINES' in src_group:
            src_group['CPPDEFINES'] = src_group['CPPDEFINES'] + group['CPPDEFINES']
        else:
            src_group['CPPDEFINES'] = group['CPPDEFINES']
    if 'ASFLAGS' in group:
        if 'ASFLAGS' in src_group:
            src_group['ASFLAGS'] = src_group['ASFLAGS'] + group['ASFLAGS']
        else:
            src_group['ASFLAGS'] = group['ASFLAGS']

    # for local CCFLAGS/CPPPATH/CPPDEFINES
    if 'LOCAL_CCFLAGS' in group:
        if 'LOCAL_CCFLAGS' in src_group:
            src_group['LOCAL_CCFLAGS'] = src_group['LOCAL_CCFLAGS'] + group['LOCAL_CCFLAGS']
        else:
            src_group['LOCAL_CCFLAGS'] = group['LOCAL_CCFLAGS']
    if 'LOCAL_CPPPATH' in group:
        if 'LOCAL_CPPPATH' in src_group:
            src_group['LOCAL_CPPPATH'] = src_group['LOCAL_CPPPATH'] + group['LOCAL_CPPPATH']
        else:
            src_group['LOCAL_CPPPATH'] = group['LOCAL_CPPPATH']
    if 'LOCAL_CPPDEFINES' in group:
        if 'LOCAL_CPPDEFINES' in src_group:
            src_group['LOCAL_CPPDEFINES'] = src_group['LOCAL_CPPDEFINES'] + group['LOCAL_CPPDEFINES']
        else:
            src_group['LOCAL_CPPDEFINES'] = group['LOCAL_CPPDEFINES']

    if 'LINKFLAGS' in group:
        if 'LINKFLAGS' in src_group:
            src_group['LINKFLAGS'] = src_group['LINKFLAGS'] + group['LINKFLAGS']
        else:
            src_group['LINKFLAGS'] = group['LINKFLAGS']
    if 'LIBS' in group:
        if 'LIBS' in src_group:
            src_group['LIBS'] = src_group['LIBS'] + group['LIBS']
        else:
            src_group['LIBS'] = group['LIBS']
    if 'LIBPATH' in group:
        if 'LIBPATH' in src_group:
            src_group['LIBPATH'] = src_group['LIBPATH'] + group['LIBPATH']
        else:
            src_group['LIBPATH'] = group['LIBPATH']
    if 'LOCAL_ASFLAGS' in group:
        if 'LOCAL_ASFLAGS' in src_group:
            src_group['LOCAL_ASFLAGS'] = src_group['LOCAL_ASFLAGS'] + group['LOCAL_ASFLAGS']
        else:
            src_group['LOCAL_ASFLAGS'] = group['LOCAL_ASFLAGS']

def AddCodeGroup(name, src, depend, **parameters):
    global Env
    if not IsDefined(depend):
        return []

    # find exist group and get path of group
    group_path = ''
    for g in Projects:
        if g['name'] == name:
            group_path = g['path']
    if group_path == '':
        group_path = PresentDir()

    group = parameters
    group['name'] = name
    group['path'] = group_path
    if type(src) == type([]):
        group['src'] = File(src)
    else:
        group['src'] = src
        
    # for item in group['src']:
        # print(item)
        
    # print('*******************************')
        
    # print(group['src'])

    if 'CCFLAGS' in group:
        Env.AppendUnique(CCFLAGS = group['CCFLAGS'])
    if 'CPPPATH' in group:
        paths = []
        for item in group['CPPPATH']:
            paths.append(os.path.abspath(item))
        group['CPPPATH'] = paths
        Env.AppendUnique(CPPPATH = group['CPPPATH'])
    if 'CPPDEFINES' in group:
        Env.AppendUnique(CPPDEFINES = group['CPPDEFINES'])
    if 'LINKFLAGS' in group:
        Env.AppendUnique(LINKFLAGS = group['LINKFLAGS'])
    if 'ASFLAGS' in group:
        Env.AppendUnique(ASFLAGS = group['ASFLAGS'])
    if 'LOCAL_CPPPATH' in group:
        paths = []
        for item in group['LOCAL_CPPPATH']:
            paths.append(os.path.abspath(item))
        group['LOCAL_CPPPATH'] = paths

    import osconfig
    if osconfig.COMPILER == 'gcc':
        if 'CCFLAGS' in group:
            group['CCFLAGS'] = utils.GCCC99Patch(group['CCFLAGS'])
        if 'LOCAL_CCFLAGS' in group:
            group['LOCAL_CCFLAGS'] = utils.GCCC99Patch(group['LOCAL_CCFLAGS'])
    '''
    # check whether to clean up library
    if GetOption('cleanlib') and os.path.exists(os.path.join(group['path'], GroupLibFullName(name, Env))):
        if group['src'] != []:
            print ('Remove library:'+ GroupLibFullName(name, Env))
            fn = os.path.join(group['path'], GroupLibFullName(name, Env))
            if os.path.exists(fn):
                os.unlink(fn)
    '''
    if 'LIBS' in group:
        Env.AppendUnique(LIBS = group['LIBS'])
    if 'LIBPATH' in group:
        Env.AppendUnique(LIBPATH = group['LIBPATH'])

    #if 'variant_dir' in group:
        
        
    # check whether to build group library
    if 'LIBRARY' in group:
        objs = Env.Library(name, group['src'])
    else:
        # only add source
        objs = group['src']

    # merge group
    for g in Projects:
        if g['name'] == name:
            # merge to this group
            MergeGroup(g, group)
            return objs

    # add a new group
    Projects.append(group)

    return objs

def PresentDir():
    conscript = File('SConscript')
    fn = conscript.rfile()
    name = fn.name
    path = os.path.dirname(fn.abspath)
    return path

PREBUILDING = []
def RegisterPreBuildingAction(act):
    global PREBUILDING
    assert callable(act), 'Could only register callable objects. %s received' % repr(act)
    PREBUILDING.append(act)

def PreBuilding():
    global PREBUILDING
    for a in PREBUILDING:
        a()

def GroupLibName(name, env):
    import osconfig
    if osconfig.COMPILER == 'armcc':
        return name + '_rvds'
    elif osconfig.COMPILER == 'gcc':
        return name + '_gcc'

    return name

def GroupLibFullName(name, env):
    return env['LIBPREFIX'] + GroupLibName(name, env) + env['LIBSUFFIX']

def BuildLibInstallAction(target, source, env):
    '''
    lib_name = GetOption('buildlib')
    
    for Group in Projects:
        if Group['name'] == lib_name:
            lib_name = GroupLibFullName(Group['name'], env)
            dst_name = os.path.join(Group['path'], lib_name)
            print ('Copy '+lib_name+' => ' +dst_name)
            do_copy_file(lib_name, dst_name)
            break
    '''

def StartCompile(target, objects):

    # merge all objects into one list
    def one_list(l):
        lst = []
        for item in l:
            if type(item) == type([]):
                lst += one_list(item)
            else:
                lst.append(item)
        return lst

    # handle local group
    def local_group(group, objects):
        if 'LOCAL_CCFLAGS' in group or 'LOCAL_CPPPATH' in group or 'LOCAL_CPPDEFINES' in group or 'LOCAL_ASFLAGS' in group:
            CCFLAGS = Env.get('CCFLAGS', '') + group.get('LOCAL_CCFLAGS', '')
            CPPPATH = Env.get('CPPPATH', ['']) + group.get('LOCAL_CPPPATH', [''])
            CPPDEFINES = Env.get('CPPDEFINES', ['']) + group.get('LOCAL_CPPDEFINES', [''])
            ASFLAGS = Env.get('ASFLAGS', '') + group.get('LOCAL_ASFLAGS', '')

            for source in group['src']:
                objects.append(Env.Object(source, CCFLAGS = CCFLAGS, ASFLAGS = ASFLAGS,
                    CPPPATH = CPPPATH, CPPDEFINES = CPPDEFINES))

            return True

        return False

    objects = one_list(objects)

    program = None
    lib_name = None
    '''
    # check whether special buildlib option
    lib_name = GetOption('buildlib')
    '''
    if lib_name:
        objects = [] # remove all of objects
        # build library with special component
        for Group in Projects:
            if Group['name'] == lib_name:
                lib_name = GroupLibName(Group['name'], Env)
                if not local_group(Group, objects):
                    objects = Env.Object(Group['src'])

                program = Env.Library(lib_name, objects)

                # add library copy action
                Env.BuildLib(lib_name, program)

                break
    else:
        # remove source files with local flags setting
        for group in Projects:
            if 'LOCAL_CCFLAGS' in group or 'LOCAL_CPPPATH' in group or 'LOCAL_CPPDEFINES' in group:
                for source in group['src']:
                    for obj in objects:
                        if source.abspath == obj.abspath or (len(obj.sources) > 0 and source.abspath == obj.sources[0].abspath):
                            objects.remove(obj)

        # re-add the source files to the objects
        for group in Projects:
            local_group(group, objects)
            
        no_exec = GetOption('no_exec')
        #print("no_exec:%d"% no_exec)
        #if no_exec != 1 :
            #Env.Install(out_dir,)
        program = Env.Program(target, objects)
        '''
        for obj in objects:
            if hasattr(obj, 'abspath') and len(obj.abspath) > 0 :
                print(obj.abspath)
            elif hasattr(obj, 'sources') and len(obj.sources) > 0 and  hasattr(obj.sources, 'abspath') and len(obj.sources[0].abspath) > 0 :
                print(obj.sources[0].abspath)
        '''

    EndBuilding(target, program)

def GenTargetProject(program = None):
    IDE = GetOption('ide')
    print("\nStarting building project: %s ..." % IDE)
    if IDE == 'mdk':
        from keil import MDKProject
        from keil import MDK4Project
        from keil import MDK5Project

        template = os.path.isfile('template.Uv2')
        if template:
            MDKProject('project.Uv2', Projects)
        else:
            template = os.path.isfile('template.uvproj')
            if template:
                MDK4Project('project.uvproj', Projects)
            else:
                template = os.path.isfile('template.uvprojx')
                if template:
                    MDK5Project('project.uvprojx', Projects)
                else:
                    print ('No template project file found.')

    if IDE == 'mdk4':
        from keil import MDK4Project
        MDK4Project('project.uvproj', Projects)

    if IDE == 'mdk5':
        from keil import MDK5Project
        MDK5Project('project.uvprojx', Projects)

    if IDE == 'iar':
        from iar import IARProject
        IARProject('project.ewp', Projects)

    if IDE == 'vs':
        from vs import VSProject
        VSProject('project.vcproj', Projects, program)

    if IDE == 'vs2012':
        from vs2012 import VS2012Project
        VS2012Project('project.vcxproj', Projects, program)

    if IDE == 'cb':
        from codeblocks import CBProject
        CBProject('project.cbp', Projects, program)

    if IDE == 'ua':
        from ua import PrepareUA
        PrepareUA(Projects, Os_Root, str(Dir('#')))

    if IDE == 'vsc':
        from vsc import GenerateVSCode
        GenerateVSCode(Env)

    if IDE == 'cdk':
        from cdk import CDKProject
        CDKProject('project.cdkproj', Projects)

    if IDE == 'ses':
        from ses import SESProject
        SESProject(Env)

    if IDE == 'makefile':
        from makefile import TargetMakefile
        TargetMakefile(Env)

    if IDE == 'eclipse':
        from eclipse import TargetEclipse
        TargetEclipse(Env)
    #print("\nBuilding project finished ! \n")

def EndBuilding(target, program = None):
    import osconfig

    need_exit = False

    Env['target']  = program
    Env['project'] = Projects

    if hasattr(osconfig, 'BSP_LIBRARY_TYPE'):
        Env['bsp_lib_type'] = osconfig.BSP_LIBRARY_TYPE

    if hasattr(osconfig, 'dist_handle'):
        Env['dist_handle'] = osconfig.dist_handle
        
    Env.AddPostAction(target, '\n rm -rf ' + osconfig.RESULT_SUFFIX + ' '+ out_dir + '/*.bin \n')
    Env.AddPostAction(target, osconfig.POST_ACTION + '\n mv *.bin *.map *.' + osconfig.RESULT_SUFFIX + ' '+ out_dir + '\n')
    
    # Add addition clean files
    Clean(target, 'cconfig.h')
    Clean(target, 'rtua.py')
    Clean(target, 'rtua.pyc')

    if GetOption('ide'):
        GenTargetProject(program)

    BSP_ROOT = Dir('#').abspath
    if GetOption('customized-proj') and program != None:
        from mkdist import MkDist
        MkDist(program, BSP_ROOT, Os_Root, Env)
        need_exit = True
    if GetOption('customized-proj-strip') and program != None:
        from mkdist import MkDist_Strip
        MkDist_Strip(program, BSP_ROOT, Os_Root, Env)
        need_exit = True
    '''
    if GetOption('cscope'):
        from cscope import CscopeDatabase
        CscopeDatabase(Projects)
    '''

    if not GetOption('help') and not GetOption('ide'):
        if not os.path.exists(osconfig.COMPILER_PATH):
            print ("Error: the toolchain path (" + osconfig.COMPILER_PATH + ") is not exist, please check 'COMPILER_PATH' in path or osconfig.py.")
            need_exit = True
    
    if need_exit:
        exit(0)

def DeleteGroupFile(objs, name, remove):
    global Projects
    for g in Projects:
        if g['name'] == name:
            DeleteSrcFile(g['src'], remove)
                
    for item in objs:
        if os.path.abspath(remove) == os.path.abspath(item.rstr()):
            objs.remove(item)
        
def DeleteSrcFile(src, remove):
    if not src:
        return

    src_bak = src[:]

    if type(remove) == type('str'):
        if os.path.isabs(remove):
            remove = os.path.relpath(remove, PresentDir())
        remove = os.path.normpath(remove)

        for item in src_bak:
            if type(item) == type('str'):
                item_str = item
            else:
                item_str = item.rstr()

            if os.path.isabs(item_str):
                item_str = os.path.relpath(item_str, PresentDir())
            item_str = os.path.normpath(item_str)

            if item_str == remove:
                src.remove(item)
    else:
        for remove_item in remove:
            remove_str = str(remove_item)
            if os.path.isabs(remove_str):
                remove_str = os.path.relpath(remove_str, PresentDir())
            remove_str = os.path.normpath(remove_str)

            for item in src_bak:
                if type(item) == type('str'):
                    item_str = item
                else:
                    item_str = item.rstr()

                if os.path.isabs(item_str):
                    item_str = os.path.relpath(item_str, PresentDir())
                item_str = os.path.normpath(item_str)

                if item_str == remove_str:
                    src.remove(item)

def GetVersion():
    import SCons.cpp
    import string

    osdef = os.path.join(Os_Root, 'include', 'cmdef.h')

    # parse cmdef.h to get CMCC IOT version
    prepcessor = PatchedPreProcessor()
    f = open(osdef, 'r')
    contents = f.read()
    f.close()
    prepcessor.process_contents(contents)
    def_ns = prepcessor.cpp_namespace

    version = int(filter(lambda ch: ch in '0123456789.', def_ns['CM_VERSION']))
    subversion = int(filter(lambda ch: ch in '0123456789.', def_ns['CM_SUBVERSION']))

    if 'CM_REVISION' in def_ns:
        revision = int(filter(lambda ch: ch in '0123456789.', def_ns['CM_REVISION']))
        return '%d.%d.%d' % (version, subversion, revision)

    return '0.%d.%d' % (version, subversion)

def GlobSubDir(sub_dir, ext_name):
    import os
    import glob

    def glob_source(sub_dir, ext_name):
        list = os.listdir(sub_dir)
        src = glob.glob(os.path.join(sub_dir, ext_name))

        for item in list:
            full_subdir = os.path.join(sub_dir, item)
            if os.path.isdir(full_subdir):
                src += glob_source(full_subdir, ext_name)
        return src

    dst = []
    src = glob_source(sub_dir, ext_name)
    for item in src:
        dst.append(os.path.relpath(item, sub_dir))
    return dst

def PackageSConscript(package):
    from package import BuildPackage

    return BuildPackage(package)
