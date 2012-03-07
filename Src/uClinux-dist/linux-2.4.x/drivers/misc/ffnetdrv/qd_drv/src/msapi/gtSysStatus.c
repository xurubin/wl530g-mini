#include <Copyright.h>

/*******************************************************************************
* gtSysStatus.c
*
* DESCRIPTION:
*       API definitions for system global status.
* 	Added for fullsail
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
* gsysGetSW_Mode
*
* DESCRIPTION:
*       This routine get the Switch mode. These two bits returen 
*       the current value of the SW_MODE[1:0] pins.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE Discard is enabled, GT_FALSE otherwise.
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
GT_STATUS gsysGetSW_Mode
(
    IN GT_QD_DEV  *dev,
    IN GT_SW_MODE *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetSW_Mode Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_QD_PLUS)) != GT_OK )
	return retVal;

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,12,2,&data);
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
* gsysGetInitReady
*
* DESCRIPTION:
*       This routine get the InitReady bit. This bit is set to a one when the ATU,
*       the Queue Controller and the Statistics Controller are done with their 
*       initialization and are ready to accept frames.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: switch is ready, GT_FALSE otherwise.
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
GT_STATUS gsysGetInitReady
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetInitReady Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_QD_PLUS)) != GT_OK )
	return retVal;

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,11,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

#ifdef DEBUG_FEATURE
/*******************************************************************************
* gsysGetPtrCollision
*
* DESCRIPTION:
*       This routine get the QC Pointer Collision.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE Discard is enabled, GT_FALSE otherwise.
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
GT_STATUS gsysGetPtrCollision
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetPtrCollision Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    if((retVal = IS_VALID_API_CALL(dev,1, DEV_QD_PLUS)) != GT_OK )
	return retVal;

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,15,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/* the following are for clippership only - not for fullsail */

/*******************************************************************************
* gsysGetDpvCorrupt
*
* DESCRIPTION:
*       This routine get the DpvCorrupt bit. This bit is set to a one when the 
*       QC detects a destination vector error
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: destination vector corrupt, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 	This feature is on clippership, but not on fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDpvCorrupt
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetDpvCorrupt Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    /* Fullsail (DEV_QD_88E6502) is an exception, and does not support this feature */

    if((IS_VALID_API_CALL(dev,1, DEV_QD_PLUS)) != GT_OK )
      return GT_NOT_SUPPORTED; /* this is quarterdeck */
    if(  IS_VALID_API_CALL(dev,1, DEV_88E6021 ) == GT_OK )
      return GT_NOT_SUPPORTED; /* this is fullsail */

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,8,1,&data);
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
* gsysGetMissingPointers
*
* DESCRIPTION:
*       This routine get the Missing Pointer bit. This bit is set to a one when  
*       the Register File detects less than 64 pointers in the Link List. 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: Missing Pointers error, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 	This feature is on clippership, but not on fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMissingPointers
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetMissingPointers Called.\n"));

  /* only devices beyond quarterdeck (6052) has this feature */
  /* Fullsail (DEV_QD_88E6502) is an exception, and does not support this feature */

  if((IS_VALID_API_CALL(dev,1, DEV_QD_PLUS)) != GT_OK )
      return GT_NOT_SUPPORTED; /* this is quarterdeck */
  if(  IS_VALID_API_CALL(dev,1, DEV_88E6021 ) == GT_OK )
      return GT_NOT_SUPPORTED; /* this is fullsail */

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,7,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}
#endif /* DEBUG_FEATURE */
