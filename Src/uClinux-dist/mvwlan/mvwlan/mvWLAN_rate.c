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
#include <linux/rtnetlink.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_rate.h"
#ifdef BOOSTER_MODE
#include "mvWLAN_booster.h"
#endif

/* Mapping of Rates to indices. */
/*#define PRINTDEBUG */
#define ADAPT_ON_TX_ERR
#define ENABLE_POWER_ADAPT 
#define ENABLE_ADAPTIVE_BAILOUT 1 
/*#define UPDATE_TX_QUEUE */
#define MULTIPATH_FIX
#define STATISTICAL_METHOD

/* Mapping of Rates to indices. */
#define RATEINDEX_1MBPS 0

#define RATEINDEX_6MBPS 5
#define RATEINDEX_11MBPS 3
#define RATEINDEX_22MBPS 4
#define RATEINDEX_36MBPS 10
#define RATEINDEX_54MBPS 12
#define RATEINDEX_72MBPS 13


/* for Falcon power adaptation */
#define POWER_STEP_VALUE                     6
/* minimum number of pkts to wait before climbing up in rate */
#define MIN_PKT_BEFORE_RISING               16
#define RETRY_THRESHOLD                      2

#ifdef BOOSTER_MODE
/*# successes to wait before updating correction */
#define MAX_TRANSMIT_SUCCESS                 1
#define MAX_TRANSMIT_FAILURE_STATS           4
#define MAX_TRANSMIT_PKT_ON_FAILURE_STATS   16  /* > 25% of failures  */
#else
/*# successes to wait before updating correction */
#define MAX_TRANSMIT_SUCCESS                 4
#define MAX_TRANSMIT_FAILURE_STATS           3
#define MAX_TRANSMIT_PKT_ON_FAILURE_STATS   15  /* > 20% of failures  */
#endif



static UINT8 OSC_HYSTERISIS[IEEEtypes_MAX_DATA_RATES_G]  =
#ifdef BOOSTER_MODE
	{3,3,3,3,3,3,3,3,3,3,3,3,3,3};  /* For each rate.. */
#else
	{6,6,6,8,8,8,8,8,8,8,8,4,4,4};  /* For each rate.. */
#endif

static UINT32 POWER_FLAG=0;

/* variables for Falcon power adaptation */
static UINT8 RFReg20_P1;
static UINT8 RFReg20_P2;
static UINT8 RFReg20Calibrated;
static UINT8 RFReg26_P1;
static UINT8 RFReg26_P2;
static UINT8 RFReg26Calibrated;

#ifdef PEREGRINE
#define CURRENT_CHANNEL      0

int POWER_LEVEL_P0  =  14;  /* 14 dBm for 54, 48, 36 Mbps */
int POWER_LEVEL_P1  =  16;  /* 16 dBm for 36, 24, 18, 12, 9, 6, 11 Mbps */
int POWER_LEVEL_P2  =  18;  /* 18 dBm for 11, 5.5, 2, 1 Mbps */

#define POWER_ADPT_SET_POWER_LEVEL(pwr_lvl) msi_wl_SetRFOutputPower(CURRENT_CHANNEL, pwr_lvl)
#endif /* PEREGRINE */

static UINT8 HighestRate;
static UINT8 HighestRateAid;

static UINT32 DisableOscFlagCounterThreshold[IEEEtypes_MAX_DATA_RATES_G] =
	{100, 100, 100, 200, 200, 200, 200, 200, 400, 1000, 1000, 1000, 1000, 500};

extern MIB_STA_CFG *mib_StaCfg_p;


static UINT8 MVWLAN_RATE_RateReductionCheck(struct sta_info *sta_p);
static UINT8 MVWLAN_RATE_RateIncreaseCheck(struct sta_info *sta_p);


extern void msi_wl_SetPowerAdapt(UINT8 valRfReg20, UINT8 valRfReg26);
extern void msi_wl_GetPowerAdaptSetting(UINT8 *valRfReg20, UINT8 *valRfReg26);

#ifdef PEREGRINE
extern BOOLEAN msi_wl_SetRFOutputPower(UINT32 chan, UINT32 power);
#define	RF_FALCON       0			/* Bill Modify and JL Write it */  
#define	RF_PEREGRINE    1			/* Bill Modify and JL Write it */
extern int rfChip;
#endif /* PEREGRINE */

