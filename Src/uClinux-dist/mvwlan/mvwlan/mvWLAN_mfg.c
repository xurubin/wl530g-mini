#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/if_arp.h>
#include <linux/random.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,44))
#include <linux/tqueue.h>
#else
#include <linux/workqueue.h>
#endif
#include <linux/kmod.h>
#include <asm/bitops.h>
#include <asm/memory.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/arch/irqs.h>


#include "IEEE_types.h"
#include "hal_sys.h"
#include "wl_mib.h"
#include "wl_hal.h"
#include "wl_macros.h"
#include "wlmac.h"
#include "wlmac_mgmt.h"
#include "idma.h"
#include "gpio.h"
#include "intr.h"
#include "memmap.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_mfg.h"


#define MAX_DATA_BODY_LEN    1024
#define PACKET_FRAME_ADDR    0xC000B000
#define TRANSMIT_INFO_ADDR   0xC000B800

#define W81_BBP_CCA_CTRL1    (0x35)
#define W81_BBP_CCA_CTRL2    (0x36)

/* TYPE DEFINITIONS
 */

typedef PACK_START struct
{
   UINT8             RetryCnt;
   UINT8             Reserved;
   UINT16            Status;
   IEEEtypes_Frame_t *HdrAddr;
   UINT16            Service;
   UINT8             Rate;
   UINT8             Power;
   UINT16            RtsDurId0;
   UINT16            RtsDurId1;
   UINT32            Tsf0;
   UINT32            Tsf1;
   UINT16            FragBasicDurId0;
   UINT16            FragBasicDurId1;
   UINT16            TxParam;
   UINT16            QosControl;

} PACK_END txInfoField_t;


typedef struct dataFrame_t
{
   IEEEtypes_GenHdr_t   Hdr;
   UINT8                FrmBody[MAX_DATA_BODY_LEN];
   UINT32               FCS;

} PACK_END dataFrame_t;


/* The structure for data frames
 */

typedef struct txPacketQ_t
{
   dataFrame_t frame;

} PACK_END txPacketQ_t;


