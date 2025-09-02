# drivers/sensors/ist8310/SConscript

from building import *

cwd = GetCurrentDir()
src = Glob('src/*.c')

if GetDepend(['PKG_USING_IST8310_SAMPLE']):
    src += Glob('sample/*.c')

CPPDEFINES = []
CPPPATH = [os.path.join(cwd, 'inc')]

if GetDepend(['PKG_USING_IST8310']):
    group = DefineGroup('IST8310', src, depend = ['PKG_USING_IST8310'], 
                      CPPDEFINES = CPPDEFINES, CPPPATH = CPPPATH)
    Return('group')