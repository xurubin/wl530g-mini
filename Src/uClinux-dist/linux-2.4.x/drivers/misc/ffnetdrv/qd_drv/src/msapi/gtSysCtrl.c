#include <Copyright.h>

/********************************************************************************
* gtSysCtrl.c
*
* DESCRIPTION:
*       API definitions for system global control.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/*******************************************************************************
* gsysSwReset
*
* DESCRIPTION:
*       This routine preforms switch software reset.
*
* INPUTS:
*       None.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSwReset
(
    IN  GT_QD_DEV *dev
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSwReset Called.\n"));

    /* Set the Software reset bit.                  */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,15,1,1);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Make sure the reset operation is completed.  */
    data = 1;
    while(data != 0)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_ATU_CONTROL,15,1,
                                     &data);
        if(retVal != GT_OK)
        {
            DBG_INFO(("Failed.\n"));
            return retVal;
        }
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetDiscardExcessive
*
* DESCRIPTION:
*       This routine set the Discard Excessive state.
*
* INPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDiscardExcessive
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetDiscardExcessive Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the Discard Exissive bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,13,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetDiscardExcessive
*
* DESCRIPTION:
*       This routine get the Discard Excessive state.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDiscardExcessive
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetDiscardExcessive Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Discard Exissive bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,13,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetSchedulingMode
*
* DESCRIPTION:
*       This routine set the Scheduling Mode.
*
* INPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetSchedulingMode
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetSchedulingMode Called.\n"));
    BOOL_2_BIT(mode,data);
    data = 1 - data;

    /* Set the Schecduling bit.             */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,11,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetSchedulingMode
