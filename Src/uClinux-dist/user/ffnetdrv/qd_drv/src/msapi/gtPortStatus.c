#include <Copyright.h>

/********************************************************************************
* gtPortCtrl.c
*
* DESCRIPTION:
*       API implementation for switch port status.
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
* gprtGetPartnerLinkPause
*
* DESCRIPTION:
*       This routine retrives the link partner pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPartnerLinkPause
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPartnerLinkPause Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,15,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetSelfLinkPause
*
* DESCRIPTION:
*       This routine retrives the link pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSelfLinkPause
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetSelfLinkPause Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,14,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetResolve
*
* DESCRIPTION:
*       This routine retrives the resolve state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Done  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetResolve
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetResolve Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,13,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetLinkState
*
* DESCRIPTION:
*       This routine retrives the link state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Up  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetLinkState
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetLinkState Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,12,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetPortMode
*
* DESCRIPTION:
*       This routine retrives the port mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortMode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPortMode Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,11,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetPhyMode
*
* DESCRIPTION:
*       This routine retrives the PHY mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII PHY  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyMode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPhyMode Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,10,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetDuplex
*
* DESCRIPTION:
*       This routine retrives the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for Full  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDuplex
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetDuplex Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,9,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetSpeed
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       speed - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for 100Mb/s  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSpeed
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *speed
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetSpeed Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,8,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *speed);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtSetDuplex
*
* DESCRIPTION:
*       This routine sets the duplex mode of MII/SNI/RMII ports.
*
* INPUTS:
*       port - 	the logical port number.
*				(for FullSail, it will be port 2, and for ClipperShip, 
*				it could be either port 5 or port 6.)
*       mode -  GT_TRUE for Full Duplex,
*				GT_FALSE for Half Duplex.
*
* OUTPUTS: None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDuplex
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    IN  GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDuplex Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_MII_DUPLEX_CONFIG)) != GT_OK)
	{
		return retVal;
	}

	/* check if phy is not configurable. */
	if(IS_CONFIGURABLE_PHY(dev, hwPort))
	{
		/* 
		 * phy is configurable. this function is not for the port where phy 
		 * can be configured.
		 */
		return GT_NOT_SUPPORTED;
	}

    /* Set the duplex mode. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,9,1,(GT_U16)mode);

    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    /* return */
    return retVal;
}


