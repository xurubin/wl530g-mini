#ifndef MVWLAN_WEXT_H
#define MVWLAN_WEXT_H

/* Linux Wireless Extensions compatibility code */

#include <linux/wireless.h>
#if WIRELESS_EXT > 12
#include <net/iw_handler.h>
#endif /* WIRELESS_EXT > 12 */
#if WIRELESS_EXT < 9
#warning Linux wireless extensions versions older than 9 are not supported
/* Compile limited version without wireless ext support */
#undef WIRELESS_EXT
#endif /* WIRELESS_EXT < 9 */


/* if wireless ext is not supported */
#ifndef IW_MODE_ADHOC
#define IW_MODE_ADHOC 1
#endif
#ifndef IW_MODE_INFRA
#define IW_MODE_INFRA 2
#endif
#ifndef IW_MODE_MASTER
#define IW_MODE_MASTER 3
#endif
#ifndef IW_MODE_REPEAT
#define IW_MODE_REPEAT 4
#endif
#ifndef IW_MODE_SECOND
#define IW_MODE_SECOND 5
#endif
#ifndef IW_MODE_MONITOR
#define IW_MODE_MONITOR 6
#endif



#ifdef WIRELESS_EXT
/* Conversion to new driver API by Jean II */

#if WIRELESS_EXT <= 12
/* Wireless extensions backward compatibility */


/* Dummy prototype, as we don't really need it */
struct iw_request_info;
#endif /* WIRELESS_EXT <= 12 */


#if WIRELESS_EXT >= 15
/* Wireless ext ver15 allows verification of iwpriv support and sub-ioctls can
 * be included even if not especially configured. */
#ifndef MVWLAN_USE_WE_SUB_IOCTLS
#define MVWLAN_USE_WE_SUB_IOCTLS
#endif /* MVWLAN_USE_WE_SUB_IOCTLS */

/* Assume that hosts using new wireless ext also have new wireless tools
 * (ver >= 25) */
#ifndef MVWLAN_USE_WE_TYPE_ADDR
#define MVWLAN_USE_WE_TYPE_ADDR
#endif /* MVWLAN_USE_WE_TYPE_ADDR */
#endif /* WIRELESS_EXT >= 15 */


#ifdef MVWLAN_USE_WE_TYPE_ADDR
/* Added in WIRELESS_EXT 15, but can be used with older versions assuming
 * iwpriv ver >= 25 */
#ifndef IW_PRIV_TYPE_ADDR
#define IW_PRIV_TYPE_ADDR 0x6000
#endif /* IW_PRIV_TYPE_ADDR */
#endif /* MVWLAN_USE_WE_TYPE_ADDR */

#endif /* WIRELESS_EXT */

#endif /* MVWLAN_WEXT_H */