*
* DESCRIPTION:
*       This routine get the Scheduling Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetSchedulingMode
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetSchedulingMode Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
    /* Get the Scheduling bit.              */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,11,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(1 - data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetMaxFrameSize
*
* DESCRIPTION:
*       This routine Set the max frame size allowed.
*
* INPUTS:
*       mode - GT_TRUE max size 1522, GT_FALSE max size 1535.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetMaxFrameSize
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetMaxFrameSize Called.\n"));
    BOOL_2_BIT(mode,data);
    data = 1 - data;

    /* Set the Max Fram Size bit.               */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,10,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetMaxFrameSize
*
* DESCRIPTION:
*       This routine Get the max frame size allowed.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE max size 1522, GT_FALSE max size 1535.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMaxFrameSize
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetMaxFrameSize Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
    /* Get the Max Frame Size bit.          */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,10,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(1 - data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysReLoad
*
* DESCRIPTION:
*       This routine cause to the switch to reload the EEPROM.
*
* INPUTS:
*       None.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysReLoad
(
    IN  GT_QD_DEV *dev
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysReLoad Called.\n"));
    /* Set the Reload bit.                  */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,9,1,1);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Should a check for reload completion be added.   */
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetWatchDog
*
* DESCRIPTION:
*       This routine Set the the watch dog mode.
*
* INPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetWatchDog
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetWatchDog Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the WatchDog bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,7,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetWatchDog
*
* DESCRIPTION:
*       This routine Get the the watch dog mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetWatchDog
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetWatchDog Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the WatchDog bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,7,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets the full duplex pause src Mac Address.
*
* INPUTS:
*       mac - The Mac address to be set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDuplexPauseMac
(
    IN GT_QD_DEV    *dev,
    IN GT_ETHERADDR *mac
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetDuplexPauseMac Called.\n"));
    if(mac == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Set the first Mac register with diffAddr bit reset.  */
    data = (((*mac).arEther[0] & 0xFE) << 8) | (*mac).arEther[1];
    retVal = hwWriteGlobalReg(dev,QD_REG_MACADDR_01,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Set the Mac23 address register.   */
    data = ((*mac).arEther[2] << 8) | (*mac).arEther[3];
    retVal = hwWriteGlobalReg(dev,QD_REG_MACADDR_23,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Set the Mac45 address register.   */
    data = ((*mac).arEther[4] << 8) | (*mac).arEther[5];
    retVal = hwWriteGlobalReg(dev,QD_REG_MACADDR_45,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysGetDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets the full duplex pause src Mac Address.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mac - the Mac address.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDuplexPauseMac
(
    IN  GT_QD_DEV    *dev,
    OUT GT_ETHERADDR *mac
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to read from register.  */

    DBG_INFO(("gsysGetDuplexPauseMac Called.\n"));
    if(mac == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Mac01 register.      */
    retVal = hwReadGlobalReg(dev,QD_REG_MACADDR_01,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
#if 0	/* This should be always a unicast. */
    /* The mac is allwasy a multicast mac   */
    (*mac).arEther[0] = (data >> 8) | 0x01;
    (*mac).arEther[1] = data & 0xFF;
#else
    (*mac).arEther[0] = (data >> 8) & ~0x01;
    (*mac).arEther[1] = data & 0xFF;
#endif
    /* Get the Mac23 register.      */
    retVal = hwReadGlobalReg(dev,QD_REG_MACADDR_23,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    (*mac).arEther[2] = data >> 8;
    (*mac).arEther[3] = data & 0xFF;

    /* Get the Mac45 register.      */
    retVal = hwReadGlobalReg(dev,QD_REG_MACADDR_45,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    (*mac).arEther[4] = data >> 8;
    (*mac).arEther[5] = data & 0xFF;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetPerPortDuplexPauseMac
(
    IN GT_QD_DEV    *dev,
    IN GT_BOOL      en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetPerPortDuplexPauseMac Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the WatchDog bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_MACADDR_01,8,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPerPortDuplexPauseMac
(
    IN GT_QD_DEV    *dev,
    IN GT_BOOL      *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetPerPortDuplexPauseMac Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Scheduling bit.              */
    retVal = hwGetGlobalRegField(dev,QD_REG_MACADDR_01,8,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysReadMiiReg
*
* DESCRIPTION:
*       This routine reads QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know which phy address(0 ~ 0x1F) will be read.		
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysReadMiiReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     phyAddr,
    IN  GT_U32	     regAddr,
    OUT GT_U32	     *data
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          u16Data;           /* The register's read data.    */

    DBG_INFO(("gsysReadMiiRegister Called.\n"));

    /* Get the Scheduling bit.              */
    retVal = hwReadMiiReg(dev,(GT_U8)phyAddr,(GT_U8)regAddr,&u16Data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*data = (GT_U32)u16Data;

    return GT_OK;
}

/*******************************************************************************
* gsysWriteMiiReg
*
* DESCRIPTION:
*       This routine writes QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know which phy address(0 ~ 0x1F) will be read.		
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysWriteMiiReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     phyAddr,
    IN  GT_U32	     regAddr,
    IN  GT_U16	     data
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    
    DBG_INFO(("gsysWriteMiiRegister Called.\n"));

    /* Get the Scheduling bit.              */
    retVal = hwWriteMiiReg(dev,(GT_U8)phyAddr,(GT_U8)regAddr,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	return GT_OK;
}


/*******************************************************************************
* gsysSetRetransmitMode
*
* DESCRIPTION:
*       This routine set the Retransmit Mode.
*
* INPUTS:
*       en - GT_TRUE Retransimt Mode is enabled, GT_FALSE otherwise.
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRetransmitMode
(
    IN  GT_QD_DEV    *dev,
    IN GT_BOOL       en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetRetransmitMode Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the Retransmit Mode bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,15,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetRetransmitMode
*
* DESCRIPTION:
*       This routine get the Retransmit Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Retransmit Mode is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRetransmitMode
(
    IN  GT_QD_DEV    *dev,
    IN GT_BOOL       *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetRetransmitMode Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,15,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetLimitBackoff
(
    IN  GT_QD_DEV    *dev,
    IN GT_BOOL       en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetLimitBackoff Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,14,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetLimitBackoff
(
    IN  GT_QD_DEV    *dev,
    IN GT_BOOL       *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetLimitBackoff Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,14,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetRsvRegPri
*
* DESCRIPTION:
*       This routine set the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvReqPri
(
    IN  GT_QD_DEV    *dev,
    IN GT_BOOL       en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetRsvReqPri Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,12,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetRsvReqPri
*
* DESCRIPTION:
*       This routine get the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvReqPri
(
    IN  GT_QD_DEV    *dev,
    IN GT_BOOL       *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetRsvReqPri Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,12,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

