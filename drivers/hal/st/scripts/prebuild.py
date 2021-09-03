import sys
import glob  
import os.path 
import re
from build_tools import *

def gen_stm32_bsp_file(prj_path, bsp_path):
    source = prj_path + "/" + bsp_path + "/main.c"
    target = prj_path + "/" + bsp_path + "/bsp.c"
     
    f1 = open(source, 'r+')
    f2 = open(target, 'w+')

    defined_sdram = False
    for ss in f1.readlines():
        if ss.find("SDRAM_HandleTypeDef", 0) != -1:
            defined_sdram = True
            
        ss = ss.replace("#include \"main.h\"", 
                        "#include \"main.h\"\n"
                      + "#include <oneos_config.h>")
            
        ss = ss.replace("int main(void)", "int hardware_init(void)")
        ss = ss.replace("/* USER CODE END WHILE */", "/* USER CODE END WHILE */\n  return 0;")
        ss = ss.replace("if (HAL_ETH_Init", "if (0 && HAL_ETH_Init")
        if defined_sdram:
            ss = ss.replace("  MX_FMC_Init();", 
                            "  MX_FMC_Init();\n"
                          + "  void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram);\n"
                          + "  SDRAM_Initialization_Sequence(&hsdram1);")
        
        ss = ss.replace("  SystemClock_Config();", 
                        "#ifndef DEFAULT_SYSTEM_CLOCK_CONFIG\n"
                      + "  SystemClock_Config();\n"
                      + "#endif")
    
        f2.write(ss)
        
    f1.close()
    f2.close()

def gen_stm32_it_file(prj_path, bsp_path):
    source = glob.glob(prj_path + "/" + bsp_path + "stm32*_it.c")[0]
    target = source.split('_it.c')[0] + "_it_bsp.c"
    
    file = open(source, 'r+')
    
    target_ss = ''

    lpuart1_fix_status = 0
    for ss in file.readlines():
        if "USER CODE END LPUART1_IRQn 0" in ss:
            lpuart1_fix_status = 1
    
        if "HAL_UART_IRQHandler(&huart1);" in ss and lpuart1_fix_status == 1:
            lpuart1_fix_status = 2
            ss = ss.replace("huart1", "hlpuart1")
            
        if "USER CODE BEGIN LPUART1_IRQn 1" in ss:
            lpuart1_fix_status = 3
    
        ss = ss.replace('/* USER CODE END EV */', \
                        '/* USER CODE END EV */\n'\
                        '#ifdef HAL_UART_MODULE_ENABLED\n'\
                        'int HAL_USART_IDLE_PROCESS(UART_HandleTypeDef *huart);\n'\
                        '#endif')
        
        if 'HAL_UART_IRQHandler(' in ss:
            huart = ss.split('&')[1].split(')')[0]
            ss = '  if (HAL_USART_IDLE_PROCESS(&%s))\n'\
                 '    return;\n'\
                 '  \n'\
                 '  HAL_UART_IRQHandler(&%s);\n' % (huart, huart)
    
        ss = ss.replace("void SDMMC1_IRQHandler(void)", "void SDMMC1_IRQHandler_remove(void)")
        ss = ss.replace("void SDMMC2_IRQHandler(void)", "void SDMMC2_IRQHandler_remove(void)")
        ss = ss.replace("void SDIO_IRQHandler(void)", "void SDIO_IRQHandler_remove(void)")
        
        target_ss += ss
        
    file.close()
        
    file = open(target, 'w+')
    file.write(target_ss)
    file.close()
    
