import sys
import glob  
import os.path 
import re
from build_tools import *

def gen_hk32_link_file_keil(prj_path, app_addr, app_size, ram_addr, ram_size):
    link_sct = prj_path + "/board/linker_scripts/link.sct"
    
    if not os.path.exists(link_sct):
        return

    sss = ''
    f1 = open(link_sct, 'r')
    
    for ss in f1.readlines():
        if "LR_IROM1" in ss:
            ss = "LR_IROM1 %s %s  {    ; load region size_region\n" % (app_addr, app_size)
            
        if "ER_IROM1" in ss:
            ss = "  ER_IROM1 %s %s  {  ; load address = execution address\n" % (app_addr, app_size)
            
        if "RW_IRAM1" in ss:
            ss = "  RW_IRAM1 %s %s  {  ; RW data\n" % (ram_addr, ram_size)
    
        sss += ss
    
    f1.close()
    
    f1 = open(link_sct, 'w+')
    f1.write(sss)
    f1.close()
    
def gen_hk32_link_file(prj_path):
    oneos_config_h  = prj_path + "/oneos_config.h"
    
    if not os.path.exists(oneos_config_h):
        return
    
    app_addr = 0x12345678
    app_size = 0x12345678
    ram_addr = 0x12345678
    ram_size = 0x12345678
    
    with open(oneos_config_h, 'r+') as f1:
        for ss in f1.readlines():
            if "#define BSP_TEXT_SECTION_ADDR " in ss:
                app_addr = ss.split()[2]
                
            if "#define BSP_TEXT_SECTION_SIZE " in ss:
                app_size = ss.split()[2]
                
            if "#define BSP_DATA_SECTION_ADDR " in ss:
                ram_addr = ss.split()[2]
                
            if "#define BSP_DATA_SECTION_SIZE " in ss:
                ram_size = ss.split()[2]
    
    if app_addr == 0x12345678 or app_size == 0x12345678\
    or ram_addr == 0x12345678 or ram_size == 0x12345678:
        return
    
    print("app_addr: %s" % app_addr)
    print("app_size: %s" % app_size)
    print("ram_addr: %s" % ram_addr)
    print("ram_size: %s" % ram_size)
    
    gen_hk32_link_file_keil(prj_path, app_addr, app_size, ram_addr, ram_size)
    
def prebuild(prj_path, bsp_path = '/board/CubeMX_Config/Src/'):
    print("project " + prj_path)
    gen_hk32_link_file(prj_path)
