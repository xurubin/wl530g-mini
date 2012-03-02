#include <Copyright.h>
/*******************************************************************************
* gtVct.c
*
* DESCRIPTION:
*       API for Marvell Virtual Cable Tester.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <msApi.h>
#include <gtVct.h>
#include <gtHwCntl.h>

/*******************************************************************************
* doesDeviceSupportVCT
*
* DESCRIPTION:
*       This routine check if the given device supports VCT.
*
* INPUTS:
*       ouiMsb - Organizationally Unique Id bits 3:18
*       ouiLsb - Organizationally Unique Id bits 19:24
*
* OUTPUTS:
*		phyType - PHY_1000M, or PHY_100M
*
* RETURNS:
*       GT_OK   			- if the device supports VCT
*       GT_NOT_SUPPORTED 	- otherwise
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
static 
GT_STATUS doesDeviceSupportVCT
(
    IN  GT_QD_DEV *dev,
    IN  GT_U16 ouiMsb,
    IN  GT_U16 ouiLsb,
    OUT GT_U16* phyType
)
{
	GT_STATUS status = GT_OK;

    DBG_INFO(("isDeviceSupportVCT Called.\n"));

	if(ouiMsb != MARVELL_OUI_MSb)
		return GT_NOT_SUPPORTED;

	*phyType = PHY_1000M;

	switch(ouiLsb & PHY_MODEL_MASK)
	{
		case DEV_E3082:
			*phyType = PHY_100M;
		    DBG_INFO(("Phy is E3082.\n"));
			break;
		case DEV_E104X:
			if ((ouiLsb & PHY_REV_MASK) < 0x3)
				status = GT_NOT_SUPPORTED;
			if ((ouiLsb & PHY_REV_MASK) == 0x3)
				*phyType = PHY_1000M_B;
		    DBG_INFO(("Phy is E104X.\n"));
			break;
		case DEV_E1111:
		case DEV_E114X:
		case DEV_E1180:
		    DBG_INFO(("Gigabit Phy.\n"));
			break;
		default:
			status = GT_NOT_SUPPORTED;
		    DBG_INFO(("Unknown Device.\n"));
			break;
	}

	return status;
}


/*******************************************************************************
* analizePhy100MVCTResult
*
* DESCRIPTION:
*       This routine analize the virtual cable test result for 10/100M Phy
*
* INPUTS:
*       regValue - test result
*
* OUTPUTS:
*       cableStatus - analized test result.
*       cableLen    - cable length or the distance where problem occurs.
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
static
GT_STATUS analizePhy100MVCTResult
(
    IN  GT_QD_DEV *dev,
    IN  GT_U16 regValue, 
    OUT GT_TEST_STATUS *cableStatus,
    OUT GT_CABLE_LEN *cableLen
)
{
	int len;

	switch((regValue & 0x6000) >> 13)
	{
		case 0:
			/* test passed. No problem found. */
			*cableStatus = GT_NORMAL_CABLE;
			cableLen->normCableLen = GT_UNKNOWN_LEN;
			break;
		case 1:
			/* test passed. Cable is short. */
			*cableStatus = GT_SHORT_CABLE;
			len = (int)FORMULA_PHY100M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 2:
			/* test passed. Cable is open. */
			*cableStatus = GT_OPEN_CABLE;
			len = (int)FORMULA_PHY100M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 3:
		default:
			/* test failed. No result is valid. */
			*cableStatus = GT_TEST_FAIL;
			break;
	}

	return GT_OK;
}


/*******************************************************************************
* getCableStatus_Phy100M
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the 10/100Mbps phy,
*       and returns the the status per Rx/Tx pair.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
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
static 
GT_STATUS getCableStatus_Phy100M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8            hwPort,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U16 reg26, reg27;

    DBG_INFO(("getCableStatus_100Phy Called.\n"));

	/* 
	 * start Virtual Cable Tester
	 */
	if((status= hwWritePhyReg(dev,hwPort,26,0x8000)) != GT_OK)
	{
		return status;
	}

	do
	{
		if((status= hwReadPhyReg(dev,hwPort,26,&reg26)) != GT_OK)
		{
			return status;
		}
		
	} while(reg26 & 0x8000);

	/*
	 * read the test result for RX Pair
	 */
	if((status= hwReadPhyReg(dev,hwPort,26,&reg26)) != GT_OK)
	{
		return status;
	}
		
	/*
	 * read the test result for TX Pair
	 */
	if((status= hwReadPhyReg(dev,hwPort,27,&reg27)) != GT_OK)
	{
		return status;
	}
		
	cableStatus->phyType = PHY_100M;

	/*
	 * analyze the test result for RX Pair
	 */
	analizePhy100MVCTResult(dev, reg26, &cableStatus->cableStatus[MDI_RX_PAIR], 
							&cableStatus->cableLen[MDI_RX_PAIR]);

	/*
	 * analyze the test result for TX Pair
	 */
	analizePhy100MVCTResult(dev, reg27, &cableStatus->cableStatus[MDI_TX_PAIR], 
							&cableStatus->cableLen[MDI_TX_PAIR]);

	return status;
}


