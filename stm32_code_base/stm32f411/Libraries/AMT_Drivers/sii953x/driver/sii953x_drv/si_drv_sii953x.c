//***************************************************************************
//!file     si_drv_sii9535.c
//!brief    Sii9535 driver.
//!brief    Driver layer internal file
//
//          The Sii9535 driver contains all functions data, and constants
//          common to all modules at the next higher, component driver level.
//          It can be thought of as a wrapper that ties all of the individual
//          Sii9535 functions together to create the Sii9535 device.
//
//          This module is also the interface from the Sii9535 to the
//          physical board environment that it is mounted on.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/


#include "sk_application.h"
#include "si_drv_internal.h"
#include "si_osal_timer.h"
#include "si_drv_device.h"
#if INC_CBUS
#include "si_drv_cbus_internal.h"
#endif
#if INC_CEC
#include "si_drv_cec_config.h"
#include "si_drv_cpi_internal.h"
#endif
#include "si_tx_component.h"
#include "si_drv_tpi_system.h"
#include "si_regs_tpi953x.h"

#include "si_drv_msw_internal.h"
#include "si_drv_audio.h"
#if INC_IPV
#include "si_drv_ipv_internal.h"
#endif
#if INC_OSD
#include "si_drv_osd_internal.h"
#endif
#include "si_regs_rxedid953x.h"
#include "si_drv_repeater.h"

#include "si_gpio.h"
#include "si_drv_gpio_internal.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif 


//------------------------------------------------------------------------------
// Device Component Register initialization list
//------------------------------------------------------------------------------

