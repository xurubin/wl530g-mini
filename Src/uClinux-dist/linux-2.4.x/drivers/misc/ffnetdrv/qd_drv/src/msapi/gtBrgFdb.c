#include <Copyright.h>

/********************************************************************************
* gtBrgFdb.c
*
* DESCRIPTION:
*       API definitions for Multiple Forwarding Databases 
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
static GT_STATUS atuOperationPerform
(
    IN      GT_QD_DEV           *dev,
    IN      GT_ATU_OPERARION    atuOp,
    IN      GT_U8               DBNum,
    INOUT   GT_U8               *entryPri,
    INOUT   GT_U8               *portVec,
    INOUT   GT_U8               *entryState,
    INOUT   GT_ETHERADDR        *atuMac
);

/*******************************************************************************
* gfdbSetAtuSize
*
* DESCRIPTION:
*       Sets the Mac address table size.
*
* INPUTS:
*       size    - Mac address table size.
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
GT_STATUS gfdbSetAtuSize
(
    IN GT_QD_DEV    *dev,
    IN ATU_SIZE     size
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gfdbSetAtuSize Called.\n"));

	switch(size)
	{
		case ATU_SIZE_256:
			if (IS_IN_DEV_GROUP(dev,DEV_ATU_256_2048))
				data = 0;
			else
				return GT_NOT_SUPPORTED;
			break;
    	case ATU_SIZE_512:
    	case ATU_SIZE_1024:
    	case ATU_SIZE_2048:
			if (IS_IN_DEV_GROUP(dev,DEV_ATU_256_2048))
				data = (GT_U16)size;
			else
				data = (GT_U16)size - 1;
			break;

    	case ATU_SIZE_4096:
			if ((IS_IN_DEV_GROUP(dev,DEV_ATU_256_2048))||(IS_IN_DEV_GROUP(dev,DEV_ATU_562_2048)))
				return GT_NOT_SUPPORTED;
			else
				data = 3;
			break;
		default:
			return GT_NOT_SUPPORTED;
	}
	
    /* Set the Software reset bit.                  */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,12,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Make sure the reset operation is completed.  */
    data = 0;
    while(data == 0)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,11,1,&data);
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
* gfdbGetAgingTimeRange
*
* DESCRIPTION:
*       Gets the maximal and minimum age times that the hardware can support.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       maxTimeout - max aging time in secounds.
*       minTimeout - min aging time in secounds.
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
GT_STATUS gfdbGetAgingTimeRange
(
    IN GT_QD_DEV    *dev,
    OUT GT_U32 *maxTimeout,
    OUT GT_U32 *minTimeout
)
{
    DBG_INFO(("gfdbGetAgingTimeRange Called.\n"));
    if((maxTimeout == NULL) || (minTimeout == NULL))
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    *minTimeout = 16;
    *maxTimeout = 4080;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbSetAgingTimeout
*
* DESCRIPTION:
*       Sets the timeout period in seconds for aging out dynamically learned
*       forwarding information. The standard recommends 300 sec.
*
* INPUTS:
*       timeout - aging time in seconds.
*
* OUTPUTS:
*       None.
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
GT_STATUS gfdbSetAgingTimeout
(
    IN GT_QD_DEV    *dev,
    IN GT_U32 timeout
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gfdbSetAgingTimeout Called.\n"));
    data = (GT_U16)(timeout/16);

    /* Set the Time Out value.              */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,4,8,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbGetAtuDynamicCount
*
* DESCRIPTION:
*       Gets the current number of dynamic unicast entries in this
*       Filtering Database.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numDynEntries - number of dynamic entries.
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
GT_STATUS gfdbGetAtuDynamicCount
(
    IN GT_QD_DEV    *dev,
    OUT GT_U32 *numDynEntries
)
{
    GT_ETHERADDR    mac;            
    GT_U8           entryPri;      
    GT_U8           portVec;      
    GT_U8           entryState;  
    GT_U32          numOfEntries;
    GT_STATUS       retVal;
    GT_U8	    maxDbNum, dbNum;

    DBG_INFO(("gfdbGetAtuDynamicCount Called.\n"));

    numOfEntries = 0;
	
	if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_FULL))
		maxDbNum = 16;
	else
		maxDbNum = 1;

	for(dbNum=0; dbNum<maxDbNum; dbNum++)
	{
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		    gtMemSet(mac.arEther,0,sizeof(GT_ETHERADDR));
		else
    		gtMemSet(mac.arEther,0xFF,sizeof(GT_ETHERADDR));

	    while(1)
    	{
	        retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,dbNum,&entryPri,
				             &portVec, &entryState,&mac);
        	if(retVal != GT_OK)
	        {
    	        DBG_INFO(("Failed.\n"));
        	    return retVal;
	        }

    	    if(GT_IS_BROADCAST_MAC(mac))
        	    break;

	        if((GT_IS_MULTICAST_MAC(mac)) || (entryState == GT_UC_STATIC))
    	    {
	            continue;
    	    }

        	numOfEntries++;
	    }
	}

    *numDynEntries = numOfEntries;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetAtuEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic MAC address entry from the ATU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from Mac[00:00:00:00:00:00]
