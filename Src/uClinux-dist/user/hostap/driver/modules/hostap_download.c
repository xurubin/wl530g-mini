static int prism2_enable_aux_port(struct net_device *dev, int enable)
{
	u16 val, reg;
	int i, tries;
	unsigned long flags;
	local_info_t *local = (local_info_t *) dev->priv;

	spin_lock_irqsave(&local->cmdlock, flags);

	/* wait until busy bit is clear */
	tries = HFA384X_CMD_BUSY_TIMEOUT;
	while (HFA384X_INW(HFA384X_CMD_OFF) & HFA384X_CMD_BUSY && tries > 0) {
		tries--;
		udelay(1);
	}
	if (tries == 0) {
		reg = HFA384X_INW(HFA384X_CMD_OFF);
		spin_unlock_irqrestore(&local->cmdlock, flags);
		printk("%s: prism2_enable_aux_port - timeout - reg=0x%04x\n",
		       dev->name, reg);
		return -ETIMEDOUT;
	}

	val = HFA384X_INW(HFA384X_CONTROL_OFF);

	if (enable) {
		HFA384X_OUTW(HFA384X_AUX_MAGIC0, HFA384X_PARAM0_OFF);
		HFA384X_OUTW(HFA384X_AUX_MAGIC1, HFA384X_PARAM1_OFF);
		HFA384X_OUTW(HFA384X_AUX_MAGIC2, HFA384X_PARAM2_OFF);

		if ((val & HFA384X_AUX_PORT_MASK) != HFA384X_AUX_PORT_DISABLED)
			printk("prism2_enable_aux_port: was not disabled!?\n");
		val &= ~HFA384X_AUX_PORT_MASK;
		val |= HFA384X_AUX_PORT_ENABLE;
	} else {
		HFA384X_OUTW(0, HFA384X_PARAM0_OFF);
		HFA384X_OUTW(0, HFA384X_PARAM1_OFF);
		HFA384X_OUTW(0, HFA384X_PARAM2_OFF);

		if ((val & HFA384X_AUX_PORT_MASK) != HFA384X_AUX_PORT_ENABLED)
			printk("prism2_enable_aux_port: was not enabled!?\n");
		val &= ~HFA384X_AUX_PORT_MASK;
		val |= HFA384X_AUX_PORT_DISABLE;
	}
	HFA384X_OUTW(val, HFA384X_CONTROL_OFF);

	udelay(5);

	i = 10000;
	while (i > 0) {
		val = HFA384X_INW(HFA384X_CONTROL_OFF);
		val &= HFA384X_AUX_PORT_MASK;

		if ((enable && val == HFA384X_AUX_PORT_ENABLED) ||
		    (!enable && val == HFA384X_AUX_PORT_DISABLED))
			break;

		udelay(10);
		i--;
	}

	spin_unlock_irqrestore(&local->cmdlock, flags);

	if (i == 0) {
		printk("prism2_enable_aux_port(%d) timed out\n",
		       enable);
		return -ETIMEDOUT;
	}

	return 0;
}


static int hfa384x_from_aux(struct net_device *dev, unsigned int addr, int len,
			    void *buf)
{
	u16 page, offset;
	if (addr & 1 || len & 1)
		return -1;

	page = addr >> 7;
	offset = addr & 0x7f;

	HFA384X_OUTW(page, HFA384X_AUXPAGE_OFF);
	HFA384X_OUTW(offset, HFA384X_AUXOFFSET_OFF);

	udelay(5);

#ifdef PRISM2_PCI
	{
		u16 *pos = (u16 *) buf;
		while (len > 0) {
			*pos++ = HFA384X_INW_DATA(HFA384X_AUXDATA_OFF);
			len -= 2;
		}
	}
#else /* PRISM2_PCI */
	HFA384X_INSW(HFA384X_AUXDATA_OFF, buf, len / 2);
#endif /* PRISM2_PCI */

	return 0;
}


static int hfa384x_to_aux(struct net_device *dev, unsigned int addr, int len,
			  void *buf)
{
	u16 page, offset;
	if (addr & 1 || len & 1)
		return -1;

	page = addr >> 7;
	offset = addr & 0x7f;

	HFA384X_OUTW(page, HFA384X_AUXPAGE_OFF);
	HFA384X_OUTW(offset, HFA384X_AUXOFFSET_OFF);

	udelay(5);

#ifdef PRISM2_PCI
	{
		u16 *pos = (u16 *) buf;
		while (len > 0) {
			HFA384X_OUTW_DATA(*pos++, HFA384X_AUXDATA_OFF);
			len -= 2;
		}
	}
#else /* PRISM2_PCI */
	HFA384X_OUTSW(HFA384X_AUXDATA_OFF, buf, len / 2);
#endif /* PRISM2_PCI */

	return 0;
}


