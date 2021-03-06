/******************************************************************************
* Copyright (C) 2019, Huada Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC
* components. This software is licensed by HDSC to be adapted only
* for use in systems utilizing HDSC components. HDSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. HDSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer:
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/
/******************************************************************************/
/** \file hc_dac.h
 **
 ** Header file for dac Converter functions
 ** @link DAC Group Some description @endlink
 **
 **   - 2019-04-10      First Version
 **
 ******************************************************************************/
#ifndef __HC_DAC_H__
#define __HC_DAC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "hc_ddl.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \brief ????????????????????????
 ******************************************************************************/ 
typedef enum 
{
    DacDisable  = 0u,         //??????
    DacEnable   = 1u          //??????
}en_en_state_t;

/**
 ******************************************************************************
 ** \brief ??????????????????DAC0?????????????????????         DAC_CR0???BOFF0
 ******************************************************************************/ 
typedef enum 
{
    DacBoffDisable = 1u,
    DacBoffEnable  = 0u
}en_dac_boff_t;

/**
 ******************************************************************************
 ** \brief ??????????????????DAC0??????????????????           DAC_CR0: TEN0
 ******************************************************************************/ 
typedef enum
{
    DacTenDisable = 0u,
    DacTenEnable  = 1u
}en_dac_ten_t;

/**
 ******************************************************************************
 ** \brief DAC0??????????????????                       DAC_CR0: TSEL0
 ******************************************************************************/ 
typedef enum 
{
    DacTim0Tradc = 0u,      //TIM0_TRADC??????
    DacTim1Tradc = 1u,      //TIM1_TRADC??????
    DacTim2Tradc = 2u,      //TIM2_TRADC??????
    DacTim3Tradc = 3u,      //TIM3_TRADC??????
    DacTim4Tradc = 4u,      //TIM4_TRADC??????
    DacTim5Tradc = 5u,      //TIM5_TRADC??????
    DacSwTriger  = 6u,      //????????????
    DacExPortTriger = 7u    //??????????????????
}en_dac_tsel_t;

/**
 ******************************************************************************
 ** \brief DAC0????????????/??????????????????????????????      DAC_CR0: WAVE0
 ******************************************************************************/ 
typedef enum
{
    DacWaveDisable  = 0u,   //???????????????
    DacNoiseEnable  = 1u,   //?????????????????????
    DacTrWaveEnable = 2u    //?????????????????????
}en_dac_wave_t;

/**
 ******************************************************************************
 ** \brief DACx????????????/???????????????                DAC_CR0: MAMP0 & MAMP1
 ******************************************************************************/
typedef enum
{
    DacMemp01   = 0u,
    DacMenp03   = 1u,
    DacMenp07   = 2u,
    DacMenp15   = 3u,
    DacMenp31   = 4u,
    DacMenp63   = 5u,
    DacMenp127  = 6u,
    DacMenp255  = 7u,
    DacMenp511  = 8u,
    DacMenp1023 = 9u,
    DacMenp2047 = 10u,
    DacMenp4095 = 11u
}en_dac_mamp_t;

/**
 ******************************************************************************
 ** \brief DACx??????DMA???????????????                 DAC_CR0: DMAEN0 & DMAEN1
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief DACx??????DMA???????????????????????????         DAC_CR0: DMAUDRIE0 & DMAUDRIE1
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief DACx??????????????????                       DAC_CR0: SREF0 & SREF1
 ******************************************************************************/
typedef enum
{
    DacVoltage1V5   = 0u,   //??????1.5V
    DacVoltage2V5   = 1u,   //??????2.5V
    DacVoltageExRef = 2u,   //??????????????????ExRef(PB01)
    DacVoltageAvcc  = 3u    //AVCC??????
}en_dac_sref_t;

/**
 ******************************************************************************
 ** \brief DACx?????????????????????DAC_ETRS              DAC_ETRS
 ******************************************************************************/
typedef enum
{
    DacPortTrigPA9 = 0u,      //???????????????PA9
    DacPortTrigPB9 = 1u,      //???????????????PB9
    DacPortTrigPC9 = 2u,      //???????????????PC9
    DacPortTrigPD9 = 3u,      //???????????????PD9
    DacPortTrigPE9 = 4u,      //???????????????PE9
    DacPortTrigPF9 = 5u       //???????????????PF9
}en_port_trig_t;

/**
 ******************************************************************************
 ** \brief ????????????              
 ******************************************************************************/
typedef enum
{
    DacRightAlign = 0u,        //?????????
    DacLeftAlign  = 1u         //?????????
}en_align_t;

/**
 ******************************************************************************
 ** \brief ????????????          
 ******************************************************************************/
typedef enum
{
    DacBit8  = 0u,               //8???
    DacBit12 = 1u                //12???
}en_bitno_t;

/**
 ******************************************************************************
 ** \brief ????????????????????????         
 ******************************************************************************/
typedef struct
{
    en_dac_boff_t  boff_t;
    en_dac_ten_t   ten_t;
    en_dac_tsel_t  tsel_t;
    en_dac_wave_t  wave_t;
    en_dac_mamp_t  mamp_t;
    en_dac_sref_t  sref_t;
    en_port_trig_t port_trig_t;
    en_align_t     align;
    uint16_t       dhr12;
    uint8_t        dhr8;
}stc_dac_cfg_t;

///< DAC0 ??????API
extern void Dac0_Init(stc_dac_cfg_t* DAC_InitStruct);
extern void Dac0_SetChannelData(en_align_t DAC_Align, en_bitno_t DAC_Bit, uint16_t Data);
extern uint16_t Dac0_GetDataOutputValue(void);
extern void Dac0_DmaCmd(boolean_t NewState);
extern void Dac0_DmaITCfg(boolean_t NewState);
extern boolean_t Dac0_GetITStatus(void);
extern void Dac0_Cmd(boolean_t NewState);
extern void Dac0_SoftwareTriggerCmd(void);


///< DAC1 ??????API
extern void Dac1_Init(stc_dac_cfg_t* DAC_InitStruct);
extern void Dac1_SetChannelData(en_align_t DAC_Align, en_bitno_t DAC_Bit, uint16_t Data);
extern uint16_t Dac1_GetDataOutputValue(void);
extern void Dac1_DmaCmd(boolean_t NewState);
extern void Dac1_DmaITCfg(boolean_t NewState);
extern boolean_t Dac1_GetITStatus(void);
extern void Dac1_Cmd(boolean_t NewState);
extern void Dac1_SoftwareTriggerCmd(void);

#ifdef __cplusplus
}
#endif
#endif //__HC_DAC_H__

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
