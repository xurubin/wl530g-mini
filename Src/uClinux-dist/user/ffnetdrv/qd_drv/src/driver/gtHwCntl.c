#include <Copyright.h>
/********************************************************************************
* gtHwCntl.c
*
* DESCRIPTION:
*       Functions declarations for Hw accessing quarterDeck phy, internal and
*       global registers.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <gtHwCntl.h>
#include <gtMiiSmiIf.h>

/****************************************************************************/
/* Phy registers related functions.                                         */
/****************************************************************************/

/*******************************************************************************
* hwReadPhyReg
*
* DESCRIPTION:
*       This function reads a switch's port phy register.
*
* INPUTS:
*       portNum - Port number to read the register for.
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadPhyReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev->baseRegAddr,
                                portNum + PHY_REGS_START_ADDR);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

    DBG_INFO(("Read from phy(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
              portNum,phyAddr,regAddr));

    return retVal;
}


/*******************************************************************************
* hwWritePhyReg
*
* DESCRIPTION:
*       This function writes to a switch's port phy register.
*
* INPUTS:
*       portNum - Port number to write the register for.
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWritePhyReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev->baseRegAddr,
                                portNum + PHY_REGS_START_ADDR);

    DBG_INFO(("Write to phy(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
              portNum,phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",data));
    return miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);
}


/*******************************************************************************
* hwGetPhyRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's port phy register.
*
* INPUTS:
*       portNum     - Port number to read the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetPhyRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;

    if(hwReadPhyReg(dev,portNum,regAddr,&tmpData) != GT_OK)
        return GT_FAIL;

    CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;

    DBG_INFO(("Read from phy(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetPhyRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's port phy register.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetPhyRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;

    if(hwReadPhyReg(dev,portNum,regAddr,&tmpData) != GT_OK)
        return GT_FAIL;

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));
    return hwWritePhyReg(dev,portNum,regAddr,tmpData);
}


/****************************************************************************/
/* Per port registers related functions.                                    */
/****************************************************************************/

/*******************************************************************************
* hwReadPortReg
*
* DESCRIPTION:
*       This function reads a switch's port register.
*
* INPUTS:
*       portNum - Port number to read the register for.
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadPortReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev->baseRegAddr,
                                portNum + PORT_REGS_START_ADDR);

    retVal =  miiSmiIfReadRegister(dev,phyAddr,regAddr,data);
    DBG_INFO(("Read from port(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
              portNum,phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}


/*******************************************************************************
* hwWritePortReg
*
* DESCRIPTION:
*       This function writes to a switch's port register.
*
* INPUTS:
*       portNum - Port number to write the register for.
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWritePortReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev->baseRegAddr,
                                portNum + PORT_REGS_START_ADDR);

    DBG_INFO(("Write to port(%d) register: phyAddr 0x%x, regAddr 0x%x, ",
              portNum,phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",data));

    return miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);
}


/*******************************************************************************
* hwGetPortRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's port register.
*
* INPUTS:
*       portNum     - Port number to read the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetPortRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;

    if(hwReadPortReg(dev,portNum,regAddr,&tmpData) != GT_OK)
        return GT_FAIL;

    CALC_MASK(fieldOffset,fieldLength,mask);

    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    DBG_INFO(("Read from port(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetPortRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's port register.
*
* INPUTS:
*       portNum     - Port number to write the register for.
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetPortRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    portNum,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;

    if(hwReadPortReg(dev,portNum,regAddr,&tmpData) != GT_OK)
        return GT_FAIL;

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);
    DBG_INFO(("Write to port(%d) register: regAddr 0x%x, ",
              portNum,regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    return hwWritePortReg(dev,portNum,regAddr,tmpData);
}


/****************************************************************************/
/* Global registers related functions.                                      */
/****************************************************************************/

