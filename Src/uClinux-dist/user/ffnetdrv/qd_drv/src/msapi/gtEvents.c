#include <Copyright.h>

/********************************************************************************
* gtEvents.c
*
* DESCRIPTION:
*       API definitions for system interrupt events handling.
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
* eventSetActive
*
* DESCRIPTION:
*       This routine enables/disables the receive of an hardware driven event.
*
* INPUTS:
*       eventType - the event type. any combination of the folowing: 
*       	GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,  
*       	GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
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
GT_STATUS eventSetActive
(
    IN GT_QD_DEV *dev,
    IN GT_U32 	 eventType
)
{
    GT_STATUS   retVal;   
    GT_U16 	data;	

    DBG_INFO(("eventSetActive Called.\n"));

	data = (GT_U16) eventType;

	if(data & ~GT_INT_MASK)
	{
	    DBG_INFO(("Invalid event type.\n"));
		return GT_FAIL;
	}

    /* Set the IntEn bit.               */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,0,7,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* eventGetIntStatus
*
* DESCRIPTION:
*       This routine reads an hardware driven event status.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       intCause -  It provides the source of interrupt of the following:
*       GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,  
*       GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
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
GT_STATUS eventGetIntStatus
(
    IN GT_QD_DEV *dev,
    OUT GT_U16   *intCause
)
{
    GT_STATUS       retVal;         /* Function calls return value.     */

    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,0,7,intCause);

    if(retVal != GT_OK)
        return GT_FALSE;

    return (*intCause)?GT_TRUE:GT_FALSE;
}


/*******************************************************************************
* gvtuGetIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of VTU interrupt occured
*
* INPUTS:
*       	intType - the type of interrupt which causes an interrupt.
*			any combination of 
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_VTU_FULL_VIOLATION
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gvtuGetIntStatus
(
    IN GT_QD_DEV          *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_STATUS       retVal;      

    DBG_INFO(("gvtuGetIntStatus Called.\n"));
   
    /* check if device supports this feature */
    if((IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK ) 
      return GT_FAIL; 

    retVal = gvtuGetViolation(dev,vtuIntStatus);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    
    return retVal;
}
