import sys
import glob  
import os.path 
import re
from build_tools import *

def gen_nxp_devices_file(prj_path, bsp_path):
    source = prj_path + "/" + bsp_path + "/peripherals.h"
    source_new = prj_path + "/" + bsp_path + "/peripherals_new.h"
    target = prj_path + "/board/devices.c"
    
    f1 = open(source, 'r+')
    f2 = open(source_new, 'w+')
    f3 = open(target, 'w+')
    
    for ss_copy in f1.readlines():
        f2.write(ss_copy)
    f1.seek(0)  
    
    AddDefined('HAL_GPIO_MODULE_ENABLED')
    #AddDefined('HAL_FLASH_MODULE_ENABLED')
    for ss in f1.readlines():
        index_start = ss.find('#include "fsl_', 0)
        if index_start >= 0:
            index_end = ss.find('.h', 0)
            instance_type_lowername = ss[14:index_end]
            instance_type_upername = str(instance_type_lowername.upper())
            key = "HAL_" + instance_type_upername + "_MODULE_ENABLED"
            #print(key)
            AddDefined(key)
            
            #f2.write('#define HAL_USED_' + instance_type_upername + '\n')
            f2.seek(0)
            instance_lowername_handle = 'OS_NULL' 
            instance_lowername_struct = 'OS_NULL'
            instance_lowername_config = 'OS_NULL'
            instance_lowername_dma_handle = 'OS_NULL'
            instance_lowername_dma_handle_name = 'OS_NULL'
            driver_type = ' '
            instance_lowername = ' '
            instance_upername = ' '
            for ss_hal in f2.readlines():
                index_end = ss_hal.find('_PERIPHERAL ((', 0)
                index_start = ss_hal.find('#define ' + instance_type_upername, 0)
                if index_end > 0 and index_start >= 0:
                    instance_upername = ss_hal[index_start+8:index_end]
                    instance_lowername = str(instance_upername.lower())
                    index_type_start = ss_hal.find('((', 0)
                    index_type_end = ss_hal.find(' *', 0)
                    driver_type = ss_hal[index_type_start+2:index_type_end]
                    instance_include = '#include "drv_' + instance_type_lowername + '.h"\n'
                    f3.write(instance_include)
                    if driver_type == 'USART_Type':
                        instance_lowername_struct = 'const hal_' + instance_type_lowername+ '_handle_t '
                        instance_lowername_handle = instance_upername + '_PERIPHERAL'
                        instance_lowername_config = '&' + instance_upername + '_config'
                        instance_lowername_dma_handle_name = '&' + instance_upername + '_' + instance_type_upername + '_DMA_Handle'
                
                if instance_lowername_dma_handle_name != 'OS_NULL':        
                    index_find_dma = ss_hal.find(instance_lowername_dma_handle_name, 0)
                    if index_find_dma >= 0:
                        instance_lowername_dma_handle = instance_lowername_dma_handle_name
                
                index_find_end = ss_hal.find('/* _PERIPHERALS_H_ */', 0)  
                if index_find_end >= 0:
                    if instance_lowername_handle != 'OS_NULL':
                        f3.write(instance_lowername_struct + instance_lowername + '_handle = {' + instance_lowername_handle + ', ' + \
                        instance_lowername_config + ', ' + instance_lowername_dma_handle_name + '};\n')
                        f3.write('OS_HALL_DEVICE_DEFINE("' + driver_type + '", "' + instance_lowername + '", ' + instance_lowername + '_handle);\n\n')
                    
    f1.close()
    f2.close()
    os.remove(source_new)
    f3.close()
    
def prebuild(prj_path, bsp_path = '/board/board/'):
    print("project " + prj_path)
    gen_nxp_devices_file(prj_path, bsp_path)
    