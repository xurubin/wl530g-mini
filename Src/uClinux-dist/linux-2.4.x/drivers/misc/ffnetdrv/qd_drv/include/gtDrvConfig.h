#include <Copyright.h>

/********************************************************************************
* gtDrvConfig.h
*
* DESCRIPTION:
*       Includes driver level configuration and initialization function.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __gtDrvConfigh
#define __gtDrvConfigh

#include <msApi.h>
#include <gtDrvSwRegs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* driverConfig
*
* DESCRIPTION:
*       This function initializes the driver level of the quarterDeck software.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success, or
*       GT_OUT_OF_CPU_MEM   - if failed to allocate CPU memory,
*       GT_FAIL             - otherwise.
*
* COMMENTS:
*       1.  This function should perform the following:
*           -   Initialize the global switch configuration structure.
*           -   Initialize Mii Interface
*           -   Set the CPU port into trailer mode (Ingress and Egress).
*
*******************************************************************************/
GT_STATUS driverConfig(IN GT_QD_DEV *dev);

/*******************************************************************************
* driverEnable
*
* DESCRIPTION:
*       This function enables the switch for full operation, after the driver
*       Config function was called.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverEnable(IN GT_QD_DEV *dev);

#ifdef __cplusplus
}
#endif

#endif /* __gtDrvConfigh */
