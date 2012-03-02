/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* 
* FILENAME:    $Workfile: mvFF_netconf.c $ 
* REVISION:    $Revision: 3 $ 
* LAST UPDATE: $Modtime: 3/02/03 10:20p $ 
* 
* mvFF_netconf.c
*
* DESCRIPTION:
*       	This file holds common definition for dynamic network configuration OS WRRAPER
*	        A unimac manager wrraper to support VLAN names and MACs properties,
*               and to init the net-config structure 
*
* DEPENDENCIES:   
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "mvFF_netconf.h"
#include "mv_unimac.h"

/* is the net-config has been inited allready? */
static bool inited = false;

/* the null-mac is a comparison helper to find un initialized MAC addresses */
static unsigned char null_mac[GT_ETHERNET_HEADER_SIZE] = {0};

/* the net-config (nc) file, are structures that have been parsed from a 'file' 
 * the setNetConfig initialize all of those structures
 * the three structures holds the net-conf that have already parsed
 */
static unsigned char mv_nc_file_macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE]; 
static char          mv_nc_file_names[MV_UNM_MAX_VID][MAX_VLAN_NAME]; 
static MV_UNM_CONFIG file_cnf;

/* the net-config (nc) , are structures holds VALID info for netconf
 * the getNetConfig functions fill those structures from the 
 * allready-parsed nc_file structures
 */
static unsigned char	mv_nc_macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE];
static WCHAR	        mv_nc_vlan_names[MV_UNM_MAX_VID][MAX_VLAN_NAME];



/*
 * ----------------------------------------------------------------------------
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
VOID mv_nc_GetVIDName(IN MV_UNM_VID vid,OUT unsigned int *pNameLength,OUT char **pszVlanName) {
  *pNameLength = strlen(mv_nc_vlan_names[vid]);
  *pszVlanName = mv_nc_vlan_names[vid];  
}

/*
 * ----------------------------------------------------------------------------
 * This function returns the MAC of a given VID.
 *
 * Inputs:
 * vid - the vlan id to retirive its mac.
 *
 * Outputs:
 * A pointer to the mac address (bytes array) of this vlan.
 *
 */
unsigned char* mv_nc_GetMacOfVlan(IN MV_UNM_VID vid) {
  return mv_nc_macs[vid];
}


/*
 * ----------------------------------------------------------------------------
 * This function prints the port association table of the unimac manger.
 *
 * Inputs:
 *
 * Outputs:
 *
 */
#ifdef ETH_DBG_INFO
void mv_nc_printConf(void)
{
  MV_UNM_CONFIG	cfg;
  GT_STATUS		status = mvUnmGetNetConfig(&cfg);
  int i;
  
  if (status == GT_OK) {
	gtOsPrintf ("Port association table:\n"); 
	gtOsPrintf("---------------------------------------\n");
	gtOsPrintf("| Port |");
	for (i = 0; i < (GT_NUM_OF_SWITCH_PORTS-2); i++) {
	  gtOsPrintf("%4d |", i);
	}		
	gtOsPrintf("\n| VID  |");
	for (i = 0; i < (GT_NUM_OF_SWITCH_PORTS-2); i++) {
	  gtOsPrintf("%4d |", cfg.vidOfPort[i]);
	}
	gtOsPrintf("\n---------------------------------------\n");
  }
  else {
    gtOsPrintf("Configuration error %d.\n", status);
  }
}
#endif /* ETH_DBG_INFO */

/*
 * ----------------------------------------------------------------------------
 * printUnimacStructs - prints a given configuration.
 *
 * Inputs:
 *
 * Outputs:
 *
 */
#ifdef ETH_DBG_INFO
static void printUnimacStructs(OUT MV_UNM_CONFIG* _cnf, 
			char _names[MV_UNM_MAX_VID][MAX_VLAN_NAME],
			unsigned char _macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE]) 
{
  int i;
  for(i=0;i<MV_UNM_MAX_VID;i++) {
    gtOsPrintf("[%d] - MAC=%x%x%x%x%x%x Name=%s\n",i, 
	       _macs[i][0], 
	       _macs[i][1],
	       _macs[i][2],
	       _macs[i][3],
	       _macs[i][4],
	       _macs[i][5], 
	       _names[i]);
  }
  for(i=0;i<GT_NUM_OF_SWITCH_PORTS;i++) {
    gtOsPrintf(" %d", _cnf->vidOfPort[i]);
  }
  gtOsPrintf("\n");
}
#endif /* ETH_DBG_INFO */

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
						unsigned char macs[MV_UNM_MAX_VID][GT_ETHERNET_HEADER_SIZE])

{
  /* init all structures to zero */
  memset(mv_nc_file_macs, 0, sizeof(mv_nc_file_macs));
  memset(mv_nc_file_names, 0, sizeof(mv_nc_file_names));; 
  memset(&file_cnf, 0, sizeof(file_cnf));
  
  /* set net-conf by structure copy 
   * this is only updates the 'file' structures, thos will be parsed
   * in the getNetConfig to initialize unimac manger
   */
  memcpy(mv_nc_file_macs, macs, sizeof(mv_nc_file_macs));
  memcpy(mv_nc_file_names, names, sizeof(mv_nc_file_names)); 
  memcpy(&file_cnf, unmConfig, sizeof(file_cnf));
  
  inited = true;

#ifdef ETH_DBG_INFO
  printUnimacStructs(&file_cnf, mv_nc_file_names, mv_nc_file_macs );
#endif /* ETH_DBG_INFO */

  return (GT_OK);

}

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
GT_STATUS  getNetConfig(OUT MV_UNM_CONFIG* unmConfig)
{
  int i;

  /* Let's fill it with the following defaults for now:
     WAN - port 0.
     LAN - ports 1,2,3,4.
  */
  memset( mv_nc_macs, 0 , sizeof(mv_nc_macs) );
  memset( mv_nc_vlan_names, 0 , sizeof(mv_nc_vlan_names) );

  if(!inited) 
    return GT_FAIL;

  memcpy(unmConfig, &file_cnf, sizeof(MV_UNM_CONFIG));

  /* init all VLAN fields */
  /* We are using two set of structures here
   * 1. The: mv_nc_conf_file_macs, mv_nc_conf_file_names, mv_nc_unmConfig
   *    those three conf structs are being filled by an IOCTL call of a configuration application
   *
   * 2. mv_nc_macs, mv_nc_vlan_names, cnf, that are being updated here from the structures above
   *    here at the context of UNM initialize, the system gets ready to work.
   */
  
  for( i = 0 ; i < MV_UNM_MAX_VID ; i++) {
    if(strlen(mv_nc_file_names[i]) > 0 ) {
      memcpy(mv_nc_vlan_names[i], mv_nc_file_names[i], strlen(mv_nc_file_names[i]));
      if( i > 0 && i < MV_UNM_VID_ISOLATED ) { 
		memcpy(mv_nc_macs[i], mv_nc_file_macs[i],  GT_ETHERNET_HEADER_SIZE);
		if( memcmp(mv_nc_file_macs[i], null_mac, GT_ETHERNET_HEADER_SIZE) == 0  ) {
		  gtOsPrintf("Error - No MAC ADDR is set for VLAN WAN\n");
		  return GT_FAIL;  
		}
      }
    }
  }
#ifdef ETH_DBG_INFO
  printUnimacStructs(unmConfig, mv_nc_vlan_names, mv_nc_macs);  
#endif /* ETH_DBG_INFO */
  return (GT_OK);
}