SINT8 StepValue=6; //For power Adaptation


void MVWLAN_RATE_SetupPower(void)
{
    if(POWER_FLAG==0)
    {
#ifdef PEREGRINE
        if (rfChip == RF_FALCON)
        {
#endif
	msi_wl_GetPowerAdaptSetting(&RFReg20Calibrated, &RFReg26Calibrated);

    	RFReg20_P1 = RFReg20Calibrated - ((StepValue - 2 > 0) ? (StepValue - 2) : 0);
    	RFReg20_P2= RFReg20Calibrated - StepValue;

	RFReg26_P1= (RFReg26Calibrated < 0x70) ? (RFReg26Calibrated + 0x10) : 0x73;
	RFReg26_P2= (RFReg26Calibrated < 0x70) ? (RFReg26Calibrated + 0x10) : 0x73;

#ifdef PEREGRINE
        }
#endif

	/* Reset HighestRate to Lowest data rate. */
	HighestRate = 0;
	HighestRateAid = RATEINDEX_1MBPS;
        /*Now by default, set the AP to high power. */
#ifdef PEREGRINE
        if (rfChip == RF_PEREGRINE)
            POWER_ADPT_SET_POWER_LEVEL(POWER_LEVEL_P2);
        else
#endif /* PEREGRINE */
        msi_wl_SetPowerAdapt((UINT8)RFReg20_P2, (UINT8)RFReg26_P2);

    	POWER_FLAG = 2; /* Tuned for far range */
    }
}


void MVWLAN_RATE_InitSTA(struct sta_info *sta_p)
{
	sta_p->tx_success = 0;
	sta_p->tx_failures = 0;
	sta_p->tx_retry = 0;
	sta_p->pkt_tx_since_fail = 0;
	sta_p->tx_failures_stats = 0;
	sta_p->rate_increase_increment = 5;
	sta_p->osc_flag = 0;
	sta_p->disable_osc_flag_counter = 0;
	sta_p->current_rssi = 0;
	sta_p->rssi_store = 0;

}


void MVWLAN_RATE_UpdatePower(UINT8 tx_rate, UINT16 aid, SINT32 result)
{

	if (mib_StaCfg_p->mib_TSMode == AUTOLINK_START)
		return;

	/* First check what is the Highest Rate */
	if (HighestRate < tx_rate) {

		HighestRate = tx_rate;
		HighestRateAid = aid;

	} else if ((HighestRateAid == aid) && (tx_rate < HighestRate)) {

		HighestRate = tx_rate;
	}

	/* Now update power if necessary */
	if ((POWER_FLAG != 2) &&
		((result != 1) && (HighestRate <= RATEINDEX_11MBPS))) {

		/* We should be at P=P1. Go to P2 */
#ifdef PEREGRINE
        if (rfChip == RF_PEREGRINE)
            msi_wl_SetRFOutputPower(CURRENT_CHANNEL, POWER_LEVEL_P2);
        else
#endif /* PEREGRINE */		
		msi_wl_SetPowerAdapt(RFReg20_P2, RFReg26_P2);
		POWER_FLAG = 2;

	} else if ((POWER_FLAG != 1) &&
		((result != 1) && (HighestRate <= RATEINDEX_36MBPS) && (HighestRate > RATEINDEX_11MBPS))) {

		/* We should be at P=P0. Go tp P1 */
#ifdef PEREGRINE
        if (rfChip == RF_PEREGRINE)
            msi_wl_SetRFOutputPower(CURRENT_CHANNEL, POWER_LEVEL_P1);
        else
#endif /* PEREGRINE */	        
		msi_wl_SetPowerAdapt(RFReg20_P1, RFReg26_P1);
		POWER_FLAG = 1;

	} else if ((POWER_FLAG != 0) &&
		((result != -1) && (HighestRate >= RATEINDEX_36MBPS))) {

		/* We should be at P=P1. Go to P0 */
#ifdef PEREGRINE
        if (rfChip == RF_PEREGRINE)
            msi_wl_SetRFOutputPower(CURRENT_CHANNEL, POWER_LEVEL_P0);
        else
#endif /* PEREGRINE */	        
		msi_wl_SetPowerAdapt(RFReg20Calibrated, RFReg26Calibrated);
		POWER_FLAG = 0;

	} else if ((POWER_FLAG != 1) &&
		((result != -1) && (HighestRate >= RATEINDEX_11MBPS) && (HighestRate < RATEINDEX_36MBPS))) {

		/* We should be at P=P2. Go to P1 */
#ifdef PEREGRINE
        if (rfChip == RF_PEREGRINE)
            msi_wl_SetRFOutputPower(CURRENT_CHANNEL, POWER_LEVEL_P1);
        else
#endif /* PEREGRINE */	        
		msi_wl_SetPowerAdapt(RFReg20_P1, RFReg26_P1);
		POWER_FLAG = 1;

	}

	return;

}


