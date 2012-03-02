#include <Copyright.h>

/********************************************************************************
* gtPhyCtrl.h
* 
* DESCRIPTION:
* API definitions for PHY control facility.
*
* DEPENDENCIES:
* None.
*
* FILE REVISION NUMBER:
* $Revision: 1 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/*******************************************************************************
* phySetAutoMode
*
* DESCRIPTION:
*       This routine will write the given mode to Autonegotiation Advertisement
*		Register. Supported mode is as follows:
*		- Auto for both speed and duplex.
*		- Auto for speed only and Full duplex.
*		- Auto for speed only and Half duplex.
*		- Auto for duplex only and speed 100Mbps.
*		- Auto for duplex only and speed 10Mbps.
*		
*
* INPUTS:
* port - The logical port number
* mode - Auto Mode to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.8, 4.7, 4.6, and 4.5 Autonegotiation Advertisement Register
*******************************************************************************/

static 
GT_STATUS phySetAutoMode
(
IN GT_QD_DEV *dev,
IN GT_U8 hwPort,
IN GT_PHY_AUTO_MODE mode
)
{
    GT_U16 			u16Data;

    DBG_INFO(("phySetAutoMode Called.\n"));

    if(hwReadPhyReg(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	/* Mask out all auto mode related bits. */
	u16Data &= ~QD_PHY_MODE_AUTO_AUTO;

	switch(mode)
	{
		case SPEED_AUTO_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_AUTO_AUTO;
				break;
		case SPEED_100_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_100_AUTO;
				break;
		case SPEED_10_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_10_AUTO;
				break;
		case SPEED_AUTO_DUPLEX_FULL:
				u16Data |= QD_PHY_MODE_AUTO_FULL;
				break;
		case SPEED_AUTO_DUPLEX_HALF:
				u16Data |= QD_PHY_MODE_AUTO_HALF;
				break;
		default:
	 	       DBG_INFO(("Unknown Auto Mode (%d)\n",mode));
				return GT_FAIL;
	}

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if(hwWritePhyReg(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data));
   	    return GT_FAIL;
	}

	return GT_OK;
}

/*******************************************************************************
* gprtPhyReset
*
* DESCRIPTION:
*       This routine preforms PHY reset.
*		After reset, phy will be in Autonegotiation mode.
*
* INPUTS:
* port - The logical port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 0.15 - Reset
* data sheet register 0.13 - Speed
* data sheet register 0.12 - Autonegotiation
* data sheet register 0.8  - Duplex Mode
*******************************************************************************/

GT_STATUS gprtPhyReset
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtPhyReset Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

	if(phySetAutoMode(dev,hwPort,SPEED_AUTO_DUPLEX_AUTO) != GT_OK)
	{
	    DBG_INFO(("Setting AutoMode Failed.\n"));
		return GT_FAIL;
	}

	u16Data = QD_PHY_RESET | QD_PHY_SPEED | QD_PHY_DUPLEX | QD_PHY_AUTONEGO;

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}


/*******************************************************************************
* gprtSetPortLoopback
*
* DESCRIPTION:
* Enable/Disable Internal Port Loopback. Enabling Loopback will disable the 
* Autonegotiation and set the phy mode to 10 Half duplex.
* To test Loopback on a different mode, such as 100 Full duplex, 
* user may need to call gprtSetPortSpeed and gprtSetPortDuplexMode.
* Disabling Loopback does not enable the Autonegotiation, so user may need to call
* gprtPortAutoNegEnable in order to enable Autonegotiation.
*
* INPUTS:
* port - logical port number
* enable - If GT_TRUE, enable loopback mode
* If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/

GT_STATUS gprtSetPortLoopback
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   enable
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtSetPortLoopback Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
        return GT_FAIL;
	}

	if(enable)
	{
		u16Data = QD_PHY_LOOPBACK | QD_PHY_RESET;
	}
	else
	{
		u16Data &= ~QD_PHY_LOOPBACK;
	}


    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}


/*******************************************************************************
* gprtSetPortSpeed
*
* DESCRIPTION:
* Sets speed for a specific logical port. This function will keep the duplex mode
* and loopback mode to the previous value, but disable others, such as Autonegotiation.
*
* INPUTS:
* port - logical port number
* speed - port speed,  	GT_TRUE=100Mb/s
* 			GT_FALSE=10Mb/s
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.13 - Speed Selection (LSB)
*
*******************************************************************************/

GT_STATUS gprtSetPortSpeed
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   speed
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtSetPortSpeed Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
        return GT_FAIL;
	}

	if(speed)
	{
		u16Data = QD_PHY_RESET | (u16Data & (QD_PHY_LOOPBACK | QD_PHY_DUPLEX)) | QD_PHY_SPEED;
	}
	else
	{
		u16Data = QD_PHY_RESET | (u16Data & (QD_PHY_LOOPBACK | QD_PHY_DUPLEX));
	}


    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}


/*******************************************************************************
* gprtPortAutoNegEnable
*
* DESCRIPTION:
* Enable/disable an Auto-Negotiation for duplex mode on specific
* logical port. When Autonegotiation is disabled, phy will be in 10Mbps Half Duplex mode.
* Enabling Autonegotiation will set 100BASE-TX Full Duplex, 100BASE-TX Full Duplex, 
* 100BASE-TX Full Duplex, and 100BASE-TX Full Duplex in AutoNegotiation Advertisement register 
*
* INPUTS:
* port - logical port number
* state - GT_TRUE for enable Auto-Negotiation for duplex mode,
* GT_FALSE otherwise
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.12 - Auto-Negotiation Enable
* data sheet register 4.8, 4.7, 4.6, 4.5 - Auto-Negotiation Advertisement bits
*
*******************************************************************************/