/*******************************************************************************
* analizePhy1000MVCTResult
*
* DESCRIPTION:
*       This routine analize the virtual cable test result for a Gigabit Phy
*
* INPUTS:
*       reg17 - original value of register 17
*       regValue - test result
*
* OUTPUTS:
*       cableStatus - analized test result.
*       cableLen    - cable length or the distance where problem occurs.
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
static
GT_STATUS analizePhy1000MVCTResult
(
    IN  GT_QD_DEV *dev,
    IN  GT_U16 reg17,
    IN  GT_U16 regValue, 
    OUT GT_TEST_STATUS *cableStatus,
    OUT GT_CABLE_LEN *cableLen
)
{
	GT_U16 u16Data;
	int len;

	switch((regValue & 0x6000) >> 13)
	{
		case 0:
			/* test passed. No problem found. */
			*cableStatus = GT_NORMAL_CABLE;

			u16Data = reg17;

			/* To get Cable Length, Link should be on and Speed should be 100M or 1000M */
			if(!(u16Data & 0x0400))
			{
				cableLen->normCableLen = GT_UNKNOWN_LEN;
				break;
			}

			if((u16Data & 0xC000) != 0x8000)
			{
				cableLen->normCableLen = GT_UNKNOWN_LEN;
				break;
			}

			/*
			 * read the test result for the selected MDI Pair
			 */

			u16Data = ((u16Data >> 7) & 0x7);

			switch(u16Data)
			{
				case 0:
					cableLen->normCableLen = GT_LESS_THAN_50M;
					break;
				case 1:
					cableLen->normCableLen = GT_50M_80M;
					break;
				case 2:
					cableLen->normCableLen = GT_80M_110M;
					break;
				case 3:
					cableLen->normCableLen = GT_110M_140M;
					break;
				case 4:
					cableLen->normCableLen = GT_MORE_THAN_140;
					break;
				default:
					cableLen->normCableLen = GT_UNKNOWN_LEN;
					break;
			}
			break;
		case 1:
			/* test passed. Cable is short. */
			*cableStatus = GT_SHORT_CABLE;
			len = (int)FORMULA_PHY1000M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 2:
			/* test passed. Cable is open. */
			*cableStatus = GT_OPEN_CABLE;
			len = (int)FORMULA_PHY1000M(regValue & 0xFF);
			if(len <= 0)
				cableLen->errCableLen = 0;
			else
				cableLen->errCableLen = (GT_U8)len;
			break;
		case 3:
		default:
			/* test failed. No result is valid. */
			*cableStatus = GT_TEST_FAIL;
			break;
	}

	return GT_OK;
}


/*******************************************************************************
* getCableStatus_Phy1000M
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the 10/100Mbps phy,
*       and returns the the status per Rx/Tx pair.
*
* INPUTS:
*       port - logical port number.
*		reg17 - original value of reg17.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
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
static 
GT_STATUS getCableStatus_Phy1000M
(	
    IN  GT_QD_DEV *dev,
    IN  GT_U8			hwPort,
    IN  GT_U16 			reg17,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U16 reg28;
	int i;

    DBG_INFO(("getCableStatus_Phy1000M Called.\n"));

	if((status= hwWritePhyReg(dev,hwPort,22,0)) != GT_OK)
	{
		return status;
	}

	/* 
	 * start Virtual Cable Tester
	 */
	if((status= hwWritePhyReg(dev,hwPort,28,0x8000)) != GT_OK)
	{
		return status;
	}

	do
	{
		if((status= hwReadPhyReg(dev,hwPort,28,&reg28)) != GT_OK)
		{
			return status;
		}
		
	} while(reg28 & 0x8000);

	cableStatus->phyType = PHY_1000M;

	for (i=0; i<GT_MDI_PAIR_NUM; i++)
	{
		/*
		 * Select a MDI Pair to be read
		 */
		if((status = hwWritePhyReg(dev,hwPort,22,(GT_U16)i)) != GT_OK)
		{
			return status;
		}
		
		/*
		 * read the test result for the selected MDI Pair
		 */
		if((status = hwReadPhyReg(dev,hwPort,28,&reg28)) != GT_OK)
		{
			return status;
		}
		
		/*
		 * analyze the test result for RX Pair
		 */
		if((status = analizePhy1000MVCTResult(dev, reg17, reg28, 
								&cableStatus->cableStatus[i], 
								&cableStatus->cableLen[i])) != GT_OK)
		{
			return status;
		}
	}

	return GT_OK;
}

