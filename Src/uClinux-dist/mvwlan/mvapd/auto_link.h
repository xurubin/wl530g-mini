#ifndef _AUTO_LINK_H_
#define _AUTO_LINK_H_

#ifdef TURBO_SETUP

extern unsigned char auto_link_marvell_hdr[6];

void AUTO_LINK_ReceiveTSFrame(hostapd *hapd_p, u8 *sa, char *buf, size_t len);
void AUTO_LINK_PatchSSID(hostapd *hapd_p, u8 *ssid);
void AUTO_LINK_UpdateSSIDRoot(u8 *ssid);
void AUTO_LINK_GetSSIDRoot(u8 *ssid);
void AUTO_LINK_LowerPower(hostapd *hapd_p);

#endif

#endif
