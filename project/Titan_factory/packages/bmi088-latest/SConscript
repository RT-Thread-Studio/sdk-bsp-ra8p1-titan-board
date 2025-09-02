# RT-Thread building script for bridge

from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

src += ['src/bmi088.c']

# add bmi088 src files.
if GetDepend('PKG_BMI088_USING_SENSOR_V1'):
    src += ['src/sensor_intf_bmi088.c']

if GetDepend('PKG_BMI088_USING_SAMPLE'):
    src += ['samples/bmi088_sample.c']

# add bmi088 include path.
path  = [cwd, cwd + '/inc']

# add src and include to group.
group = DefineGroup('bmi088', src, depend = ['PKG_USING_BMI088'], CPPPATH = path)

Return('group')