static int prism2_pda_ok(u8 *buf)
{
	u16 *pda = (u16 *) buf;
	int pos;
	u16 len, pdr;

	if (buf[0] == 0xff && buf[1] == 0x00 && buf[2] == 0xff &&
	    buf[3] == 0x00)
		return 0;

	pos = 0;
	while (pos + 1 < PRISM2_PDA_SIZE / 2) {
		len = le16_to_cpu(pda[pos]);
		pdr = le16_to_cpu(pda[pos + 1]);
		if (len == 0 || pos + len > PRISM2_PDA_SIZE / 2)
			return 0;

		if (pdr == 0x0000 && len == 2) {
			/* PDA end found */
			return 1;
		}

		pos += len + 1;
	}

	return 0;
}


static u8 * prism2_read_pda(struct net_device *dev)
{
	u8 *buf;
	int res, i, found = 0;
#define NUM_PDA_ADDRS 3
	unsigned int pda_addr[NUM_PDA_ADDRS] = {
		0x7f0000 /* others than HFA3841 */,
		0x3f0000 /* HFA3841 */,
		0x390000 /* apparently used in older cards */
	};

	buf = (u8 *) kmalloc(PRISM2_PDA_SIZE, GFP_KERNEL);
	if (buf == NULL)
		return NULL;

	/* Note: wlan card should be in initial state (just after init cmd)
	 * and no other operations should be performed concurrently. */

	prism2_enable_aux_port(dev, 1);

	for (i = 0; i < NUM_PDA_ADDRS; i++) {
		printk(KERN_DEBUG "%s: trying to read PDA from 0x%08x",
		       dev->name, pda_addr[i]);
		res = hfa384x_from_aux(dev, pda_addr[i], PRISM2_PDA_SIZE, buf);
		if (res)
			continue;
		if (res == 0 && prism2_pda_ok(buf)) {
			printk(": OK\n");
			found = 1;
			break;
		} else {
			printk(": failed\n");
		}
	}

	prism2_enable_aux_port(dev, 0);

	if (!found) {
		kfree(buf);
		buf = NULL;
	}

	return buf;
}


static int prism2_download_volatile(local_info_t *local,
				    struct prism2_download_param *param,
				    u8 **copied_data)
{
	struct net_device *dev = local->dev;
	int ret = 0, i;
	u16 param0, param1;

	if (local->hw_downloading) {
		printk(KERN_WARNING "%s: Already downloading - aborting new "
		       "request\n", dev->name);
		return -1;
	}

	local->hw_downloading = 1;
	prism2_hw_shutdown(dev, 0);

	if (prism2_hw_init(dev, 0)) {
		printk(KERN_WARNING "%s: Could not initialize card for "
		       "download\n", dev->name);
		ret = -1;
		goto out;
	}

	if (prism2_enable_aux_port(dev, 1)) {
		printk(KERN_WARNING "%s: Could not enable AUX port\n",
		       dev->name);
		ret = -1;
		goto out;
	}

	param0 = param->start_addr & 0xffff;
	param1 = param->start_addr >> 16;

	HFA384X_OUTW(0, HFA384X_PARAM2_OFF);
	HFA384X_OUTW(param1, HFA384X_PARAM1_OFF);
	if (hfa384x_cmd_wait(dev, HFA384X_CMDCODE_DOWNLOAD |
			     (HFA384X_PROGMODE_ENABLE_VOLATILE << 8),
			     param0)) {
		printk(KERN_WARNING "%s: Download command execution failed\n",
		       dev->name);
		ret = -1;
		goto out;
	}

	for (i = 0; i < param->num_areas; i++) {
		printk(KERN_DEBUG "%s: Writing %d bytes at 0x%08x\n",
		       dev->name, param->data[i].len, param->data[i].addr);
		if (hfa384x_to_aux(dev, param->data[i].addr,
				   param->data[i].len, copied_data[i])) {
			printk(KERN_WARNING "%s: RAM download at 0x%08x "
			       "(len=%d) failed\n", dev->name,
			       param->data[i].addr, param->data[i].len);
			ret = -1;
			goto out;
		}
	}

	HFA384X_OUTW(param1, HFA384X_PARAM1_OFF);
	HFA384X_OUTW(0, HFA384X_PARAM2_OFF);
	if (hfa384x_cmd_no_wait(dev, HFA384X_CMDCODE_DOWNLOAD |
				(HFA384X_PROGMODE_DISABLE << 8), param0)) {
		printk(KERN_WARNING "%s: Download command execution failed\n",
		       dev->name);
		ret = -1;
		goto out;
	}
	/* ProgMode disable causes the hardware to restart itself from the
	 * given starting address. Give hw some time and ACK command just in
	 * case restart did not happen. */
	mdelay(5);
	HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_EVACK_OFF);

	if (prism2_enable_aux_port(dev, 0)) {
		printk(KERN_DEBUG "%s: Disabling AUX port failed\n",
		       dev->name);
		/* continue anyway.. restart should have taken care of this */
	}

	mdelay(5);
	local->hw_downloading = 0;
	if (prism2_hw_config(dev, 2)) {
		printk(KERN_WARNING "%s: Card configuration after RAM "
		       "download failed\n", dev->name);
		ret = -1;
		goto out2;
	}

	goto out2;
 out:
	local->hw_downloading = 0;
 out2:
	return ret;
}


