#ifndef _MVWLAN_RATE_H_
#define _MVWLAN_RATE_H_


void MVWLAN_RATE_SetupPower(void);
void MVWLAN_RATE_InitSTA(struct sta_info *sta_p);
void MVWLAN_RATE_UpdatePower(UINT8 tx_rate, UINT16 aid, SINT32 result);
UINT8 MVWLAN_RATE_GetStationRate(struct sta_info *sta_p);
void MVWLAN_RATE_UpdateStationTxStats(struct sta_info *sta_p, MRVL_TX_DONE_INFO *doneInfo_p);
void MVWLAN_RATE_UpdateStationRxStats(struct sta_info *sta_p, struct mvwlan_80211_rx_status *rx_stats);
void MVWLAN_RATE_apiSetHiPower(void);

#endif
