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
 * FILENAME:    $Workfile: qdIoctl.c $ 
 * REVISION:    $Revision: 3 $ 
 * LAST UPDATE: $Modtime: 3/02/03 10:20p $ 
 * 
 * qdIoctl.c
 *
 * DESCRIPTION:
 *       QD IOCTL API
 *
 * DEPENDENCIES:   Platform.
 *
 * FILE REVISION NUMBER:
 *
 *******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include "qdModule.h"
#include <mvFF_netconf.h>

/*
 * #define IOCTL_DEBUG
 *
 */

#define ERROR_SUCCESS           (0)
#define ERROR_NOT_SUPPORTED     (-1)
#define ERROR_INVALID_PARAMETER (-2)
#define ERROR_GEN_FAILURE       (-3)


extern GT_QD_DEV* qd_dev;
extern int mvFF_eth_start(void);

static bool initialized = false;

static void SetLastError(DWORD dwErr)
{
  static DWORD dwLastError = 0;

  dwLastError = dwErr;
}


/************************************************************************/
/* IOCTL API                                                            */
/************************************************************************/
static bool qdUnmNetIoctl(DWORD  hOpenContext,
			  DWORD  dwCode,
			  UCHAR *pInBuf,
			  DWORD  InBufLen,
			  UCHAR *pOutBuf,
			  DWORD  OutBufLen,
			  DWORD *pdwBytesTransferred);
static GT_STATUS UNM_GetVlanNames(UCHAR *pOutBuf,
				  DWORD  OutBufLen,
				  DWORD *pdwBytesTransferred);
static GT_STATUS UNM_GetVlanParams(UCHAR *pInBuf,
				   DWORD  InBufLen,
				   UCHAR *pOutBuf,
				   DWORD  OutBufLen,
				   DWORD *pdwBytesTransferred);
static GT_STATUS UNM_GetPortVlan(DWORD port,
				 UCHAR *pOutBuf,
				 DWORD  OutBufLen,
				 DWORD *pdwBytesTransferred);
static GT_STATUS UNM_DisassocPort(DWORD port);
static GT_STATUS UNM_AssocPort(DWORD port,
			       WCHAR* szVlanName);

