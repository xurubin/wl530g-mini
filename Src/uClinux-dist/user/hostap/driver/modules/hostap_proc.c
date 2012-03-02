/* /proc routines for Host AP driver */

#define PROC_LIMIT (PAGE_SIZE - 80)


#ifndef PRISM2_NO_PROCFS_DEBUG
static int prism2_debug_proc_read(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	int i;

	if (off != 0) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "next_txfid=%d next_alloc=%d\n",
		     local->next_txfid, local->next_alloc);
	for (i = 0; i < PRISM2_TXFID_COUNT; i++)
		p += sprintf(p, "FID: tx=%04X intransmit=%04X\n",
			     local->txfid[i], local->intransmitfid[i]);
	p += sprintf(p, "FW TX rate control: %d\n", local->fw_tx_rate_control);
	p += sprintf(p, "beacon_int=%d\n", local->beacon_int);
	p += sprintf(p, "dtim_period=%d\n", local->dtim_period);
	p += sprintf(p, "wds_max_connections=%d\n",
		     local->wds_max_connections);
	p += sprintf(p, "dev_enabled=%d\n", local->dev_enabled);
	p += sprintf(p, "sw_tick_stuck=%d\n", local->sw_tick_stuck);
	if (local->crypt && local->crypt->ops)
		p += sprintf(p, "crypt=%s\n", local->crypt->ops->name);

	return (p - page);
}
#endif /* PRISM2_NO_PROCFS_DEBUG */


static int prism2_stats_proc_read(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	struct comm_tallies_sums *sums = (struct comm_tallies_sums *)
		&local->comm_tallies;

	if (off != 0) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "TxUnicastFrames=%u\n", sums->tx_unicast_frames);
	p += sprintf(p, "TxMulticastframes=%u\n", sums->tx_multicast_frames);
	p += sprintf(p, "TxFragments=%u\n", sums->tx_fragments);
	p += sprintf(p, "TxUnicastOctets=%u\n", sums->tx_unicast_octets);
	p += sprintf(p, "TxMulticastOctets=%u\n", sums->tx_multicast_octets);
	p += sprintf(p, "TxDeferredTransmissions=%u\n",
		     sums->tx_deferred_transmissions);
	p += sprintf(p, "TxSingleRetryFrames=%u\n",
		     sums->tx_single_retry_frames);
	p += sprintf(p, "TxMultipleRetryFrames=%u\n",
		     sums->tx_multiple_retry_frames);
	p += sprintf(p, "TxRetryLimitExceeded=%u\n",
		     sums->tx_retry_limit_exceeded);
	p += sprintf(p, "TxDiscards=%u\n", sums->tx_discards);
	p += sprintf(p, "RxUnicastFrames=%u\n", sums->rx_unicast_frames);
	p += sprintf(p, "RxMulticastFrames=%u\n", sums->rx_multicast_frames);
	p += sprintf(p, "RxFragments=%u\n", sums->rx_fragments);
	p += sprintf(p, "RxUnicastOctets=%u\n", sums->rx_unicast_octets);
	p += sprintf(p, "RxMulticastOctets=%u\n", sums->rx_multicast_octets);
	p += sprintf(p, "RxFCSErrors=%u\n", sums->rx_fcs_errors);
	p += sprintf(p, "RxDiscardsNoBuffer=%u\n",
		     sums->rx_discards_no_buffer);
	p += sprintf(p, "TxDiscardsWrongSA=%u\n", sums->tx_discards_wrong_sa);
	p += sprintf(p, "RxDiscardsWEPUndecryptable=%u\n",
		     sums->rx_discards_wep_undecryptable);
	p += sprintf(p, "RxMessageInMsgFragments=%u\n",
		     sums->rx_message_in_msg_fragments);
	p += sprintf(p, "RxMessageInBadMsgFragments=%u\n",
		     sums->rx_message_in_bad_msg_fragments);
	/* FIX: this may grow too long for one page(?) */

	return (p - page);
}


static int prism2_wds_proc_read(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	prism2_wds_info_t *wds;
	unsigned long flags;

	if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}

	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL) {
		p += sprintf(p, "%s\t" MACSTR "\n",
			     wds->dev.name, MAC2STR(wds->remote_addr));
		if ((p - page) > PROC_LIMIT) {
			printk(KERN_DEBUG "%s: wds proc did not fit\n",
			       local->dev->name);
			break;
		}
		wds = wds->next;
	}
	spin_unlock_irqrestore(&local->wdslock, flags);

	if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}

	*start = page + off;

	return (p - page - off);
}


static int prism2_pda_proc_read(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	local_info_t *local = (local_info_t *) data;

	if (local->pda == NULL || off >= PRISM2_PDA_SIZE) {
		*eof = 1;
		return 0;
	}

	if (off + count > PRISM2_PDA_SIZE)
		count = PRISM2_PDA_SIZE - off;

	memcpy(page, local->pda + off, count);
	return count;
}


