#include <Copyright.h>
/********************************************************************************
* gtDrvConfig.h
*
* DESCRIPTION:
*       Includes driver level configuration and initialization function.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtDrvConfig.h>
#include <gtMiiSmiIf.h>
#include <gtHwCntl.h>
#include <msApi.h>

/*******************************************************************************
* driverConfig
*
* DESCRIPTION:
*       This function initializes the driver level of the quarterDeck software.
*
* INPUTS:
*		None.
* OUTPUTS:
*		None.
* RETURNS:
*       GT_OK               - on success, or
*       GT_OUT_OF_CPU_MEM   - if failed to allocate CPU memory,
*       GT_FAIL             - otherwise.
*
* COMMENTS:
*       1.  This function should perform the following:
*           -   Initialize the global switch configuration structure.
*           -   Initialize Mii Interface
*           -   Set the CPU port into trailer mode (Ingress and Egress).
*
*******************************************************************************/
GT_STATUS driverConfig
(
    IN GT_QD_DEV    *dev
)
{
    GT_U16          deviceId;
    GT_BOOL         highSmiDevAddr;
    GT_U32	    portsCount;

    /* Initialize the MII / SMI interface                       */
    if(miiSmiIfInit(dev,&highSmiDevAddr) != GT_OK)
    {
        return GT_FAIL;
    };

	dev->baseRegAddr = (highSmiDevAddr)?0x10:0;
	
    if(hwReadPortReg(dev,1,QD_REG_SWITCH_ID,&deviceId) != GT_OK)
    {
        return GT_FAIL;                     
    };
    
    /* Init the device's config struct.             */
    dev->deviceId       = deviceId >> 4;
    dev->revision       = (GT_U8)deviceId & 0xF; 

    /* Get the number of active ports               */

	switch (dev->deviceId)
	{
		case GT_88E6051:
	                portsCount = 5;
			break;
		case GT_88E6021:
		case GT_FF_XP:
	                portsCount = 3; 
			break;
		case GT_FF_EG:
		case GT_88E6060:
		case GT_88E6061:
		case GT_FH_VPN_L:
	                portsCount = 6; 
			break;

		case GT_88E6052:
		case GT_88E6063:
		case GT_FF_HG:
		case GT_FH_VPN:
		default:
	                portsCount = 7; 
			break;
    }

    dev->numOfPorts = (GT_U8)portsCount;

    printk("Device ID:%x Num Ports: %d\n", dev->deviceId, dev->numOfPorts);
    return GT_OK;
}


/*******************************************************************************
* driverEnable
*
* DESCRIPTION:
*       This function enables the switch for full operation, after the driver
*       Config function was called.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverEnable
(
	IN GT_QD_DEV    *dev
)
{
    return GT_OK;
}