bool qdUnmNetIoctl(DWORD  hOpenContext,
		   DWORD  dwCode,
		   UCHAR *pInBuf,
		   DWORD  InBufLen,
		   UCHAR *pOutBuf,
		   DWORD  OutBufLen,
		   DWORD *pdwBytesTransferred)
{
  bool bRc = false;
  GT_STATUS rc;
  DWORD port;

  if (!pdwBytesTransferred) {
    return false;
  }
  *pdwBytesTransferred = 0;

  switch(dwCode) {
    
  case IOCTL_UNM_READ_REG:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      break;
    }
    if (OutBufLen < sizeof(GT_IOCTL_PARAM)) {
      break;
    }
    ((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data =
      *(GT_U32*)((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data;
    bRc = true;
    *pdwBytesTransferred = sizeof(GT_U32);
    break;
    
  case IOCTL_UNM_WRITE_REG:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      break;
    }
    *(GT_U32*)((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data =
      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u32Data;
    bRc = true;
    *pdwBytesTransferred = 0;
    break;
    
  case IOCTL_UNM_STATUS:
    if (OutBufLen < sizeof(bool)) {
      break;
    }
    *pdwBytesTransferred = sizeof(bool);
    *(bool*)pOutBuf = initialized;
    bRc = true;
    break;
    
  case IOCTL_UNM_INIT:
    if (initialized || (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    rc = setNetConfig(&(((GT_IOCTL_PARAM*)pInBuf)->FirstParam.netconf),
		      (((GT_IOCTL_PARAM*)pInBuf)->SecondParam.vlan_names),
		      (((GT_IOCTL_PARAM*)pInBuf)->ThirdParam.macs)); 
    if (rc != GT_OK) {
      break;
    }
    /* init net-device and qd-device */
    mvFF_eth_start();
    initialized = true;
    bRc = true;
    break;
    
  case IOCTL_UNM_GET_VLAN_NAMES:
    *pdwBytesTransferred = 0;
    if (!initialized || (OutBufLen < 2)) {
      break;
    }
    rc = UNM_GetVlanNames(pOutBuf, OutBufLen, pdwBytesTransferred);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  case IOCTL_UNM_GET_VLAN_PARMS:
    *pdwBytesTransferred = 0;
    if (!initialized || (OutBufLen < sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    rc = UNM_GetVlanParams( pInBuf, InBufLen, pOutBuf, OutBufLen, pdwBytesTransferred);
    if( rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  case IOCTL_UNM_GET_PORT_VLAN:
    *pdwBytesTransferred = 0;
    if (!initialized || (OutBufLen < sizeof(GT_IOCTL_PARAM))
	|| (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    port = ((GT_IOCTL_PARAM*)pInBuf)->FirstParam.port;
    if ((port == GT_CPU_SWITCH_PORT) || (port > GT_NUM_OF_SWITCH_PORTS)) {
      break;
    }
    rc = UNM_GetPortVlan(port, pOutBuf, OutBufLen, pdwBytesTransferred);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  case IOCTL_UNM_ASSOC_PORT:
    *pdwBytesTransferred = 0;
    if (!initialized || (InBufLen != sizeof(GT_IOCTL_PARAM))) {
#ifdef IOCTL_DEBUG
      printk("ASSOC: InBufLen != sizeof(GT_IOCTL_PARAM) \n");
#endif
      break;
    }
    port = ((GT_IOCTL_PARAM*)pInBuf)->FirstParam.port;
    if ((port == GT_CPU_SWITCH_PORT) || (port > GT_NUM_OF_SWITCH_PORTS)) {
#ifdef IOCTL_DEBUG
      printk("ASSOC: port = 5 || port > 6 \n");
#endif
      break;
    }
    rc = UNM_AssocPort(port, ((GT_IOCTL_PARAM*)pInBuf)->SecondParam.szVlanName);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
      
  case IOCTL_UNM_DISASSSOC_PORT:
    if(!initialized || (InBufLen != sizeof(GT_IOCTL_PARAM))) {
      break;
    }
    port = ((GT_IOCTL_PARAM*)pInBuf)->FirstParam.port;
    if ((port == GT_CPU_SWITCH_PORT) || (port > GT_NUM_OF_SWITCH_PORTS)) {
      break;
    }
    rc = UNM_DisassocPort(port);
    if (rc != GT_OK) {
      break;
    }
    bRc = true;
    break;
    
  default:
    break;
  }
  return bRc;
}


bool qdSystemConfig(DWORD  hOpenContext,
  DWORD  Ioctl,
	       UCHAR *pInBuf,
	       DWORD  InBufLen, 
	       UCHAR *pOutBuf,
	       DWORD  OutBufLen,
	       DWORD * pdwBytesTransferred
	       )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_sysConfig: 
      if (OutBufLen < sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM);
#ifdef IOCTL_DEBUG
      printk("IOCTL_sysConfig: qd dev = ");
      {
	int i;
	for (i = 0; i < sizeof(GT_QD_DEV); i++) {
	  printk("0x%02x ", ((char*)&qd_dev)[i]);
	}
	printk("\n");
      }
#endif
      memcpy( &( ((GT_IOCTL_PARAM*)pOutBuf)->FirstParam.qd_dev), qd_dev, sizeof(GT_QD_DEV));
      break;

    case IOCTL_gsysReadMiiReg:
      if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	  
      if (OutBufLen < sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	
      if(gsysReadMiiReg(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u32Data,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data
			) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U32);
      break;

    case IOCTL_gsysWriteMiiReg:
      if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
	dwErr = ERROR_INVALID_PARAMETER;
	break;
      }
	
      if(gsysWriteMiiReg(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u32Data,
			 ((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.u16Data
			 ) != GT_OK) {
	dwErr = ERROR_GEN_FAILURE;
	break;
      }
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gtVersion:
      if (InBufLen < GT_QD_VERSION_MAX_LEN)
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gtVersion((GT_VERSION*)pInBuf) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = strlen(pInBuf);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

bool
qdFdbIoctl(
	   DWORD  hOpenContext,
	   DWORD  Ioctl,
	   UCHAR *pInBuf,
	   DWORD  InBufLen, 
	   UCHAR *pOutBuf,
	   DWORD  OutBufLen,
	   DWORD * pdwBytesTransferred
	   )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gfdbSetAtuSize: 
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbSetAtuSize(qd_dev,((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuSize) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbGetAgingTimeRange:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAgingTimeRange(qd_dev,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data,
			       &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.u32Data
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 2*sizeof(GT_U32);
      break;

    case IOCTL_gfdbSetAgingTimeout:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbSetAgingTimeout(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbGetAtuDynamicCount:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAtuDynamicCount(qd_dev,
				&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u32Data
				) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U32);
      break;

    case IOCTL_gfdbGetAtuEntryFirst:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAtuEntryFirst(qd_dev,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.atuEntry
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ATU_ENTRY);
      break;

    case IOCTL_gfdbGetAtuEntryNext:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbGetAtuEntryNext(qd_dev,
			     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(
	     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.atuEntry,
	     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry,
	     sizeof(GT_ATU_ENTRY)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ATU_ENTRY);
      break;

    case IOCTL_gfdbFindAtuMacEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbFindAtuMacEntry(qd_dev,
			     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry,
			     &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}

      memcpy(&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.atuEntry,
	     &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry,
	     sizeof(GT_ATU_ENTRY)
	     );

      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ATU_ENTRY);
      break;

    case IOCTL_gfdbFlush:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbFlush(qd_dev,
		   ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.flushCmd
		   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbAddMacEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbAddMacEntry(qd_dev,
			 &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.atuEntry
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbDelMacEntry:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbDelMacEntry(qd_dev,
			 &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.etherAddr
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gfdbLearnEnable:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gfdbLearnEnable(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
qdStpIoctl(
	   DWORD  hOpenContext,
	   DWORD  Ioctl,
	   UCHAR *pInBuf,
	   DWORD  InBufLen, 
	   UCHAR *pOutBuf,
	   DWORD  OutBufLen,
	   DWORD * pdwBytesTransferred
	   )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gstpSetMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gstpSetMode(qd_dev,
		     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
		     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gstpSetPortState:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gstpSetPortState(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.stpState
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gstpGetPortState:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
	
      if(gstpGetPortState(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.stpState
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_PORT_STP_STATE);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
qdVlanIoctl(
	    DWORD  hOpenContext,
	    DWORD  Ioctl,
	    UCHAR *pInBuf,
	    DWORD  InBufLen, 
	    UCHAR *pOutBuf,
	    DWORD  OutBufLen,
	    DWORD * pdwBytesTransferred
	    )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl) {
  case IOCTL_gprtSetEgressMode: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetEgressMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.egressMode
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gprtGetEgressMode:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtGetEgressMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.egressMode
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_EGRESS_MODE);
    break;

  case IOCTL_gprtSetVlanTunnel: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtSetVlanTunnel(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gprtGetVlanTunnel:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gprtGetVlanTunnel(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;

  case IOCTL_gvlnSetPortVlanPorts: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVlanPorts(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.portList,
			    ((PGT_IOCTL_PARAM)pInBuf)->ThirdParam.u8Data
			    ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVlanPorts:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVlanPorts(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.portList,
			    &((PGT_IOCTL_PARAM)pOutBuf)->SecondParam.u8Data
			    ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM);
    break;

  case IOCTL_gvlnSetPortUserPriLsb: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortUserPriLsb(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			     ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortUserPriLsb:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortUserPriLsb(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			     ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_BOOL);
    break;


  case IOCTL_gvlnSetPortVid: 
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnSetPortVid(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u16Data
		      ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = 0;
    break;

  case IOCTL_gvlnGetPortVid:
    if (InBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
    if (OutBufLen != sizeof(GT_IOCTL_PARAM)) {
      dwErr = ERROR_INVALID_PARAMETER;
      break;
    }
	
    if(gvlnGetPortVid(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
		      ) != GT_OK) {
      dwErr = ERROR_GEN_FAILURE;
      break;
    }
    bRc = true;
    dwErr = ERROR_SUCCESS;
    *pdwBytesTransferred = sizeof(GT_U16);
    break;

  default:
    break;
  }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
qdSysEventIoctl(
		DWORD  hOpenContext,
		DWORD  Ioctl,
		UCHAR *pInBuf,
		DWORD  InBufLen, 
		UCHAR *pOutBuf,
		DWORD  OutBufLen,
		DWORD * pdwBytesTransferred
		)
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_eventSetActive:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(eventSetActive(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u32Data
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_eventGetIntStatus:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(eventGetIntStatus(qd_dev,
			   &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

bool
qdPhyCtrlIoctl(
	       DWORD  hOpenContext,
	       DWORD  Ioctl,
	       UCHAR *pInBuf,
	       DWORD  InBufLen, 
	       UCHAR *pOutBuf,
	       DWORD  OutBufLen,
	       DWORD * pdwBytesTransferred
	       )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtPhyReset:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPhyReset(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortLoopback:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortLoopback(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortSpeed:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortSpeed(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtPortAutoNegEnable:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPortAutoNegEnable(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			       ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtPortPowerDown:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPortPowerDown(qd_dev,
			   ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			   ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtPortRestartAutoNeg:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPortRestartAutoNeg(qd_dev,
				((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port
				) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortDuplexMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortDuplexMode(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			       ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPortAutoMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPortAutoMode(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.phyAutoMode
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtSetPause:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetPause(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

bool
qdPhyIntIoctl(
	      DWORD  hOpenContext,
	      DWORD  Ioctl,
	      UCHAR *pInBuf,
	      DWORD  InBufLen, 
	      UCHAR *pOutBuf,
	      DWORD  OutBufLen,
	      DWORD * pdwBytesTransferred
	      )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtPhyIntEnable:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtPhyIntEnable(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u16Data
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetPhyIntStatus:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPhyIntStatus(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    case IOCTL_gprtGetPhyIntPortSummary:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPhyIntPortSummary(qd_dev,
				  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u16Data
				  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U16);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}

bool
qdPortCtrlIoctl(
		DWORD  hOpenContext,
		DWORD  Ioctl,
		UCHAR *pInBuf,
		DWORD  InBufLen, 
		UCHAR *pOutBuf,
		DWORD  OutBufLen,
		DWORD * pdwBytesTransferred
		)
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtSetForceFc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetForceFc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetForceFc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetForceFc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtSetTrailerMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetTrailerMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetTrailerMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetTrailerMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtSetIngressMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetIngressMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.ingressMode
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetIngressMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetIngressMode(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.ingressMode
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_INGRESS_MODE);
      break;


    case IOCTL_gprtSetMcRateLimit:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetMcRateLimit(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.mcRate
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetMcRateLimit:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetMcRateLimit(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.mcRate
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_MC_RATE);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
qdPortStatsIoctl(
		 DWORD  hOpenContext,
		 DWORD  Ioctl,
		 UCHAR *pInBuf,
		 DWORD  InBufLen, 
		 UCHAR *pOutBuf,
		 DWORD  OutBufLen,
		 DWORD * pdwBytesTransferred
		 )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtSetCtrMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtSetCtrMode(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.ctrMode
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtClearAllCtr:

      if(gprtClearAllCtr(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gprtGetPortCtr:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPortCtr(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.portStat
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_PORT_STAT);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
qdPortStatusIoctl(
		  DWORD  hOpenContext,
		  DWORD  Ioctl,
		  UCHAR *pInBuf,
		  DWORD  InBufLen, 
		  UCHAR *pOutBuf,
		  DWORD  OutBufLen,
		  DWORD * pdwBytesTransferred
		  )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gprtGetPartnerLinkPause:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPartnerLinkPause(qd_dev,
				 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
				 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetSelfLinkPause:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetSelfLinkPause(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetResolve:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetResolve(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetLinkState:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetLinkState(qd_dev,
			  ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			  &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			  ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetPortMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPortMode(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetPhyMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetPhyMode(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetDuplex:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetDuplex(qd_dev,
		       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
		       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gprtGetSpeed:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gprtGetSpeed(qd_dev,
		      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
		      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
		      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
qdQoSMapIoctl(
	      DWORD  hOpenContext,
	      DWORD  Ioctl,
	      UCHAR *pInBuf,
	      DWORD  InBufLen, 
	      UCHAR *pOutBuf,
	      DWORD  OutBufLen,
	      DWORD * pdwBytesTransferred
	      )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gcosSetPortDefaultTc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosSetPortDefaultTc(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosSetPrioMapRule:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosSetPrioMapRule(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosGetPrioMapRule:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosGetPrioMapRule(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gqosIpPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosIpPrioMapEn(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			 ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosGetIpPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosGetIpPrioMapEn(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gqosUserPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosUserPrioMapEn(qd_dev,
			   ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			   ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.boolData
			   ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gqosGetUserPrioMapEn:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gqosGetUserPrioMapEn(qd_dev,
			      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.port,
			      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;


    case IOCTL_gcosSetUserPrio2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosSetUserPrio2Tc(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			    ((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gcosGetUserPrio2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosGetUserPrio2Tc(qd_dev,
			    ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			    &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u8Data
			    ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U8);
      break;

    case IOCTL_gcosSetDscp2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosSetDscp2Tc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			((PGT_IOCTL_PARAM)pInBuf)->SecondParam.u8Data
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gcosGetDscp2Tc:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gcosGetDscp2Tc(qd_dev,
			((PGT_IOCTL_PARAM)pInBuf)->FirstParam.u8Data,
			&((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.u8Data
			) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_U8);
      break;

    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}



bool
qdSysCtrlIoctl(
	       DWORD  hOpenContext,
	       DWORD  Ioctl,
	       UCHAR *pInBuf,
	       DWORD  InBufLen, 
	       UCHAR *pOutBuf,
	       DWORD  OutBufLen,
	       DWORD * pdwBytesTransferred
	       )
{
  DWORD  dwErr = ERROR_NOT_SUPPORTED;
  bool   bRc = false;

  switch (Ioctl)
    {
    case IOCTL_gsysSwReset:

      if(gsysSwReset(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysSetDiscardExcessive:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetDiscardExcessive(qd_dev,
				 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
				 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetDiscardExcessive:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetDiscardExcessive(qd_dev,
				 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysSetSchedulingMode:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetSchedulingMode(qd_dev,
			       ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetSchedulingMode:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetSchedulingMode(qd_dev,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysSetMaxFrameSize:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetMaxFrameSize(qd_dev,
			     ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetMaxFrameSize:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetMaxFrameSize(qd_dev,
			     &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			     ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysReLoad:

      if(gsysReLoad(qd_dev) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysSetWatchDog:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetWatchDog(qd_dev,
			 ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetWatchDog:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetWatchDog(qd_dev,
			 &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
			 ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;

    case IOCTL_gsysSetDuplexPauseMac:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetDuplexPauseMac(qd_dev,
			       &((PGT_IOCTL_PARAM)pInBuf)->FirstParam.etherAddr
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetDuplexPauseMac:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetDuplexPauseMac(qd_dev,
			       &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.etherAddr
			       ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_ETHERADDR);
      break;

    case IOCTL_gsysSetPerPortDuplexPauseMac:
      if (InBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysSetPerPortDuplexPauseMac(qd_dev,
				      ((PGT_IOCTL_PARAM)pInBuf)->FirstParam.boolData
				      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = 0;
      break;

    case IOCTL_gsysGetPerPortDuplexPauseMac:
      if (OutBufLen != sizeof(GT_IOCTL_PARAM))
	{
	  dwErr = ERROR_INVALID_PARAMETER;
	  break;
	}

      if(gsysGetPerPortDuplexPauseMac(qd_dev,
				      &((PGT_IOCTL_PARAM)pOutBuf)->FirstParam.boolData
				      ) != GT_OK)
	{
	  dwErr = ERROR_GEN_FAILURE;
	  break;
	}
      bRc = true;
      dwErr = ERROR_SUCCESS;
      *pdwBytesTransferred = sizeof(GT_BOOL);
      break;


    default:
      break;
    }

  if (ERROR_SUCCESS != dwErr) {
    SetLastError(dwErr);
    bRc = false;
  }

  return bRc;
}


bool
UNM_IOControl(
	      DWORD  hOpenContext,
	      DWORD  Ioctl,
	      UCHAR *pInBuf,
	      DWORD  InBufLen, 
	      UCHAR *pOutBuf,
	      DWORD  OutBufLen,
	      DWORD *pdwRet
	      )
{
  bool   bRc = false;

  switch (SUB_FUNC_MASK & GET_FUNC_FROM_CTL_CODE(Ioctl)) {
  case SYS_CFG_FUNC_MASK:
    bRc = qdSystemConfig(hOpenContext, Ioctl,pInBuf, InBufLen, 
			 pOutBuf,OutBufLen,pdwRet);
    break;
      
  case FDB_FUNC_MASK:
    bRc = qdFdbIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen, pdwRet);
    break;
      
  case STP_FUNC_MASK:
    bRc = qdStpIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		     pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case VLAN_FUNC_MASK:
    bRc = qdVlanIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
		      pOutBuf, OutBufLen,pdwRet);       

    break;
      
  case SYS_EVENT_FUNC_MASK:
    bRc = qdSysEventIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			  pOutBuf, OutBufLen,pdwRet);       
    break;
  case PHY_CTRL_FUNC_MASK:
    bRc = qdPhyCtrlIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			 pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PHY_INT_FUNC_MASK:
    bRc = qdPhyIntIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PORT_CTRL_FUNC_MASK:
    bRc = qdPortCtrlIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			  pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PORT_STATS_FUNC_MASK:
    bRc = qdPortStatsIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			   pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case PORT_STATUS_FUNC_MASK:
    bRc = qdPortStatusIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			    pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case QOS_FUNC_MASK:
    bRc = qdQoSMapIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case SYS_CTRL_FUNC_MASK:
    bRc = qdSysCtrlIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			 pOutBuf, OutBufLen,pdwRet);       
    break;
      
  case UNM_NET_FUNC_MASK:
    bRc = qdUnmNetIoctl(hOpenContext, Ioctl, pInBuf, InBufLen, 
			pOutBuf, OutBufLen,pdwRet);       
    break;
      
  default:
#ifdef IOCTL_DEBUG
    printk("Unsupported IOCTL code\n");
#endif
    break;
  }
    
  return bRc;
}


/* IOCTL - replies from UNM
 *
 * we need to access the global NETCONFIG (NetConfig) structure
 * we will do it here
 * 
 * its not so OOD, but in future NetConf lib, QD and IM will be one DLL!
 */
GT_STATUS UNM_GetVlanNames(UCHAR *pOutBuf,  DWORD  OutBufLen, DWORD *pdwBytesTransferred)
{
  GT_STATUS rc = GT_OK;
  UINT i;
  DWORD bytesCtr = 0, bytesPerName;
  UCHAR *pOutTemp = pOutBuf;
  WCHAR szNullChar[1] = { '\0' }; 
  WCHAR *pszVlanName; 

  do {
    
    if( OutBufLen < 2 ) {
      *pdwBytesTransferred = 0;
      rc = GT_FAIL;
      break;
    }
		
    for( i=1; i <= mvUnmGetNumOfVlans() ; i++ ) {
      mv_nc_GetVIDName(i, &bytesPerName, &pszVlanName);
      bytesPerName += 1; /* for the '\0' wchar */
      bytesCtr += bytesPerName;
      if(bytesCtr > OutBufLen ) {
	rc = GT_FAIL;
	*pdwBytesTransferred = 0;
	break;
      }
      memcpy(pOutTemp, pszVlanName, bytesPerName );
      pOutTemp[bytesPerName-1] = '\0';
      pOutTemp += bytesPerName;
    }
    if(rc != GT_OK)
      break;
		
    /* okay, now lets add the last '\0' */
    if( bytesCtr == 0 ) {
      ((WCHAR*)pOutBuf)[0] = '\0';
      *pdwBytesTransferred = 1;
      break;
    }
		
    bytesCtr += 1;
    if(bytesCtr > OutBufLen ) {
      rc = GT_FAIL;
      *pdwBytesTransferred = 0;
      break;
    }
    memcpy(pOutTemp, szNullChar, 1);
    *pdwBytesTransferred = bytesCtr;
  }while(0);
  return rc;

}

GT_STATUS 
UNM_GetVlanParams(UCHAR *pInBuf,
		  DWORD  InBufLen,
		  UCHAR *pOutBuf,
		  DWORD  OutBufLen,
		  DWORD *pdwBytesTransferred 
		  )
{
  DWORD bytesPerName;
  GT_STATUS rc = GT_OK;
  UINT i;
  bool found = false;
  WCHAR *pszVlanName;   
  GT_IOCTL_PARAM *pOutIoctlParam;
  do {
    for( i=1; i <= MV_UNM_MAX_VID; i++ ) {
      if( i <= mvUnmGetNumOfVlans() ) {
	mv_nc_GetVIDName(i, &bytesPerName, &pszVlanName);
	if( !memcmp( pInBuf, pszVlanName, InBufLen) ) {
	  found = true;
	  break;
	}
      }
    }
    if(!found) {
      rc = GT_FAIL;
      break;
    }
    pOutIoctlParam = (GT_IOCTL_PARAM *)pOutBuf;
    memmove( pOutIoctlParam->FirstParam.etherAddr.arEther , 
	     mv_nc_GetMacOfVlan(i),
	     GT_ETHERNET_HEADER_SIZE  );
    /* memmove( pOutBuf, &(NetConfig.CpuVlans[i].NetParams), sizeof(VLAN_NET_PARAMS) ); */
    *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM);
  }while(0);
  return rc;
}

GT_STATUS 
UNM_GetPortVlan( DWORD  port,
		 UCHAR *pOutBuf,
		 DWORD  OutBufLen,
		 DWORD *pdwBytesTransferred 
		 )
{
  DWORD bytesPerName;
  WCHAR *pszVlanName; 

  GT_STATUS rc = GT_OK;
  DWORD vid;
  GT_IOCTL_PARAM *pOutIoctlParam;
  do {
    
    vid = (DWORD)mvUnmGetVidOfPort(port);
    if( vid == 0 ) {
      /* this port is not belong to any VLAN */
      *pdwBytesTransferred = 0;
      break;
    }

    pOutIoctlParam = (GT_IOCTL_PARAM *)pOutBuf;
    mv_nc_GetVIDName(vid, &bytesPerName, &pszVlanName);
    /* memcpy(pOutBuf, NetConfig.CpuVlans[vid].szVlanName, NetConfig.CpuVlans[vid].NameLength + 2 ); */
    memmove( pOutIoctlParam->SecondParam.szVlanName, pszVlanName, bytesPerName + 1);
    pOutIoctlParam->SecondParam.szVlanName[bytesPerName] = '\0';
    *pdwBytesTransferred = sizeof(GT_IOCTL_PARAM); /* NetConfig.CpuVlans[vid].NameLength + 1; */

  }while(0);
  return rc;
}



GT_STATUS 
UNM_DisassocPort( DWORD port )
{
  GT_STATUS status;
  MV_UNM_VID vidFrom;
  GT_STATUS rc = GT_OK;

  do {
    /* 1. check validity */
    if( port == GT_CPU_SWITCH_PORT || port > GT_NUM_OF_SWITCH_PORTS )
      {
	rc = GT_FAIL;
	break;
      }
		
    vidFrom = (DWORD)mvUnmGetVidOfPort(port);

    ASSERT( vidFrom <= mvUnmGetNumOfVlans() );
		
    /* if its allready free just return a success */
    if( vidFrom == 0 )
      {
	rc = GT_OK;
	break;		
      }

    /* in order to free a port we need to do the follwoing:
     * QD
     *    1. close this port in the switch
     *    2. if its connected to the cpu, disconnect it from cpu table
     *    3. erase its VLAN table (????)
     * IM
     *    4. find its vlan
     *    5. adjust the trailer of its vlan
     * NETCONF
     *    6. adjust netconf structure
     *    7. update registry (???)
     *		
     * Done with in unimac manager
     * gstpSetPortState(qd_dev, port, GT_PORT_DISABLE);
     *	
     * we need to update netconf and recalculate LAN_PORTS_MODE
     * this port is not belong to any vlan now
     */	
    status = mvUnmPortMoveTo(port, MV_UNM_VID_ISOLATED);
    if(status != GT_OK)
      {
	rc = GT_FAIL;
	/* TODO: rollback???? */
	break;
		
      }
    /* TODO: update registry!!! */

			
  }while(0);
	
  return rc;

}
GT_STATUS 
UNM_AssocPort( DWORD port, WCHAR* szVlanName)
{
  
  MV_UNM_VID vidTo = 0, vidFrom = 0;
  GT_STATUS rc = GT_OK;
  UINT i, uiNumOfVlans;
  WCHAR *pszVlnaName;
  UINT uiVlanNameLen;

  do {
    uiNumOfVlans = mvUnmGetNumOfVlans();
    /* 1. check validity */
    if( port == GT_CPU_SWITCH_PORT || port > GT_NUM_OF_SWITCH_PORTS )
      {
	rc = GT_FAIL;
	break;
      }
    /* do we have this VLAN? */
    for( i=1 ; i <= uiNumOfVlans; i++ )
      {
	mv_nc_GetVIDName(i, &uiVlanNameLen, &pszVlnaName);
	if( !memcmp( szVlanName, pszVlnaName, uiVlanNameLen) ) {
	  vidTo = i;
	}
      }
		
    if( vidTo == 0 ) /* couldn't find the VLAN name to add to */
      {
	rc = GT_FAIL;
	break;		
      }

    vidFrom = (DWORD)mvUnmGetVidOfPort(port);
    ASSERT( vidFrom <= mvUnmGetNumOfVlans() );
    if( vidFrom == vidTo)
      {
	rc = GT_OK;
	break;		
      }
		
		
    /* if its not free we first need to free the port */
    if( vidFrom != 0 )
      {
	rc = UNM_DisassocPort(port);
	if( rc != GT_OK )
	  {
	    rc = GT_FAIL;
	    break;		
	  }
      }
    /* okay, we have a free port and a valid vidTo to add it to
     * in order to free a port we need to do the follwoing:
     * NETCONF
     *    1. adjust netconf structure
     * IM
     *    2. find its vlan
     *    3. adjust the trailer of its vlan
     * QD
     *    1. update the VLAN table of this port
     *	  2. connect it to the cpu
     *    3. open this port in the switch
     * NETCONF
     *    7. update registry (???)
     *	
     * we need to update netconf and recalculate LAN_PORTS_MODE
     * this port is not belong to any vlan now
     */
    rc = mvUnmPortMoveTo(port, vidTo);
    if(rc != GT_OK)
      break;



    /* gstpSetPortState(qd_dev,port, GT_PORT_FORWARDING); */

    /* TODO: update registry!!! */
			
  }while(0);
	
  return rc;


}