*
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryFirst
(
    IN GT_QD_DEV    *dev,
    OUT GT_ATU_ENTRY    *atuEntry
)
{
    GT_ETHERADDR    mac;            
    GT_U8           DBNum;	   
    GT_U8           entryPri;     
    GT_U8           portVec;     
    GT_U8           entryState; 
    GT_STATUS       retVal;

    DBG_INFO(("gfdbGetAtuEntryFirst Called.\n"));

	if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
	    gtMemSet(mac.arEther,0,sizeof(GT_ETHERADDR));
	else
    	gtMemSet(mac.arEther,0xFF,sizeof(GT_ETHERADDR));

	DBNum = atuEntry->DBNum;

    DBG_INFO(("DBNum : %i\n",DBNum));

    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,DBNum,&entryPri,
		                 &portVec,&entryState, &mac);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (atuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    if(GT_IS_BROADCAST_MAC(mac))
    {
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
		else if(entryState == 0)
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
    }

    gtMemCpy(atuEntry->macAddr.arEther,mac.arEther,6);
    atuEntry->portVec   = portVec;
    atuEntry->prio      = entryPri;

    if(GT_IS_MULTICAST_MAC(mac))
    {
        if(dev->deviceId == GT_88E6051)
        {
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
        }

        atuEntry->entryState.mcEntryState = entryState;
    }
    else
    {
		atuEntry->entryState.ucEntryState = entryState;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbGetAtuEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic MAC address from the specified Mac Addr.
*
* INPUTS:
*       atuEntry - the Mac Address to start the search.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from atu.macAddr[xx:xx:xx:xx:xx:xx] specified by the
*       user.
*
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryNext
(
    IN GT_QD_DEV    *dev,
    INOUT GT_ATU_ENTRY  *atuEntry
)
{
    GT_ETHERADDR    mac;            
    GT_U8           DBNum;	   
    GT_U8           entryPri;     
    GT_U8           portVec;     
    GT_U8           entryState; 
    GT_STATUS       retVal;

    DBG_INFO(("gfdbGetAtuEntryNext Called.\n"));
    gtMemCpy(mac.arEther,atuEntry->macAddr.arEther,6);

	DBNum = atuEntry->DBNum;
    DBG_INFO(("DBNum : %i\n",DBNum));

    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,DBNum,&entryPri,
		                 &portVec,&entryState, &mac);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (atuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    if(GT_IS_BROADCAST_MAC(mac))
    {
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
		else if(entryState == 0)
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
    }

    gtMemCpy(atuEntry->macAddr.arEther,mac.arEther,6);
    atuEntry->portVec   = portVec;
    atuEntry->prio      = entryPri;

    if(GT_IS_MULTICAST_MAC(mac))
    {
        if(dev->deviceId == GT_88E6051)
        {
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
        }

        atuEntry->entryState.mcEntryState = entryState;
    }
    else
    {
		atuEntry->entryState.ucEntryState = entryState;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbFindAtuMacEntry
*
* DESCRIPTION:
*       Find FDB entry for specific MAC address from the ATU.
*
* INPUTS:
*       atuEntry - the Mac address to search.
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       atuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbFindAtuMacEntry
(
    IN GT_QD_DEV    *dev,
    INOUT GT_ATU_ENTRY  *atuEntry,
    OUT GT_BOOL         *found
)
{
    GT_ETHERADDR    mac;            
    GT_U8           DBNum;	   
    GT_U8           entryPri;     
    GT_U8           portVec;     
    GT_U8           entryState;
    GT_STATUS       retVal;
    int           i;

    DBG_INFO(("gfdbFindAtuMacEntry Called.\n"));
    *found = GT_FALSE;
    gtMemCpy(mac.arEther,atuEntry->macAddr.arEther,6);
	DBNum = atuEntry->DBNum;

    /* Decrement 1 from mac address.    */
    for(i=5; i >= 0; i--)
    {
        if(mac.arEther[i] != 0)
        {
            mac.arEther[i] -= 1;
            break;
        }
		else
            mac.arEther[i] = 0xFF;
    }

    /* Check if the given mac equals zero   */
    if(i == -1)
    {
        DBG_INFO(("Failed.\n"));
        return GT_NO_SUCH;
    }

    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,DBNum,&entryPri,
		                 &portVec,&entryState, &mac);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    if(GT_IS_BROADCAST_MAC(mac))
    {
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
		else if(entryState == 0)
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
    }

	if(gtMemCmp(atuEntry->macAddr.arEther,mac.arEther, GT_ETHERNET_HEADER_SIZE))
	{
        DBG_INFO(("Failed.\n"));
        return GT_NO_SUCH;
	}

    atuEntry->portVec   = portVec;
    atuEntry->prio      = entryPri;

    if(GT_IS_MULTICAST_MAC(mac))
    {
        if(dev->deviceId == GT_88E6051)
        {
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
        }

        atuEntry->entryState.mcEntryState = entryState;
    }
    else
    {
		atuEntry->entryState.ucEntryState = entryState;
    }

    *found = GT_TRUE;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbFlush
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the MAC Address
*       Table.
*
* INPUTS:
*       flushCmd - the flush operation type.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlush
(
    IN GT_QD_DEV    *dev,
    IN GT_FLUSH_CMD flushCmd
)
{
    GT_STATUS       retVal;

    DBG_INFO(("gfdbFlush Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if(flushCmd == GT_FLUSH_ALL)
        retVal = atuOperationPerform(dev,FLUSH_ALL,0,NULL,NULL,NULL,NULL);
    else
        retVal = atuOperationPerform(dev,FLUSH_UNLOCKED,0,NULL,NULL,NULL,NULL);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbFlushInDB
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the particular
*       ATU Database (DBNum). If multiple address databases are being used, this
*		API can be used to flush entries in a particular DBNum database.
*
* INPUTS:
*       flushCmd - the flush operation type.
*		DBNum	 - ATU MAC Address Database Number. 
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlushInDB
(
    IN GT_QD_DEV    *dev,
    IN GT_FLUSH_CMD flushCmd,
	IN GT_U8 DBNum
)
{
    GT_STATUS       retVal;

    DBG_INFO(("gfdbFlush Called.\n"));
    DBG_INFO(("gfdbFush: dev=%x, dev->atuRegsSem=%d \n",dev, dev->atuRegsSem));

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_DBNUM_FULL)) != GT_OK) 
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return retVal;
	}

    if(flushCmd == GT_FLUSH_ALL)
        retVal = atuOperationPerform(dev,FLUSH_ALL_IN_DB,DBNum,NULL,NULL,NULL,NULL);
    else
        retVal = atuOperationPerform(dev,FLUSH_UNLOCKED_IN_DB,DBNum,NULL,NULL,NULL,NULL);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbAddMacEntry
*
* DESCRIPTION:
*       Creates the new entry in MAC address table.
*
* INPUTS:
*       macEntry    - mac address entry to insert to the ATU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*       GT_NO_RESOURCE    - failed to allocate a t2c struct
*       GT_OUT_OF_CPU_MEM - oaMalloc failed
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbAddMacEntry
(
    IN GT_QD_DEV    *dev,
    IN GT_ATU_ENTRY *macEntry
)
{
    GT_ETHERADDR    mac;            
    GT_U8           DBNum;	   
    GT_U8           entryPri;     
    GT_U8           portVec;     
    GT_U8           entryState;
    GT_STATUS       retVal;

    DBG_INFO(("gfdbAddMacEntry Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    gtMemCpy(mac.arEther,macEntry->macAddr.arEther,6);
	DBNum		= macEntry->DBNum;
    entryPri    = macEntry->prio;
    portVec     = macEntry->portVec;

    if(GT_IS_MULTICAST_MAC(mac))
    {
        entryState = macEntry->entryState.mcEntryState;
    }
    else
        entryState = macEntry->entryState.ucEntryState;

    retVal = atuOperationPerform(dev,LOAD_PURGE_ENTRY,DBNum,
		                 &entryPri,&portVec, &entryState,&mac);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gfdbDelMacEntry
*
* DESCRIPTION:
*       Deletes MAC address entry.
*
* INPUTS:
*       macAddress - mac address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbDelMacEntry
(
    IN GT_QD_DEV    *dev,
    IN GT_ETHERADDR  *macAddress
)
{
    GT_U8   entryPri = 0;    
    GT_U8   portVec = 0;    
    GT_U8   entryState = 0;
    GT_STATUS retVal;

    DBG_INFO(("gfdbDelMacEntry Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    retVal = atuOperationPerform(dev,LOAD_PURGE_ENTRY,0,&entryPri,
		                 &portVec, &entryState,macAddress);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbDelAtuEntry
*
* DESCRIPTION:
*       Deletes ATU entry.
*
* INPUTS:
*       atuEntry - the ATU entry to be deleted.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbDelAtuEntry
(
    IN GT_QD_DEV    *dev,
    IN GT_ATU_ENTRY  *atuEntry
)
{
    GT_ETHERADDR    mac;      
    GT_U8   DBNum;	     
    GT_U8   entryPri = 0;      
    GT_U8   portVec = 0;        
    GT_U8   entryState = 0; 
    GT_STATUS retVal;

    DBG_INFO(("gfdbDelMacEntry Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    gtMemCpy(mac.arEther,atuEntry->macAddr.arEther,6);
	DBNum = atuEntry->DBNum;
    retVal = atuOperationPerform(dev,LOAD_PURGE_ENTRY,DBNum,
		                 &entryPri,&portVec,&entryState,&mac);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbLearnEnable
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       en - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
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
GT_STATUS gfdbLearnEnable
(
    IN GT_QD_DEV    *dev,
    IN GT_BOOL  en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */

    DBG_INFO(("gfdbLearnEnable Called.\n"));
    BOOL_2_BIT(en,data);
    data = 1 - data;

    /* Set the Learn Enable bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,14,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* atuOperationPerform
*
* DESCRIPTION:
*       This function is used by all ATU control functions, and is responsible
*       to write the required operation into the ATU registers.
*
* INPUTS:
*       atuOp       - The ATU operation bits to be written into the ATU
*                     operation register.
*       DBNum       - ATU Database Number for CPU accesses
*       entryPri    - The EntryPri field in the ATU Data register.
*       portVec     - The portVec field in the ATU Data register.
*       entryState  - The EntryState field in the ATU Data register.
*       atuMac      - The Mac address to be written to the ATU Mac registers.
*
* OUTPUTS:
*       entryPri    - The EntryPri field in case the atuOp is GetNext.
*       portVec     - The portVec field in case the atuOp is GetNext.
*       entryState  - The EntryState field in case the atuOp is GetNext.
*       atuMac      - The returned Mac address in case the atuOp is GetNext.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  if atuMac == NULL, nothing needs to be written to ATU Mac registers.
*
*******************************************************************************/
static GT_STATUS atuOperationPerform
(
    IN      GT_QD_DEV           *dev,
    IN      GT_ATU_OPERARION    atuOp,
    IN      GT_U8               DBNum,
    INOUT   GT_U8               *entryPri,
    INOUT   GT_U8               *portVec,
    INOUT   GT_U8               *entryState,
    INOUT   GT_ETHERADDR        *atuMac
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           i;

    gtSemTake(dev,dev->atuRegsSem,OS_WAIT_FOREVER);

    /* Wait until the ATU in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->atuRegsSem);
            return retVal;
        }
    }

    /* Set the ATU data register    */
    if((atuOp != FLUSH_ALL) && (atuOp != FLUSH_UNLOCKED) &&
		(atuOp != FLUSH_ALL_IN_DB) && (atuOp != FLUSH_UNLOCKED_IN_DB) &&
		(atuOp != GET_NEXT_ENTRY))
    {
        data = ( (((*entryPri) & 0x3) << 14) | (((*portVec) & 0x7F) << 4) |
                 (((*entryState) & 0xF)) );
        retVal = hwWriteGlobalReg(dev,QD_REG_ATU_DATA_REG,data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->atuRegsSem);
            return retVal;
        }
    }

    /* Set the Atu Mac registers    */
    if((atuOp != FLUSH_ALL) && (atuOp != FLUSH_UNLOCKED) && 
    	(atuOp != FLUSH_ALL_IN_DB) && (atuOp != FLUSH_UNLOCKED_IN_DB))
    {
        for(i = 0; i < 3; i++)
        {
            data=((*atuMac).arEther[2*i] << 8)|((*atuMac).arEther[1 + 2*i]);
            retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->atuRegsSem);
                return retVal;
            }
        }
    }

    /* Set the ATU Operation register   */
    data = (1 << 15) | (atuOp << 12) | (DBNum & 0xF);
    retVal = hwWriteGlobalReg(dev,QD_REG_ATU_OPERATION,data);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->atuRegsSem);
        return retVal;
    }

    /* If the operation is a gen next operation wait for the response   */
    if(atuOp == GET_NEXT_ENTRY)
    {
        /* Wait until the ATU in ready. */
        data = 1;
        while(data == 1)
        {
            retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->atuRegsSem);
                return retVal;
            }
        }

        /* Get the Mac address  */
        for(i = 0; i < 3; i++)
        {
            retVal = hwReadGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->atuRegsSem);
                return retVal;
            }
            (*atuMac).arEther[2*i] = data >> 8;
            (*atuMac).arEther[1 + 2*i] = data & 0xFF;
        }

        retVal = hwReadGlobalReg(dev,QD_REG_ATU_DATA_REG,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->atuRegsSem);
            return retVal;
        }

        /* Get the Atu data register fields */
        *entryPri   = data >> 14;
        *portVec    = (data >> 4) & 0x7F;
        *entryState = data & 0xF;
    }

    gtSemGive(dev,dev->atuRegsSem);
    return GT_OK;
}
