#include <Copyright.h>

/********************************************************************************
* gtPortRateCtrl.c
*
* DESCRIPTION:
*       API definitions to handle port rate control registers (0xA).
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
* grcSetLimitMode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*       mode 	- rate control ingress limit mode. 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetLimitMode
(
    IN GT_QD_DEV             *dev,
    IN GT_LPORT 	     port,
    IN GT_RATE_LIMIT_MODE    mode
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetLimitMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,14,2,(GT_U16)mode );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetLimitMode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       mode 	- rate control ingress limit mode. 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetLimitMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_RATE_LIMIT_MODE    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetLimitMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,14,2,&data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *mode = data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetPri3Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
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
GT_STATUS grcSetPri3Rate
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetPri3Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    BOOL_2_BIT(mode,data);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,13,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetPri3Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetPri3Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri3Rate Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,13,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcSetPri2Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
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
GT_STATUS grcSetPri2Rate
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetPri2Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    BOOL_2_BIT(mode,data);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,12,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetPri2Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetPri2Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri2Rate Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,12,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcSetPri1Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
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
GT_STATUS grcSetPri1Rate
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetPri1Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);
    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    BOOL_2_BIT(mode,data);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,11,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetPri1Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetPri1Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri1Rate Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);
    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,11,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcSetPri0Rate
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached 
*       	  or exceeded. 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetPri0Rate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_PRI0_RATE    rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetPri0Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,8,3,(GT_U16)rate );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetPri0Rate
*
* DESCRIPTION:
*       This routine gets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number to set.
*
* OUTPUTS:
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached 
*       	  or exceeded. 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetPri0Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_PRI0_RATE    *rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri0Rate Called.\n"));

    if(rate == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,8,3,&data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *rate = data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetBytesCount
*
* DESCRIPTION:
*       This routine sets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	  - logical port number to set.
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetBytesCount
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL 	 limitMGMT,
    IN GT_BOOL 	 countIFG,
    IN GT_BOOL 	 countPre
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U16          data;           /* data for bytes count         */

    DBG_INFO(("grcSetBytesCount Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;

    data = 0;
    if(	limitMGMT == GT_TRUE ) data |=4;
    if(	 countIFG == GT_TRUE ) data |=2;
    if(	 countPre == GT_TRUE ) data |=1;

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,4,3,data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetBytesCount
*
* DESCRIPTION:
*       This routine gets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	- logical port number 
*
* OUTPUTS:
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetBytesCount
(	
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL 	 *limitMGMT,
    IN GT_BOOL 	 *countIFG,
    IN GT_BOOL 	 *countPre
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetBytesCount Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;

    if (limitMGMT == NULL || countIFG == NULL || countPre == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,4,3,&data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *limitMGMT = *countIFG = *countPre = GT_FALSE;
    if ( data & 4 ) *limitMGMT = GT_TRUE;
    if ( data & 2 ) *countIFG  = GT_TRUE;
    if ( data & 1 ) *countPre  = GT_TRUE;
	    
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetEgressRate
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*       rate    - egress data rate limit.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetEgressRate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_EGRESS_RATE  rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetEgressRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_EGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,0,3,(GT_U16)rate );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetEgressRate
*
* DESCRIPTION:
*       This routine gets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rate    - egress data rate limit.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetEgressRate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_EGRESS_RATE  *rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetEgressRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_EGRESS_RATE_KBPS)) != GT_OK ) 
      return retVal;
	
    if(rate == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,0,3,&data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *rate = data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}