int MVWLAN_MFG_TxMulticastFrame(TX_MULTI_FRAMES_T *pkTxFrame)
{
   int    error;
   UINT32 frameCount;
   UINT32 cbQWrPtr;
   UINT32 tmp;
   UINT32 txPtrBase;
   UINT32 doneRdPtr;
   UINT32 doneWrPtr;
   UINT32 doneTxBase;
   UINT32 doneQPtr;
   UINT32 framePtr;
   UINT32 count = 0;
   UINT32 tx_11b_mode;
   IEEEtypes_MacAddr_t dst_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
   IEEEtypes_MacAddr_t my_addr;
   UINT8 ccaCtrl1;
   UINT8 ccaCtrl2;
   txPacketQ_t *ptrTxData;
   txInfoField_t *ptrDataTxInfo;

   if (msi_wl_GetMACAddr((UINT8 *) &my_addr) != TRUE)
      return FAIL;

   sys_InitBBPAccess();

   /* read BB registers CCA register 1 and 2
    * and change the ED thresholds to the max values
    */
   msi_wl_ReadBBP(W81_BBP_CCA_CTRL1, &ccaCtrl1);
   msi_wl_ReadBBP(W81_BBP_CCA_CTRL2, &ccaCtrl2);
   msi_wl_WriteBBP(W81_BBP_CCA_CTRL1, ccaCtrl1 | 0x3F);
   msi_wl_WriteBBP(W81_BBP_CCA_CTRL2, ccaCtrl2 | 0x1F);

   /* set MTD_BY_PASS register
    */
   WL_READ_WORD(MTD_BY_PASS, tmp);
   WL_WRITE_WORD(MTD_BY_PASS, tmp & ~0x2000);

   /* pointer to the frame
    */
   ptrTxData = (txPacketQ_t *) ((UINT32) PACKET_FRAME_ADDR);

   /* pointer to the data transmit information
    */
   ptrDataTxInfo = (txInfoField_t *) ((UINT32) TRANSMIT_INFO_ADDR);

   /* fill the info field
    */
   tx_11b_mode = (pkTxFrame->data_rate > 4) ? 0 : 1 ;
   ptrDataTxInfo->RetryCnt = 1;
   ptrDataTxInfo->Reserved = 0;
   ptrDataTxInfo->TxParam = 0x40 |
       (tx_11b_mode << 8) |
       (((pkTxFrame->short_prem) << 2) & 0x00000004);
   ptrDataTxInfo->RtsDurId0 = 0x0FFF;  /* RTS_Dur0 */
   ptrDataTxInfo->RtsDurId1 = 0x0FFF;  /* RTS_Dur1 */
   ptrDataTxInfo->Service = 0x00;
   ptrTxData->frame.Hdr.DurationId = 0x8000;
   ptrDataTxInfo->Status = 0;
   ptrDataTxInfo->HdrAddr = (IEEEtypes_Frame_t *) &ptrTxData->frame;
   ptrDataTxInfo->Rate = pkTxFrame->data_rate;
   ptrDataTxInfo->Power = 0;
   ptrDataTxInfo->FragBasicDurId0 = 0;
   ptrDataTxInfo->FragBasicDurId1 = 0;
   ptrDataTxInfo->Tsf0 = 0;
   ptrDataTxInfo->Tsf1 = 0;

   /* file the frame header
    */
   if (pkTxFrame->frame_len > MAX_DATA_BODY_LEN)
       pkTxFrame->frame_len = MAX_DATA_BODY_LEN;
   ptrTxData->frame.Hdr.FrmBodyLen = pkTxFrame->frame_len;
   ptrTxData->frame.Hdr.FrmCtl.ProtocolVersion = 0;
   ptrTxData->frame.Hdr.FrmCtl.Type = IEEE_TYPE_DATA;
   ptrTxData->frame.Hdr.FrmCtl.Subtype = DATA;
   ptrTxData->frame.Hdr.FrmCtl.ToDs = 0;
   ptrTxData->frame.Hdr.FrmCtl.FromDs = 0;
   ptrTxData->frame.Hdr.FrmCtl.MoreFrag = 0;
   ptrTxData->frame.Hdr.FrmCtl.Retry = 0;
   ptrTxData->frame.Hdr.FrmCtl.PwrMgmt = 0;
   ptrTxData->frame.Hdr.FrmCtl.MoreData = 0;
   ptrTxData->frame.Hdr.FrmCtl.Wep = 0;
   ptrTxData->frame.Hdr.FrmCtl.Order = 0;
   memcpy (&ptrTxData->frame.Hdr.Addr1, &dst_addr, sizeof (IEEEtypes_MacAddr_t));
   memcpy (&ptrTxData->frame.Hdr.Addr2, &my_addr,
           sizeof (IEEEtypes_MacAddr_t));
   memcpy (&ptrTxData->frame.Hdr.Addr3, &pkTxFrame->bssid,
           sizeof (IEEEtypes_MacAddr_t));
   ptrTxData->frame.Hdr.SeqCtl = 0;
   memcpy (&ptrTxData->frame.Hdr.Addr4, &dst_addr, sizeof (IEEEtypes_MacAddr_t));

   /* file the frame body with the pattern
    */
   memset (&ptrTxData->frame.FrmBody, pkTxFrame->frame_pattern,
           pkTxFrame->frame_len);

   /* get the transmit pointer base (bits 31:11)
    */
   WL_READ_WORD(TX_PTR_BASE, tmp);
   txPtrBase = tmp & 0xFFFFF800;

   /* update the TX queue
    */
   for (frameCount = 0, error = SUCCESS;
        (frameCount < pkTxFrame->frame_count) && (error == SUCCESS);
        frameCount++)
   {
      /* get the CB queue pointer (bits 8:0)
       */
      WL_READ_WORD(TX_TCQ0_WRPTR, tmp);
      cbQWrPtr = tmp & 0x000001FF;

      /* data frame pointer is placed into the CB queue pointer
       * and the CB pointer is incremented (one new frame to send)
       */
      WL_WRITE_WORD((txPtrBase | cbQWrPtr), (UINT32) ptrDataTxInfo);
      WL_WRITE_WORD((TX_TCQ0_WRPTR), (cbQWrPtr + 4));

      /* check the Tx done queue
       */
      count = 0;
      do
      {
          doneRdPtr = txPtrBase |
              (DONE_QID << 8) |
              (WL_REGS32(TX_DONE_RDPTR) & 0xFF);
          doneWrPtr = txPtrBase |
              (DONE_QID << 8) |
              (WL_REGS32(TX_DONE_WRPTR) & 0xFF);

          count++;
          if (count > 0x00030000)
          {
              msi_wl_WriteBBP(W81_BBP_CCA_CTRL1, ccaCtrl1);
              msi_wl_WriteBBP(W81_BBP_CCA_CTRL2, ccaCtrl2);
              return FAIL;
          }
      }
      while (doneRdPtr == doneWrPtr);

      /* a frame is in the doneQ and we check its status
       * get the TX done base pointer
       * TC0 queue QID is 0
       */
      doneTxBase = txPtrBase | (TC0_QID << 0);
      doneQPtr = doneTxBase | doneRdPtr;

      /* get the data frame address
       */
      framePtr = WL_REGS32(doneQPtr);

      /* get the status from the header
       */
      tmp = WL_REGS32(framePtr & 0xF0FFFFFF); /* mask retry cnt value */
      error = ((tmp & 0x00010000) == 0x00010000) ? FAIL : SUCCESS;

      /* increment the done read pointer
       */
      WL_REGS32(TX_DONE_RDPTR) = doneRdPtr + 4;

      msi_wl_ResetSysWatchDog();

      udelay(1);

   }

   /* restore BB registers CCA register 1 and 2
    */
   msi_wl_WriteBBP(W81_BBP_CCA_CTRL1, ccaCtrl1);
   msi_wl_WriteBBP(W81_BBP_CCA_CTRL2, ccaCtrl2);

   return error;
}


