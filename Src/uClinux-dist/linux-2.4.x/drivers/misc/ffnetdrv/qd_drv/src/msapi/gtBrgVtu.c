#include <Copyright.h>

/*******************************************************************************
* gtBrgVtu.c
*
* DESCRIPTION:
*       API definitions for Vlan Translation Unit for 802.1Q.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/****************************************************************************/
/* Forward function declaration.                                            */
/****************************************************************************/
static GT_STATUS vtuOperationPerform
(
    IN	    GT_QD_DEV           *dev,
    IN      GT_VTU_OPERATION    vtuOp,
    INOUT   GT_U8               *DBNum,
    INOUT   GT_U16              *vid,
    INOUT   GT_U8               *valid,
    INOUT   GT_VTU_DATA         *vtuData
);

/*******************************************************************************
* gvtuGetEntryCount
*
* DESCRIPTION:
*       Gets the current number of entries in the VTU table
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numEntries - number of VTU entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - vlan does not exist.
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryCount
(
    IN  GT_QD_DEV *dev,
    OUT GT_U32    *numEntries
)
{
    GT_U8               DBNum;
    GT_U16               vid;
    GT_U8               valid;
    GT_VTU_DATA         vtuData;
    GT_U32		numOfEntries;
    GT_STATUS       	retVal;

    DBG_INFO(("gvtuGetEntryCount Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;

    vid = 0xFFF;
    DBNum=0;

    numOfEntries = 0;
    while(1)
    {
        retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&DBNum,&vid,&valid,&vtuData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
	    return retVal;
	}
	if( vid==0xFFF )
	{
	  if (valid==1) numOfEntries++;

	  break;
	}

        numOfEntries++;
    }

    *numEntries = numOfEntries;

    DBG_INFO(("OK.\n"));
    return GT_OK;

}


/*******************************************************************************
* gvtuGetEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic entry from the VTU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuEntry - match VTU entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from vid of all one's
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryFirst
(
    IN  GT_QD_DEV       *dev,
    OUT GT_VTU_ENTRY    *vtuEntry
)
{
    GT_U8               DBNum;
    GT_U16               vid;
    GT_U8               valid;
    GT_VTU_DATA         vtuData;
    GT_STATUS       	retVal;
    GT_U8       	i; 

    DBG_INFO(("gvtuGetEntryFirst Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;

    vid = 0xFFF;
    DBNum=0;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&DBNum,&vid,&valid, &vtuData);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */ 

    if((vid == 0xFFF) && (valid == 0))
	return GT_NO_SUCH;

    vtuEntry->DBNum = DBNum;
    vtuEntry->vid   = vid;
    for( i =0; i< dev->numOfPorts;i++)
    {
      vtuEntry->vtuData.memberTagP[i]=vtuData.memberTagP[i];
      vtuEntry->vtuData.portStateP[i]=vtuData.portStateP[i];
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gvtuGetEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic VTU entry from the specified VID.
*
* INPUTS:
*       vtuEntry - the VID to start the search.
*
* OUTPUTS:
*       vtuEntry - match VTU  entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from the VID specified by the user.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryNext
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_VTU_ENTRY  *vtuEntry
)
{
    GT_U8               DBNum;
    GT_U16               vid;
    GT_U8               valid;
    GT_VTU_DATA         vtuData;
    GT_STATUS       	retVal;
    GT_U8       	i; 

    DBG_INFO(("gvtuGetEntryNext Called.\n"));
    
    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;

    DBNum = vtuEntry->DBNum;
    vid   = vtuEntry->vid;
    valid = 0;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&DBNum,&vid,&valid, &vtuData);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */ 

	if((vid == 0xFFF) && (valid == 0))
		return GT_NO_SUCH;

    vtuEntry->DBNum = DBNum;
    vtuEntry->vid   = vid;
    for( i =0; i< dev->numOfPorts;i++)
    {
      vtuEntry->vtuData.memberTagP[i]=vtuData.memberTagP[i];
      vtuEntry->vtuData.portStateP[i]=vtuData.portStateP[i];
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvtuFindVidEntry
*
* DESCRIPTION:
*       Find VTU entry for a specific VID, it will return the entry, if found, 
*       along with its associated data 
*
* INPUTS:
*       vtuEntry - contains the VID to searche for 
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       vtuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuFindVidEntry
(
    IN  GT_QD_DEV       *dev,
    INOUT GT_VTU_ENTRY  *vtuEntry,
    OUT GT_BOOL         *found
)
{
    GT_U8               DBNum;
    GT_U16               vid;
    GT_U8               valid;
    GT_VTU_DATA         vtuData;
    GT_STATUS       	retVal;
    GT_U8               i;

    DBG_INFO(("gvtuFindVidEntry Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;

    *found = GT_FALSE;


    /* Decrement 1 from vid    */
    vid   = vtuEntry->vid-1;
    valid = 0; /* valid is not used as input in this operation */
    DBNum = vtuEntry->DBNum;

    retVal = vtuOperationPerform(dev,GET_NEXT_ENTRY,&DBNum,&vid,&valid, &vtuData);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    /* retrive the value from the operation */ 

    if( (vid !=vtuEntry->vid) | (valid !=1) )
    {
          DBG_INFO(("Failed.\n"));
          return GT_NO_SUCH;
    }
    vtuEntry->DBNum = DBNum;
    for( i =0; i< dev->numOfPorts;i++)
    {
      vtuEntry->vtuData.memberTagP[i]=vtuData.memberTagP[i];
      vtuEntry->vtuData.portStateP[i]=vtuData.portStateP[i];
    }

    *found = GT_TRUE;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvtuFlush
*
* DESCRIPTION:
*       This routine removes all entries from VTU Table.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuFlush
(
    IN  GT_QD_DEV       *dev
)
{
    GT_STATUS       retVal;

    DBG_INFO(("gvtuFlush Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;

    retVal = vtuOperationPerform(dev,FLUSH_ALL,NULL,NULL,NULL,NULL);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvtuAddEntry
*
* DESCRIPTION:
*       Creates the new entry in VTU table based on user input.
*
* INPUTS:
*       vtuEntry    - vtu entry to insert to the VTU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*       GT_FULL			  - vtu table is full
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuAddEntry
(
    IN  GT_QD_DEV   *dev,
    IN GT_VTU_ENTRY *vtuEntry
)
{
    GT_U8               DBNum;
    GT_U16              vid;
    GT_U8               valid;
    GT_VTU_DATA         vtuData;
    GT_STATUS       	retVal;
    GT_U8       	i; 
    GT_VTU_ENTRY 	tmpVtuEntry;
	GT_BOOL		 	found;
	int				count = 5000000;

    DBG_INFO(("gvtuAddEntry Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;

    DBNum = vtuEntry->DBNum;
    vid   = vtuEntry->vid;
    valid = 1; /* for load operation */
    for( i =0; i< dev->numOfPorts;i++)
    {
      vtuData.memberTagP[i] = vtuEntry->vtuData.memberTagP[i];
      vtuData.portStateP[i] = vtuEntry->vtuData.portStateP[i];
    }

    retVal = vtuOperationPerform(dev,LOAD_PURGE_ENTRY,&DBNum,&vid,&valid, &vtuData);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	/* verify that the given entry has been added */
	tmpVtuEntry.vid = vtuEntry->vid;
	tmpVtuEntry.DBNum = vtuEntry->DBNum;

	if((retVal = gvtuFindVidEntry(dev,&tmpVtuEntry,&found)) != GT_OK)
	{
		while(count--);
		if((retVal = gvtuFindVidEntry(dev,&tmpVtuEntry,&found)) != GT_OK)
		{
			DBG_INFO(("Added entry cannot be found\n"));
			return retVal;
		}
	}
	if(found == GT_FALSE)
	{
		DBG_INFO(("Added entry cannot be found\n"));
		return GT_FAIL;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvtuDelEntry
*
* DESCRIPTION:
*       Deletes VTU entry specified by user.
*
* INPUTS:
*       vtuEntry - the VTU entry to be deleted 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuDelEntry
(
    IN  GT_QD_DEV   *dev,
    IN GT_VTU_ENTRY *vtuEntry
)
{
    GT_U8               DBNum;
    GT_U16               vid;
    GT_U8               valid;
    GT_VTU_DATA         vtuData;
    GT_STATUS       	retVal;

    DBG_INFO(("gvtuDelEntry Called.\n"));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;
    
    DBNum = vtuEntry->DBNum;
    vid   = vtuEntry->vid;
    valid = 0; /* for delete operation */

    retVal = vtuOperationPerform(dev,LOAD_PURGE_ENTRY,&DBNum,&vid,&valid, &vtuData);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvtuGetViolation
*
* DESCRIPTION:
*       Get VTU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation
(
    IN  GT_QD_DEV         *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_U8               spid;
    GT_U16               vid;
    GT_U16               intCause;
    GT_STATUS       	retVal;

    DBG_INFO(("gvtuGetViolation Called.\n"));

#if 0
    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK) 
      return retVal;
#endif
    
	/* check which Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,4,3,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read VTU OPERATION Register.\n"));
        return retVal;
    }

	if (intCause == 0)
	{
		/* No Violation occurred. */
		vtuIntStatus->vtuIntCause = 0;
		return GT_OK;
	}

    spid = 0;

    retVal = vtuOperationPerform(dev,SERVICE_VIOLATIONS,&spid,&vid,NULL, NULL);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (vtuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

	if(spid == 0xF)
	{
		vtuIntStatus->vtuIntCause = GT_VTU_FULL_VIOLATION;
		vtuIntStatus->spid = spid;
		vtuIntStatus->vid = 0;
	}
	else
	{
		vtuIntStatus->vtuIntCause = intCause & (GT_MEMBER_VIOLATION | GT_MISS_VIOLATION);
		vtuIntStatus->spid = spid;
		vtuIntStatus->vid = vid;
	}
	
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* vtuOperationPerform
*
* DESCRIPTION:
*       This function is used by all VTU control functions, and is responsible
*       to write the required operation into the VTU registers.
*
* INPUTS:
*       vtuOp       - The VTU operation bits to be written into the VTU
*                     operation register.
*       entryPri    - The EntryPri field in the VTU Data register.
*       portVec     - The portVec field in the VTU Data register.
*       entryState  - The EntryState field in the VTU Data register.
*       vtuMac      - The Mac address to be written to the VTU Mac registers.
*
* OUTPUTS:
*       entryPri    - The EntryPri field in case the vtuOp is GetNext.
*       portVec     - The portVec field in case the vtuOp is GetNext.
*       entryState  - The EntryState field in case the vtuOp is GetNext.
*       vtuMac      - The returned Mac address in case the vtuOp is GetNext.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/

static GT_STATUS vtuOperationPerform
(
    IN	    GT_QD_DEV           *dev,
    IN      GT_VTU_OPERATION    vtuOp,
    INOUT   GT_U8               *DBNum,
    INOUT   GT_U16              *vid,
    INOUT   GT_U8               *valid,
    INOUT   GT_VTU_DATA         *vtuData
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */

    gtSemTake(dev,dev->vtuRegsSem,OS_WAIT_FOREVER);

    /* Wait until the VTU in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
    }

    /* Set the VTU data register    */
    /* There is no need to setup data reg. on flush, get next, or service violation */
    if((vtuOp != FLUSH_ALL) && (vtuOp != GET_NEXT_ENTRY) && (vtuOp != SERVICE_VIOLATIONS))
    {
	/* get data from data register for ports 0 to 3 */

	data = (vtuData ->memberTagP[0] & 3)     |
	      ((vtuData ->memberTagP[1] & 3))<<4 | 
	      ((vtuData ->memberTagP[2] & 3))<<8 ;
	data |= ((vtuData ->portStateP[0] & 3)<<2|
	      ((vtuData ->portStateP[1] & 3))<<6 | 
	      ((vtuData ->portStateP[2] & 3))<<10) ;

	/****************** if this is for CliperShip *******************/
        if(dev->numOfPorts > 3)
	{
	  /* fullsail has 3 ports(0,1,2), clippership has 7 prots(3 to 6)      */
	  /* port 3 is set on VTU_DATA1_REG, port 4-6 are set on VTU_DATA2_REG */
	  data |= ((vtuData ->memberTagP[3] & 3))<<12 ;
	  data |= ((vtuData ->portStateP[3] & 3))<<14 ;
	}
	/****************** end of  CliperShip **************************/

        retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA1_REG,data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

	/****************** if this is for CliperShip *******************/

        if(dev->numOfPorts > 4)
	{
	  /* also need to set data register  ports 4 to 6 */

	  data = (vtuData ->memberTagP[4] & 3)   |
	      ((vtuData ->memberTagP[5] & 3))<<4 |
	      ((vtuData ->memberTagP[6] & 3))<<8 ;
      data |= ((vtuData ->portStateP[4] & 3)<<2|
	      ((vtuData ->portStateP[5] & 3))<<6 | 
	      ((vtuData ->portStateP[6] & 3))<<10) ;

          retVal = hwWriteGlobalReg(dev,QD_REG_VTU_DATA2_REG,data);
          if(retVal != GT_OK)
          {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
          }
	}
	/****************** end of  CliperShip *******************/
    }

    /* Set the Vtu VID registers    */
    /* There is no need to setup VID reg. on flush and service violation */
    if((vtuOp != FLUSH_ALL) && (vtuOp != SERVICE_VIOLATIONS) )
    {
            data= ( (*vid) & 0xFFF ) | ( (*valid) << 12 );
            retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_VTU_VID_REG),data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->vtuRegsSem);
                return retVal;
            }
    }

    /* Start the VTU Operation by defining the DBNum, vtuOp and VTUBusy    */
    /* Flush operation will skip the above two setup (for data and vid), and 
     * come to here directly
     */

    if ( DBNum == NULL )
	data = (1 << 15) | (vtuOp << 12);
    else
	data = (1 << 15) | (vtuOp << 12) | (*DBNum);

    retVal = hwWriteGlobalReg(dev,QD_REG_VTU_OPERATION,data);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->vtuRegsSem);
        return retVal;
    }

    /* only two operations need to go through the mess below to get some data 
     * after the operations -  service violatio and get next entry
     */

    /* If the operation is to service violation operation wait for the response   */
    if(vtuOp == SERVICE_VIOLATIONS)
    {
        /* Wait until the VTU in ready. */
        data = 1;
        while(data == 1)
        {
            retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,15,1,&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->vtuRegsSem);
                return retVal;
            }
        }

    	/* get the Source Port ID that was involved in the violation */
        retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,0,4,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

		*DBNum = (GT_U8)(data & 0xF);

    	/* get the VID that was involved in the violation */
	
        retVal = hwReadGlobalReg(dev,QD_REG_VTU_VID_REG,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }

        /* Get the vid - bits 0-11 */
        *vid   = data & 0xFFF;


     } /* end of service violations */

    /* If the operation is a get next operation wait for the response   */
    if(vtuOp == GET_NEXT_ENTRY)
    {
        /* Wait until the VTU in ready. */
        data = 1;
        while(data == 1)
        {
            retVal = hwGetGlobalRegField(dev,QD_REG_VTU_OPERATION,15,1,&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->vtuRegsSem);
                return retVal;
            }
        }

	/****************** get the DBNum *******************/
		*DBNum = data & 0xF;

	/****************** get the vid *******************/

        retVal = hwReadGlobalReg(dev,QD_REG_VTU_VID_REG,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
        }
        /* the vid is bits 0-11 */
        *vid   = data & 0xFFF;
        /* the vid valid is bits 12 */
        *valid   = (data >> 12) & 1;
	

	/****************** get the MemberTagP *******************/
        retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA1_REG,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->vtuRegsSem);
            return retVal;
	}
	/* get data from data register for ports 0 to 3 */
        vtuData ->memberTagP[0]  =  data & 3 ;
        vtuData ->memberTagP[1]  = (data >> 4) & 3 ;
        vtuData ->memberTagP[2]  = (data >> 8) & 3 ;
        vtuData ->portStateP[0]  = (data >> 2) & 3 ;
        vtuData ->portStateP[1]  = (data >> 6) & 3 ;
        vtuData ->portStateP[2]  = (data >> 10) & 3 ;

	/****************** if this is for CliperShip *******************/

        if(dev->numOfPorts > 3)
	{
	  /* fullsail has 3 ports, clippership has 7 prots */
	  vtuData->memberTagP[3]  = (data >>12) & 3 ;
	  vtuData->portStateP[3]  = (data >>14) & 3 ;

	  /* get data from data register for ports 4 to 6 */
	  retVal = hwReadGlobalReg(dev,QD_REG_VTU_DATA2_REG,&data);
	  if(retVal != GT_OK)
	  {
	      gtSemGive(dev,dev->vtuRegsSem);
	      return retVal;
	  }
	  vtuData ->memberTagP[4]  = data & 3 ;
	  vtuData ->memberTagP[5]  = (data >> 4) & 3 ;
	  vtuData ->memberTagP[6]  = (data >> 8) & 3 ;
	  vtuData ->portStateP[4]  = (data >> 2) & 3 ;
	  vtuData ->portStateP[5]  = (data >> 6) & 3 ;
	  vtuData ->portStateP[6]  = (data >> 10) & 3 ;
	}
	/****************** end of  CliperShip *******************/

    } /* end of get next entry */

    gtSemGive(dev,dev->vtuRegsSem);
    return GT_OK;
}
