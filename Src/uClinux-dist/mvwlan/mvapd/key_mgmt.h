#ifndef _KEY_MGMT_H_
#define _KEY_MGMT_H_

#define KEY_INFO_ERROR 0x0004

int     key_mgmt_init(hostapd *hapd);
void    key_mgmt_send_msg1(hostapd *hapd, struct sta_info *sta);
void    key_mgmt_send_msg3(hostapd *hapd, struct sta_info *sta);
void    key_mgmt_send_grp_msg1(hostapd *hapd, struct sta_info *sta);
Boolean key_mgmt_recv_msg2(hostapd *hapd, struct sta_info *sta);
Boolean key_mgmt_recv_msg4(hostapd *hapd, struct sta_info *sta);
Boolean key_mgmt_recv_grp_msg2(hostapd *hapd, struct sta_info *sta);
void    key_mgmt_MIC_counter_measure_invoke(hostapd *hapd);
void    key_mgmt_dump_eapol_key_frame(hostapd *hapd, EAPOL_KeyMsg_t *eapol_ptr);

#endif