static uint16_t initRegsList [] =
{
        REG_MISC_CTRL0,             0xCE,
        REG_MISC_CTRL1,             0x0A,
        REG_PAUTH_INV_CTRL,         0x00,       // No clock inversion in pipes and Rx ports
        REG_PA_CONFIG_1,            0x59,       // Enable MHL 1x control
        REG_DPLL_MULTZONE_RDLY1,    0x60,       // Set Rx Zone Control Threshold
        REG_TMDST_TXDAT1,           0x60,       // Set Rx Zone Control Threshold

        REG_PAUTH_MHL_IEEE_ID1,     0x1D,   // MHL 3D VSIF IEEE ID
        REG_PAUTH_MHL_IEEE_ID2,     0xA6,
        REG_PAUTH_MHL_IEEE_ID3,     0x7C,

        REG_PAUTH_MHL3D_CFG1,       0x19,   // MHL to HDMI conversion configuration
        REG_PAUTH_MHL3D_CFG2,       0x81,   // MHL 3D VSIF Header

        REG_3D_MARKER_CTRL,         0x49,
//        REG_A0_EQ_DATA0,            0x00,
//        REG_A0_EQ_DATA1,            0x20,
//        REG_A0_EQ_DATA2,            0x40,
//        REG_A0_EQ_DATA3,            0x70,
//        REG_A0_EQ_DATA4,            0x23,
//        REG_A0_EQ_DATA5,            0x43,
//        REG_A0_EQ_DATA6,            0x53,
//        REG_A0_EQ_DATA7,            0x20,

        REG_A1_EQ_DATA4,            0x23,      // REG_A1_EQ_DATA0-3 are adjusted dynamically
        REG_A1_EQ_DATA5,            0x43,      // depending on the matrix switch mode enable status
        REG_A1_EQ_DATA6,            0x53,      // in Matrix Switch Driver
        REG_A1_EQ_DATA7,            0x20,

        REG_CLR_PACKET_BUFFER,      ( BIT_VSI_CLR_EN       //Enable to clear the VSI contents
                                    | BIT_VSI_ID_CHK_EN    //Enable VSI IEEE ID check
                                    /*| BIT_USE_AIF_FOR_VSI*/),//Enable the 2nd ID VSI packet uses AIF buffer

        REG_DDC_FILTER_SEL,         0x02,
        REG_SHA_CTRL,               (BIT_SHA_DS_MODE | BIT_SHA_EN), // Enable downstream SHA engine
        REG_TX_OTP_SEL_CTRL,        0x06,   // Assign different OTP AKSVs to Tx0 and Tx1

        //REG_PLL0_CALREFSEL,         0x44,   // Set Rx Zone control reference  //kamal
        REG_PLL0_CALREFSEL,         0x08,   // Set Rx Zone control reference    //kamal
       // REG_PLL0_CALREFSEL,         0x04,
        //REG_PLL1_CALREFSEL,         0x04,//kamal
        REG_PLL1_CALREFSEL,         0x08,//kamal
        //REG_PLL2_CALREFSEL,         0x04,//kamal
        REG_PLL2_CALREFSEL,         0x08,//kamal
        //REG_PLL3_CALREFSEL,         0x04,//kamal
        REG_PLL3_CALREFSEL,         0x08,//kamal
        //REG_PLL4_CALREFSEL,         0x04,//kamal
        REG_PLL4_CALREFSEL,         0x37,//kamal
        REG_PLL5_CALREFSEL,         0x04,
        REG_PLL0_VCOCAL,            0x06,
        REG_PLL1_VCOCAL,			0x06,//kamal
        REG_PLL2_VCOCAL,			0x06,//kamal
        REG_PLL3_VCOCAL,			0x06,//kamal

        REG_TMDS0_CNTL,             0x02,   // new way to set PLL bw at MP
        REG_TMDS0_CNTL2,            0x42,

        REG_A0_EQ_DATA0,            0x04,
        REG_A0_EQ_DATA1,            0x65,
        REG_A0_EQ_DATA2,            0x46,
        REG_A0_EQ_DATA3,            0x36,
        REG_A0_EQ_DATA4,            0x74,
        REG_A0_EQ_DATA5,            0x37,
        REG_A0_EQ_DATA6,            0x75,
        REG_A0_EQ_DATA7,            0x47,

        REG_TMDS0_BW_DATA0,         0x14,   // PLL bandwidth for HDMI at MP
        REG_TMDS0_BW_DATA1,         0x14,
        REG_TMDS0_BW_DATA2,         0x1B,   // PLL bandwidth for MHL at MP
        REG_TMDS0_BW_DATA3,         0x1B,
        REG_TMDS1_BW_DATA0,         0x08,   // PLL bandwidth for HDMI at RP
        REG_TMDS1_BW_DATA2,         0x08,   // PLL bandwidth for MHL at RP
        REG_TMDS1_CNTL,             0x00,   // new way to set PLL bw at RP
#if INC_CBUS
        REG_MHL1X_EQ_DATA0,         0x00,   // MHL 1x EQ table entry0
        REG_MHL1X_EQ_DATA1,         0x20,   // MHL 1x EQ table entry1
        REG_MHL1X_EQ_DATA2,         0x40,   // MHL 1x EQ table entry2
        REG_MHL1X_EQ_DATA3,         0x70,   // MHL 1x EQ table entry3
        REG_PAUTH_NUM_SMPS,         0x85,   // Enable MHL 1x EQ table
#endif

        // RX termination for MHL (ports 0 & 1) and HDMI (ports 1-2) types of inputs
        // NOTE: these settings can depend on value of external termination resistors
        REG_TMDS_TERMCTRL1,         ((VAL_TMDS_TERM_MHL << SFT_TMDS_PORT_EVEN) | VAL_TMDS_TERM_MHL),
        REG_TMDS_TERMCTRL3,         ((VAL_TMDS_TERM_HDMI << SFT_TMDS_PORT_EVEN) | VAL_TMDS_TERM_HDMI),
        REG_XTAL_CAL_CONTRL,        0x37,

        REG_RX_TMDS_TERM_0,         0x00,

        REG_A0_EQ_I2C,              0x20,
        REG_PAUTH_MP_AOVR,          BIT_MP_OVR_P0,
        REG_PAUTH_MPOVR,            0x01,
        REG_SYS_CTRL_1,             0x48,
  //      REG_TMDS2_CCTRL1,           0x37,
        REG_TPI__REG_SEL,           VAL_TPI__REG_SEL__PAGE_L0,
        REG_TPI__REG_OFFSET, 		0x81,
        REG_TPI__REG_DATA,          0xFD,
        REG_TPI__REG_SEL,           VAL_TPI__REG_SEL__PAGE_L0,
        REG_TPI__REG_OFFSET, 		0x80,
        REG_TPI__REG_DATA,          0xA3,
        REG_TPI__REG_SEL,           VAL_TPI__REG_SEL__PAGE_L0,
        REG_TPI__REG_OFFSET, 		0x84,
        REG_TPI__REG_DATA,          0x10,
        REG_TPI__REG_SEL,           VAL_TPI__REG_SEL__PAGE_L0,
        REG_TPI__REG_OFFSET, 		0x83,
        REG_TPI__REG_DATA,          0x88,
        REG_TPI__REG_SEL,           VAL_TPI__REG_SEL__PAGE_L0,
        REG_TPI__REG_OFFSET, 		0x86,
        REG_TPI__REG_DATA,          0x88,
        REG_TPI__REG_SEL,           VAL_TPI__REG_SEL__PAGE_L0,
        REG_TPI__REG_OFFSET, 		0x82,
        REG_TPI__REG_DATA,          0x22,
        REG_TX__TMDS_CTRL2,         0xFD,
        REG_TX__TMDS_CTRL3,         0x3A,//0x23

	REG_TMDS0_CLKDETECT_CTL, 	0x0E,
	REG_TMDS1_CLKDETECT_CTL,	0x0E,//kamal
	REG_TMDS2_CLKDETECT_CTL,	0x0E,//kamal
	REG_TMDS3_CLKDETECT_CTL,	0x0E//kamal
};

//------------------------------------------------------------------------------
//  Device Driver data
//------------------------------------------------------------------------------

DeviceDrvInstanceData_t deviceDrvInstance[1] =
{
{
    0,                          // structVersion
    SiiDevice_SUCCESS,          // lastResultCode Contains the result of the last API function called
    0,                          // statusFlags

    0,                          // devType;
    0,                          // devRev;

    false                       // hardwareInt;
}
};

DeviceDrvInstanceData_t *pDrvDevice = &deviceDrvInstance[0];
#if ( configSII_DEV_953x_PORTING == 0 )
extern uint8_t  dispData[ EDID_TABLE_LEN];
#else
static uint8_t  dispData[ EDID_TABLE_LEN];
#endif 