def gen_stm32_devices_file(prj_path, bsp_path):
    for name in glob.glob(prj_path + "/" + bsp_path + '*msp.c'):
        name = os.path.basename(name)
        print(name)
    
    source = prj_path + "/" + bsp_path + "/main.c"
    target = prj_path + "/board/peripherals.c"
    msp = prj_path + "/" + bsp_path + "/" + name
    
    f1 = open(source, 'r+')
    f2 = open(target, 'w+')
    f3 = open(msp, 'r+')
    
    device_type_list = [
            'ADC_HandleTypeDef',
            'CAN_HandleTypeDef',
            'CEC_HandleTypeDef',
            'CRC_HandleTypeDef',
            'CRYP_HandleTypeDef',
            'DAC_HandleTypeDef',
            'DCMI_HandleTypeDef',
            'DFSDM_Channel_HandleTypeDef',
            'DFSDM_Filter_HandleTypeDef',
            'DMA_HandleTypeDef',
            'DMA2D_HandleTypeDef',
            'DSI_HandleTypeDef',
            'ETH_HandleTypeDef',
            'EXTI_HandleTypeDef',
            'HASH_HandleTypeDef',
            'HCD_HandleTypeDef',
            'I2C_HandleTypeDef',
            'I2S_HandleTypeDef',
            'IRDA_HandleTypeDef',
            'IWDG_HandleTypeDef',
            'JPEG_HandleTypeDef',
            'LPTIM_HandleTypeDef',
            'LTDC_HandleTypeDef',
            'MDIOS_HandleTypeDef',
            'MMC_HandleTypeDef',
            'NAND_HandleTypeDef',
            'NOR_HandleTypeDef',
            'PCD_HandleTypeDef',
            'QSPI_HandleTypeDef',
            'RNG_HandleTypeDef',
            'RTC_HandleTypeDef',
            'SAI_HandleTypeDef',
            'SD_HandleTypeDef',
            'SDRAM_HandleTypeDef',
            'SMARTCARD_HandleTypeDef',
            'SMBUS_HandleTypeDef',
            'SPDIFRX_HandleTypeDef',
            'SPI_HandleTypeDef',
            'SRAM_HandleTypeDef',
            'TIM_HandleTypeDef',
            'UART_HandleTypeDef',
            'USART_HandleTypeDef',
            'WWDG_HandleTypeDef',
            'HRTIM_HandleTypeDef',
            ]

    AddDefined('HAL_GPIO_MODULE_ENABLED')
    AddDefined('HAL_FLASH_MODULE_ENABLED')
    for ss in f1.readlines():
        for device_type in device_type_list:
            index = ss.find(device_type, 0)
            if index != 0:
                continue
            index1 = ss.find(';', 0)
            instance = ss[len(device_type)+2:index1]
            f2.write('extern ' + ss)
                
            instance_NAME = str(instance.upper())

            index_type_name = ss.find('_HandleTypeDef', 0)
            type_name = ss[index:index_type_name]
            type_NAME = str(type_name.upper())
            key = "HAL_" + type_NAME + "_MODULE_ENABLED"
            #print(key)
            AddDefined(key)
                
            if device_type == 'I2C_HandleTypeDef':
                index2 = -1
                index3 = -1
                index4 = -1
                gpio_pin = ['0x00','0x00']
                f3 = open(msp, 'r+')
                for gpio in f3.readlines(): 
                    index = gpio.find(instance_NAME + ' GPIO Configuration', 0)        
                    if index > 0:
                        index2 = index
                        
                    index3 = gpio.find(' P', 0)
                    index_SCL = gpio.find('_SCL', 0)
                    index_SDA = gpio.find('_SDA', 0)
                    if index2 != -1 and index3 != -1:
                        gpio_type = gpio[index3+2:index3+3]
                        gpio_pin_byte0 = gpio[index3+3:index3+4]
                        gpio_pin_byte1 = gpio[index3+4:index3+5]
                        if gpio_pin_byte1 == ' ':
                            gpio_num = (ord(gpio_type) - ord('A'))*16 + ord(gpio_pin_byte0)-ord('0')
                        else:
                            gpio_num = (ord(gpio_type) - ord('A'))*16 + (ord(gpio_pin_byte0)-ord('0'))*10 + ord(gpio_pin_byte1)-ord('0')
                        
                        if (index_SCL > 0): 
                            gpio_pin[0] = hex(gpio_num)
                            index_SCL = -1
                        if (index_SDA > 0):
                            gpio_pin[1] = hex(gpio_num)
                            index_SDA = -1
                        continue
                          
                    index4 = gpio.find('*/', 0)
                    flag_fined_pin = 0
                    if index4 != -1 and index2 != -1:
                        index2 = -1
                        flag_fined_pin = 1
                        instance_intercept = instance[0:3]
                        f2.write('struct stm32_' + instance_intercept + '_info ' + instance + '_info = {.instance = &h' + instance + ', ')
                        f2.write('.scl = ' + gpio_pin[0] + ', ')
                        f2.write('.sda = ' + gpio_pin[1] + '};\n')
                        f3.close()
                        break
                if flag_fined_pin == 1:
                    f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '", "hard_' + instance + '", ' + instance + "_info);\n\n")
            else:  
                f2.write('OS_HAL_DEVICE_DEFINE("' + device_type + '", "' + instance + '", h' + instance + ');\n\n')
            
    f1.close()
    f2.close()

def gen_stm32_link_file_gcc(prj_path, app_addr, app_size, ram_addr, ram_size):
    link_sct = prj_path + "/board/linker_scripts/link.lds"
    
    if not os.path.exists(link_sct):
        return
        
    sss = ''
    f1 = open(link_sct, 'r')
    
    for ss in f1.readlines():
        if "ROM" in ss and "ORIGIN" in ss and "LENGTH" in ss:
            ss = "    ROM  (rx) : ORIGIN = %s, LENGTH = %s /* %s flash */\n" % (app_addr, app_size, app_size)
            
        if "RAM1" in ss and "ORIGIN" in ss and "LENGTH" in ss:
            ss = "    RAM1 (rx) : ORIGIN = %s, LENGTH = %s /* %s sram */\n" % (ram_addr, ram_size, ram_size)
    
        sss += ss
    
    f1.close()
    
    f1 = open(link_sct, 'w+')
    f1.write(sss)
    f1.close()

def gen_stm32_link_file_keil(prj_path, app_addr, app_size, ram_addr, ram_size):
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

def gen_stm32_link_file_iar(prj_path, app_addr, app_size, ram_addr, ram_size):
    link_sct = prj_path + "/board/linker_scripts/link.icf"
    
    if not os.path.exists(link_sct):
        return
    
def gen_stm32_link_file(prj_path):
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
    
    gen_stm32_link_file_gcc(prj_path, app_addr, app_size, ram_addr, ram_size)
    gen_stm32_link_file_keil(prj_path, app_addr, app_size, ram_addr, ram_size)
    gen_stm32_link_file_iar(prj_path, app_addr, app_size, ram_addr, ram_size)
    
def prebuild(prj_path, bsp_path = '/board/CubeMX_Config/Src/'):
    print("project " + prj_path)
    gen_stm32_bsp_file(prj_path, bsp_path)
    gen_stm32_devices_file(prj_path, bsp_path)
    gen_stm32_it_file(prj_path, bsp_path)
    gen_stm32_link_file(prj_path)
