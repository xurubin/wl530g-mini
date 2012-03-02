#include <Copyright.h>

/********************************************************************************
* gtSysConfig.c
*
* DESCRIPTION:
*       API definitions for system configuration, and enabling.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <msApi.h>
#include <gtDrvConfig.h>
#include <gtSem.h>

static GT_BOOL gtRegister(GT_QD_DEV *dev, BSP_FUNCTIONS* pBSPFunctions);

/*******************************************************************************
* qdLoadDriver
*
* DESCRIPTION:
*       QuarterDeck Driver Initialization Routine. 
*       This is the first routine that needs be called by system software. 
*       It takes *cfg from system software, and retures a pointer (*dev) 
*       to a data structure which includes infomation related to this QuarterDeck
*       device. This pointer (*dev) is then used for all the API functions. 
*
* INPUTS:
*       cfg  - Holds device configuration parameters provided by system software.
*
* OUTPUTS:
*       dev  - Holds device information to be used for each API call.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_ALREADY_EXIST    - if device already started
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
* 	qdUnloadDriver is also provided to do driver cleanup.
*
*******************************************************************************/
GT_STATUS qdLoadDriver
(
    IN  GT_SYS_CONFIG   *cfg,
    OUT GT_QD_DEV	*dev
)
{
    GT_STATUS   retVal;

    DBG_INFO(("qdLoadDriver Called.\n"));

    /* Check for parameters validity        */
    if(dev == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Check for parameters validity        */
    if(cfg == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* The initialization was already done. */
    if(dev->devEnabled)
    {
        DBG_INFO(("QuarterDeck already started.\n"));
        return GT_ALREADY_EXIST;
    }

    if(gtRegister(dev,&(cfg->BSPFunctions)) != GT_TRUE)
    {
       DBG_INFO(("gtRegister Failed.\n"));
       return GT_FAIL;
    }

    /* Initialize the driver    */
    retVal = driverConfig(dev);
    if(retVal != GT_OK)
    {
        DBG_INFO(("driverConfig Failed.\n"));
        return retVal;
    }

    /* Initialize dev fields.         */
    dev->cpuPortNum = cfg->cpuPortNum;
    dev->maxPhyNum = 5;

    /* Assign Device Name */
    switch(dev->deviceId)
    {
		case GT_88E6021:
		     dev->maxPhyNum = 2;
	             dev->devName = DEV_88E6021;
		     break;

		case GT_88E6051:
	             dev->devName = DEV_88E6051;
		     break;

		case GT_88E6052:
	             dev->devName = DEV_88E6052;
		     break;

		case GT_88E6060:
		     if((dev->cpuPortNum != 4)&&(dev->cpuPortNum != 5))
		     {
			dev = NULL;
        		return GT_FAIL;
		     }
	             dev->devName = DEV_88E6060;
		     break;

		case GT_88E6061:
	             dev->devName = DEV_88E6061;
		     break;

		case GT_88E6062:
	             dev->devName = DEV_88E6062;
		     break;

		case GT_88E6063:
	             dev->devName = DEV_88E6063;
		     break;

		case GT_FH_VPN_L:
		     if(dev->cpuPortNum != 5)
		     {
			dev = NULL;
			return GT_FAIL;
		     }
	             dev->devName = DEV_FH_VPN_L;
		     break;

		case GT_FH_VPN:
	             dev->devName = DEV_FH_VPN;
		     break;

		case GT_FF_XP:
		     dev->maxPhyNum = 2;
	             dev->devName = DEV_FF_XP;
		     break;

		case GT_FF_EG:
		    if(dev->cpuPortNum != 5)
		    {
			dev = NULL;
        		return GT_FAIL;
		    }
	            dev->devName = DEV_FF_EG;
		    break;

		case GT_FF_HG:
	             dev->devName = DEV_FF_HG;
		     break;

		default:
		     DBG_INFO(("Unknown Device. Initialization failed\n"));
		     dev = NULL;
		     return GT_FAIL;
    }

    /* Initialize the ATU semaphore.    */
    if((dev->atuRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
	qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the VTU semaphore.    */
    if((dev->vtuRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
	qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the STATS semaphore.    */
    if((dev->statsRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
	qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the ports states. */
    if(cfg->initPorts == GT_TRUE)
    {
        retVal = gstpSetMode(dev,GT_FALSE);
        if(retVal != GT_OK)
        {
            DBG_INFO(("gstpSetMode Failed.\n"));
	    qdUnloadDriver(dev);
            return retVal;
        }
    }

    dev->devEnabled = 1;
    dev->devNum = cfg->devNum;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* sysEnable
*
* DESCRIPTION:
*       This function enables the system for full operation.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           qdLoadDriver().
*
*******************************************************************************/
GT_STATUS sysEnable( GT_QD_DEV *dev)
{
    DBG_INFO(("sysEnable Called.\n"));
    DBG_INFO(("OK.\n"));
    return driverEnable(dev);
}


/*******************************************************************************
* qdUnloadDriver
*
* DESCRIPTION:
*       This function unloads the QuaterDeck Driver.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           qdLoadDriver().
*
*******************************************************************************/
GT_STATUS qdUnloadDriver
(
    IN GT_QD_DEV* dev
)
{
    DBG_INFO(("qdUnloadDriver Called.\n"));

    /* Delete the ATU semaphore.    */
    if(gtSemDelete(dev,dev->atuRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the VTU semaphore.    */
    if(gtSemDelete(dev,dev->vtuRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the STATS semaphore.    */
    if(gtSemDelete(dev,dev->statsRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	gtMemSet(dev,0,sizeof(GT_QD_DEV));
	return GT_OK;
}


/*******************************************************************************
* gtRegister
*
* DESCRIPTION:
*       BSP should register the following functions:
*		1) MII Read - (Input, must provide)
*			allows QuarterDeck driver to read QuarterDeck device registers.
*		2) MII Write - (Input, must provice)
*			allows QuarterDeck driver to write QuarterDeck device registers.
*		3) Semaphore Create - (Input, optional)
*			OS specific Semaphore Creat function.
*		4) Semaphore Delete - (Input, optional)
*			OS specific Semaphore Delete function.
*		5) Semaphore Take - (Input, optional)
*			OS specific Semaphore Take function.
*		6) Semaphore Give - (Input, optional)
*			OS specific Semaphore Give function.
*		Notes: 3) ~ 6) should be provided all or should not be provided at all.
*
* INPUTS:
*		pBSPFunctions - pointer to the structure for above functions.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_TRUE, if input is valid. GT_FALSE, otherwise.
*
* COMMENTS:
*       This function should be called only once.
*
*******************************************************************************/
static GT_BOOL gtRegister(GT_QD_DEV *dev, BSP_FUNCTIONS* pBSPFunctions)
{
	dev->fgtReadMii =  pBSPFunctions->readMii;
	dev->fgtWriteMii = pBSPFunctions->writeMii;
	
	dev->semCreate = pBSPFunctions->semCreate;
	dev->semDelete = pBSPFunctions->semDelete;
	dev->semTake   = pBSPFunctions->semTake  ;
	dev->semGive   = pBSPFunctions->semGive  ;
	
	return GT_TRUE;
}

