#ifndef MVWLAN_H
#define MVWLAN_H


/* mvWLAN.c */
void mvWLAN_netif_wake_queues(local_info_t *local);
void mvWLAN_netif_stop_queues(local_info_t *local);
int mvWLAN_80211_header_parse(struct sk_buff *skb, unsigned char *haddr);
int mvWLAN_80211_get_hdrlen(u16 fc);
struct net_device_stats *mvWLAN_get_stats(struct net_device *dev);
void mvWLAN_setup_dev(struct net_device *dev, local_info_t *local,
		      int main_dev);

/* mvWLAN_crypt.c */
void mvWLAN_init_crypt_lib(local_info_t *local);
void mvWLAN_deinit_crypt_lib(local_info_t *local);

/* mvWLAN_proc.c */
void mvWLAN_init_dev_proc(local_info_t *local);
void mvWLAN_remove_dev_proc(local_info_t *local);
void mvWLAN_init_ap_proc(local_info_t *local);
void mvWLAN_remove_ap_proc(local_info_t *local);
void mvWLAN_init_proc(void);
void mvWLAN_remove_proc(void);

/* mvWLAN_ioctl.c */
int mvWLAN_init_dev(void *data);
int mvWLAN_free_dev(void);
int mvWLAN_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

/* mvWLAN_wds.c */
void mvWLAN_init_wds(local_info_t *local);
int mvWLAN_wds_add(local_info_t *local, u8 *remote_addr, int rtnl_locked);
void mvWLAN_wds_update_sta_info(local_info_t *local, u8* remote_addr, IEEEtypes_SuppRatesElement_t *Rates_p, IEEEtypes_ExtSuppRatesElement_t *ExtRates_p, IEEEtypes_CapInfo_t CapInfo);
int mvWLAN_wds_is_wds_link(local_info_t *local, u8 *remote_addr);
#endif /* MVWLAN_H */
