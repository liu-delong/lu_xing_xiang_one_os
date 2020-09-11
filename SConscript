# for module compiling
import os
Import('OS_ROOT')
Import('build_vdir')


objs = []
libs = ["kernel", "arch", "common", "components", "drivers", "thirdparty", "libc", "osal"]
for com in libs:
    SConscript_file = os.path.join(OS_ROOT, f'{com}/SConscript')
    objs.extend(SConscript(SConscript_file, variant_dir=f'{com}', duplicate=0))

Return('objs')