GT_STATUS gprtPortAutoNegEnable
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   state
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtPortAutoNegEnable Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

	if(state)
	{
		if(phySetAutoMode(dev,hwPort,SPEED_AUTO_DUPLEX_AUTO) != GT_OK)
		{
		    DBG_INFO(("Setting AutoMode Failed.\n"));
			return GT_FAIL;
		}

		u16Data = QD_PHY_RESET | QD_PHY_SPEED | QD_PHY_DUPLEX | QD_PHY_AUTONEGO;
	}
	else
	{
		u16Data = QD_PHY_RESET;
	}


    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}

/*******************************************************************************
* gprtPortPowerDown
*
* DESCRIPTION:
* Enable/disable (power down) on specific logical port. When this function is called
* with normal operation request, phy will set to Autonegotiation mode.
*
* INPUTS:
* port - logical port number
* state - GT_TRUE: power down
*         GT_FALSE: normal operation
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.11 - Power Down
*
*******************************************************************************/

GT_STATUS gprtPortPowerDown
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   state
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtPortPowerDown Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

	if(state)
	{
		if(phySetAutoMode(dev, hwPort,SPEED_AUTO_DUPLEX_AUTO) != GT_OK)
		{
		    DBG_INFO(("Setting AutoMode Failed.\n"));
			return GT_FAIL;
		}

		u16Data = QD_PHY_POWER;
	}
	else
	{
		u16Data = QD_PHY_SPEED | QD_PHY_DUPLEX | QD_PHY_AUTONEGO;
	}

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));

    return retVal;
}

/*******************************************************************************
* gprtPortRestartAutoNeg
*
* DESCRIPTION:
* Restart AutoNegotiation. If AutoNegotiation is not enabled, it'll enable it.
* Loopback and Power Down will be disabled by this routine.
*
* INPUTS:
* port - logical port number
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.9 - Restart Auto-Negotiation
*
*******************************************************************************/

GT_STATUS gprtPortRestartAutoNeg
( 
IN GT_QD_DEV *dev,
IN GT_LPORT  port
)
{
    GT_STATUS       retVal;      
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtPortRestartAutoNeg Called.\n"));
  
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
        return GT_FAIL;
	}

	u16Data &= (QD_PHY_DUPLEX | QD_PHY_SPEED);
	u16Data |= (QD_PHY_RESTART_AUTONEGO | QD_PHY_AUTONEGO);

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}

/*******************************************************************************
* gprtSetPortDuplexMode
*
* DESCRIPTION:
* Sets duplex mode for a specific logical port. This function will keep the speed
* and loopback mode to the previous value, but disable others, such as Autonegotiation.
*
* INPUTS:
* port - logical port number
* dMode - dulpex mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
GT_STATUS gprtSetPortDuplexMode
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   dMode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtSetPortDuplexMode Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
        return GT_FAIL;
	}

	if(dMode)
	{
		u16Data = QD_PHY_RESET | (u16Data & (QD_PHY_LOOPBACK | QD_PHY_SPEED)) | QD_PHY_DUPLEX;
	}
	else
	{
		u16Data = QD_PHY_RESET | (u16Data & (QD_PHY_LOOPBACK | QD_PHY_SPEED));
	}


    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}


/*******************************************************************************
* gprtSetPortAutoMode
*
* DESCRIPTION:
*       This routine sets up the port with given Auto Mode.
*		Supported mode is as follows:
*		- Auto for both speed and duplex.
*		- Auto for speed only and Full duplex.
*		- Auto for speed only and Half duplex.
*		- Auto for duplex only and speed 100Mbps.
*		- Auto for duplex only and speed 10Mbps.
*		
*
* INPUTS:
* port - The logical port number
* mode - Auto Mode to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.8, 4.7, 4.6, and 4.5 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtSetPortAutoMode
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_PHY_AUTO_MODE mode
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtSetPortAutoMode Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

	if(phySetAutoMode(dev,hwPort,mode) != GT_OK)
	{
	    DBG_INFO(("Setting AutoMode Failed.\n"));
		return GT_FAIL;
	}

	u16Data = QD_PHY_RESET | QD_PHY_SPEED | QD_PHY_DUPLEX | QD_PHY_AUTONEGO;

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
    if(retVal != GT_OK)
        DBG_INFO(("Failed.\n"));
    else
        DBG_INFO(("OK.\n"));
    return retVal;
}


/*******************************************************************************
* gprtSetPause
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*		Register. And restart the autonegotiation.
*
* INPUTS:
* port - The logical port number
* state - either GT_TRUE(for enable) or GT_FALSE(for disable)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtSetPause
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   state
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

    DBG_INFO(("phySetPause Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	if(state == GT_TRUE)
	{
		/* Set the Pause bit. */
		u16Data |= QD_PHY_PAUSE;
	}
	else
	{
		/* Reset the Pause bit. */
		u16Data &= ~QD_PHY_PAUSE;
	}

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if(hwWritePhyReg(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data));
   	    return GT_FAIL;
	}

    return retVal;
}