UINT8 MVWLAN_RATE_GetStationRate(struct sta_info *sta_p)
{
	UINT8 RateIndex, RateIncrement;

	/* note: this routine do not check client support rates */

	/* Initialise the Rate */
	RateIndex = sta_p->tx_rate_idx;

	/* If Osc flag is set, increment the DisableOscFlagCounter.
	 * Below is a cute trick to do it. Osc Flag is either 0 or 1.
	 */
	sta_p->disable_osc_flag_counter += sta_p->osc_flag;

	/* Check if we should disable the Osc Flag.
	 * We will disable it only if the failures recorded is less than 6% of total pkts transmitted
	 */
#ifdef BOOSTER_MODE
	if (((sta_p->disable_osc_flag_counter >= DisableOscFlagCounterThreshold[sta_p->tx_rate_idx])
		&& (sta_p->tx_failures <= ((sta_p->disable_osc_flag_counter) >> 4)))
		|| (MVWLAN_BOOSTER_GetRATimer() != 0)) {
#else
	if ((sta_p->disable_osc_flag_counter >= DisableOscFlagCounterThreshold[sta_p->tx_rate_idx])
		&& (sta_p->tx_failures <= ((sta_p->disable_osc_flag_counter) >> 4))) {
#endif

		/* Disable the Osc Flag. */
		sta_p->osc_flag = 0;
		/* Reset the counter. */
		sta_p->disable_osc_flag_counter = 0;
		/* reset the failure counter */
		sta_p->tx_failures = 0;

#ifdef BOOSTER_MODE
		MVWLAN_BOOSTER_ClearRATimer();
#endif
	}

	/* Use the Statistical function. */
	if (MVWLAN_RATE_RateReductionCheck(sta_p) &&  (RateIndex > 0)) {

		/* Check if we are NOT  at 36 or 11Mbps and at low power. */
		if (((((RateIndex == RATEINDEX_36MBPS) && (POWER_FLAG!=1))
			|| ((RateIndex == RATEINDEX_11MBPS) && (POWER_FLAG!=2)))
			&& (sta_p->aid == HighestRateAid))) {
            /* no code for this case */

		} else {
#ifndef MULTIPATH_FIX
            //before decrementing Rate, handle corner cases.
            
            if((RateIndex >= 8) || (RateIndex <= 2) || (RateIndex==6)) 
                RateIndex--;
            else
            {
                //Case 1. If Decrementing from 12Mbps, go to 11Mbps
                if(RateIndex==7)
                    RateIndex=3;
                else if(RateIndex==3)
                    RateIndex=6;  //11Mbps -> 9Mbps
                else if (RateIndex==5)                        
                    RateIndex=2; //6Mbps->5.5Mbps
            }
#else //MULTIPATH_FIX
            //Currently, do not go to 6 and 9Mbps data rates. Jump straight to 11Mbps
			if(RateIndex != 7)
				RateIndex--;
			else
				RateIndex=3;
#endif //#ifndef MULTIPATH_FIX
		}

#ifdef ENABLE_POWER_ADAPT
        //Update the power if needed.
        //Note that here I know that I will be going down in rate by only one unit.
        //Hence I can afford to call update power in the else statement.
        //But if we are jumping down by more than one unit, we should call update power each time.
			MVWLAN_RATE_UpdatePower(RateIndex, sta_p->aid, -1);
#endif //#ifdef ENABLE_POWER_ADAPT

		/* Now if we have gone down in rate, set the OSC_FLag */
		sta_p->osc_flag = 1;
		/* Reset the DisableOscFlagCounter and start counting again. */
		sta_p->disable_osc_flag_counter = 0;
		/* Update the RSSI_STore as well. */
		sta_p->rssi_store = sta_p->current_rssi;
		/* reset the failure counter */
		sta_p->tx_failures = 0;

	} else {

		RateIncrement = MVWLAN_RATE_RateIncreaseCheck(sta_p);

		if (RateIncrement != 0) {

			/* Check if we are NOT  at 36 or 11Mbps and at high power. */
			if (((((RateIndex == RATEINDEX_36MBPS) && (POWER_FLAG!=0))
				|| ((RateIndex == RATEINDEX_11MBPS) && (POWER_FLAG!=1)))
				&& (sta_p->aid== HighestRateAid))) {
                /* no code for this case
                 */

			} else {
				if(RateIndex != 3)
					RateIndex += RateIncrement;
                else
                {
                    //if we are climbing in rate from 11Mbps, go straight to 12Mbps
					RateIndex=7;
				}
			}

#ifdef ENABLE_POWER_ADAPT
				/* Update the power. */
				MVWLAN_RATE_UpdatePower(RateIndex, sta_p->aid, 1);
#endif  /* #ifdef  ENABLE_POWER_ADAPT */

			if (RateIndex != RATEINDEX_54MBPS) {				
				/* Now if we have gone up in rate, set the OSC_FLag
				 * to prevent from going up more than one step.
				 */
				sta_p->osc_flag = 1;
				/* Reset the DisableOscFlagCounter and start counting again. */
				sta_p->disable_osc_flag_counter = 0;
				/* Update the RSSI_STore as well. */
				sta_p->rssi_store = sta_p->current_rssi;
				/* reset the failure counter */
				sta_p->tx_failures = 0;
			}
		}
	}

	/* currently, disallow 72 Mbps and .b mode 22Mbps */
	if ((RateIndex == RATEINDEX_22MBPS) || (RateIndex >= RATEINDEX_72MBPS)) {

		RateIndex--;
	}

	/* Disallow G rates if client is a B only client. */
	if (sta_p->b_only_client) {

		if (RateIndex > RATEINDEX_11MBPS) {

			/* Initialise to a 11Mbps rate. */
			RateIndex = RATEINDEX_11MBPS;
		}
	}

	/* Reset some rate adapt variables, if we change the rate. */
	if (RateIndex != sta_p->tx_rate_idx) {

		sta_p->tx_success = 0;
		sta_p->tx_retry = 0;
		sta_p->pkt_tx_since_fail = 0;
		sta_p->tx_failures_stats = 0;
	}

	return RateIndex;

}


void MVWLAN_RATE_UpdateStationTxStats(struct sta_info *sta_p, MRVL_TX_DONE_INFO *doneInfo_p)
{
	local_info_t *local = sta_p->local;
	MIB_OP_DATA *mib_opdata_p = &local->sysConfig->Mib802dot11->OperationTable;

	if (local->fixed_tx_data_rate)
		return;

	/*
	 * Check to see if the the packet was successfully
	 * transmitted.
	 */

	/* Now do the following only if the rate at which the pkt
	 * was transmitted is the same as the rate in the Station's Database
	 * If it is not, we should not make any adjustments to correction factor etc.
	 */
	if (doneInfo_p->rateID == sta_p->tx_rate_idx) {

		/* Update the number of retries */
		sta_p->tx_retry = doneInfo_p->retryCnt;

		if ((doneInfo_p->status & 0x01) == IEEEtypes_STATUS_SUCCESS) {

			if ((mib_opdata_p->ShortRetryLim - doneInfo_p->retryCnt) <= RETRY_THRESHOLD) {

				/* Increment the number of successes */
				sta_p->tx_success++;
				sta_p->tx_count[sta_p->tx_rate_idx]++;

			} else {  /* too many retries. Count them as failures */

				/*Reinitialise the Success count to zero */
				sta_p->tx_success = 0;
				sta_p->tx_failures_stats++;
				/* Also log the failures when we are in the lower rate */

				if (sta_p->osc_flag)
					sta_p->tx_failures++;
			}

		} else {   /*Failure. Pkt Drop. */

			/* Reinitialise the Success count to zero */
			sta_p->tx_success = 0;
			sta_p->tx_failures_stats++;

			if (sta_p->osc_flag)
				sta_p->tx_failures++;
		}

		/* Increment the Pkts Transmitted since last fail. */
		if(sta_p->tx_failures_stats)
			sta_p->pkt_tx_since_fail++;

	}

}


void MVWLAN_RATE_UpdateStationRxStats(struct sta_info *sta_p, struct mvwlan_80211_rx_status *rx_stats)
{
	local_info_t *local = sta_p->local;

	if (local->fixed_tx_data_rate)
		return;

	/*Get the current RSSI. */
	sta_p->current_rssi = rx_stats->signal;

	/*Check if OSC flag has been set and if so, check if we should reset it. */
	if (sta_p->osc_flag) {

		if (((sta_p->current_rssi - sta_p->rssi_store >= OSC_HYSTERISIS[sta_p->tx_rate_idx]))
			&& (sta_p->tx_failures <= (sta_p->disable_osc_flag_counter >> 4))
			&& (sta_p->disable_osc_flag_counter >= MIN_PKT_BEFORE_RISING)) {

			/* reset osc_flag */
			sta_p->osc_flag = 0;
			/* reset the counter */
			sta_p->disable_osc_flag_counter = 0;
			/* reset the failure count */
			sta_p->tx_failures = 0;
		}
	}

}

void MVWLAN_RATE_apiSetHiPower(void)
{
#ifdef PEREGRINE
        if (rfChip == RF_FALCON)
        {
#endif
        msi_wl_GetPowerAdaptSetting(&RFReg20Calibrated, &RFReg26Calibrated);
        RFReg20_P1= RFReg20Calibrated - ((StepValue-2>0)?(StepValue-2):0);;
        RFReg20_P2= RFReg20Calibrated - StepValue;
        //RFReg20_P1= 0x5c;
        RFReg26_P1= (RFReg26Calibrated < 0x70)?RFReg26Calibrated + 0x10:0x73;
        RFReg26_P2= (RFReg26Calibrated < 0x70)?RFReg26Calibrated + 0x10:0x73;
#ifdef PEREGRINE
        }
#endif
        /*Reset HighestRate to Lowest data rate. */
        HighestRate=0;
        HighestRateAid=RATEINDEX_1MBPS;
        /*Now by default, set the AP to high power. */
#ifdef PEREGRINE
        if (rfChip == RF_PEREGRINE)
            POWER_ADPT_SET_POWER_LEVEL(POWER_LEVEL_P2);
        else
#endif /* AP_PGRINE */
        msi_wl_SetPowerAdapt((UINT8)RFReg20_P2, (UINT8)RFReg26_P2);

        POWER_FLAG=2;/*Tuned for far range */
}

static UINT8 MVWLAN_RATE_RateReductionCheck(struct sta_info *sta_p)
{
	UINT8 RateReductionAmt = 0;

	if (sta_p->tx_failures_stats >= MAX_TRANSMIT_FAILURE_STATS) {

		if (sta_p->pkt_tx_since_fail <= MAX_TRANSMIT_PKT_ON_FAILURE_STATS) {

			RateReductionAmt = 1;

			/* Each time we have to go down in rate, decrement the RateIncreaseIncrement */
			if (sta_p->rate_increase_increment > 1)
				sta_p->rate_increase_increment--;

		}

		sta_p->tx_failures_stats = 0;
		sta_p->pkt_tx_since_fail = 0;
	}

	return RateReductionAmt;

}


static UINT8 MVWLAN_RATE_RateIncreaseCheck(struct sta_info *sta_p)
{
	if (!sta_p->osc_flag) {

		if (sta_p->tx_success >= MAX_TRANSMIT_SUCCESS) {

			/* reinitialise the tx_success Variable */
			sta_p->tx_success = 0;
			return (((sta_p->tx_rate_idx + sta_p->rate_increase_increment) <= 12) ? (sta_p->rate_increase_increment) : (RATEINDEX_54MBPS - sta_p->tx_rate_idx));
		}
	}

	return 0;
}
