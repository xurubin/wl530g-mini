#include <Copyright.h>

/********************************************************************************
* gtPortCtrl.c
*
* DESCRIPTION:
*       API implementation for switch port control.
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
* gprtSetForceFc
*
* DESCRIPTION:
*       This routine set the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
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
GT_STATUS gprtSetForceFc
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    force
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_PORT_STP_STATE  state;

    DBG_INFO(("gprtSetForceFc Called.\n"));

	/* Port should be disabled before Set Force Flow Control bit */
	retVal = gstpGetPortState(dev,port, &state);
    if(retVal != GT_OK)
	{
	    DBG_INFO(("gstpGetPortState failed.\n"));
		return retVal;
	}

	retVal = gstpSetPortState(dev,port, GT_PORT_DISABLE);
    if(retVal != GT_OK)
	{
	    DBG_INFO(("gstpSetPortState failed.\n"));
		return retVal;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(force, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Set the force flow control bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,15,1,data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

	/* Restore original stp state. */
	if(gstpSetPortState(dev,port, state) != GT_OK)
	{
	    DBG_INFO(("gstpSetPortState failed.\n"));
		return GT_FAIL;
	}

    return retVal;
}



/*******************************************************************************
* gprtGetForceFc
*
* DESCRIPTION:
*       This routine get the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
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
GT_STATUS gprtGetForceFc
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *force
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetForceFc Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,15,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *force);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gprtSetTrailerMode
*
* DESCRIPTION:
*       This routine set the egress trailer mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
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
GT_STATUS gprtSetTrailerMode
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetTrailerMode Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRAILER|DEV_TRAILER_P5|DEV_TRAILER_P4P5))
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
	    /* check if device supports this feature for this port */
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5|DEV_TRAILER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
	    /* check if device supports this feature for this port*/
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Set the trailer mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,1,data);

    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}



/*******************************************************************************
* gprtGetTrailerMode
*
* DESCRIPTION:
*       This routine get the egress trailer mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
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
GT_STATUS gprtGetTrailerMode
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetTrailerMode Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRAILER|DEV_TRAILER_P5|DEV_TRAILER_P4P5))
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
	    /* check if device supports this feature for this port */
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5|DEV_TRAILER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
	    /* check if device supports this feature for this port */
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Get the Trailer mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}




/*******************************************************************************
* gprtSetIngressMode
*
* DESCRIPTION:
*       This routine set the ingress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the ingress mode.
*
* OUTPUTS:
*       None.
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
GT_STATUS gprtSetIngressMode
(
    IN  GT_QD_DEV      *dev,
    IN GT_LPORT        port,
    IN GT_INGRESS_MODE mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIngressMode Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

#if 0
	/* 
	 * if IGMP/MLD Snooping is supported, 
	 * GT_TRAILER_INGRESS and GT_CPUPORT_INGRESS modes can be assigned only to
	 * CPU port.
	 */
    if(IS_IN_DEV_GROUP(dev,DEV_IGMP_SNOOPING))
	{
		if ((mode == GT_TRAILER_INGRESS) || (mode == GT_CPUPORT_INGRESS))
		{
			if(port != dev->cpuPortNum)
			{
			    DBG_INFO(("Given ingress mode is supported by CPU port only\n"));
				return GT_NOT_SUPPORTED;
			}
		}
	}
#endif

    /* Set the the Ingress Mode.        */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,8,2,(GT_U16)mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gprtGetIngressMode
*
* DESCRIPTION:
*       This routine get the ingress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the ingress mode.
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
GT_STATUS gprtGetIngressMode
(
    IN  GT_QD_DEV      *dev,
    IN  GT_LPORT        port,
    OUT GT_INGRESS_MODE *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIngressMode Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the Ingress Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 8, 2,&data);
    *mode = data;
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gprtSetMcRateLimit
*
* DESCRIPTION:
*       This routine set the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
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
GT_STATUS gprtSetMcRateLimit
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_MC_RATE   rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetMcRateLimit Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* this feature only exits in 6051, 6052, and 6012. It is replace with
     * Rate Cotrol Register in the future products, starting from clippership
     */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_MC_RATE_PERCENT)) != GT_OK)
        return retVal;

    /* Set the multicast rate limit.    */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,2,2,(GT_U16)rate);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gprtGetMcRateLimit
*
* DESCRIPTION:
*       This routine Get the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gprtGetMcRateLimit
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_MC_RATE  *rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read data        */

    DBG_INFO(("gprtGetMcRateLimit Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* this feature only exits in 6051, 6052, and 6012. It is replace with
     * Rate Cotrol Register in the future products, starting from clippership
     */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_MC_RATE_PERCENT)) != GT_OK)
        return retVal;

    /* Get the multicast rate limit.    */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 2, 2,&data);
    *rate = data;
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}


/* the following two APIs are added to support fullsail and clippership */

/*******************************************************************************
* gprtSetIGMPSnoop
*
* DESCRIPTION:
*       This routine set the IGMP Snoop. When set to one and this port receives
*       IGMP frame, the frame is switched to the CPU port, overriding all other 
*     	switching decisions, with exception for CPU's Trailer.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for IGMP Snoop or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIGMPSnoop
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIGMPSnoop Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_IGMP_SNOOPING)) != GT_OK)
      return retVal;

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the trailer mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,10,1, data);

    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}



/*******************************************************************************
* gprtGetIGMPSnoop
*
* DESCRIPTION:
*       This routine get the IGMP Snoop mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: IGMP Snoop enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIGMPSnoop
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIGMPSnoop Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_IGMP_SNOOPING)) != GT_OK)
      return retVal;

    /* Get the Ingress Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 10, 1, &data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}

/* the following two APIs are added to support clippership */

/*******************************************************************************
* gprtSetHeaderMode
*
* DESCRIPTION:
*       This routine set ingress and egress header mode of a switch port. 
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for header mode  or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetHeaderMode
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetHeaderMode Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    /* Fullsail (DEV_QD_88E6502) is an exception, and does not support this feature */
    if(IS_VALID_API_CALL(dev,port, DEV_HEADER|DEV_HEADER_P5|DEV_HEADER_P4P5) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5|DEV_HEADER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Set the header mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,11,1, data);

    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}



/*******************************************************************************
* gprtGetHeaderMode
*
* DESCRIPTION:
*       This routine gets ingress and egress header mode of a switch port. 
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: header mode enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHeaderMode
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetHeaderMode Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    /* Fullsail (DEV_QD_88E602) is an exception, and does not support this feature */
    if(IS_VALID_API_CALL(dev,port, DEV_HEADER|DEV_HEADER_P5|DEV_HEADER_P4P5) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5|DEV_HEADER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Get the Header Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 11, 1, &data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}

