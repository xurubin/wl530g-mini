/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
*********************************************************************************/
/* 
 * FILENAME:    $Workfile: mvFF_netconf.h $ 
 * REVISION:    $Revision: 3 $ 
 * LAST UPDATE: $Modtime: 3/02/03 10:20p $ 
 * 
 * DESCRIPTION: A header for the net conf wrraper
 *     
 */
#ifndef _FF_NETCNF_H_
#define _FF_NETCNF_H_

#include <mv_os.h>
#include <mv_unimac.h>
#include <mvFF_eth.h>

/*
 * This function returns the name of a given VID.
 *
 * Inputs:
 * vid - the vlan id to retirive its name.
 *
 * Outputs:
 * pNameLength - the length of this string's name.
 * pszVlanName - a const pointer to the name.
 *
 * NOTICE: this function returns an unsafe pointer to the string itself
 *         it doesn't copy nor allocate.
 */
VOID           mv_nc_GetVIDName(IN MV_UNM_VID vid,OUT unsigned int *pNameLength,OUT char **pszVlanName);

/*
 * This function returns the MAC of a given VID.
 *
 * Inputs:
 * vid - the vlan id to retirive its mac.
 *
 * Outputs:
 * A pointer to the mac address (bytes array) of this vlan.
 *
 */
unsigned char* mv_nc_GetMacOfVlan(IN MV_UNM_VID vid);

/*
 * ----------------------------------------------------------------------------
 * GT_STATUS  getNetConfig(OUT MV_UNM_CONFIG* unmConfig)
 *
 * This function is a part of the unimca manager API.
 * The unimac manager API (mv_unimac_mgr.c) requires a get function 
 * in order to init the net-conf data.
 * While being loaded, the unimac manager request for its init 
 * configuration data with that function.
 *
 * Inputs:
 *
 * Outputs:
 * unmConfig - A pointer to the net config to be filled with the init data.
 *
 */
GT_STATUS      getNetConfig(OUT MV_UNM_CONFIG* unmConfig);


/*
 * ----------------------------------------------------------------------------
 * GT_STATUS  setNetConfig(OUT MV_UNM_CONFIG* unmConfig, 
 *						char names[MV_UNM_MAX_VID][MAX_VLAN_NAME],
 *  					unsigned char macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE])
 *
 * The initialization phase calls this set function in order to 
 * init private fields of this modules.
 * the private fields holds the net-conf data.
 * a caller to the 'get' function will retreive this data (see above).
 *
 * Inputs:
 * unmConfig - A pointer to the net config with the init data.
 * names     - An array of the vlan names in the net-conf.
 * macs      - An array of mac addresses in the net-conf.
 *
 * Outputs:
 * GT_STATUS - the status of the operation.
 *
 */
GT_STATUS  setNetConfig(OUT MV_UNM_CONFIG* unmConfig, 
						char names[MV_UNM_MAX_VID][MAX_VLAN_NAME],
						unsigned char macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE]);

/*
 * A debug function to print the configuration in the unimac manager.
 * This function should be used to test that the get config, called by the manager
 * inited the manager with the correct data. 
 *
 * Inputs:
 *
 * Outputs:
 *
 */
#ifdef ETH_DBG_INFO
void           mv_nc_printConf(void);
#endif

#endif /* _FHNETCNF_H_ */
