#ifndef MVWLAN_CONFIG_H
#define MVWLAN_CONFIG_H


/* In the previous versions of Host AP driver, support for user space version
 * of IEEE 802.11 management (hostapd) used to be disabled in the default
 * configuration. From now on, support for hostapd is always included and it is
 * possible to disable kernel driver version of IEEE 802.11 management with a
 * separate define, MVWLAN_NO_KERNEL_IEEE80211_MGMT. */
#define MVWLAN_NO_KERNEL_IEEE80211_MGMT

/* Maximum number of events handler per one interrupt */
#define MVWLAN_MAX_INTERRUPT_EVENTS 20

/* Include wireless extensions sub-ioctl support even if wireless extensions
 * version is less than 15 (actually, only if it is 12 .. 14). If ver >= 15,
 * these will be included. Please note, that this requires iwpriv version 25
 * or higher (older versions will segfault due to long ioctl list). */
/* #define MVWLAN_USE_WE_SUB_IOCTLS */

/* Use IW_PRIV_TYPE_ADDR with private WE ioctls taking MAC address argument
 * (instead of old 18*char). This requires iwpriv ver >= 25. This will be
 * automatically included for WIRELESS_EXT >= 15. */
/* #define MVWLAN_USE_WE_TYPE_ADDR */

/* Following defines can be used to remove unneeded parts of the driver, e.g.,
 * to limit the size of the kernel module. Definitions can be added here in
 * mvWLAN_config.h or they can be added to make command with EXTRA_CFLAGS,
 * e.g.,
 * 'make pccard EXTRA_CFLAGS="-DMVWLAN_NO_DEBUG -DMVWLAN_NO_PROCFS_DEBUG"'
 */

/* Do not include debug messages into the driver */
/* #define MVWLAN_NO_DEBUG */

/* Do not include /proc/net/mvwlan/wlan#/{registers,debug} */
/* #define MVWLAN_NO_PROCFS_DEBUG */

/* Do not include station functionality (i.e., allow only Master (Host AP) mode
 */
/* #define MVWLAN_NO_STATION_MODES */

#endif /* MVWLAN_CONFIG_H */