//-------------------------------------------------------------------------------------------------
//! @brief      Check to see if the NVRAM has been initialized and do it if needed.
//!
//! @retval     - true: successful.
//! @retval     - false: failure
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvDeviceNonVolatileInitialize ( bool_t forceInit )
{
    bool_t success = true;

    pDrvDevice->lastResultCode = SiiDevice_SUCCESS;     // Start out OK.

#if (FPGA_BUILD == DISABLE)
    if (( (SiiRegRead( REG_NVM_STAT ) & VAL_NVM_VALID) != VAL_NVM_VALID ) || forceInit )  // Bug 32696 - NVRAM programming issue when NVRAM is in "In progress" state 
#endif
    {
        DEBUG_PRINT( MSG_ERR, "\nNVRAM NOT INITIALIZED, initializing..." );
        //Enable Replacing of BSM registers with alternate values
		if ( !(SiiRegRead( REG_BSM_STAT ) & BIT_BOOT_DONE) )
		{
			DEBUG_PRINT( MSG_ERR, "\nNVRAM Boot Failed..." );

			SiiRegWrite( REG_NVM_BSM_REPLACE, 0x01 );
			SiiRegBitsSet( REG_SYS_RESET_2, BIT_NVM_SRST, true );      // NVM Soft Reset
			SiiRegBitsSet( REG_SYS_RESET_2, BIT_NVM_SRST, false );     // Release NVM soft reset
			//SiiRegWrite( REG_NVM_BSM_REPLACE, 0x00 );   // Bug 32696 - NVRAM programming issue when NVRAM is in "In progress" state 
		}


        success = SiiDrvNvramNonVolatileWrite( NVRAM_BOOTDATA, (uint8_t *)&gEdidFlashDevBootData, 0, EDID_DEVBOOT_LEN );

       // DEBUG_PRINT( MSG_ERR, "\nNVRAM Status: %d  \n", success );

        if ( success )
        {
            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable, 0, EDID_TABLE_LEN );
            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable, 256, EDID_TABLE_LEN );
            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable, 512, EDID_TABLE_LEN );
            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable, 768, EDID_TABLE_LEN );

            if ( success )
            {
                // Force a boot load to get the new EDID data from the NVRAM to the chip.
                SiiRegWrite( REG_NVM_BSM_REPLACE, 0x00 );   // Bug 32696 - NVRAM programming issue when NVRAM is in "In progress" state 
                SiiRegWrite( REG_BSM_INIT, BIT_BSM_INIT );
                success = SiiDrvEdidRxIsBootComplete( true );
            }
        }

        //DEBUG_PRINT( MSG_ERR, "\nNVRAM Status: %d  \n", success );

        if ( success )
        {
            DEBUG_PRINT( MSG_ALWAYS, "successful.\n" );
        }
        else
        {
            DEBUG_PRINT( MSG_ERR, "FAILED!\n" );
            pDrvDevice->lastResultCode = SiiDevice_FAIL_NVRAM_INIT;
        }
    }
    else
    {
       // Bug 33154 - Melbourne SiI9533 EDID ID Product Code is NOT correct
    	bool_t bootDataMismatch = 0;
    	bool_t EdidDataMismatch = 0;
    	int16_t i;
    	uint8_t *pBootData = (uint8_t *)&gEdidFlashDevBootData;
    	uint8_t *pEdidData = (uint8_t *)&gEdidFlashEdidTable[0];     

    	DEBUG_PRINT( MSG_ALWAYS, "Verifying NVRAM Boot Data and EDID Data..\n" );

    	// Read Boot data
    	success = SiiDrvNvramNonVolatileRead( NVRAM_BOOTDATA, dispData, 0, EDID_DEVBOOT_LEN );

    	if(success)
    	{
    		// compare boot data
			for(i=0;i<EDID_DEVBOOT_LEN;i++)
			{
				if(dispData[i] != pBootData[i])
				{
					bootDataMismatch = 1;
					break;
				}
			}

			if(!bootDataMismatch)      // Bug 33154 - Melbourne SiI9533 EDID ID Product Code is NOT correct
			{
				DEBUG_PRINT( MSG_ALWAYS, "Boot Data Matched..\n" );

				// Read EDID data
				success = SiiDrvNvramNonVolatileRead( NVRAM_HDMI_EDID, dispData, 0, EDID_TABLE_LEN );
				if(success)
				{
					// compare Edid Data
					if( (dispData[127] != pEdidData[127]) || (dispData[255] != pEdidData[255]) )
					{
							EdidDataMismatch = 1;
							DEBUG_PRINT( MSG_ALWAYS, "Edid Data Mismatched..\n");
					}
				}
				else
				{
					DEBUG_PRINT( MSG_ALWAYS, "EDID Data Read Failed..\n" );
					pDrvDevice->lastResultCode = SiiDevice_FAIL_NVRAM_INIT;
				}
			}
			else
			{
				DEBUG_PRINT( MSG_ALWAYS, "Boot Data Mismatched..\n" );
			}


			if(bootDataMismatch || EdidDataMismatch)
			{
				DEBUG_PRINT( MSG_ALWAYS, "Updating Boot Data and EDID Data to NVRAM..\n" );

				success = SiiDrvNvramNonVolatileWrite( NVRAM_BOOTDATA, (uint8_t *)&gEdidFlashDevBootData, 0, EDID_DEVBOOT_LEN );

		        if ( success )
		        {
		            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable[0], 0, EDID_TABLE_LEN );
		            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable[0], 256, EDID_TABLE_LEN );
		            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable[0], 512, EDID_TABLE_LEN );
		            success = SiiDrvNvramNonVolatileWrite( NVRAM_HDMI_EDID, (uint8_t *)&gEdidFlashEdidTable[0], 768, EDID_TABLE_LEN );

		            if ( success )
		            {
		                // Force a boot load to get the new EDID data from the NVRAM to the chip.
		                SiiRegWrite( REG_BSM_INIT, BIT_BSM_INIT );
		                success = SiiDrvEdidRxIsBootComplete( true );
		            }
		        }

	            if(success)
	            {
	            	DEBUG_PRINT( MSG_ALWAYS, "Successful.\n" );
	            }
	            else
	            {
	            	DEBUG_PRINT( MSG_ALWAYS, "Failed.\n" );
	            	pDrvDevice->lastResultCode = SiiDevice_FAIL_NVRAM_INIT;
	            }
			}
			else
			{
				DEBUG_PRINT( MSG_ALWAYS, "Boot Data and Edid Data Matched..\n" );
				success = SiiDrvNvramNonVolatileRead( NVRAM_HDMI_EDID, dispData, 0, EDID_TABLE_LEN );
			}
    	}
		else
		{
			DEBUG_PRINT( MSG_ALWAYS, "Boot Data Read Failed..\n" );
			pDrvDevice->lastResultCode = SiiDevice_FAIL_NVRAM_INIT;
		}
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    DrvDeviceInitRegisters
// Description: Initialize registers that need to be set to non-default values
//              at startup.  In general, these registers are not changed
//              after startup.
// Parameters:  none
// Returns:     none
//-----------------------------------------------------------------------------c