/* constants used by MVWLAN_HW_TxDutyCycle()
 */
#define DATARATE_22M   4
#define TX_RX_ACTIVE   0


int MVWLAN_MFG_TxDutyCycle(TX_DUTY_CYCLE_T *pkDutyCycle)
{
   UINT32 dataRate;
   UINT32 percent;
   UINT32 baseAddress;      /* beacon base address */
   UINT32 rxMode;
   UINT32 tmp;
   int    payLoadLen;
   double dataRateNum[] =
   {
      1.0, 2.0, 5.5, 11.0, 22.0, 6.0, 9.0, 12.0, 18.0, 24.0, 36.0, 48.0, 54.0, 72.0
   };
   txBcnInfo_t *txInfoField;
   IEEEtypes_GenHdr_t *headerFrame;


   /* get parameters
    */
   dataRate = pkDutyCycle->data_rate;
   percent = pkDutyCycle->percent;

   /* enable duty cycle
    */
   if (pkDutyCycle->enable == 1)
   {
      /* Tx mode STATION
       */
      WL_REGS32 (TX_MODE) = STA_MODE;

      /* disable beacon
       */
      WL_REGS32 (TX_TIMER_RUN) = 0;

      /* get the beacon base address
       */
      baseAddress = WL_REGS32 (TX_BCN_BASE);
      if (((int)baseAddress&0xF0000000) == 0)
          txInfoField = (txBcnInfo_t *) 0xC0000008;                  
      else
          txInfoField = (txBcnInfo_t *) baseAddress;

      /* change the data rate
       */
      txInfoField->Rate = dataRate;

      /* 22 Mbps rate requires the service field to be changed
       */
      if (dataRate == DATARATE_22M)
         txInfoField->Service |= 0x10;
      else
         txInfoField->Service &= ~0x10;

      /* change txParam (mode b or g)
       */
      if (dataRate > DATARATE_22M)
         txInfoField->TxParam &= ~0x100;   /* g mode */
      else
         txInfoField->TxParam |= 0x100;   /* b mode */

      /* compute the beacon length (function of the duty cycle percentage)
       * payLoadLen = (percentage / ( 1 - percentage)) * dataRate * 1000
       * in Mbps
       */
      payLoadLen = (UINT32) ((1.0 * percent / (100 - percent)) * dataRateNum[dataRate] * 1000);
      payLoadLen /= 8;   /* byte number */
      if (payLoadLen > 0x1FF)
         payLoadLen = 0x1FF;
      else if (payLoadLen < 0x100)
         payLoadLen = 0x100;

      /* get the header frame address
       */
      headerFrame = (IEEEtypes_GenHdr_t *) txInfoField->HdrAddr;
      headerFrame->FrmBodyLen = payLoadLen;

      /* configure test mode, BCN period, TSF, ACC
       */
      tmp = WL_REGS32 (MTD_BY_PASS);
      WL_REGS32 (MTD_BY_PASS) = tmp & ~0x2000;
      WL_REGS32 (MPP_PSM) = TX_RX_ACTIVE;
      rxMode = WL_REGS32 (RX_PROMISCUOUS);
      rxMode &= ~0x08;
      WL_REGS32 (RX_PROMISCUOUS) = rxMode;
      WL_REGS32 (TX_BCN_PERIOD) = 1;
      WL_REGS32 (TX_TSF_LO) = 0;
      WL_REGS32 (TX_TSF_HI) = 0;
      WL_REGS32 (TX_ACC_INTVL_LO) = 0x2000;
      WL_REGS32 (TX_ACC_INTVL_HI) = 0;
      WL_REGS32 (TX_TIMER_COMMIT) = COMMIT_BCN_PERD | COMMIT_ACC_INTVL | COMMIT_TSF;

      /* Tx mode AP and start beacon
       */
      WL_REGS32 (TX_MODE) = AP_MODE;
      WL_REGS32 (TX_TIMER_RUN) = 1;
   }
   else  /* disable duty cycle */
   {
      /* Tx mode STATION
       */
      WL_REGS32 (TX_MODE) = STA_MODE;

      /* disable beacon
       */
      WL_REGS32 (TX_TIMER_RUN) = 0;

      /* promiscuous register: receiver enable
       */
      rxMode = WL_REGS32 (RX_PROMISCUOUS);
      rxMode |= 0x08;
      WL_REGS32 (RX_PROMISCUOUS) = rxMode;

      /* reset Tx
       */
      WL_REGS32 (MTD_MCU_RESET) = RESET_TX;
   }

   return (SUCCESS);
}
