#include <Copyright.h>

/********************************************************************************
* gtQosMap.c
*
* DESCRIPTION:
*       API implementation for qos mapping.
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
* gqosSetPortDefaultTc
*
* DESCRIPTION:
*       Sets the default traffic class for a specific port.
*
* INPUTS:
*       port      - logical port number
*       trafClass - default traffic class of a port.
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
GT_STATUS gcosSetPortDefaultTc
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_U8      trafClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gcosSetPortDefaultTc Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* Set the default port pri.  */
    retVal = hwSetPortRegField(dev,hwPort,QD_REG_PVID,14,2,trafClass);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}




/*******************************************************************************
* gqosSetPrioMapRule
*
* DESCRIPTION:
*       This routine sets priority mapping rule.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosSetPrioMapRule
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetPrioMapRule Called.\n"));
    /* translate bool to binary */
    BOOL_2_BIT(mode, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* Set the TaIfBoth.  */
    retVal = hwSetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL,6,1,data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}



/*******************************************************************************
* gqosGetPrioMapRule
*
* DESCRIPTION:
*       This routine get the priority mapping rule.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosGetPrioMapRule
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetPrioMapRule Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* get the TaIfBoth.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,6,1,&data);
    /* translate bool to binary */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gqosIpPrioMapEn
*
* DESCRIPTION:
*       This routine enables the IP priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gqosIpPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosIpPrioMapEn Called.\n"));
    /* translate bool to binary */
    BOOL_2_BIT(en, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* Set the useIp.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,5,1,data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}




/*******************************************************************************
* gqosGetIpPrioMapEn
*
* DESCRIPTION:
*       This routine return the IP priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosGetIpPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetIpPrioMapEn Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* Get the UseIp.  */
    retVal = hwGetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL,5,1,&data);
    /* translate bool to binary */
    BIT_2_BOOL(data, *en);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}




/*******************************************************************************
* gqosUserPrioMapEn
*
* DESCRIPTION:
*       This routine enables the user priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gqosUserPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosUserPrioMapEn Called.\n"));
    /* translate bool to binary */
    BOOL_2_BIT(en, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* Set the useTag.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,4,1,data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}




/*******************************************************************************
* gqosGetUserPrioMapEn
*
* DESCRIPTION:
*       This routine return the user priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosGetUserPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetUserPrioMapEn Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK ) 
      return retVal;
	
    /* Get the UseTag.  */
    retVal = hwGetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL,4,1,&data);
    /* translate bool to binary */
    BIT_2_BOOL(data, *en);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}




/*******************************************************************************
* gcosGetUserPrio2Tc
*
* DESCRIPTION:
*       Gets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
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
GT_STATUS gcosGetUserPrio2Tc
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     userPrior,
    OUT GT_U8     *trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */
    GT_U16          data;           /* store the read data          */

    DBG_INFO(("gcosGetUserPrio2Tc Called.\n"));

    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;
	
    /* calc the bit offset */
    bitOffset = ((userPrior & 0x7) * 2);
    /* Get the traffic class for the VPT.  */
    retVal = hwGetGlobalRegField(dev,QD_REG_IEEE_PRI,bitOffset,2,&data);
    *trClass = (GT_U8)data;
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gcosSetUserPrio2Tc
*
* DESCRIPTION:
*       Sets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority of a port.
*       trClass   - the Traffic Class the received frame is assigned.
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
GT_STATUS gcosSetUserPrio2Tc
(
    IN  GT_QD_DEV *dev,
    IN GT_U8      userPrior,
    IN GT_U8      trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */

    DBG_INFO(("gcosSetUserPrio2Tc Called.\n"));
    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;
	
    /* calc the bit offset */
    bitOffset = ((userPrior & 0x7) * 2);
    /* Set the traffic class for the VPT.  */
    retVal = hwSetGlobalRegField(dev,QD_REG_IEEE_PRI, bitOffset,2,trClass);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gcosGetDscp2Tc
*
* DESCRIPTION:
*       This routine retrieves the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to query.
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
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
GT_STATUS gcosGetDscp2Tc
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     dscp,
    OUT GT_U8     *trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */
    GT_U8           regOffset;      /* the reg offset in the IP tbl */
    GT_U16          data;           /* store the read data          */

    DBG_INFO(("gcosGetDscp2Tc Called.\n"));
    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;
	
    /* calc the bit offset */
    bitOffset = (((dscp & 0x3f) % 8) * 2);
    regOffset = ((dscp & 0x3f) / 8);
    /* Get the traffic class for the IP dscp.  */
    retVal = hwGetGlobalRegField(dev,(GT_U8)(QD_REG_IP_PRI_BASE+regOffset),
                                 bitOffset, 2, &data);
    *trClass = (GT_U8)data;
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



/*******************************************************************************
* gcosSetDscp2Tc
*
* DESCRIPTION:
*       This routine sets the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to map.
*       trClass - the Traffic Class the received frame is assigned.
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
GT_STATUS gcosSetDscp2Tc
(
    IN  GT_QD_DEV *dev,
    IN GT_U8      dscp,
    IN GT_U8      trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */
    GT_U8           regOffset;      /* the reg offset in the IP tbl */

    DBG_INFO(("gcosSetDscp2Tc Called.\n"));
    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;
	
    /* calc the bit offset */
    bitOffset = (((dscp & 0x3f) % 8) * 2);
    regOffset = ((dscp & 0x3f) / 8);
    /* Set the traffic class for the IP dscp.  */
    retVal = hwSetGlobalRegField(dev,(GT_U8)(QD_REG_IP_PRI_BASE+regOffset),
                                 bitOffset, 2, trClass);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}



