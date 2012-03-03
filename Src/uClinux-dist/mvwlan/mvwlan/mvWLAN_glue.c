/* Here are the glue functions for use with the HAL layer. */
#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"
#include "pool.h"

/* Used by msi_wl_RFTxDone(MRVL_TX_DONE_INFO *info) in hal_wlmac.c */
POOL pool_hd = {
	.free__     = NULL,              /* linked list of free blocks */
	.bufSz__    = 0,                 /* maximum buffer size (in bytes) */
	.nTot__     = 0,                 /* total number of buffers */
	.nFree__    = 0,                 /* number of free blocks remaining */
	.nMin__     = 0,                 /* minimum number of free blocks */
	.start__    = NULL,              /* 1st buffer in the pool */
	.end__      = (void *)0xFFFFFFFF,/* last buffer in the pool */
	.freeTail__ = NULL
};

BOOLEAN WlanEnabled = TRUE;
UINT8 mib_rxAntenna = AP_ANT_B;

void os_TaskDelay(os_TickCount_t Count)
{
	volatile os_TickCount_t i;
	volatile int j;

	for (i = 0; i < Count; i++) {
		for (j = 0; j < 5000; j++);
	}
}

/* For uClinux we have ffnetdrv already so hal_eth.c was excluded from our
 * library */
BOOLEAN msi_wl_EthInit(NET_DEV *netDev)
{
	return TRUE;
}

/* For uClinux we do not rely on buffer management mechanism so bufmgt.c
 * was excluded from our library */
int msi_wl_FreePktBuf(char *buf)
{
	return 0;
}

/* Used by void Hw_Init(void) in hal_wlmac.c */
void hw_Init(void)
{
}

/* Used by msi_wl_RFTxDone(MRVL_TX_DONE_INFO *info) in hal_wlmac.c */
WL_STATUS ProcessTxDone(void)
{
	return 0;
}

/* Used by void MonitorTimerProcess(UINT8 *data) in hal_wlmac.c */
void sendNullData(unsigned char *StnMacAddr)
{
}

/* Used by void MonitorTimerProcess(UINT8 *data) in hal_wlmac.c */
void RxBeaconIsr(void)
{
}