#ifdef PRISM2_NON_VOLATILE_DOWNLOAD
/* Note! Non-volatile downloading functionality has not yet been tested
 * thoroughly and it may corrupt flash image and effectively kill the card that
 * is being updated. You have been warned. */

static inline int prism2_download_block(struct net_device *dev,
					struct prism2_download_param *param,
					u8 **copied_data, u32 bufaddr,
					int idx, int rest_len, int data_off)
{
	u16 param0, param1;
	int block_len;

	block_len = rest_len < 4096 ? rest_len : 4096;

	param0 = (param->data[idx].addr + data_off) & 0xffff;
	param1 = (param->data[idx].addr + data_off) >> 16;

	HFA384X_OUTW(block_len, HFA384X_PARAM2_OFF);
	HFA384X_OUTW(param1, HFA384X_PARAM1_OFF);

	if (hfa384x_cmd_wait(dev, HFA384X_CMDCODE_DOWNLOAD |
			     (HFA384X_PROGMODE_ENABLE_NON_VOLATILE << 8),
			     param0)) {
		printk(KERN_WARNING "%s: Flash download command execution "
		       "failed\n", dev->name);
		return -1;
	}

	if (hfa384x_to_aux(dev, bufaddr,
			   block_len, copied_data[idx] + data_off)) {
		printk(KERN_WARNING "%s: flash download at 0x%08x "
		       "(len=%d) failed\n", dev->name,
		       param->data[idx].addr, param->data[idx].len);
		return -1;
	}

	HFA384X_OUTW(0, HFA384X_PARAM2_OFF);
	HFA384X_OUTW(0, HFA384X_PARAM1_OFF);
	if (hfa384x_cmd_wait(dev, HFA384X_CMDCODE_DOWNLOAD |
			     (HFA384X_PROGMODE_PROGRAM_NON_VOLATILE << 8),
			     0)) {
		printk(KERN_WARNING "%s: Flash write command execution "
		       "failed\n", dev->name);
		return -1;
	}

	return block_len;
}