static
GT_STATUS workAround_Phy1000M
(
  GT_QD_DEV *dev,
  GT_U8 hwPort
)
{
	GT_STATUS status;

    DBG_INFO(("workAround for Gigabit Phy Called.\n"));

	if((status = hwWritePhyReg(dev,hwPort,29,0x1e)) != GT_OK)
	{
		return status;
	}
		
	if((status = hwWritePhyReg(dev,hwPort,30,0xcc00)) != GT_OK)
	{
		return status;
	}

	if((status = hwWritePhyReg(dev,hwPort,30,0xc800)) != GT_OK)
	{
		return status;
	}
	if((status = hwWritePhyReg(dev,hwPort,30,0xc400)) != GT_OK)
	{
		return status;
	}
	if((status = hwWritePhyReg(dev,hwPort,30,0xc000)) != GT_OK)
	{
		return status;
	}
	if((status = hwWritePhyReg(dev,hwPort,30,0xc100)) != GT_OK)
	{
		return status;
	}

    DBG_INFO(("workAround for Gigabit Phy completed.\n"));
	return GT_OK;
}


/*******************************************************************************
* gvctGetCableStatus
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the requested port,
*       and returns the the status per MDI pair.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Command - vctGetCableDiag
*
*******************************************************************************/
GT_STATUS gvctGetCableDiag
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT        port,
    OUT GT_CABLE_STATUS *cableStatus
)
{
	GT_STATUS status;
	GT_U8 hwPort;
	GT_U16 ouiMsb, ouiLsb, phyType;
	GT_U16 u16Data, orgReg0, orgReg17;

    DBG_INFO(("gvctGetCableDiag Called.\n"));
	hwPort = GT_LPORT_2_PORT(port);

	if((status= hwReadPhyReg(dev,hwPort,2,&ouiMsb)) != GT_OK)
	{
	    DBG_INFO(("Not able to read Phy Register.\n"));
		return status;
	}

	if((status= hwReadPhyReg(dev,hwPort,3,&ouiLsb)) != GT_OK)
	{
	    DBG_INFO(("Not able to read Phy Register.\n"));
		return status;
	}

	if((status = doesDeviceSupportVCT(dev,ouiMsb,ouiLsb,&phyType)) != GT_OK)
	{
	    DBG_INFO(("Device is not supporting VCT.\n"));
		return status;
	}
	
	/*
	 * save original register 17 value, which will be used later depending on
	 * test result.
	 */
	if((status= hwReadPhyReg(dev,hwPort,17,&orgReg17)) != GT_OK)
	{
	    DBG_INFO(("Not able to reset the Phy.\n"));
		return status;
	}

	/*
	 * reset the phy
	 */
	if((status= hwReadPhyReg(dev,hwPort,0,&orgReg0)) != GT_OK)
	{
	    DBG_INFO(("Not able to reset the Phy.\n"));
		return status;
	}

	if((status= hwWritePhyReg(dev,hwPort,0,(GT_U16)(0x8000|orgReg0))) != GT_OK)
	{
	    DBG_INFO(("Not able to reset the Phy.\n"));
		return status;
	}

	do
	{
		if((status= hwReadPhyReg(dev,hwPort,0,&u16Data)) != GT_OK)
		{
	    	DBG_INFO(("Not able to read Phy Register.\n"));
			return status;
		}
	} while (u16Data & 0x8000);

	switch(phyType)
	{
		case PHY_100M:
			status = getCableStatus_Phy100M(dev,hwPort,cableStatus);
			break;
		case PHY_1000M:
			status = getCableStatus_Phy1000M(dev,hwPort,orgReg17,cableStatus);
			break;
		case PHY_1000M_B:
			workAround_Phy1000M(dev,hwPort);
			status = getCableStatus_Phy1000M(dev,hwPort,orgReg17,cableStatus);
			break;
		default:
			status = GT_FAIL;
			break;
	}
	
	/*
	 * reset the phy
	 */
	if((status= hwWritePhyReg(dev,hwPort,0,(GT_U16)(0x8000|orgReg0))) != GT_OK)
	{
	    DBG_INFO(("Not able to reset the Phy.\n"));
		return status;
	}

	do
	{
		if((status= hwReadPhyReg(dev,hwPort,0,&u16Data)) != GT_OK)
		{
	    	DBG_INFO(("Not able to read Phy Register.\n"));
			return status;
		}
	} while (u16Data & 0x8000);

	return status;	
}

