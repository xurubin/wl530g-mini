#ifndef _MVWLAN_BOOSTER_H_
#define _MVWLAN_BOOSTER_H_

#ifdef BOOSTER_MODE

int    MVWLAN_BOOSTER_CheckToSetBoosterMode(local_info_t *local);
void   MVWLAN_BOOSTER_SetBoosterMode(int option, local_info_t *local);
int    MVWLAN_BOOSTER_GetBoosterMode(local_info_t *local);
void   MVWLAN_BOOSTER_ClearRATimer(void);
UINT32 MVWLAN_BOOSTER_GetRATimer(void);
void   MVWLAN_BOOSTER_UpdateTxDataCount(local_info_t *local);
void   MVWLAN_BOOSTER_UpdateReceiveDataCount(local_info_t *local);
void   MVWLAN_BOOSTER_CheckBoosterMode(local_info_t *local);

#endif

#endif
