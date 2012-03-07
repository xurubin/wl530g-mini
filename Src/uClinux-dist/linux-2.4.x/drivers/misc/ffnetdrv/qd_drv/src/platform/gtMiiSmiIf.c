#include <Copyright.h>
/********************************************************************************
* gtMiiSmiIf.c
*
* DESCRIPTION:
*       Includes functions prototypes for initializing and accessing the
*       MII / SMI interface.
*       This is the only file to be included from upper layers.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtHwCntl.h>
#include <gtMiiSmiIf.h>

/*******************************************************************************
* miiSmiIfInit
*
* DESCRIPTION:
*       This function initializes the MII / SMI interface.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       highSmiDevAddr - Indicates whether to use the high device register
*                     addresses when accessing switch's registers (of all kinds)
*                     i.e, the devices registers range is 0x10 to 0x1F, or to
*                     use the low device register addresses (range 0x0 to 0xF).
*                       GT_TRUE     - use high addresses (0x10 to 0x1F).
*                       GT_FALSE    - use low addresses (0x0 to 0xF).
*
* RETURNS:
*       GT_OK       - on success
*       GT_FAIL     - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS miiSmiIfInit
(
	IN  GT_QD_DEV    *dev,
	OUT GT_BOOL * highSmiDevAddr
)
{
	GT_STATUS status;
	GT_U16 data, data1;

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR,QD_REG_SWITCH_ID,&data)) != GT_OK)
	{
		return status;
	}

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+1,QD_REG_SWITCH_ID,&data1)) != GT_OK)
	{
		return status;
	}

	switch(data & 0xFF00)
	{
		case 0x0200:
		case 0x0500:
		case 0x0600:
		case 0x1500:
		case 0xF500:
		case 0xF900:
			if (data == data1)
			{
				*highSmiDevAddr = GT_FALSE;
				return GT_OK;
			}
			break;
		default:
			break;
	}

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+0x10,QD_REG_SWITCH_ID,&data)) != GT_OK)
	{
		return status;
	}

	if((status = miiSmiIfReadRegister(dev,PORT_REGS_START_ADDR+0x11,QD_REG_SWITCH_ID,&data1)) != GT_OK)
	{
		return status;
	}

	switch(data & 0xFF00)
	{
		case 0x0200:
		case 0x0500:
		case 0x0600:
		case 0x1500:
		case 0xF500:
		case 0xF900:
			if (data == data1)
			{
				*highSmiDevAddr = GT_TRUE;
				return GT_OK;
			}
			break;
		default:
			break;
	}

    return GT_FAIL;
}


/*******************************************************************************
* miiSmiIfReadRegister
*
* DESCRIPTION:
*       This function reads a register throw the SMI / MII interface, to be used
*       by upper layers.
*
* INPUTS:
*       phyAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*
* OUTPUTS:
*       data        - The register's data.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS miiSmiIfReadRegister
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U8        phyAddr,
    IN  GT_U8        regAddr,
    OUT GT_U16       *data
)
{
    unsigned int tmpData;

    if(dev->fgtReadMii(dev,(GT_U32)phyAddr,(GT_U32)regAddr,&tmpData) != GT_TRUE)
    {
        return GT_FAIL;
    }

    *data = (GT_U16)tmpData;
    return GT_OK;
}


/*******************************************************************************
* miiSmiIfWriteRegister
*
* DESCRIPTION:
*       This function writes to a register throw the SMI / MII interface, to be
*       used by upper layers.
*
* INPUTS:
*       phyAddr     - The PHY address to be read.
*       regAddr     - The register address to read.
*       data        - The data to be written to the register.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS miiSmiIfWriteRegister
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U8        phyAddr,
    IN  GT_U8        regAddr,
    IN  GT_U16       data
)
{
    if(dev->fgtWriteMii(dev,(GT_U32)phyAddr,(GT_U32)regAddr,(GT_U32)data) != GT_TRUE)
    	return GT_FAIL;
	else
	return GT_OK;
}