static void DrvDeviceInitRegisters ( void )
{
    uint8_t index;

    // Turn off port change logic and hold device in
    // software reset until finished update.
    SiiRegModify( REG_MISC_CTRL1, BIT_PORT_CHG_ENABLE, CLEAR_BITS );
    SiiRegModify( REG_SYS_RESET_1, BIT_SWRST, SET_BITS );

    SiiRegModify( REG_SYS_RESET_3, BIT_HDCP_RST, SET_BITS   ); // HDCP arbitration and OTP reset
    SiiRegModify( REG_SYS_RESET_3, BIT_HDCP_RST, CLEAR_BITS ); // Release

    /* Perform the majority of the required register initialization.    */

    for ( index = 0; index < (sizeof( initRegsList) / 2); index += 2 )
    {
        SiiRegWrite( initRegsList[ index], initRegsList[ index + 1] );
    }

    SiiRegWrite( REG_HPE_HW_CTRL_OVR, MSK_INVALIDATE_HW_HPD_CTRL ); // Disable Auto-HPD control

    //NOTE: Caller must call SiiDrvDeviceRelease() function to bring out of reset
    SiiRegModify( REG_PD_SYS_EXT2, 0xff, 0x0f );
    SiiRegModify( REG_PD_SYS4, 0xff, 0x0f );// TMDS Clock Channel power mode
    //  Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
    SiiRegModify( RX_EDID_DDC_EN, MSK_EN_DDC_ALL, CLEAR_BITS );
    SiiRegModify( REG_RX_HDCP_DDC_EN, VAL_DDC_PORT_ALL_EN, CLEAR_BITS );
}


//------------------------------------------------------------------------------
// Function:    ProcessResolutionChangeInterrupts
// Description: Handles resolution change in MP and SP
//------------------------------------------------------------------------------

static void ProcessResolutionChangeInterrupts(void)
{
    uint8_t mpResStatus, spResStatus, mpResStatusEnabled, spResStatusEnabled;

    mpResStatus = SiiRegRead( REG_INT_STATUS_19 );
    spResStatus = SiiRegRead( REG_INT_STATUS_20 );
    mpResStatusEnabled = (SiiRegRead( REG_INT_MASK_19 ) & (BIT_MP_HRES_CHG | BIT_MP_VRES_CHG));
    spResStatusEnabled = (SiiRegRead( REG_INT_MASK_20 ) & (BIT_SP_HRES_CHG | BIT_SP_VRES_CHG));

    if(mpResStatusEnabled)
    {

		if (mpResStatus & (BIT_MP_HRES_CHG | BIT_MP_VRES_CHG))
		{
			DEBUG_PRINT( MSG_DBG, "Resolution Change in MP interrupt : %s %s\n",(mpResStatus & BIT_MP_HRES_CHG)?"HRES CHANGED":"HRES STABLE",(mpResStatus & BIT_MP_VRES_CHG)?"VRES CHANGED":"VRES STABLE");
			pDrvDevice->statusFlags |= SiiDEV_STATUS_MP_RES_CHG;
		}
    }
    if(spResStatusEnabled)
    {

		if (spResStatus & (BIT_SP_HRES_CHG | BIT_SP_VRES_CHG))
		{
		  //  DEBUG_PRINT( MSG_DBG, "Resolution Change in SP interrupt\n");
			pDrvDevice->statusFlags |= SiiDEV_STATUS_SP_RES_CHG;
		}
    }

    // Clear interrupts
    if (mpResStatus)
    {
        SiiRegWrite(REG_INT_STATUS_19, mpResStatus);
    }
    if (spResStatus)
    {
        SiiRegWrite(REG_INT_STATUS_20, spResStatus);
    }
}