/*******************************************************************************
* hwReadGlobalReg
*
* DESCRIPTION:
*       This function reads a switch's global register.
*
* INPUTS:
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadGlobalReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    OUT GT_U16   *data
)
{
    GT_U8       phyAddr;
    GT_STATUS   retVal;

    phyAddr = CALC_SMI_DEV_ADDR(dev->baseRegAddr,
                                GLOBAL_REGS_START_ADDR);

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);
    DBG_INFO(("read from global register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",*data));
    return retVal;
}


/*******************************************************************************
* hwWriteGlobalReg
*
* DESCRIPTION:
*       This function writes to a switch's global register.
*
* INPUTS:
*       regAddr - The register's address.
*       data    - The data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWriteGlobalReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_U8   phyAddr;

    phyAddr = CALC_SMI_DEV_ADDR(dev->baseRegAddr,
                                GLOBAL_REGS_START_ADDR);

    DBG_INFO(("Write to global register: phyAddr 0x%x, regAddr 0x%x, ",
              phyAddr,regAddr));
    DBG_INFO(("data 0x%x.\n",data));

    return miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);
}


/*******************************************************************************
* hwGetGlobalRegField
*
* DESCRIPTION:
*       This function reads a specified field from a switch's global register.
*
* INPUTS:
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to read.
*
* OUTPUTS:
*       data        - The read register field.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwGetGlobalRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    OUT GT_U16   *data
)
{
    GT_U16 mask;            /* Bits mask to be read */
    GT_U16 tmpData;

    if(hwReadGlobalReg(dev,regAddr,&tmpData) != GT_OK)
        return GT_FAIL;

    CALC_MASK(fieldOffset,fieldLength,mask);
    tmpData = (tmpData & mask) >> fieldOffset;
    *data = tmpData;
    DBG_INFO(("Read from global register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("fOff %d, fLen %d, data 0x%x.\n",fieldOffset,fieldLength,*data));

    return GT_OK;
}


/*******************************************************************************
* hwSetGlobalRegField
*
* DESCRIPTION:
*       This function writes to specified field in a switch's global register.
*
* INPUTS:
*       regAddr     - The register's address.
*       fieldOffset - The field start bit index. (0 - 15)
*       fieldLength - Number of bits to write.
*       data        - Data to be written.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  The sum of fieldOffset & fieldLength parameters must be smaller-
*           equal to 16.
*
*******************************************************************************/
GT_STATUS hwSetGlobalRegField
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    regAddr,
    IN  GT_U8    fieldOffset,
    IN  GT_U8    fieldLength,
    IN  GT_U16   data
)
{
    GT_U16 mask;
    GT_U16 tmpData;

    if(hwReadGlobalReg(dev,regAddr,&tmpData) != GT_OK)
        return GT_FAIL;

    CALC_MASK(fieldOffset,fieldLength,mask);

    /* Set the desired bits to 0.                       */
    tmpData &= ~mask;
    /* Set the given data into the above reset bits.    */
    tmpData |= ((data << fieldOffset) & mask);

    DBG_INFO(("Write to global register: regAddr 0x%x, ",
              regAddr));
    DBG_INFO(("fieldOff %d, fieldLen %d, data 0x%x.\n",fieldOffset,
              fieldLength,data));

    return hwWriteGlobalReg(dev,regAddr,tmpData);
}

/*******************************************************************************
* hwReadQDReg
*
* DESCRIPTION:
*       This function reads a switch register.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwReadMiiReg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     phyAddr,
    IN  GT_U8     regAddr,
    OUT GT_U16    *data
)
{
    GT_STATUS   retVal;

    retVal = miiSmiIfReadRegister(dev,phyAddr,regAddr,data);

    DBG_INFO(("Read from phy(0x%x) register: regAddr 0x%x, data 0x%x.\n",
              phyAddr,regAddr,*data));

    return retVal;
}


/*******************************************************************************
* hwWriteMiiReg
*
* DESCRIPTION:
*       This function writes a switch register.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK on success, or
*       GT_FAIL otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS hwWriteMiiReg
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    phyAddr,
    IN  GT_U8    regAddr,
    IN  GT_U16   data
)
{
    GT_STATUS   retVal;

    retVal = miiSmiIfWriteRegister(dev,phyAddr,regAddr,data);

    DBG_INFO(("Write to phy(0x%x) register: regAddr 0x%x, data 0x%x.\n",
              phyAddr,regAddr,data));

    return retVal;
}


