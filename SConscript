# for module compiling
import os
Import('OS_ROOT')
Import('build_vdir')

objs = []
folders = ["arch", "oneos-kernel", "drivers", "components"]
for folder in folders:
    SConscript_file = os.path.join(OS_ROOT, '{0}/SConscript'.format(folder))
    objs.extend(SConscript(SConscript_file, variant_dir=folder, duplicate=0))

Return('objs')