//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceConfigure
// Description: Before it is initialized for use, the device must be properly
//              configured for the target system requirements.  This function
//              is called to configure the parts of the device that may not
//              directly fall under the auspices of one of the other components.
// Parameters:  none
//------------------------------------------------------------------------------

bool_t SiiDrvDeviceConfigure ( void )
{

	return( true );
}
//---------------------------------------------------------------------------------
// Function:    SiiDrvVideoMute
// Description: Mute video only
// Parameters:  none
// Returns:     true if the configuration was successful, or false if some
//				failure occurred
//------------------------------------------------------------------------------

bool_t SiiDrvVideoMute ( bool_t isEnable )
{

//	  if (isEnable)
//	    {
//	        // Memorize current source selection and VPG settings
//	        app.tpgClockSrc = SI_TPG_CLK_XCLK;
//	        app.tpgVideoPattern = SI_ETPG_PATTERN_BLACK;
//	        app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
//	        app.isTxHdcpRequired[0] = true;//false;
//
//	        app.newSource[0]    = SiiSwitchSourceInternalVideo;
//	    }
//	    else
//	    {
//	        app.isTxHdcpRequired[0] = true;
//	        app.newSource[0] = SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT);
//	    }
	// SiiRegBitsSet( REG_MISC_CTRL0, BIT_VIDEO_MUTE_SYNC | BIT_AUDIO_MUTE_SYNC, isEnable );
	 //SiiDrvTpgEnable(isEnable);
	 if (isEnable)
	 {
		app.newSource[app.currentZone] = SiiSwitchSourceInternalVideo;
		app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;
		app.tpgVideoPattern = SI_ETPG_PATTERN_BLACK;
	 }
	 else
	 {
		 app.tpgVideoPattern = SI_ETPG_PATTERN_BLUE;
		 app.newSource[0] = SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT);
	 }
	 return true;
}


