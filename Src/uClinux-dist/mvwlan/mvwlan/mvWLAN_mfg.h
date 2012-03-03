#ifndef _MVWLAN_MFG_H_
#define _MVWLAN_MFG_H_

typedef struct
{
   u32 enable;
   u32 data_rate;
   u32 percent;

} TX_DUTY_CYCLE_T;

typedef struct
{
   u32 data_rate;
   u32 frame_pattern;
   u32 frame_len;
   u32 frame_count;
   u32 short_prem;
   u8  bssid[6];

} TX_MULTI_FRAMES_T;


int  MVWLAN_MFG_TxMulticastFrame(TX_MULTI_FRAMES_T *pkTxFrame);
int  MVWLAN_MFG_TxDutyCycle(TX_DUTY_CYCLE_T *pkDutyCycle);


#endif
