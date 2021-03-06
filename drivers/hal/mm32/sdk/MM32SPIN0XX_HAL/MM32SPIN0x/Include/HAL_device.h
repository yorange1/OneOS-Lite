/**************************************************************************//**
* @file HAL_device.h
* @brief CMSIS Cortex-M Peripheral Access Layer for MindMotion
*        microcontroller devices
*
* This is a convenience header file for defining the part number on the
* build command line, instead of specifying the part specific header file.
*
* Example: Add "-MM32SPIN0x" to your build options, to define part
*          Add "#include "HAL_device.h" to your source files
*
*
* @version 1.0.2
*
*
*****************************************************************************/

#ifndef __HAL_device_H
#define __HAL_device_H
#ifndef MM32SPIN0X

#define  MM32SPIN0X

#endif
 
#if defined MM32SPIN0X
#include "MM32SPIN0x.h"
#else
#error "HAL_device.h: PART NUMBER undefined"
#endif

#endif /* __HAL_device_H */
/*-------------------------(C) COPYRIGHT 2017 MindMotion ----------------------*/
