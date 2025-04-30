/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2018, 2022 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_TAUD0_0.c
* Component Version: 1.7.0
* Device(s)        : R7F701684
* Description      : This file implements device driver for Config_TAUD0_0.
***********************************************************************************************************************/
/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_TAUD0_0.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint32_t g_cg_sync_read;
volatile uint32_t  g_taud0_ch0_width;
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_TAUD0_0_Create
* Description  : This function initializes the TAUD00 channel
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAUD0_0_Create(void)
{
    /* Disable channel 0 counter operation */
    TAUD0.TT |= _TAUD_CHANNEL0_COUNTER_STOP;
    /* Disable INTTAUD0I0 operation and clear request */
    INTC1.ICTAUD0I0.BIT.MKTAUD0I0 = _INT_PROCESSING_DISABLED;
    INTC1.ICTAUD0I0.BIT.RFTAUD0I0 = _INT_REQUEST_NOT_OCCUR;
    /* Set INTTAUD0I0 setting */
    INTC1.ICTAUD0I0.BIT.TBTAUD0I0 = _INT_TABLE_VECTOR;
    INTC1.ICTAUD0I0.UINT16 &= _INT_PRIORITY_LOWEST;
    TAUD0.TPS &= _TAUD_CK0_PRS_CLEAR;
    TAUD0.TPS |= _TAUD_CK0_PRE_PCLK_0;
    /* Set channel 0 setting */
    TAUD0.CMOR0 = _TAUD_SELECTION_CK0 | _TAUD_COUNT_CLOCK_PCLK | _TAUD_INDEPENDENT_CHANNEL | 
                  _TAUD_START_TRIGGER_VALID_EDGE | _TAUD_OVERFLOW_AUTO_CLEAR | _TAUD_CAPTURE_MODE | 
                  _TAUD_START_INT_NOT_GENERATED;
    /* Set compare match register */
    TAUD0.CMUR0 = _TAUD_INPUT_EDGE_BOTH_MEASURE_LOW;
    /* Set output mode setting */
    TAUD0.TOE &= _TAUD_CHANNEL0_DISABLES_OUTPUT_MODE;
    /* Synchronization processing */
    g_cg_sync_read = TAUD0.TPS;
    __syncp();

    /* Set TAUD0I0 pin */
    PORT.PIBC9 &= _PORT_CLEAR_BIT0;
    PORT.PBDC9 &= _PORT_CLEAR_BIT0;
    PORT.PM9 |= _PORT_SET_BIT0;
    PORT.PMC9 &= _PORT_CLEAR_BIT0;
    PORT.PFC9 |= _PORT_SET_BIT0;
    PORT.PFCE9 &= _PORT_CLEAR_BIT0;
    PORT.PFCAE9 &= _PORT_CLEAR_BIT0;
    PORT.PMC9 |= _PORT_SET_BIT0;
    DNF.ATAUD0IENL.BIT.ATAUD0IENL0 = _DIGITAL_FILTER_ENABLED;
    DNF.ATAUD0ICTL = _DIGITAL_FILTER_SAMPLING_NUMBER_5 | _DIGITAL_FILTER_SAMPLING_CLOCK_128;

    R_Config_TAUD0_0_Create_UserInit();
}

/***********************************************************************************************************************
* Function Name: R_Config_TAUD0_0_Start
* Description  : This function starts the TAUD00 channel counter
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAUD0_0_Start(void)
{
    /* Clear INTTAUD0I0 request and enable operation */
    INTC1.ICTAUD0I0.BIT.RFTAUD0I0 = _INT_REQUEST_NOT_OCCUR;
    INTC1.ICTAUD0I0.BIT.MKTAUD0I0 = _INT_PROCESSING_ENABLED;
    /* Enable channel 0 counter operation */
    TAUD0.TS |= _TAUD_CHANNEL0_COUNTER_START;
}

/***********************************************************************************************************************
* Function Name: R_Config_TAUD0_0_Stop
* Description  : This function stop the TAUD00 channel counter
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAUD0_0_Stop(void)
{
    /* Disable channel 0 counter operation */
    TAUD0.TT |= _TAUD_CHANNEL0_COUNTER_STOP;
    /* Disable INTTAUD0I0 operation and clear request */
    INTC1.ICTAUD0I0.BIT.MKTAUD0I0 = _INT_PROCESSING_DISABLED;
    INTC1.ICTAUD0I0.BIT.RFTAUD0I0 = _INT_REQUEST_NOT_OCCUR;
    /* Synchronization processing */
    g_cg_sync_read = TAUD0.TT;
    __syncp();
}

/***********************************************************************************************************************
* Function Name: R_Config_TAUD0_0_Get_PulseWidth
* Description  : This function measures TAUD00 input pulse width
* Arguments    : width -
*                    Input pulse width
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAUD0_0_Get_PulseWidth(uint32_t * const width)
{
    *width = g_taud0_ch0_width;
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