#ifdef PRISM2_IO_DEBUG
static int prism2_io_debug_proc_read(char *page, char **start, off_t off,
				     int count, int *eof, void *data)
{
	local_info_t *local = (local_info_t *) data;
	int head = local->io_debug_head;
	int start_bytes, left, copy, copied;

	if (off + count > PRISM2_IO_DEBUG_SIZE * 4) {
		*eof = 1;
		if (off >= PRISM2_IO_DEBUG_SIZE * 4)
			return 0;
		count = PRISM2_IO_DEBUG_SIZE * 4 - off;
	}

	copied = 0;
	start_bytes = (PRISM2_IO_DEBUG_SIZE - head) * 4;
	left = count;

	if (off < start_bytes) {
		copy = start_bytes - off;
		if (copy > count)
			copy = count;
		memcpy(page, ((u8 *) &local->io_debug[head]) + off, copy);
		left -= copy;
		if (left > 0)
			memcpy(&page[copy], local->io_debug, left);
	} else {
		memcpy(page, ((u8 *) local->io_debug) + (off - start_bytes),
		       left);
	}

	*start = page;

	return count;
}
#endif /* PRISM2_IO_DEBUG */


#ifndef PRISM2_NO_STATION_MODES
static int prism2_scan_results_proc_read(char *page, char **start, off_t off,
					 int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	int entry, i, len, total = 0;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);
	p += sprintf(p, "Total ScanResults: %d\n",
		     local->last_scan_results_count);
	for (entry = 0; entry < local->last_scan_results_count; entry++) {
		struct hfa384x_scan_result *scanres =
			&local->last_scan_results[entry];

		if (total + (p - page) <= off) {
			total += p - page;
			p = page;
		}
		if (total + (p - page) > off + count)
			break;
		if ((p - page) > (PAGE_SIZE - 200))
			break;

		p += sprintf(p, "ScanResult %d:\n"
			     "  CHID=%d ANL=%d SL=%d BcnInt=%d Capab=0x%02x "
			     "Rate=%d\n"
			     "  bssid=" MACSTR " SSID='",
			     entry,
			     le16_to_cpu(scanres->chid),
			     le16_to_cpu(scanres->anl),
			     le16_to_cpu(scanres->sl),
			     le16_to_cpu(scanres->beacon_interval),
			     le16_to_cpu(scanres->capability),
			     le16_to_cpu(scanres->rate),
			     MAC2STR(scanres->bssid));
		len = le16_to_cpu(scanres->ssid_len);
		if (len > 32)
			len = 32;
		for (i = 0; i < len; i++) {
			unsigned char c = scanres->ssid[i];
			if (c >= 32 && c < 127)
				p += sprintf(p, "%c", c);
			else
				p += sprintf(p, "<%02x>", c);
		}
		p += sprintf(p, "'\n"
			     "  SupRates=");
		for (i = 0; i < sizeof(scanres->sup_rates); i++) {
			if (scanres->sup_rates[i] == 0)
				break;
			p += sprintf(p, "%02x ", scanres->sup_rates[i]);
		}
		p += sprintf(p, "\n");
	}
	spin_unlock_irqrestore(&local->lock, flags);

	total += (p - page);
	if (total >= off + count)
		*eof = 1;

	if (total < off) {
		*eof = 1;
		return 0;
	}

	len = total - off;
	if (len > (p - page))
		len = p - page;
	*start = p - len;
	if (len > count)
		len = count;

	return len;
}
#endif /* PRISM2_NO_STATION_MODES */


void hostap_init_proc(local_info_t *local)
{
	local->proc = NULL;

	if (hostap_proc == NULL) {
		printk(KERN_WARNING "%s: hostap proc directory not created\n",
		       local->dev->name);
		return;
	}

	local->proc = proc_mkdir(local->dev->name, hostap_proc);
	if (local->proc == NULL) {
		printk(KERN_INFO "/proc/net/hostap/%s creation failed\n",
		       local->dev->name);
		return;
	}

#ifndef PRISM2_NO_PROCFS_DEBUG
	create_proc_read_entry("debug", 0, local->proc,
			       prism2_debug_proc_read, local);
#endif /* PRISM2_NO_PROCFS_DEBUG */
	create_proc_read_entry("stats", 0, local->proc,
			       prism2_stats_proc_read, local);
	create_proc_read_entry("wds", 0, local->proc,
			       prism2_wds_proc_read, local);
	create_proc_read_entry("pda", 0, local->proc,
			       prism2_pda_proc_read, local);
#ifdef PRISM2_IO_DEBUG
	create_proc_read_entry("io_debug", 0, local->proc,
			       prism2_io_debug_proc_read, local);
#endif /* PRISM2_IO_DEBUG */
#ifndef PRISM2_NO_STATION_MODES
	create_proc_read_entry("scan_results", 0, local->proc,
			       prism2_scan_results_proc_read, local);
#endif /* PRISM2_NO_STATION_MODES */
}


void hostap_remove_proc(local_info_t *local)
{
	if (local->proc != NULL) {
#ifndef PRISM2_NO_STATION_MODES
		remove_proc_entry("scan_results", local->proc);
#endif /* PRISM2_NO_STATION_MODES */
#ifdef PRISM2_IO_DEBUG
		remove_proc_entry("io_debug", local->proc);
#endif /* PRISM2_IO_DEBUG */
		remove_proc_entry("pda", local->proc);
		remove_proc_entry("wds", local->proc);
		remove_proc_entry("stats", local->proc);
#ifndef PRISM2_NO_PROCFS_DEBUG
		remove_proc_entry("debug", local->proc);
#endif /* PRISM2_NO_PROCFS_DEBUG */
		if (local->dev != NULL && local->dev->name != NULL &&
		    hostap_proc != NULL)
			remove_proc_entry(local->dev->name, hostap_proc);
	}
}


EXPORT_SYMBOL(hostap_init_proc);
EXPORT_SYMBOL(hostap_remove_proc);