//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceIdGet
// Description: This function returns the chip ID.
// Parameters:  none
// Returns:     I16 bit chip id.
//------------------------------------------------------------------------------
uint16_t SiiDrvDeviceIdGet(void)
{
	uint16_t devType;
    devType    = SiiRegRead( REG_DEV_IDH_RX );
    devType    = ( devType << 8) | SiiRegRead( REG_DEV_IDL_RX );
    return devType;
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceRevGet
// Description: This function returns the chip ID.
// Parameters:  none
// Returns:     I16 bit chip id.
//------------------------------------------------------------------------------
uint8_t SiiDrvDeviceRevGet(void)
{
    return  SiiRegRead( REG_DEV_REV );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceInitialize
// Description: This function disables the device or places the device in
//				software reset if it does not power up in a disabled state.
//				It may be used to initialize registers that require a value
//				different from the power-up state and are common to one or
//				more of the other component modules.
// Parameters:  none
// Returns:     It returns true if the initialization is successful, or false
//				if some failure occurred.
//
// Note:		This function leaves the device in a disabled state until the
//				SiiDrvDeviceRelease function is called.
//------------------------------------------------------------------------------

bool_t SiiDrvDeviceInitialize ( bool_t allPowerOn )
{
    bool_t  success = false;

    for ( ;; )
    {
        if ( !SiiDrvDevicePowerUpBoot())   break;

        pDrvDevice->devType    = SiiRegRead( REG_DEV_IDH_RX );
        pDrvDevice->devType    = ( pDrvDevice->devType << 8) | SiiRegRead( REG_DEV_IDL_RX );
        pDrvDevice->devRev     = SiiRegRead( REG_DEV_REV );

#if ( configSII_DEV_9535 == 1 )
        if (( pDrvDevice->devType & 0xFFFF ) != 0x9535 )
#else
        if (( pDrvDevice->devType & 0xFFFF ) != 0x9533 )
#endif
        {
            pDrvDevice->lastResultCode = SiiDevice_FAIL_WRONGDEVICE;
            break;
        }

        if ( allPowerOn )
        {
            DrvDeviceInitRegisters();   // Initialize registers to the Programmers Reference default list.

            // Enable the resolution stable interrupt for use by multiple drivers
            SiiRegBitsSet( REG_INT_ENABLE_IP4, BIT_MP_RES_STABLE_CHG, true );

            // Resolution change detection is disabled at initialization
            SiiDrvDeviceMpResDetectionEnable(false);
            SiiDrvDeviceSpResDetectionEnable(false);
        }

        success = true;
        break;
    }
	return( success );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceStatus
// Description: Returns a status flag word containing device-specific
//				information about the state of the device.
// Parameters:  none
// Returns:     Sixteen-bit Device status flags word for the Device Component
//------------------------------------------------------------------------------

SiiDrvDeviceStatus_t SiiDrvDeviceStatus ( void )
{
	uint16_t x;

    x = pDrvDevice->statusFlags;
    pDrvDevice->statusFlags = 0;
	return( x );
}


//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceStandby
// Description: Places the device into standby mode if available
// Parameters:  none
// Returns:     true if the device entered standby mode successfully, or false
//				if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiDrvDeviceStandby ( void )
{

    // Disable interrupts used by this driver
    SiiRegBitsSet( REG_INT_ENABLE_IP4, BIT_MP_RES_STABLE_CHG, false );
    SiiDrvDeviceMpResDetectionEnable(false);
    SiiDrvDeviceSpResDetectionEnable(false);

    // Clear any pending interrupts
    SiiRegWrite( REG_INT_STATUS_IP4, BIT_MP_RES_STABLE_CHG );
    SiiRegWrite( REG_INT_STATUS_19, BIT_MP_HRES_CHG | BIT_MP_VRES_CHG );
    SiiRegWrite( REG_INT_STATUS_20, BIT_SP_HRES_CHG | BIT_SP_VRES_CHG );

    // Turn Power down domain clocks off.  This will reduce power in the
    // event that the system does not actually remove the main power from
    // the SiI9535.  To come out of standby, the firmware resets the
    // chip, so the opposite action is not necessary in SiiDrvDeviceResume()
    SiiRegBitsSet( REG_SYS_CTRL_1, BIT_PWD_CLKS, true );
    SiiRegWrite( REG_PD_TOT, 0x00 );

    // Power down all TMDS channels
    SiiRegWrite( REG_PD_SYS3,       0x3F );     // Force firmware control of TMDS channel 0 power mode
    SiiRegWrite( REG_PD_SYS2,       0x00 );     // TMDS Channel 0 power mode
    SiiRegWrite( REG_PD_SYS4,       0x00 );     // TMDS Clock Channel power mode
    SiiRegWrite( REG_PD_SYS_EXT3,   0x00 );     // TMDS Channel 1&2 power mode

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceResume
// Description: Causes operation of the device to return to the state it was
//				prior to the previous call to SiiDeviceStandby.
// Parameters:  none
// Returns:     true if the device successfully exited standby mode , or false
//				if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiDrvDeviceResume ( void )
{
#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith fixes: when first dc on, ARC should be disable.*/
    SiiDrvArcConfigureArcPin( 0, 0, ARC_DISABLE_MODE);
#endif
	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiDeviceRelease
// Description: Enables the device for normal operation and is called as the
//				last part of device initialization.
//				Since the SiiDeviceInitialize function leaves the device output
//				disabled, this function should enable the device output as a
//				minimum.  All other configuration and initialization functions
//				must be called in between SiiDeviceInitialize and SiiDeviceRelease.
// Parameters:  none
// Returns:     true if the release is successful and the device is ready for
//				operation or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiDrvDeviceRelease ( void )
{
    // Release software reset and finite state machine. Must delay between the two events.
    SiiRegModify( REG_SYS_RESET_1, BIT_SWRST, CLEAR_BITS );
    SiiOsTimerWait( 120 );
    SiiRegModify( REG_MISC_CTRL1, BIT_PORT_CHG_ENABLE, SET_BITS );

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceStart
// Description: Enables the device for normal operation.  This function performs
//              the procedures necessary for correct operation of the device after
//              all other initialization has been performed.
// Parameters:  none
// Returns:     true if the release is successful and the device is ready for
//              operation or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiDrvDeviceStart ( void )
{

    // Set RX controls to enabled for all ports. (Instaport mode)
    SiiDrvRxHpeStateSet( SiiPORT_ALL, SiiHPD_ACTIVE );

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceGetLastResult
// Description: Returns the result of the last SiiDevicexxx function called
// Parameters:  none
// Returns:     Result of last device driver function.
//------------------------------------------------------------------------------

int_t SiiDrvDeviceGetLastResult ( void )
{

	return( pDrvDevice->lastResultCode );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDevicePowerUpBoot
// Description: Perform the device startup sequence.  Reset, wait for boot to
//				complete, optionally initialize EDID NVRAM, and initialize
//				device registers.
// Parameters:  none
// Returns:     true if device booted and initialized correctly
//------------------------------------------------------------------------------
bool_t SiiDrvDevicePowerUpBoot ( void )
{
    pDrvDevice->lastResultCode = SiiDevice_SUCCESS;  // Start out OK.
    bool_t recoverNVRAM=false;  // Bug 32696 - NVRAM programming issue when NVRAM is in "In progress" state 

    for ( ;; )
    {
        // Wait for boot to complete if this is a cold power-up
    	if ( !SiiDrvEdidRxIsBootComplete( true ) )
        {
            DEBUG_PRINT( MSG_ALWAYS, "\nNVRAM Boot Failed. Force NVRAM Recovery..." );   // Bug 32696 - NVRAM programming issue when NVRAM is in "In progress" state 
            TRACE_DEBUG((0, " NVRAM Boot Failed. Force NVRAM Recovery... "));
            recoverNVRAM = true;
        }

        // Force a soft Hard Reset to ensure that the Always On Domain 
		// logic is reset.  This is needed in case this is NOT a cold 
		// power-up, but a resume from standby or a cold power-up when 
		// one or more powered sources are connected.
        SiiRegWrite( REG_SPECIAL_PURPOSE, BIT_HARDRESET );

#if ( configSII_DEV_953x_PORTING == 1 )	
        SiiOsTimerWait( 10 ); // Smith fixes: hdmi 9533 boot up fail issue.
#endif

        (void) SiiRegRead( REG_SPECIAL_PURPOSE );  // Dummy read to clear write interrupted by reset

        {   //SWWA: FP1557
            // The audio PLLs may not have been properly powered up if the
            // 3.3v and 1.3v power supplies were improperly sequenced, so
            // turn them off and on to ensure proper operation.
            SiiRegWrite( REG_APLL_POWER, 0x00);
            SiiOsTimerWait( 1 );
            SiiRegWrite( REG_APLL_POWER, BIT_APLL0_PWR_ON | BIT_APLL1_PWR_ON);
        }

        // Check NVRAM status to determine if it must be initialized (first time boot).
        // We do this here because NVRAM contains some register default information
        // as well as the correct EDID.  If the NVRAM is not initialized, we must
        // initialize it and then reload so that the appropriate registers are
        // loaded.
#if ( DO_EDID_INIT == 1 ) 
        if ( !SiiDrvDeviceNonVolatileInitialize( recoverNVRAM ))   // Bug 32696 - NVRAM programming issue when NVRAM is in "In progress" state 
        {
            break;
        }
#endif
        break;
    }

    return( pDrvDevice->lastResultCode == SiiDevice_SUCCESS );
}


//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceSpResDetectionEnable
// Description: Enable resolution change detection in Sub Pipe.
//------------------------------------------------------------------------------

void SiiDrvDeviceSpResDetectionEnable(bool_t isEnabled)
{
    SiiRegBitsSet(REG_INT_MASK_20, BIT_SP_HRES_CHG | BIT_SP_VRES_CHG, isEnabled);
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceMpResDetectionEnable
// Description: Enable resolution change detection in Main Pipe.
//------------------------------------------------------------------------------

void SiiDrvDeviceMpResDetectionEnable(bool_t isEnabled)
{
    SiiRegBitsSet(REG_INT_MASK_19, BIT_MP_HRES_CHG | BIT_MP_VRES_CHG, isEnabled);
}


//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceInfo
// Description: Return the requested information from the device.
//------------------------------------------------------------------------------

uint_t SiiDrvDeviceInfo ( SiiDrvDeviceInfo_t infoIndex )
{
    uint8_t uData;
    uint_t  regValue = 0;
    uint8_t portStat = 0;

    switch ( infoIndex )
    {
        case SiiDEV_INPUT_CONNECTED:
            // Join P4 with P3:0 of PWR5V status
            uData = SiiRegRead( REG_PWR5V_STATUS );
            regValue = ((uData >> 2) & 0x0F) | (uData & 0x0F);
#if INC_CBUS
            // Add in CBUS ports connected
            regValue |= SiiRegRead( REG_CBUS_DEV_CONNECTED );
#endif
            break;
        case SiiDEV_BOOT_STATE_MACHINE:
            regValue = SiiRegRead( REG_BSM_STAT );
            break;
        case SiiDEV_NVRAM:
            regValue = (SiiRegRead( REG_NVM_STAT ) == VAL_NVM_VALID);
            break;
        case SiiDEV_ID:
            regValue = pDrvDevice->devType;
            break;
        case SiiDEV_REV:
            regValue = pDrvDevice->devRev;
            break;
        case SiiDEV_ACTIVE_PORT:
            regValue = SiiRegRead( REG_RX_PORT_SEL ) & MSK_MP_PORT_SEL;
            break;
        case SiiDEV_SELECTED_PORT_BITFIELD:
            portStat = SiiRegRead( REG_SELECTED_PORT_0 );
            regValue = ((portStat & ROVE_PIPE_MASK) >> 4) | ( portStat & MAIN_PIPE_MASK);
            break;
    }

    return( regValue );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvDeviceManageInterrupts
// Description: Monitors Sii953x interrupts and calls an interrupt processor
//              function in the applicable driver if an interrupt is encountered.
// Parameters:  fullPowerMode - true if ALL interrupts should be processed,
//                              false if only standby mode interrupts.
// NOTE:    This function is not designed to be called directly from a physical
//          interrupt handler unless provisions have been made to avoid conflict
//          with normal level I2C accesses.
//          It is intended to be called from normal level by monitoring a flag
//          set by the physical handler.
//------------------------------------------------------------------------------
#define INTERRUPT_DEBUG 0
bool_t SiiDrvDeviceManageInterrupts ( bool_t fullPowerMode )
{
    uint8_t intStatus = 0; 
    uint8_t intStatus1 = 0;
    uint8_t i;

#if INTERRUPT_DEBUG
    static uint8_t lastIntStatus = 0;
    static uint8_t lastIntStatus1 = 0;
    static bool lastInitGpioStatus = 0;
#endif
    bool_t initGpioStatus = false;

    // Check the hardware interrupt
    if ( SiiPlatformInterruptStateGet() == false )
    {
       return( false );
    }
    
    // Get the register interrupt info
    intStatus = SiiRegRead( REG_INT_GROUP_STATE_0 );
    intStatus1 = SiiRegRead( REG_INT_GROUP_STATE_1 );

#if INTERRUPT_DEBUG
    if( ( lastIntStatus != intStatus ) || ( lastIntStatus1 != intStatus1 ) )
    {
        TRACE_DEBUG((0, "[ HDMI INT ] status - 0 = 0x%X", intStatus ));
        TRACE_DEBUG((0, "[ HDMI INT ] status - 1 = 0x%X", intStatus1 ));

        lastIntStatus = intStatus;
        lastIntStatus1 = intStatus1;
    }
#endif    

    // Determine the pending interrupts and service them with driver calls
    // Each function will call its appropriate callback function if a status
    // change is detected that needs upper level attention.

    if ( fullPowerMode ) 
    {
        if ( intStatus & BIT_INTR_TX0 )     // Tx0 interrupts
        {
            SiiTxInstanceSet(0);
            SiiDrvTpiProcessInterrupts();
        }
        
        /*
            if ( intStatus & BIT_INTR_TX1 )     // Tx1 interrupts
            {
                SiiTxInstanceSet(1);
                SiiDrvTpiProcessInterrupts();
            }
            */
        if (( intStatus & BIT_INTR_GRP1 ) || ( intStatus & BIT_INTR_GRP_PA ))   // PA interrupts
        {
            uint8_t tempStatus;

            tempStatus = SiiRegRead( REG_INT_STATUS_P3 );
            if( tempStatus & BIT_MP_NEW_AVI)
            {
            	pDrvDevice->statusFlags |= SiiDEV_STATUS_AVI_READY;
            }
            if( tempStatus & BIT_MP_NEW_AIF)
            {
            	pDrvDevice->statusFlags |= SiiDEV_STATUS_AIF_READY;
            }

            SiiDrvSwitchProcessInterrupts( intStatus );
            DrvGpioProcessInterrupts();

            // Resolution stable interrupt is used by a number of modules.
            tempStatus = SiiRegRead( REG_INT_STATUS_IP4 ) & BIT_MP_RES_STABLE_CHG;
            if ( tempStatus )
            {
                DEBUG_PRINT( MSG_DBG, "Resolution Stable interrupt: %02X\n", SiiRegRead( REG_MP_GCP_STATUS ) & BIT_MP_RES_STABLE );

                SiiRegWrite( REG_INT_STATUS_IP4, tempStatus );
#if INC_IPV
                SiiDrvIpvProcessInterrupts();
#endif
#if INC_OSD
                SiiDrvOsdProcessInterrupts();
#endif  // INC_OSD
                pDrvDevice->statusFlags |= SiiDEV_STATUS_RES_STB_CHG;
            }
        }

        if (( intStatus & BIT_INTR_GRP0 ) || ( intStatus & BIT_INTR_GRP_PA ))   // PA interrupts
        {
        	for(i = 0; i < SII_NUM_TX; i++)
        	{
        		SiiRepeaterInstanceSet(i);
        		SiiDrvRepeaterInterruptsSet(true);
        	}
        }

        if ( intStatus1 & (BIT_INTR_GRP7 ))  //Audio Main Pipe Interrupts
        {
            SiiDrvRxAudioInstanceSet(0);
            SiiDrvRxAudioProcessInterrupts();
        }
        /*
        if ( intStatus1 & (BIT_INTR_GRP8 ))  //Audio Sub Pipe Interrupts
        {
            SiiDrvRxAudioInstanceSet(1);
            SiiDrvRxAudioProcessInterrupts();
        }*/

        if ( intStatus1 & (BIT_INTR_PWD ))  //Resolution change in MP/SP, etc.
        {
            ProcessResolutionChangeInterrupts();
            if (pDrvDevice->statusFlags & SiiDEV_STATUS_MP_RES_CHG)
            {
                SiiRepeaterInstanceSet(0);
                SiiDrvRepeaterResChngStatusSet(true);
            }
            /*
            if (pDrvDevice->statusFlags & SiiDEV_STATUS_SP_RES_CHG)
            {
                SiiRepeaterInstanceSet(1);
                SiiDrvRepeaterResChngStatusSet(true);
            }*/
        }

        if( intStatus & BIT_INTR_GRP_PA ) //for ACP interrupt
        {
            SiiDrvRxAudioInstanceSet(0);
            SiiDrvRxAudioProcessInterrupts();
            SiiDrvRxAudioInstanceSet(1);
            SiiDrvRxAudioProcessInterrupts();
        }

#if( configSII_DEV_953x_PORTING_CBUS == 1 )
#if INC_CBUS
        if ( intStatus & BIT_INTR_GRP5 )
        {
            SiiDrvCbusInstanceSet(0);
            SiiDrvCbusProcessInterrupts();
            SiiDrvCbusInstanceSet(1);
            SiiDrvCbusProcessInterrupts();
        }
#endif
#endif

    }

    // The following interrupts can be checked unless power is completely off
    // in which case this function will not be called.
#if INC_CEC
    if ( intStatus & BIT_INTR_GRP6 )                    // CEC interrupts
    {
        SiiDrvCpiInstanceSet( CPI_INSTANCE_AVR );
        DrvCpiProcessInterrupts();
    }
#endif

    // Check if all interrupts are cleared so that h/w interrupt line goes high.
    // If the interrupt line is still low, don't clear the interrupt status
    // to process and clear pending interrupts in the next round
    SiiPlatformInterruptDisable();
    initGpioStatus = SiiPlatformInterruptPinStateGet();
    if (initGpioStatus == true )
    {
        SiiPlatformInterruptClear();
    }
    
#if INTERRUPT_DEBUG
    if ( initGpioStatus != lastInitGpioStatus )
    {
        TRACE_DEBUG((0, "[ HDMI INT ] Interrupt status = %d", initGpioStatus ));
    
        lastInitGpioStatus = initGpioStatus;
    }
#endif    
    
    SiiPlatformInterruptEnable();
    return( true );
}

