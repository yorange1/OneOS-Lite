import sys
import glob  
import os.path 
import re
from build_tools import *

def gen_gd32_link_file(prj_path):
    board_h  = prj_path + "/board/board.h"
    link_sct = prj_path + "/board/linker_scripts/link.sct"
    
    if IsDefined('OS_USE_BOOTLOADER'):
        print("defined OS_USE_BOOTLOADER")
    else:
        print("not defined OS_USE_BOOTLOADER")
    
    app_addr = 0x12345678
    app_size = 0x12345678
    
    f1 = open(board_h, 'r+')
    
    bootloader_defined = False
    for ss in f1.readlines():
        if ("#ifdef" in ss and "OS_USE_BOOTLOADER" in ss):
            bootloader_defined = True
            
        if bootloader_defined == True and "#else" in ss:
            bootloader_defined = False
    
        if IsDefined('OS_USE_BOOTLOADER') and bootloader_defined == True:
            if "GD32_APP_ADDR" in ss:
                app_addr = ss.split()[2]
        
            if "GD32_APP_SIZE" in ss:
                app_size = ss.split()[2]
            
        if not IsDefined('OS_USE_BOOTLOADER') and bootloader_defined == False:
            if "GD32_APP_ADDR" in ss:
                app_addr = ss.split()[2]
        
            if "GD32_APP_SIZE" in ss:
                app_size = ss.split()[2]
    
    f1.close()
    
    if app_addr == 0x12345678 or app_size == 0x12345678:
        return
    
    print("app_addr: %s" % app_addr)
    print("app_size: %s" % app_size)
    
    sss = ''
    f1 = open(link_sct, 'r')
    
    for ss in f1.readlines():
        if "LR_IROM1" in ss:
            ss = "LR_IROM1 %s %s  {    ; load region size_region\n" % (app_addr, app_size)
            
        if "ER_IROM1" in ss:
            ss = "  ER_IROM1 %s %s  {  ; load address = execution address\n" % (app_addr, app_size)
    
        sss += ss
    
    f1.close()
    
    f1 = open(link_sct, 'w+')
    f1.write(sss)
    f1.close()
    
def prebuild(prj_path, bsp_path = '/board/CubeMX_Config/Src/'):
    print("project " + prj_path)
    gen_gd32_link_file(prj_path)
    