#include <Copyright.h>

/********************************************************************************
* gtBrgStp.c
*
* DESCRIPTION:
*       API definitions to handle port spanning tree state.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>



/*******************************************************************************
* gstpSetMode
*
* DESCRIPTION:
*       This routine Enable the Spanning tree.
*
* INPUTS:
*       en - GT_TRUE for enable, GT_FALSE for disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       when enabled, this function sets all port to blocking state, and inserts
*       the BPDU MAC into the ATU to be captured to CPU, on disable all port are
*       being modified to be in forwarding state.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetMode
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL  en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_PORT_STP_STATE   state;      /* STP state to be set.         */
    GT_ATU_ENTRY        atuEntry;   /* The ATU entry data to be set */
    GT_U8           i;

    DBG_INFO(("gstpSetMode Called.\n"));
    if(dev->deviceId == GT_88E6051)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    if((en == GT_TRUE) && (dev->stpMode == 1))
    {
        DBG_INFO(("OK.\n"));
        return GT_OK;
    }

    /* Set the STP state in the relevant register.  */
    if(en == GT_TRUE)
        state = GT_PORT_BLOCKING;
    else
        state = GT_PORT_FORWARDING;

    for(i = 0; i < dev->numOfPorts; i++)
    {
        retVal = gstpSetPortState(dev,i,state);
        if(retVal != GT_OK)
        {
            DBG_INFO(("Failed.\n"));
            return retVal;
        }
    }

    /* Set the Atu entry parameters.    */
    atuEntry.macAddr.arEther[0] = 0x01;
    atuEntry.macAddr.arEther[1] = 0x80;
    atuEntry.macAddr.arEther[2] = 0xC2;
    atuEntry.macAddr.arEther[3] = 0x00;
    atuEntry.macAddr.arEther[4] = 0x00;
    atuEntry.macAddr.arEther[5] = 0x00;
    atuEntry.portVec = ( 1 << GT_PORT_2_LPORT(dev->cpuPortNum));
    atuEntry.prio    = 3;
	atuEntry.DBNum = 0;
    atuEntry.entryState.mcEntryState = GT_MC_PRIO_MGM_STATIC;

    if(en == GT_TRUE)
    {
        retVal = gfdbAddMacEntry(dev,&atuEntry);
    }
    else if((en == GT_FALSE) && (dev->stpMode != 0))
        retVal = gfdbDelMacEntry(dev,&(atuEntry.macAddr));
    else
        retVal = GT_OK;

    if(en == GT_TRUE)
        dev->stpMode = 1;
    else
        dev->stpMode = 2;
    if(retVal == GT_OK)
        DBG_INFO(("OK.\n"));
    else
        DBG_INFO(("Failed.\n"));
    return retVal;
}



/*******************************************************************************
* gstpSetPortState
*
* DESCRIPTION:
*       This routine set the port state.
*
* INPUTS:
*       port  - the logical port number.
*       state - the port state to set.
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
GT_STATUS gstpSetPortState
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT           port,
    IN GT_PORT_STP_STATE  state
)
{
    GT_U8           phyPort;        /* Physical port                */
    GT_U16          data;           /* Data to write to register.   */
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gstpSetPortState Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);
    data    = state;

    /* Set the port state bits.             */
    retVal= hwSetPortRegField(dev,phyPort, QD_REG_PORT_CONTROL,0,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gstpGetPortState
*
* DESCRIPTION:
*       This routine returns the port state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       state - the current port state.
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
GT_STATUS gstpGetPortState
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT           port,
    OUT GT_PORT_STP_STATE  *state
)
{
    GT_U8           phyPort;        /* Physical port                */
    GT_U16          data;           /* Data read from register.     */
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gstpGetPortState Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* Get the port state bits.             */
    retVal = hwGetPortRegField(dev,phyPort, QD_REG_PORT_CONTROL,0,2,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *state = data & 0x3;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}