static int prism2_download_nonvolatile(local_info_t *local,
				       struct prism2_download_param *param,
				       u8 **copied_data)
{
	struct net_device *dev = local->dev;
	int ret = 0, i;
	struct {
		u16 page;
		u16 offset;
		u16 len;
	} dlbuffer;
	u32 bufaddr;

	if (local->hw_downloading) {
		printk(KERN_WARNING "%s: Already downloading - aborting new "
		       "request\n", dev->name);
		return -1;
	}

	local->hw_downloading = 1;

	ret = local->func->get_rid(dev, HFA384X_RID_DOWNLOADBUFFER,
				   &dlbuffer, 6, 0);

	if (ret < 0) {
		printk(KERN_WARNING "%s: Could not read download buffer "
		       "parameters\n", dev->name);
		goto out;
	}

	dlbuffer.page = le16_to_cpu(dlbuffer.page);
	dlbuffer.offset = le16_to_cpu(dlbuffer.offset);
	dlbuffer.len = le16_to_cpu(dlbuffer.len);

	printk(KERN_DEBUG "Download buffer: %d bytes at 0x%04x:0x%04x\n",
	       dlbuffer.len, dlbuffer.page, dlbuffer.offset);

	bufaddr = (dlbuffer.page << 7) + dlbuffer.offset;

	prism2_hw_shutdown(dev, 0);

	if (prism2_hw_init(dev, 0)) {
		printk(KERN_WARNING "%s: Could not initialize card for "
		       "download\n", dev->name);
		ret = -1;
		goto out;
	}

	hfa384x_disable_interrupts(dev);

	if (prism2_enable_aux_port(dev, 1)) {
		printk(KERN_WARNING "%s: Could not enable AUX port\n",
		       dev->name);
		ret = -1;
		goto out;
	}

	printk(KERN_DEBUG "%s: starting flash download\n", dev->name);
	for (i = 0; i < param->num_areas; i++) {
		int rest_len = param->data[i].len;
		int data_off = 0;

		while (rest_len > 0) {
			int block_len;

			block_len = prism2_download_block(
				dev, param, copied_data, bufaddr,
				i, rest_len, data_off);

			if (block_len < 0) {
				ret = -1;
				goto out;
			}

			rest_len -= block_len;
			data_off += block_len;
		}
	}

	HFA384X_OUTW(0, HFA384X_PARAM1_OFF);
	HFA384X_OUTW(0, HFA384X_PARAM2_OFF);
	if (hfa384x_cmd_wait(dev, HFA384X_CMDCODE_DOWNLOAD |
				(HFA384X_PROGMODE_DISABLE << 8), 0)) {
		printk(KERN_WARNING "%s: Download command execution failed\n",
		       dev->name);
		ret = -1;
		goto out;
	}

	if (prism2_enable_aux_port(dev, 0)) {
		printk(KERN_DEBUG "%s: Disabling AUX port failed\n",
		       dev->name);
		/* continue anyway.. restart should have taken care of this */
	}

	mdelay(5);

	local->func->hw_reset(dev);
	local->hw_downloading = 0;
	if (prism2_hw_config(dev, 2)) {
		printk(KERN_WARNING "%s: Card configuration after flash "
		       "download failed\n", dev->name);
		ret = -1;
	} else {
		printk(KERN_INFO "%s: Card initialized successfully after "
		       "flash download\n", dev->name);
	}

 out:
	local->hw_downloading = 0;
	return ret;
}
#endif /* PRISM2_NON_VOLATILE_DOWNLOAD */



static int prism2_download(local_info_t *local,
			   struct prism2_download_param *param)
{
	int ret = 0;
	int i;
	u32 total_len = 0;
	u8 **copied_data;

	printk(KERN_DEBUG "prism2_download: dl_cmd=%d start_addr=0x%08x "
	       "num_areas=%d\n",
	       param->dl_cmd, param->start_addr, param->num_areas);

	copied_data = (u8 **) kmalloc(param->num_areas * sizeof(u8 *),
				      GFP_KERNEL);
	if (copied_data == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	memset(copied_data, 0, param->num_areas * sizeof(u8 *));

	for (i = 0; i < param->num_areas; i++) {
		printk(KERN_DEBUG "  area %d: addr=0x%08x len=%d ptr=0x%p\n",
		       i, param->data[i].addr, param->data[i].len,
		       param->data[i].ptr);
		total_len += param->data[i].len;
		if (param->data[i].len > PRISM2_MAX_DOWNLOAD_AREA_LEN ||
		    total_len > PRISM2_MAX_DOWNLOAD_LEN) {
			ret = -E2BIG;
			goto out;
		}

		copied_data[i] = (u8 *)
			kmalloc(param->data[i].len, GFP_KERNEL);
		if (copied_data[i] == NULL) {
			ret = -ENOMEM;
			goto out;
		}

		if (copy_from_user(copied_data[i], param->data[i].ptr,
				   param->data[i].len)) {
			ret = -EFAULT;
			goto out;
		}
	}

	switch (param->dl_cmd) {
	case PRISM2_DOWNLOAD_VOLATILE:
		ret = prism2_download_volatile(local, param, copied_data);
		break;
	case PRISM2_DOWNLOAD_NON_VOLATILE:
#ifdef PRISM2_NON_VOLATILE_DOWNLOAD
		ret = prism2_download_nonvolatile(local, param, copied_data);
#else /* PRISM2_NON_VOLATILE_DOWNLOAD */
		printk(KERN_INFO "%s: non-volatile downloading not enabled\n",
		       local->dev->name);
		ret = -EOPNOTSUPP;
#endif /* PRISM2_NON_VOLATILE_DOWNLOAD */
		break;
	default:
		printk(KERN_DEBUG "%s: unsupported download command %d\n",
		       local->dev->name, param->dl_cmd);
		ret = -EINVAL;
		break;
	};

 out:
	if (copied_data) {
		for (i = 0; i < param->num_areas; i++)
			if (copied_data[i] != NULL)
				kfree(copied_data[i]);
		kfree(copied_data);
	}
	return ret;
}
