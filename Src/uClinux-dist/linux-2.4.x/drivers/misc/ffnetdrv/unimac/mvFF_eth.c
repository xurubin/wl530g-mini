/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* 
* FILENAME:    $Workfile: mvFF_eth.c $ 
* REVISION:    $Revision: 4 $
* LAST UPDATE: $Modtime: 3/03/03 12:36p $ 
* 
* mvFF_eth.c
*
* DESCRIPTION:
*		Ethernet driver for FireFox/Libertas
*
* DEPENDENCIES:   
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#ifndef CONFIG_ARCH_FIREFOX
#ifndef CONFIG_ARCH_LIBERTAS
#error  No Device marcro definition.
#endif
#endif


/*
 * General note - Driver assumes that booter has resetted the ethernet interface
 * and it's fully functional.
 * The reset code can (and probably better) be added to this driver.
 */

#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ip.h>
#include <linux/mii.h>

#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/pgtable.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>

#include <asm/arch/hardware.h>
#include <asm/arch/irqs.h>

#include "mv_unimac.h"
#include "mvFF_eth.h"
#include "mvFF_addrtbl.h"
#include "mv_os.h"
#include "mvFF_netconf.h"

/***************************************************/
/* Definitions                                     */
/***************************************************/

#define BIT(n) (1<<(n))

#if !defined(HEADERS) && !defined(TRAILERS)
#error "You must define headers or trailers!!!"
#endif

#if defined(HEADERS) && defined(TRAILERS)
#error "You must define headers or trailers not both !!!"
#endif

#define SW_COUNTER	1

/***************************************************/
/* Gloabl vars                                     */
/***************************************************/

/* The _binding structs represents per vlan binding
 * the binding is between the ethernet device (struct net_device)
 * and its fields: HW access, link state, dev state and trailer/header
 */ 
typedef struct _binding {
  struct net_device *dev;       /* the device associated with this binding entry */
  mv_eth_hw         *pmvEthHw;  /* the global HW HAL */
  bool               boLinkUp;  /* link state */
  bool               boOpened;  /* device has allready been opened by the stack (hasn't been closed yet) */
#ifdef HEADERS
  char               header[HEADER_SIZE];
#elif defined (TRAILERS)
  char               trailer[TRAILER_SIZE];
#endif
} BINDING, *PBINDING;

/* the irq number assigned to our MAC HW by th kernel */
static u32  irq;       /* irq of our shared MAC HW */

/*
 * The static global array of bindings.
 * each vlan has a binding that represents it
 * the index in the array is the binding for it
 * we are allocating the number of needed bindings in 
 * mvFF_eth_start
 */ 
static BINDING * mvBindings[MV_UNM_VID_ISOLATED]= {0};

/* a global spin lock to sync transmits */
spinlock_t lock;

/* the sole HW MAC presentation is SW */
static mv_eth_hw   mvEthHw;

/* In some implemantation we use the broadcast MAC to update HW tables */
static unsigned char brdcast_mac[GT_ETHERNET_HEADER_SIZE] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; 

/* the receive handler is not a part of the driver interface,
 * rather, its a private function that signals packets to the kernel
 * via the netif_rx function 
 */
static void mvFF_eth_receive_queue(unsigned long data);
static void mvFF_eth_free_tx_queue(unsigned long queue);
DECLARE_TASKLET(rxComplete, mvFF_eth_receive_queue, Q_INDEX_LIMIT);
DECLARE_TASKLET(txComplete, mvFF_eth_free_tx_queue, MVFF_TX_QUEUE);

extern void qdStatus(void);

/*
 * ----------------------------------------------------------------------------
 * Set the MAC address for the specified interface
 * to the specified value, forsaking all others.
 *
 * Input : pointer to ethernet interface network device structure and
 *         a pointer to the designated entry to be added to the cache.
 * Output : zero upon success, negative upon failure
 */
s32
mvFF_eth_set_mac_address(struct net_device *dev, void *addr)
{
  u32 i;
  struct sockaddr *sock;
  u8 port = ((gt_eth_priv *) (dev->priv))->port;
  
  sock = (struct sockaddr *) addr;
  for (i = 0; i < 6; i++) {
    dev->dev_addr[i] = sock->sa_data[i];
  }
#ifdef ETH_DBG_INFO
  printk("Setting MAC address for dev [%d]\n",((gt_eth_priv *) (dev->priv))->vid);
#endif

  /* addressTableClear(port); */	/* Does flush */
  mvFF_eth_update_mac_address(dev->dev_addr, port );
  return (0);
}

/*****************************************************************************
*
* int etherReadMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int* value)
*
* Description
* This function will access the MII registers and will read the value of
* the MII register , and will retrieve the value in the pointer.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* Outputs
* value - pointer to unsigned int which will receive the value.
* Returns Value
* 1 if success.
* 0 if fail to make the assignment.
* Error types (and exceptions if exist)
*/
static int etherReadMIIReg(unsigned int portNum, unsigned int miiReg,
			   unsigned int *value);

/*****************************************************************************
* 
* int etherWriteMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int value)
*
* Description
* This function will access the MII registers and will write the value
* to the MII register.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* value -the value that will be written.
* Outputs
* Returns Value
* 1 if success.
* 0 if fail to make the assignment.
* Error types (and exceptions if exist)
*/
static int etherWriteMIIReg(unsigned int portNum, unsigned int miiReg,
			    unsigned int value);


/* set port config value */
inline void set_port_config(u32 value, unsigned int port) {
#ifdef CONFIG_ARCH_FIREFOX
  firefox_reg_write(FIREFOX_ENET_E0PCR + ETHERNET_PORTS_DIFFERENCE_OFFSETS * port, value);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  libertas_reg_write(LIBERTAS_ENET_E0PCR + ETHERNET_PORTS_DIFFERENCE_OFFSETS * port, value);
#endif
}

/* get port config value */
inline u32 get_port_config(unsigned int port) {
#ifdef CONFIG_ARCH_FIREFOX
  return firefox_reg_read(FIREFOX_ENET_E0PCR + ETHERNET_PORTS_DIFFERENCE_OFFSETS * port);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  return libertas_reg_read(LIBERTAS_ENET_E0PCR + ETHERNET_PORTS_DIFFERENCE_OFFSETS * port);
#endif
}

/*
 * Set Promiscuous mode to normal mode.
 * in order to enable Filtering.     XXX this should probably go away
 */
inline void enableFiltering(u32 port)
{
  ETHERNET_PCR portControlReg;
  
  portControlReg = get_port_config(port);
  portControlReg &= ~(1 << PROMISCUOUS_MODE);
  set_port_config(portControlReg, port);
}

/* MC suppport */
static inline void disableMulticastFiltering(u32 port)
{
  ETHERNET_PCR portControlReg;
  portControlReg = get_port_config(port);
  portControlReg |= (0x8);
  set_port_config(portControlReg, port);
}

/*
 * This function will set the Promiscuous mode to Promiscuous mode.
 * in order to disable Filtering.    XXX this should probably go away
 */
static inline void disableFiltering(u32 port)
{
  ETHERNET_PCR portControlReg;
  portControlReg = get_port_config(port);
#ifdef ENABLE_MC
  portControlReg |= (9);
#else
  portControlReg |= (1 << PROMISCUOUS_MODE);
#endif
  set_port_config(portControlReg, port);
}

/*
 * The TX and RX descriptors are being placed in a non-chaced area.
 * This is to reduce HW-SW synchronization
 * The unchached area is handled here 
 * 
 * 40KB are being mapped as unchaced in the MPU,
 * each page in the kernel is 4KB.
 * the management of the 10 pages pool is being done here
 */
u32 uncachedPages(u32 pages)
{
  /* allocate memory from the uncached 40KB we defined in the MPU */
  static unsigned int last = (DRAM_BASE+DRAM_SIZE), ret;
  ret = last;
  last += pages * 0x1000;
  if(last >= (DRAM_BASE+DRAM_SIZE + (0x1000*10) ) ) {
    last -=  pages * 0x1000;
    ASSERT(0);
    return (u32)NULL;
  }
#ifdef ETH_DBG_INFO
  printk ("Allocated %d pages in address %08x\n",pages, ret); 
#endif
  return ret;  
}

/* Sometimes it seems the phy and the GT don't always agree. 
 * Make sure they do, or restart autoneg. */
static void
check_phy_state(struct net_device *dev)
{
  gt_eth_priv *private = (gt_eth_priv *) dev->priv;
  struct mii_if_info *mii = &private->mii_if;
  int bmsr = mii->mdio_read(mii->dev, mii->phy_id, MII_BMSR);

  if (mii_link_ok(&private->mii_if) && (bmsr & BMSR_ANEGCOMPLETE)) {
    int advert = mii->mdio_read(dev, mii->phy_id, MII_ADVERTISE);
    int lpa = mii->mdio_read(dev, mii->phy_id, MII_LPA);
    int nego = mii_nway_result(advert & lpa);
    int psr, wanted = -1;
    
    switch (nego) {
    case LPA_100FULL:
      wanted = 0x3;
      break;
    case LPA_100HALF:
      wanted = 0x1;
      break;
    case LPA_10FULL:
      wanted = 0x2;
      break;
    case LPA_10HALF:
      wanted = 0x0;
      break;
    default:
#ifdef ETH_DBG_INFO
      printk("%s: MII negotiated strange settings %d\n",dev->name, nego);
#endif
      break;
    }
#ifdef CONFIG_ARCH_FIREFOX
    psr = firefox_reg_read(FIREFOX_ENET_E0PSR + (ETH_ADDR_GAP * private->port));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    psr = libertas_reg_read(LIBERTAS_ENET_E0PSR + (ETH_ADDR_GAP * private->port));
#endif

    if ((psr & 0x3) != wanted) {
#ifdef ETH_DBG_ERROR
      printk("%s: MII said %x, GT said %x, restarting autoneg\n", dev->name, wanted, psr);
#endif
      mii_nway_restart(mii);
    }
  }
}

/*
 * dump_link_state - prints the link state
 * it informs: dev->name, speed, link, duplex, flow-control
 */
#ifdef ETH_DBG_INFO
static void dump_link_state(struct net_device *dev)
{
  gt_eth_priv *private = (gt_eth_priv *) dev->priv;
  unsigned int psr;

#ifdef CONFIG_ARCH_FIREFOX
  psr = firefox_reg_read(FIREFOX_ENET_E0PSR + (ETH_ADDR_GAP * private->port));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  psr = libertas_reg_read(LIBERTAS_ENET_E0PSR + (ETH_ADDR_GAP * private->port));
#endif
  printk("%s: link state:\n"
	 "  GT:%s:%s:%s:%s\n",
	 dev->name,
	 psr & 1 ? "100" : " 10",
	 psr & 8 ? " Link" : "nLink",
	 psr & 2 ? "FD" : "HD", psr & 4 ? " FC" : "nFC");

}
#endif /* ETH_DBG_INFO */

/*
 * ----------------------------------------------------------------------------
 *  Update the statistics structure in the private data structure
 *
 *  Input : pointer to ethernet interface network device structure
 *  Output : N/A
 */
void
mvFF_eth_update_stat(struct net_device *dev)
{
    gt_eth_priv             *private;
    struct net_device_stats *stat;
    u32                     base;
    static int              first_time = 1;

    private =   dev->priv;
    stat    = &(private->stat);
#ifdef CONFIG_ARCH_FIREFOX
    base    =   FIREFOX_ENET_0_MIB_CTRS;
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    base    =   LIBERTAS_ENET_0_MIB_CTRS;
#endif

    if (first_time) {
           stat->rx_bytes = 0;
           stat->tx_bytes = 0;
           stat->rx_packets = 0;
           stat->tx_packets = 0;
           stat->rx_errors = 0;
           stat->rx_dropped = 0;
           stat->multicast = 0;
           stat->collisions = 0;
           stat->rx_length_errors = 0;
           stat->rx_length_errors = 0;
           stat->rx_crc_errors = 0;

#ifndef SW_COUNTER
	   /* Clear chips MIB counters */
#ifdef CONFIG_ARCH_FIREFOX
           firefox_reg_read( base + 0x00 );
           firefox_reg_read( base + 0x04 );
           firefox_reg_read( base + 0x08 );
           firefox_reg_read( base + 0x0c );
           firefox_reg_read( base + 0x50 );
           firefox_reg_read( base + 0x20 );
           firefox_reg_read( base + 0x1c );
           firefox_reg_read( base + 0x30 );
           firefox_reg_read( base + 0x60 );
           firefox_reg_read( base + 0x24 );
           firefox_reg_read( base + 0x20 );
#endif
#ifdef CONFIG_ARCH_LIBERTAS
           libertas_reg_read( base + 0x00 );
           libertas_reg_read( base + 0x04 );
           libertas_reg_read( base + 0x08 );
           libertas_reg_read( base + 0x0c );
           libertas_reg_read( base + 0x50 );
           libertas_reg_read( base + 0x20 );
           libertas_reg_read( base + 0x1c );
           libertas_reg_read( base + 0x30 );
           libertas_reg_read( base + 0x60 );
           libertas_reg_read( base + 0x24 );
           libertas_reg_read( base + 0x20 );
#endif
#endif /* SW_COUNTER */

           first_time = 0;
    }

#ifndef SW_COUNTER
#ifdef CONFIG_ARCH_FIREFOX
    stat->rx_bytes         += firefox_reg_read( base + 0x00 );
    stat->tx_bytes         += firefox_reg_read( base + 0x04 );
    stat->rx_packets       += firefox_reg_read( base + 0x08 );
    stat->tx_packets       += firefox_reg_read( base + 0x0c );
    stat->rx_errors        += firefox_reg_read( base + 0x50 );
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    stat->rx_bytes         += libertas_reg_read( base + 0x00 );
    stat->tx_bytes         += libertas_reg_read( base + 0x04 );
    stat->rx_packets       += libertas_reg_read( base + 0x08 );
    stat->tx_packets       += libertas_reg_read( base + 0x0c );
    stat->rx_errors        += libertas_reg_read( base + 0x50 );
#endif

    /*
     * Rx dropped is for received packet with CRC error
     */
#ifdef CONFIG_ARCH_FIREFOX
    stat->rx_dropped       += firefox_reg_read( base + 0x20 );
    stat->multicast        += firefox_reg_read( base + 0x1c );
    stat->collisions       += firefox_reg_read( base + 0x30 );
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    stat->rx_dropped       += libertas_reg_read( base + 0x20 );
    stat->multicast        += libertas_reg_read( base + 0x1c );
    stat->collisions       += libertas_reg_read( base + 0x30 );
#endif

    /*
     * detailed rx errors
     */
#ifdef CONFIG_ARCH_FIREFOX
    stat->rx_length_errors += firefox_reg_read( base + 0x60 );
    stat->rx_length_errors += firefox_reg_read( base + 0x24 );
    stat->rx_crc_errors    += firefox_reg_read( base + 0x20 );
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    stat->rx_length_errors += libertas_reg_read( base + 0x60 );
    stat->rx_length_errors += libertas_reg_read( base + 0x24 );
    stat->rx_crc_errors    += libertas_reg_read( base + 0x20 );
#endif
#endif /* SW_COUNTER */
}

/*
 * ----------------------------------------------------------------------------
 * Returns a pointer to the interface statistics.
 *
 * Input : dev - a pointer to the required interface
 *
 * Output : a pointer to the interface's statistics
 */
struct net_device_stats *
mvFF_eth_get_stats(struct net_device *dev)
{
  gt_eth_priv *private;
  
  mvFF_eth_update_stat(dev);
  private = dev->priv;
  return (&(private->stat));
}

/*
 * ----------------------------------------------------------------------------
 *  change rx mode
 *
 *  Input : pointer to ethernet interface network device structure
 *  Output : N/A
 */
void
mvFF_eth_set_rx_mode(struct net_device *dev)
{
  gt_eth_priv *private;
  int i;
  struct dev_mc_list *mcList = dev->mc_list;
  char* addr;
    
  private = dev->priv;
	    
  if (dev->flags & IFF_PROMISC) 
  {
    disableFiltering(private->port);
#ifdef ETH_DBG_MC
    printk ("Promiscuous mode is on.\n");
#endif
  } 
  else if (dev->flags & IFF_ALLMULTI)
  {
    disableMulticastFiltering(private->port);
#ifdef ETH_DBG_MC
    printk ("All Multicast mode is on.\n");
#endif
  }
  else 
  {
#ifdef ENABLE_MC
    disableFiltering(private->port);
#ifdef ETH_DBG_MC
    printk ("Multicast Addresses (%i)\n", dev->mc_count);
#endif
    for(i=0; i<dev->mc_count; i++)
    {
	addr = mcList->dmi_addr;
#ifdef ETH_DBG_MC
	printk("%0#x-%0#x-%0#x-%0#x-%0#x-%0#x\n",addr[0],addr[1],
                            addr[2],addr[3],addr[4],addr[5]);
#endif
	mcList = mcList->next;
    }
#else
    enableFiltering(private->port);
#ifdef ETH_DBG_MC
    printk ("Multicast Addresses (%i)\n", dev->mc_count);
#endif
    for(i=0; i<dev->mc_count; i++)
    {
	addr = mcList->dmi_addr;
	mvFF_eth_update_mac_address(addr, private->port );
#ifdef ETH_DBG_MC
	printk("%0#x-%0#x-%0#x-%0#x-%0#x-%0#x\n",addr[0],addr[1],
                            addr[2],addr[3],addr[4],addr[5]);
#endif
	mcList = mcList->next;
    }
#endif
  }
}

static int
mdio_read(struct net_device *dev, int phy_id, int location)
{
  unsigned int val;

  etherReadMIIReg(phy_id & 0x1f, location & 0x1f, &val);

  return (val & 0xffff);
}

static void
mdio_write(struct net_device *dev, int phy_id, int location, int value)
{
  etherWriteMIIReg(phy_id & 0x1f, location & 0x1f, value);
}

#ifdef ETHERTOOL_FIXED
static int
netdev_ethtool_ioctl(struct net_device *dev, void *useraddr)
{
  gt_eth_priv *private = (gt_eth_priv *) dev->priv;
  u32 ethcmd;

  if (copy_from_user(&ethcmd, useraddr, sizeof (ethcmd)))
    return -EFAULT;
  
  switch (ethcmd) {
    
    /* Get driver info */
  case ETHTOOL_GDRVINFO:{
    struct ethtool_drvinfo info = { ETHTOOL_GDRVINFO };
#ifdef CONFIG_ARCH_FIREFOX
    strncpy(info.driver, "firefox",
	    sizeof (info.driver) - 1);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    strncpy(info.driver, "qd",
	    sizeof (info.driver) - 1);
#endif
    strncpy(info.version, version,
	    sizeof (info.version) - 1);
    if (copy_to_user(useraddr, &info, sizeof (info)))
      return -EFAULT;
    return 0;
  }
    /* get settings */
  case ETHTOOL_GSET:{
    struct ethtool_cmd ecmd = { ETHTOOL_GSET };
    spin_lock_irq(&private->lock);
    mii_ethtool_gset(&private->mii_if, &ecmd);
    spin_unlock_irq(&private->lock);
    if (copy_to_user(useraddr, &ecmd, sizeof (ecmd)))
      return -EFAULT;
    return 0;
  }
    /* set settings */
  case ETHTOOL_SSET:{
    int r;
    struct ethtool_cmd ecmd;
    if (copy_from_user(&ecmd, useraddr, sizeof (ecmd)))
      return -EFAULT;
    spin_lock_irq(&private->lock);
    r = mii_ethtool_sset(&private->mii_if, &ecmd);
    spin_unlock_irq(&private->lock);
    return r;
  }
    /* restart autonegotiation */
  case ETHTOOL_NWAY_RST:{
    return mii_nway_restart(&private->mii_if);
  }
    /* get link status */
  case ETHTOOL_GLINK:{
    struct ethtool_value edata = { ETHTOOL_GLINK };
    edata.data = mii_link_ok(&private->mii_if);
    if (copy_to_user(useraddr, &edata, sizeof (edata)))
      return -EFAULT;
    return 0;
  }
    /* get message-level */
  case ETHTOOL_GMSGLVL:{
    struct ethtool_value edata = { ETHTOOL_GMSGLVL };
    edata.data = 0;	/* XXX */
    if (copy_to_user(useraddr, &edata, sizeof (edata)))
      return -EFAULT;
    return 0;
  }
    /* set message-level */
  case ETHTOOL_SMSGLVL:{
    struct ethtool_value edata;
    if (copy_from_user(&edata, useraddr, sizeof (edata)))
      return -EFAULT;
    /* debug = edata.data; */
    return 0;
  }
  }
  return -EOPNOTSUPP;
}
#endif
static int
mvFF_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
  struct mii_ioctl_data *data = (struct mii_ioctl_data *) &rq->ifr_data;
  int phy = dev->base_addr & 0x1f;
  int retval = -EOPNOTSUPP;

  switch (cmd) {
  case SIOCETHTOOL:
#ifdef ETHERTOOL_FIXED
    retval = netdev_ethtool_ioctl(dev, (void *) rq->ifr_data);
#endif
    break;
    
  case SIOCGMIIPHY:	/* Get address of MII PHY in use. */
  case SIOCDEVPRIVATE:	/* for binary compat, remove in 2.5 */
    data->phy_id = phy;
    /* Fall through */

  case SIOCGMIIREG:	/* Read MII PHY register. */
  case SIOCDEVPRIVATE + 1:	/* for binary compat, remove in 2.5 */
    data->val_out =
      mdio_read(dev, data->phy_id & 0x1f, data->reg_num & 0x1f);
    retval = 0;
    break;

  case SIOCSMIIREG:	/* Write MII PHY register. */
  case SIOCDEVPRIVATE + 2:	/* for binary compat, remove in 2.5 */
    if (!capable(CAP_NET_ADMIN)) {
      retval = -EPERM;
    } else {
      mdio_write(dev, data->phy_id & 0x1f,
		 data->reg_num & 0x1f, data->val_in);
      retval = 0;
    }
    break;
    
  default:
    retval = -EOPNOTSUPP;
    break;
  }
  return retval;
}

/*
 * ----------------------------------------------------------------------------
 * Initializes the ethernet interface's private structure.
 * Statistics, descriptors, etc...
 *
 * Input : pointer to network device structure to be filled
 * Output : N/A
 */
void
mvFF_eth_init_priv(struct net_device *dev)
{
  gt_eth_priv *private;

  private = (gt_eth_priv *) kmalloc(sizeof (*private), GFP_KERNEL);
  if (!private) {
    panic("mvFF_eth_init_priv : kmalloc failed\n");
  }
  dev->priv = (void *) private;
  memset(private, 0, sizeof (*(private)));

  private->mii_if.dev = dev;
  private->mii_if.phy_id = dev->base_addr;
  private->mii_if.mdio_read = mdio_read;
  private->mii_if.mdio_write = mdio_write;
  private->mii_if.advertising = mdio_read(dev, dev->base_addr, MII_ADVERTISE);

  spin_lock_init(&private->lock);
  private->inited = false;

#ifdef ETH_DBG_INFO
  printk("Successful init of priv\n");
#endif

}

/*
 * ----------------------------------------------------------------------------
 * Currently a no-op.  According to previous commentary, the hardware
 * "cannot [!] be stuck because it is a built-in hardware - if we reach
 * here the ethernet port might be [physically] disconnected..."
 */
void
mvFF_eth_tx_timeout(struct net_device *dev)
{
#ifdef ETH_DBG_WARN
#ifdef CONFIG_ARCH_FIREFOX
  u32 cause = firefox_reg_read(FIREFOX_ENET_E0ICR );
  u32 mask = firefox_reg_read(FIREFOX_ENET_E0IMR );
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  u32 cause = libertas_reg_read(LIBERTAS_ENET_E0ICR );
  u32 mask = libertas_reg_read(LIBERTAS_ENET_E0IMR );
#endif
  printk("mvFF_eth_tx_timeout: cause =%x, mask=%x\n", cause, mask);
#endif /* ETH_DBG_WARN */
}

/*
 * ----------------------------------------------------------------------------
 *  First function called after registering the network device.
 *  It's purpose is to initialize the device as an ethernet device,
 *  fill the structure that was given in registration with pointers
 *  to functions, and setting the MAC address of the interface
 *
 *  Input : pointer to network device structure to be filled
 *  Output : -ENONMEM if failed, 0 if success
 */
s32
mvFF_eth_init(struct net_device * dev)
{
  int iNumOfVlans, i;
  u32 queue;
  gt_eth_priv *private;
  bool boFound = false;
  BINDING *pBinding = NULL;
  int vid = 0;
  int size;
  char *namebuf;
  iNumOfVlans = mvUnmGetNumOfVlans();
  
  ether_setup(dev);	/* auto assign some of the fields by kernel */
	
  dev = init_etherdev(dev, sizeof (gt_eth_priv));
  if (!dev) {
    panic("mvFF_eth_init : init_etherdev failed\n");
  }
  /* let's find which device is this */
  for( i=1 ; i <= iNumOfVlans ; i++) {
    ASSERT( mvBindings );
    ASSERT( mvBindings[i] );
    if( mvBindings[i]->dev == dev ) {
      boFound = true;
      pBinding = mvBindings[i];
      vid = i;
      break;
    }
  }
  if( !boFound || !pBinding || vid == 0 ) {
    panic("mvFF_eth_init : can't find my own device\n");
  }


  if( vid == 0 || vid > mvUnmGetNumOfVlans() ) {
    panic("mvFF_eth_init : init_etherdev dev->vid error\n");
  }
#ifdef ETH_DBG_INFO
  printk("eth_init of [%d]\n",vid);
#endif
	
  mvFF_eth_init_priv(dev);
  private = (gt_eth_priv *) (dev->priv);
  private->vid = vid; 

  /* getting the interface names from netconf */
  mv_nc_GetVIDName( vid, &size, &namebuf);
  memcpy(dev->name, namebuf, size+1);
  dev->name[size] = '\0';

#ifdef ETH_DBG_INFO
  printk("dev[%d]->name=%s\n", vid, dev->name);
#endif
  
  dev->open = mvFF_eth_open;
  dev->stop = mvFF_eth_stop;
  dev->set_config = NULL;	/* no runtime config support for now */
  dev->hard_start_xmit = mvFF_eth_start_xmit;
  dev->do_ioctl = mvFF_ioctl;
  dev->get_stats = mvFF_eth_get_stats;
  dev->set_mac_address = mvFF_eth_set_mac_address;
  dev->set_multicast_list = mvFF_eth_set_rx_mode;
  dev->tx_timeout = mvFF_eth_tx_timeout;	/* Currently no-op */
  dev->watchdog_timeo = 2 * HZ;
  dev->flags &= ~IFF_RUNNING;


  if (!private->inited) {
	memcpy(dev->dev_addr, mv_nc_GetMacOfVlan(vid), GT_ETHERNET_HEADER_SIZE );
	private->port = 0;
	private->inited = true;
  }

  if(dev->base_addr != 0x80008000) {
	panic("wrong dev addr!!!\n");
	return (-ENODEV);	/* Trouble if we haven't returned by this point... */
  }

  if( mvEthHw.hwState == HW_UNKNOWN  ) {
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0SDCR, (0x3 << 12) | (1 << 9) | (0xf << 2) | (1 << 6) | (1 << 7));	/*  0000.203c */
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0SDCR, (0x3 << 12) | (1 << 9) | (0xf << 2) | (1 << 6) | (1 << 7));	/*  0000.203c */
#endif

    /* enable */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0PCR, 
			firefox_reg_read(FIREFOX_ENET_E0PCR) | (1 << 7));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0PCR, 
			libertas_reg_read(LIBERTAS_ENET_E0PCR) | (1 << 7));
#endif

    /* no promisc */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0PCR, 
			firefox_reg_read(FIREFOX_ENET_E0PCR) & ~(1 << 0));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0PCR, 
			libertas_reg_read(LIBERTAS_ENET_E0PCR) & ~(1 << 0));
#endif

    /* set full duplex  */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0PCR, 
			firefox_reg_read(FIREFOX_ENET_E0PCR) | (1 << 15));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0PCR, 
			libertas_reg_read(LIBERTAS_ENET_E0PCR) | (1 << 15));
#endif

    /*
     * Receive packets in 1536 bit max length and enable DSCP
     */
    /* in the past MAC has only 10 mbps and rmii
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0PCXR,
			PORT_CONTROL_EXTEND_VALUE | (1 << 19) | (1 << 20)); 
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0PCXR,
			PORT_CONTROL_EXTEND_VALUE | (1 << 19) | (1 << 20)); 
#endif
    */
#ifdef CONFIG_ARCH_FIREFOX
#ifdef HEADERS
    firefox_reg_write(FIREFOX_ENET_E0PCXR,( (1<<22) | (1<<19) | (1<<18) | (1<<16) | (1<<14) | (1<<10) | (1<<9) /* |(1<<8)*/  ) );
#else
    firefox_reg_write(FIREFOX_ENET_E0PCXR,( (1<<16) | (1<<14) | (1<<8) ) );
#endif
#endif
#ifdef CONFIG_ARCH_LIBERTAS
#ifdef HEADERS
    libertas_reg_write(LIBERTAS_ENET_E0PCXR,( (1<<22) | (1<<19) | (1<<18) | (1<<16) | (1<<14) | (1<<10) | (1<<9) /* |(1<<8)*/  ) );
#else
    libertas_reg_write(LIBERTAS_ENET_E0PCXR,( (1<<16) | (1<<14) | (1<<8) ) );
#endif
#endif

    /*
     * The FIREFOX Enet engine accesses its queues of DMA
     * descriptors in uncached mode, so we dedicate a page
     * for each queue and mark each such page uncached.
     */
    for (queue = 0; queue < NUM_TX_QUEUES; queue++) {
      ASSERT(mvEthHw.TXqueue[queue] ==  NULL);
      if (queue == MVFF_TX_QUEUE) {
	mvEthHw.TXqueue[queue] = (gt_dma_desc *) uncachedPages(1);
	if(mvEthHw.TXqueue[queue] == NULL) {
	  panic("Can't allocate memory for descriptors\n");
	}
      }
      else {
	mvEthHw.TXqueue[queue] = (gt_dma_desc *)0;
      }
    }
    
    for (queue = 0; queue < NUM_RX_QUEUES; queue++) {
      ASSERT(mvEthHw.RXqueue[queue] == NULL);
      if (queue == MVFF_RX_QUEUE) {
	mvEthHw.RXqueue[queue] = (gt_dma_desc *) uncachedPages(1);
	if(mvEthHw.RXqueue[queue] == NULL) {
	  panic("Can't allocate memory for descriptors\n");
	}
      }
      else {
	mvEthHw.RXqueue[queue] = (gt_dma_desc *)0;
      }
    }

    /*
     * Initialize address table for hash mode 0 with 1/2K size
     */
// HACK: force to pass the address even if it's not matched
//    initAddressTable(private->port, 0, 1, 0);
    initAddressTable(private->port, 0, 1, 1);

    mvEthHw.hwState = HW_INIT;

  }
  
  /* The address will be added to address table at eth_open */

  /*
   * Read MIB counters on the GT in order to reset them,
   * then zero all the stats fields in memory
   */
  mvFF_eth_update_stat(dev);
  
  return 0;
}

/*
 * ----------------------------------------------------------------------------
 *  This function is called when opening the network device. The function
 *  should initialize all the hardware, initialize cyclic Rx/Tx
 *  descriptors chain and buffers and allocate an IRQ to the network
 *  device.
 *
 *  Input : a pointer to the network device structure
 *
 *  Output : zero if success, nonzero if fails.
 */
s32
mvFF_eth_open(struct net_device * dev)
{
  gt_dma_desc *desc;
  gt_eth_priv *priv;
  s32 retval;
  struct sk_buff *sk;
  u32 count;
  u32 gap = 0;
  u32 port;
  u32 port_status;
  u32 queue;
  unsigned int portsBitMask;
  GT_STATUS status;
  priv = dev->priv;


  ASSERT( mvEthHw.hwState != HW_UNKNOWN);

  /* check if we need to move into HW_READY state */
  if( mvEthHw.hwState == HW_INIT  ) {
    /*
     * Initialize the lists of Tx/Rx descriptors (as circular chains,
     * each in its own uncached page) using the physical addresses in
     * the "next" pointers that the Enet DMA engine expects.  The Rx
     * descriptors also get an sk_buff pre-allocated for them and their
     * "data" pointers set to point to the corresponding sk_buff buffer.
     */
    for (queue = 0; queue < NUM_TX_QUEUES; queue++) {
      mvEthHw.TXskbIndex[queue] =  mvEthHw.TXindex[queue] = 0;
      desc = mvEthHw.TXqueue[queue];
      if (desc) {
	memset((void *) desc, 0, PAGE_SIZE);	/* The whole list. */
	for (count = 0; count < Q_INDEX_LIMIT; count++, desc++) {
	  desc->next = virt_to_phys((void *) (desc + 1));
	  mvEthHw.TXskbuff[queue][count] = 0;
	}
	--desc;		/* Link last back to first. */
	desc->next = virt_to_phys((void *) (mvEthHw.TXqueue[queue]));
	flush_dcache_addr_size((u32) (mvEthHw.TXqueue[queue]), PAGE_SIZE);
      }
    }

    for (queue = 0; queue < NUM_RX_QUEUES; queue++) {
      mvEthHw.RXindex[queue] = 0;
      desc = mvEthHw.RXqueue[queue];
      if (desc) {
	memset((void *) desc, 0, PAGE_SIZE);	/* The whole list. */
	for (count = 0; count < Q_INDEX_LIMIT; count++, desc++) {
	  desc->next = virt_to_phys((void *) (desc + 1));
	  desc->count.rx.bufferBytes = MAX_BUFF_SIZE;
	  desc->command_status = GT_ENET_DESC_OWNERSHIP
	    | GT_ENET_DESC_INT_ENABLE;

	  sk = dev_alloc_skb(MAX_BUFF_SIZE);
	  desc->data = (void *) virt_to_phys((void *) (sk->data));
	  mvEthHw.RXskbuff[queue][count] = sk;
	  invalidate_dcache_range((u32) (sk->data),
				  (u32) (sk->data) +
				  MAX_BUFF_SIZE);
	}
	--desc;		/* Link last back to first. */
	desc->next = virt_to_phys((void *) (mvEthHw.RXqueue[queue]));
	flush_dcache_addr_size((u32) (mvEthHw.RXqueue[queue]), PAGE_SIZE);
      }
    }

    /*
     * Initialize DMA descriptor-pointer registers
     */
    port = priv->port;
    gap = ETH_ADDR_GAP * port;

    /* Set Tx queues 0..1 */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0CTDP0 + gap,
			virt_to_phys((void *)mvEthHw.TXqueue[0]));
    firefox_reg_write(FIREFOX_ENET_E0CTDP1 + gap,
			virt_to_phys((void *)mvEthHw.TXqueue[1]));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0CTDP0 + gap,
			virt_to_phys((void *)mvEthHw.TXqueue[0]));
    libertas_reg_write(LIBERTAS_ENET_E0CTDP1 + gap,
			virt_to_phys((void *)mvEthHw.TXqueue[1]));
#endif

    /* Set Rx queues 0..3 */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0FRDP0 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[0]));
    firefox_reg_write(FIREFOX_ENET_E0CRDP0 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[0]));
    firefox_reg_write(FIREFOX_ENET_E0FRDP1 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[1]));
    firefox_reg_write(FIREFOX_ENET_E0CRDP1 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[1]));
    firefox_reg_write(FIREFOX_ENET_E0FRDP2 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[2]));
    firefox_reg_write(FIREFOX_ENET_E0CRDP2 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[2]));
    firefox_reg_write(FIREFOX_ENET_E0FRDP3 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[3]));
    firefox_reg_write(FIREFOX_ENET_E0CRDP3 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[3]));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0FRDP0 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[0]));
    libertas_reg_write(LIBERTAS_ENET_E0CRDP0 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[0]));
    libertas_reg_write(LIBERTAS_ENET_E0FRDP1 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[1]));
    libertas_reg_write(LIBERTAS_ENET_E0CRDP1 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[1]));
    libertas_reg_write(LIBERTAS_ENET_E0FRDP2 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[2]));
    libertas_reg_write(LIBERTAS_ENET_E0CRDP2 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[2]));
    libertas_reg_write(LIBERTAS_ENET_E0FRDP3 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[3]));
    libertas_reg_write(LIBERTAS_ENET_E0CRDP3 + gap,
			virt_to_phys((void *)mvEthHw.RXqueue[3]));
#endif
  
    /*
     * Allocate IRQ
     */
    retval = request_irq(IRQ_FE,
			 mvFF_eth_int_handler,
			 0, /* (SA_INTERRUPT | SA_SAMPLE_RANDOM),*/
			 "mvFF_Eth", NULL); 
    if (!retval) {
      irq = IRQ_FE;
#ifdef ETH_DBG_INFO
      printk("mvFF_eth_open : Assigned IRQ %d to mvFF_eth \n",irq);
#endif
    }
    if (retval) { 
      panic("Can not assign IRQ number to mvFF_eth%d\n", port);
      return (-1);
    }

    /*
     * clear all interrupts
     */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR + gap, 0x000000000);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR + gap, 0x000000000);
#endif
    
    /*
     * enable relevant interrupts on GT
     */
#ifdef CONFIG_ARCH_FIREFOX
#ifdef DEBUG
    firefox_reg_write(FIREFOX_ENET_E0IMR + gap, 0x90f10109);
#else
    firefox_reg_write(FIREFOX_ENET_E0IMR + gap, 0x90000009);
#endif
#endif
#ifdef CONFIG_ARCH_LIBERTAS
#ifdef DEBUG
    libertas_reg_write(LIBERTAS_ENET_E0IMR + gap, 0x90f10109);
#else
    libertas_reg_write(LIBERTAS_ENET_E0IMR + gap, 0x90000009);
#endif
#endif

    /* Set rx_task pointers */
    mvEthHw.hwState = HW_READY;
  }

  /*
   * Check Link status on MAC 
   */
#ifdef CONFIG_ARCH_FIREFOX
  port_status = firefox_reg_read(FIREFOX_ENET_E0PSR + gap);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  port_status = libertas_reg_read(LIBERTAS_ENET_E0PSR + gap);
#endif
  
  ASSERT( priv->vid != 0 );
  
  /* in FF, no link on MAC, indicate a HW unrecoverable error */
  if( !(port_status & 0x8) ) {
    panic("Error no link on MAC\n");
  }  
  /* check phy state */
  check_phy_state(dev); 

#ifdef ETH_DBG_INFO
  dump_link_state(dev);
#endif
  
  mvFF_eth_update_mac_address(dev->dev_addr, priv->port );
  
  if( mvEthHw.hwState == HW_READY ) {
    /*
     * start RX  (BIT(7) == EnableRXdma)
     */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0SDCMR + gap, (1 << 7));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0SDCMR + gap, (1 << 7));
#endif
    mvEthHw.hwState = HW_ACTIVE;
    mvFF_eth_update_mac_address(brdcast_mac, priv->port );
  }
  
  spin_lock_init(&dev->xmit_lock);
  spin_lock_init(&lock);
  
  portsBitMask = mvUnmGetPortMaskOfVid( priv->vid);

  ASSERT(priv->vid != 0);
#ifdef HEADERS  
  mvBindings[priv->vid]->header[0] = (priv->vid - 1) << 4;
  mvBindings[priv->vid]->header[1] = portsBitMask;
#elif defined (TRAILERS)
  mvBindings[priv->vid]->trailer[0] = (1<<7); /* DBNUM = 0 */
  mvBindings[priv->vid]->trailer[1] = portsBitMask;
  mvBindings[priv->vid]->trailer[2] = 0; /* DBNUM = 0 */
  mvBindings[priv->vid]->trailer[3] = 0;
#endif

  mvBindings[priv->vid]->boOpened = true;
#ifdef ETH_DBG_INFO
  printk("eth_open for [%d] link is %s\n", priv->vid, (portsBitMask==0) ? "DOWN" : "UP" );
#endif
  /* update logical link state, RUNNING flag and queue state */
  if(portsBitMask == 0 ) {
    netif_stop_queue(dev);
    dev->flags &= ~IFF_RUNNING;
    mvBindings[priv->vid]->boLinkUp = false;
    netif_carrier_off(dev);
  } else {
    netif_start_queue(dev);
    dev->flags |= IFF_RUNNING;
    mvBindings[priv->vid]->boLinkUp = true;
    netif_carrier_on(dev);
  }

  status = mvUnmCreateVlan(priv->vid,portsBitMask);
  if( status != GT_OK ) {
    panic("Can't create VLAN %d, with bitmask %x\n", priv->vid, portsBitMask);
    return -1;
  }

  MOD_INC_USE_COUNT;
  
  return (0);
}

/*
 * ----------------------------------------------------------------------------
 * This function is used when closing the network device.  It should update
 * the hardware, release all memory that holds buffers and descriptors and
 * release the IRQ.
 * Input : a pointer to the device structure
 * Output : zero if success, nonzero if fails
 */
int
mvFF_eth_stop(struct net_device *dev)
{
  gt_eth_priv *priv;
  u32 queue;
  u32 count;
  bool boLastDev = false;
  int iNumOfVlans, i;

  iNumOfVlans = mvUnmGetNumOfVlans();
  priv = dev->priv;
  
  ASSERT(priv->vid);
#ifdef ETH_DBG_INFO
  printk("eth_stop for [%d]\n", priv->vid);
#endif

  if(  mvBindings[priv->vid]->boOpened == false ) {
#ifdef ETH_DBG_ERROR
    printk("Kernel is trying to close a closed interface!\n");
#endif
    return(0);    
  }
  
  /* we are going into off state */
  mvBindings[priv->vid]->boOpened = false; 

  for( i=1; i <= iNumOfVlans; i++  ) {
    if( mvBindings[i]->boOpened ) {
      /* we are not last there is still an open  device */
      break;
    }
    if( i == iNumOfVlans ) {
      boLastDev = true;
    }
  }

#ifdef ETH_DBG_INFO
    printk("Kernel is closing instance number [%d]", priv->vid );
    if(boLastDev)
      printk(" Last one!\n");
    else
      printk(" NOT Last\n");
#endif

  netif_stop_queue(dev);

  if( boLastDev ) {
    /*
     * stop RX and mask interrupts
     */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0SDCMR
			+ (ETH_ADDR_GAP * priv->port), 0x00008000);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0SDCMR
			+ (ETH_ADDR_GAP * priv->port), 0x00008000);
#endif

    /* going down to ready */
    mvEthHw.hwState = HW_READY;

    /*
     * Free RX pre allocated SKB's
     */
    for (queue = 0; queue < NUM_RX_QUEUES; queue++) {
      for (count = 0; count < Q_INDEX_LIMIT; count++) {
	if (mvEthHw.RXskbuff[queue][count]) {
	  dev_kfree_skb(mvEthHw.RXskbuff[queue][count]);
	  mvEthHw.RXskbuff[queue][count] = 0;
	}
      }
    }
    
    free_irq(irq, NULL); /* free_irq(dev->irq, dev); */
    mvEthHw.hwState = HW_READY;
  } 
  MOD_DEC_USE_COUNT;
    
  return (0);
}

/*
 * ----------------------------------------------------------------------------
 * This function queues a packet in the Tx descriptor for required
 * port.  It checks the IPTOS_PREC_FLASHOVERRIDE bit of the ToS
 * field in the IP header and decides where to queue the packet,
 * in the high priority queue or in the low priority queue.
 *
 * Input : skb - a pointer to socket buffer
 *         dev - a pointer to the required port
 *
 * Output : zero upon success, negative number upon failure
 *         (-EBUSY when the interface is busy)
 */
int
mvFF_eth_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
  gt_eth_priv *priv;
  gt_dma_desc *tx;
  struct iphdr *iph;
  u32 queue;
  u32 TXindex;
  unsigned long flags;
  int i;
#if defined (HEADERS) || defined (TRAILERS)
  bool boCopy = false;
  struct sk_buff *skb_copy;
#endif

  iph = skb->nh.iph;
  priv = dev->priv;
  spin_lock_irqsave(&lock, flags);

  /* check - this shouldn't happen */
  if (skb == NULL) {
    priv->stat.tx_dropped++;
    spin_unlock_irqrestore(&lock, flags);
#ifdef ETH_DBG_ERROR
    printk(KERN_CRIT "Fatal Error SKB == NULL in start tx\n");
#endif /* ETH_DBG_ERROR */
    return (-EBUSY);
  }

  ASSERT( netif_carrier_ok(mvBindings[priv->vid]->dev) );
  ASSERT( mvBindings[priv->vid]->boLinkUp == true);

  /*
   * Queue packet to either queue 0 (high priority) or 1 (low
   * priority) depending on the MSB of the TOS in the IP header
   */
  queue = MVFF_TX_QUEUE;

#ifdef HEADERS
  /*
   * Here we need to check header insertion and padding,
   * if we have no place at the beginning of buffer for header, or we have no sufficient padding space
   * we copy the packet to a fresh SKB
   */
#ifdef ETH_DBG_ECHO
  printk(" skb->data - skb->head == %d\n",(skb->data - skb->head));
#endif /*  ETH_DBG_ECHO */
  /* header check */
  if( (skb->data - skb->head) < 2 ) {
    boCopy = true;
#ifdef ETH_DBG_ERROR
    printk("Dev [%d] - need to copy beacuse of header\n", priv->vid);
#endif /* ETH_DBG_ERROR */
  }

  /* padding check */
  /* instead of copy, we can DMA garbage from the packet's end ... (Security!!!) */
#if 0
  if( skb->len < MIN_ETH_PACKET_LEN ) { /*  MINIMAL_ETH_PACKET_LEN == 60 */
    if( (skb->end - skb->tail) < (MIN_ETH_PACKET_LEN - skb->len)  ) {
      boCopy = true;
#ifdef ETH_DBG_ERROR
      printk("Dev [%d] - need to copy beacuse of padding\n", priv->vid);
#endif
    }
  }
#endif /* 0 */

  if( boCopy ) {
    skb_copy = dev_alloc_skb(MAX_BUFF_SIZE);
    priv->privstat.tx_needed_copy++;
    if( skb_copy == NULL ) { /* silently drop */
#ifdef ETH_DBG_ERROR
      printk(KERN_CRIT "silently drop\n");
#endif /* ETH_DBG_ERROR */
      priv->stat.tx_dropped++;
      spin_unlock_irqrestore(&lock, flags);
      return (-EBUSY);
    }
    skb_copy->dev = dev;
    /* let's copy (we are placing the data in the real data, idea is to reflect a 'normal' packet */
    skb_copy->data = skb_copy->head + HEADER_SIZE;
    memcpy( skb_copy->data, skb->data, skb->len );
    skb_copy->len = skb->len;
    /* free the original skb */ 
    dev_kfree_skb(skb);

    /* in any casy skb is now pointing to an skb with a place to put the header in front and
     * the length is not updated but we do have a sufficient valid buffer at packet's end
     */
    skb = skb_copy;
  }
  /* we need to check the length before adding the header's two bytes, after switch will extract
   * header the packet (with 4 bytes crc) should be 64 or longer
   */
  if(skb->len < MIN_ETH_PACKET_LEN ) {
    skb->len = MIN_ETH_PACKET_LEN; /* this is safe because we allocated this skb */
  }

  /* prepare the header place and insert the header */
  skb->data -= 2;
  skb->len += 2;
  ASSERT( ((skb->data - skb->head) >= 0) );
  /* skb->data[0] = mvBindings[priv->vid]->header[0]; */
  /* skb->data[1] = mvBindings[priv->vid]->header[1]; */
  ((unsigned short *)skb->data)[0] = ((unsigned short *)(mvBindings[priv->vid]->header))[0];

#elif defined (TRAILERS)
  /*
   * Here we need to check trailer insertion and padding,
   * if we have no place at the end of buffer for trailer, or we have no sufficient padding space
   * we copy the packet to a fresh SKB 
   */
#ifdef ETH_DBG_ECHO
  printk(" skb->end - skb->tail == %d\n",(skb->end - skb->tail));
#endif 

  /* padding check */
  if( skb->len < MIN_ETH_PACKET_LEN ) { /*  MINIMAL_ETH_PACKET_LEN == 60 */
    if( (skb->end - skb->tail) < (MIN_ETH_PACKET_LEN - skb->len +4 )  ) {
      boCopy = true;
#ifdef ETH_DBG_ERROR
      printk("Dev [%d] - need to copy beacuse of padding\n", priv->vid);
#endif
    }
  } 
  /* trailer check */
  else if( (skb->end - skb->tail) < 4 ) {
    boCopy = true;
#ifdef ETH_DBG_ERROR
    printk("Dev [%d] - need to copy beacuse of trailer\n", priv->vid);
#endif
  }
  
  
  if( boCopy ) {
    skb_copy = dev_alloc_skb(MAX_BUFF_SIZE);
    priv->privstat.tx_needed_copy++;
    if( skb_copy == NULL ) { /* silently drop */
      priv->stat.tx_dropped++;
      spin_unlock_irqrestore(&lock, flags);
      return (-EBUSY);
    }
    skb_copy->dev = dev;
    /* let's copy (we are placing the data in the real data, idea is to reflect a 'normal' packet */
    memcpy( skb_copy->data, skb->data, skb->len );
    skb_copy->len = skb->len;
    /* free the original skb */ 
    dev_kfree_skb(skb);

    /* in any casy skb is now pointing to an skb with a place to put the header in front and
     * the length is not updated but we do have a sufficient valid buffer at packet's end
     */
    skb = skb_copy;
  }
  /* we need to check the length before adding the header's two bytes, after switch will extract
   * header the packet (with 4 bytes crc) should be 64 or longer
   */
  if(skb->len < MIN_ETH_PACKET_LEN ) {
    skb->len = MIN_ETH_PACKET_LEN; /* this is safe because we allocated this skb */
  }

  /* prepare the trailer place and insert the trailer */

  ASSERT( ((skb->end - skb->tail) >= 0) );
  /* TODO - use a faster memcpy */
  skb->data[skb->len]   = mvBindings[priv->vid]->trailer[0];
  skb->data[skb->len+1] = mvBindings[priv->vid]->trailer[1];
  skb->data[skb->len+2] = mvBindings[priv->vid]->trailer[2];
  skb->data[skb->len+3] = mvBindings[priv->vid]->trailer[3];
  skb->len += 4;
#endif /*  HEADERS elif TRAILERS */

  dev->trans_start = jiffies;	/* timestamp */
  TXindex = mvEthHw.TXindex[queue];
  if (mvEthHw.TXskbuff[queue][TXindex]) {
    panic("Error on mvFF_eth device driver");
  }

  mvEthHw.TXskbuff[queue][TXindex] = skb;
  tx = &( mvEthHw.TXqueue[queue][TXindex]);

#ifdef DESC_CACHABLE
  invalidate_dcache_range((u32) tx, (u32) tx);
#endif /* DESC_CACHABLE */
  /* mb(); */
  tx->data = (void *) virt_to_phys((void *) skb->data);
  tx->count.tx.bytes = skb->len;
  
  tx->count.tx.reserved = (u16)priv->vid;
  
  /*
   * Flush/sync now before transferring ownership.
   */
  flush_dcache_addr_size((u32) skb->data, skb->len);

  wmb();
  
  tx->command_status = (1 << 31) | (1 << 23) | (1 << 22) | (7 << 16);

  /*
   * Officially transfer ownership of descriptor to GT
   */
#ifdef DESC_CACHABLE
  flush_dcache(tx);
#endif /* DESC_CACHABLE */
  /* mb(); */

  TXindex++;
  if (TXindex == Q_INDEX_LIMIT) {
    TXindex = 0;
  }

  /*
   * If next descriptor is GT owned then the tx queue is full
   */
  if (mvEthHw.TXskbuff[queue][TXindex]) {
#ifdef ETH_DBG_WARN
    printk ("Stopping queue on [%d]\n",priv->vid);
#endif
    /* we will stop all queues, as HW is a shared resource */
    for( i = 1 ; i <= mvUnmGetNumOfVlans(); i++ ) { 
      netif_stop_queue(mvBindings[i]->dev);
    }
  }
  mvEthHw.TXindex[queue] = TXindex;
 
  /*
   * Start Tx LOW dma
   */
#ifdef CONFIG_ARCH_FIREFOX
  firefox_reg_write(FIREFOX_ENET_E0SDCMR
		      + (ETH_ADDR_GAP * priv->port), queue ? (1 << 23) : (1 << 24));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  libertas_reg_write(LIBERTAS_ENET_E0SDCMR
		      + (ETH_ADDR_GAP * priv->port), queue ? (1 << 23) : (1 << 24));
#endif

  spin_unlock_irqrestore(&lock, flags);
  
  return (0);		/* success */
}

/*
 * ----------------------------------------------------------------------------
 * This function is forward packets that are received from the port's
 * queues toward kernel core or FastRoute them to another interface.
 *
 * Input : dev - a pointer to the required interface
 *
 * Output : number of served packets
 */
void
mvFF_eth_receive_queue(unsigned long data)
{
  gt_dma_desc *rx;
  struct sk_buff *skb;
  struct sk_buff *new_skb;
  u32 RXindex;
  u32 served;
  MV_UNM_VID vid;
  u32 eth_int_mask;
  int queue = MVFF_RX_QUEUE;
#ifdef SW_COUNTER
  struct net_device *dev;
  gt_eth_priv *priv;
#endif /* SW_COUNTER */

#ifdef DEBUG
  int port = 0;
#endif

  unsigned int max = (unsigned int)data;

#if defined (HEADERS) || defined (TRAILERS)
  unsigned char ucSrcPort;
#endif

  /* Taked from original ISR */
  /* We are clearing the interrupt rx bit here, in order to avoid spending unneccesary
   * time in acking spurse interrupts
   */
#ifdef CONFIG_ARCH_FIREFOX
  firefox_reg_write(FIREFOX_ENET_E0ICR, (u32)~ 0x1);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  libertas_reg_write(LIBERTAS_ENET_E0ICR, (u32)~ 0x1);
#endif

  RXindex =  mvEthHw.RXindex[queue];	/* Where we left off... */
  served = 0;
  while (max) {
    rx = &(mvEthHw.RXqueue[queue][RXindex]);
#ifdef DESC_CACHABLE
    invalidate_dcache(rx);
#endif /* DESC_CACHABLE */
    /* mb(); */

    if (rx->command_status & GT_ENET_DESC_OWNERSHIP) {
      break;
    }
    max--;
    
    /*
     * If received packet has errors, keep the socket buffer and change
     * descriptor to GT owner ship and continue analyzing next descriptor.
     */
    if (rx->command_status & GT_ENET_DESC_ERROR_SUMMARY) {
      rx->command_status = GT_ENET_DESC_OWNERSHIP | GT_ENET_DESC_INT_ENABLE;
#ifdef DESC_CACHABLE
      flush_dcache(rx);
#endif
      /* mb(); */
      RXindex++;
      if (RXindex == Q_INDEX_LIMIT) {
	RXindex = 0;
      }
      continue;
    }

    /*
     * We never leave descriptors without an skb
     * we will allocate new skb, if we cannot allocate we will return the 'just-closed'
     * descriptor to HW ownership and will not! indicate current packet up the stack
     */
    new_skb = dev_alloc_skb(MAX_BUFF_SIZE);
    if( new_skb == NULL ) {
#ifdef ETH_DBG_ERROR
      printk(KERN_CRIT "Could not alloc new skb\n");
#endif /* ETH_DBG_ERROR */
      rx->command_status = GT_ENET_DESC_OWNERSHIP | GT_ENET_DESC_INT_ENABLE;
#ifdef DESC_CACHABLE
      flush_dcache(rx);
#endif /* DESC_CACHABLE */
      /* mb(); */
      RXindex++;
      if (RXindex == Q_INDEX_LIMIT) {
	RXindex = 0;
      }
      continue;
    }
    
    /* okay, let's deal with the packet */
    served++;
    skb = mvEthHw.RXskbuff[queue][RXindex];
#ifdef DESC_CACHABLE    
    invalidate_dcache(rx);
#endif /* DESC_CACHABLE */
    /* mb(); */

#ifdef ETH_DBG_ERROR
    if( skb == NULL || skb->len != 0 )
      printk("mvFF_eth_receive_queue: SKB error\n");
#endif /* ETH_DBG_ERROR */

    if (skb) {
      if (skb->len) {
#ifdef ETH_DBG_ERROR
	printk("mvFF_eth_receive_queue: nonzero existing SKB\n");
#endif /* ETH_DBG_ERROR */
	dev_kfree_skb(skb);
	skb = dev_alloc_skb(MAX_BUFF_SIZE);
      }
      /* tail - point to data end, len and data fileds will be updated manualy to extract header */


#ifdef HEADERS
      ucSrcPort = ((skb->data[1]) & 0xf );

#ifdef ETH_DBG_MC
      if(skb->data[2] & 0x1)
      {
	printk("Received a multi/broadcast packet (PCR : %0#x)\n",get_port_config(0));
	printk("%0#x-%0#x-%0#x-%0#x-%0#x-%0#x\n",skb->data[2],skb->data[3],
                        skb->data[4],skb->data[5],skb->data[6],skb->data[7]);
      }		
#endif
      
      skb_put(skb, (rx->count.rx.bytesReceived - 4 ));
      skb->data += 2;
      skb->len = skb->len - 2; /* skb_put above updated the init value */

#elif defined (TRAILERS)
      ucSrcPort = ((skb->data[rx->count.rx.bytesReceived - 8 + 1])    & 0xf );
       memmove( skb->data + 2, skb->data, (rx->count.rx.bytesReceived - 8));
      skb->data += 2;
      skb_put(skb, (rx->count.rx.bytesReceived - 8 ));
#else
      memmove( skb->data + 2, skb->data, (rx->count.rx.bytesReceived - 4));
      skb->data += 2;
      skb_put(skb, (rx->count.rx.bytesReceived - 4 ));
#endif /* HEADERS */


#ifdef DESC_CACHABLE      
      invalidate_dcache(rx);
#endif /* DESC_CACHABLE */
      /* mb(); */


#if defined (HEADERS) || defined (TRAILERS) 
      /* we need to find our device from the source port */

      ASSERT( ucSrcPort <= GT_NUM_OF_SWITCH_PORTS );
      ASSERT( ucSrcPort != GT_CPU_SWITCH_PORT );
      vid = mvUnmGetVidOfPort(ucSrcPort);

      ASSERT( vid != 0);
      ASSERT( mvBindings[vid] != NULL );
      ASSERT( mvBindings[vid]->dev != NULL);
      
      skb->dev = mvBindings[vid]->dev;
      skb->protocol = eth_type_trans(skb,  mvBindings[vid]->dev);
      
      ASSERT( netif_carrier_ok(mvBindings[vid]->dev) );
      ASSERT( mvBindings[vid]->boLinkUp == true);
 
#else /*  ! (defined (HEADERS) || defined (TRAILERS)) */
      skb->dev = mvBindings[1]->dev;
      skb->protocol = eth_type_trans(skb,  mvBindings[1]->dev);
#endif /*  defined (HEADERS) || defined (TRAILERS) */

#ifdef SW_COUNTER
      /* we need the dev (and the VID in tx descriptor - only for counters */
      dev = mvBindings[vid]->dev;
      priv = dev->priv;  
      priv->stat.rx_packets++;
      priv->stat.rx_bytes += skb->len;
#endif /* SW_COUNTER */
	  
      /*       skb->pkt_type = PACKET_HOST; */
      /*       skb->ip_summed = CHECKSUM_NONE;	-- UNNECESSARY; CRC performed by hardware */
      netif_rx(skb);
      /*
	if( netif_rx(skb) != NET_RX_SUCCESS ) {
	printk(KERN_CRIT "netif_rx error\n");
	((gt_eth_priv*)(mvBindings[vid]->dev->priv))->stat.rx_fifo_errors++;
	}*/
    }
    
    skb = new_skb; /*dev_alloc_skb(MAX_BUFF_SIZE);*/
    mvEthHw.RXskbuff[queue][RXindex] = skb;
    ASSERT(skb);


    invalidate_dcache_range((u32) (skb->data),
			    (u32) (skb->data) +
			    MAX_BUFF_SIZE);
    /* skb->dev = dev; GM: removed */
    rx->data = (void *) virt_to_phys((void *) skb->data);
    
    /*
     * Officially transfer ownership of descriptor to GT
     */
    rx->command_status = GT_ENET_DESC_OWNERSHIP
	    | GT_ENET_DESC_INT_ENABLE;
#ifdef DESC_CACHABLE
    flush_dcache(rx);
#endif /* DESC_CACHABLE */
    /* mb(); */
    
    RXindex++;
    if (RXindex == Q_INDEX_LIMIT) {
      RXindex = 0;
    }
  }
  
  mvEthHw.RXindex[queue] = RXindex;
  
#ifdef ETH_DBG_ECHO
  if(served) {
    printk("Rx served %d\n",served);
  }
#endif /* ETH_DBG_ECHO */
  
  /* here we open the rx interrupt back again */
#ifdef CONFIG_ARCH_FIREFOX
#ifdef DEBUG
  firefox_reg_write(FIREFOX_ENET_E0IMR + (ETH_ADDR_GAP * port), 0x90f10109);
#else
  eth_int_mask = firefox_reg_read(FIREFOX_ENET_E0IMR); /* Enhance to shadow this register */
  firefox_reg_write(FIREFOX_ENET_E0IMR, (eth_int_mask | 0x00000001) );
#endif
#endif
#ifdef CONFIG_ARCH_LIBERTAS
#ifdef DEBUG
  libertas_reg_write(LIBERTAS_ENET_E0IMR + (ETH_ADDR_GAP * port), 0x90f10109);
#else
  eth_int_mask = libertas_reg_read(LIBERTAS_ENET_E0IMR); /* Enhance to shadow this register */
  libertas_reg_write(LIBERTAS_ENET_E0IMR, (eth_int_mask | 0x00000001) );
#endif
#endif

  /* This is copied from the original place in the interrupt handler */
  /*
   * start/continue ethernet 0 RX
   */
  /*
#ifdef CONFIG_ARCH_FIREFOX
  firefox_reg_write(FIREFOX_ENET_E0SDCMR + (ETH_ADDR_GAP * port), (1 << 7));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  libertas_reg_write(LIBERTAS_ENET_E0SDCMR + (ETH_ADDR_GAP * port), (1 << 7));
#endif
  */
  return; /*  (served); */
}

/*
 * ----------------------------------------------------------------------------
 * Input : dev - a pointer to the required interface
 *
 * Output : N/A
 */
static void
mvFF_eth_free_tx_queue(unsigned long queue)
{
  gt_eth_priv *priv;
  gt_dma_desc *tx;
  struct sk_buff *sk;
  u32 freed_skbs;
  u32 TXskbIndex;
  struct net_device * dev;
  MV_UNM_VID vid;
  int i;
  u32 eth_int_mask;
  
  spin_lock(&lock);
  freed_skbs = 0;
  TXskbIndex = mvEthHw.TXskbIndex[queue];

  while (1) {
    sk =  mvEthHw.TXskbuff[queue][TXskbIndex];
    if (!sk) {
      break;
    }
    tx = &(mvEthHw.TXqueue[queue][TXskbIndex]);	/* No write to tx here */
    if (tx->command_status & 0x80000000) {
      break;
    }
    vid = (int)(tx->count.tx.reserved);
    ASSERT(vid != 0);

#ifdef ETH_DBG_ECHO
    if(vid == 0) {
      panic("something went wrong with VID in TX END\n");
    }
    printk("Freeing TX packet for [%d]\n",vid);
#endif

    /* we need the dev (and the VID in tx descriptor - only for counters */
    dev = mvBindings[vid]->dev;
    priv = dev->priv;  
#ifdef DESC_CACHABLE
    invalidate_dcache(tx);
#endif /* DESC_CACHABLE */
    /* mb(); */
    if (tx->command_status & 0x40) {
      priv->stat.tx_fifo_errors++;
#ifdef SW_COUNTER
    } else {
      priv->stat.tx_packets++;
#ifdef HEADERS
      priv->stat.tx_bytes += sk->len - 2;
#else
      priv->stat.tx_bytes += sk->len - 4;
#endif
#endif /* SW_COUNTER */
    }
    dev_kfree_skb_irq(sk);
    mvEthHw.TXskbuff[queue][TXskbIndex] = 0;
    TXskbIndex++;
    if (TXskbIndex == Q_INDEX_LIMIT) {
      TXskbIndex = 0;
    }
    freed_skbs++;
  }
  mvEthHw.TXskbIndex[queue] = TXskbIndex;
  
  spin_unlock(&lock);

  for( i = 1 ; i <= mvUnmGetNumOfVlans(); i++ ) { 
      if ((netif_queue_stopped(mvBindings[i]->dev)) && 
	  (mvBindings[i]->boLinkUp) && 
	  /* (mvBindings[i]->dev->flags & IFF_RUNNING) && */
	  (mvEthHw.TXskbuff[queue][mvEthHw.TXindex[queue]] == 0) 
	 ) {
		netif_wake_queue(mvBindings[i]->dev);
      }
    }

#ifdef CONFIG_ARCH_FIREFOX
  eth_int_mask = firefox_reg_read(FIREFOX_ENET_E0IMR); /* Enhance to shadow this register */
  firefox_reg_write(FIREFOX_ENET_E0IMR, (eth_int_mask | 0x00000008) );
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  eth_int_mask = libertas_reg_read(LIBERTAS_ENET_E0IMR); /* Enhance to shadow this register */
  libertas_reg_write(LIBERTAS_ENET_E0IMR, (eth_int_mask | 0x00000008) );
#endif
  return;
}


/*
 * ----------------------------------------------------------------------------
 */
void
mvFF_eth_int_handler(s32 irq, void * unused, struct pt_regs *regs)
{
  u32 eth_int_cause;
  u32 eth_int_mask;
  
#ifdef CONFIG_ARCH_FIREFOX
  eth_int_cause = firefox_reg_read(FIREFOX_ENET_E0ICR);
  eth_int_mask = firefox_reg_read(FIREFOX_ENET_E0IMR); /* Enhance to shadow this register */
#endif
#ifdef CONFIG_ARCH_LIBERTAS
  eth_int_cause = libertas_reg_read(LIBERTAS_ENET_E0ICR);
  eth_int_mask = libertas_reg_read(LIBERTAS_ENET_E0IMR); /* Enhance to shadow this register */
#endif

#ifdef ETH_DBG_INFO
  printk("mvFF_eth_int_handler: irq=%d, cause=0x%08x, mask=0x%08x\n",
	 irq, eth_int_cause, eth_int_mask);
#endif

  if (eth_int_cause & 0x8) {
    eth_int_mask &= (~0x00000008);
    eth_int_cause &= (~0x00000008);
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0IMR, eth_int_mask);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0IMR, eth_int_mask);
#endif
    tasklet_schedule(&txComplete);
    /* here we want to restart transmit queues because we have HW resource */
    /* we will simply start/stop all queues */

#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR, ~(u32) 0x8);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR, ~(u32) 0x8);
#endif
  }
  
  if (eth_int_cause & 0x1) {	/*RxBuffer, RxResource Error */
    /*
     * Rx Priority Return Buffer queue 0
     */
    /*  will be called from task context */
    /* mvFF_eth_receive_queue(NULL, 0, 32); */

    /*  unmask all interrupts - task will reopen */
    eth_int_mask &= (~0x00000001);
    eth_int_cause &= (~0x00000001);
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0IMR, eth_int_mask);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0IMR, eth_int_mask);
#endif
    tasklet_schedule(&rxComplete);

    /* moved this into the receive function */
    /*
     * start/continue ethernet 0 RX
     */
    /*
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0SDCMR + gap, (1 << 7));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0SDCMR + gap, (1 << 7));
#endif
    */

    /* 
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR + gap, (u32)~ 0x1); -- Moved to RX task 
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR + gap, (u32)~ 0x1); -- Moved to RX task 
#endif
    */

#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR, ~(u32) 0x1);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR, ~(u32) 0x1);
#endif
 }

#if 0
  
  if (eth_int_cause & 0x10000000) {	/* MII PHY status changed */
    u32 port_status;
#ifdef ETH_DBG_INFO
      printk(" MII PHY status interrupt\n");
#endif
    
    /*
     * Check Link status on Ethernet0
     */
#ifdef CONFIG_ARCH_FIREFOX
    port_status = firefox_reg_read(FIREFOX_ENET_E0PSR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    port_status = libertas_reg_read(LIBERTAS_ENET_E0PSR);
#endif
    if (!(port_status & 0x8)) {
      /* here we have link down for our MAC, this should not happen in our QD<-->MAC connection */
      panic("Phy link for MAC port is down!\n");
    } 
    eth_int_cause &= ~0x10000000;
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR, ~0x10000000);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR, ~0x10000000);
#endif
  }

  if (eth_int_cause & 0x20000000) {	/* MII SMI complete */
    eth_int_cause &= ~0x20000000;
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR, ~0x20000000);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR, ~0x20000000);
#endif
  }
  
  if (eth_int_cause & ~(1 << 31)) {
#ifdef ETH_DBG_ERROR
    printk("unhandled int %08x\n", eth_int_cause);
#endif /* ETH_DBG_ERROR */
#ifdef CONFIG_ARCH_FIREFOX
    firefox_reg_write(FIREFOX_ENET_E0ICR, 0);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
    libertas_reg_write(LIBERTAS_ENET_E0ICR, 0);
#endif
  }
#endif
  
  return;
}

/*********************************************************************/
/* MII/SMI interface                                                 */
/*********************************************************************/

/*****************************************************************************
*
* int etherReadMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int* value)
*
* Description
* This function will access the MII registers and will read the value of
* the MII register , and will retrieve the value in the pointer.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* Outputs
* value - pointer to unsigned int which will receive the value.
* Returns Value
* 1 if success.
* 0 if fail to make the assignment.
* Error types (and exceptions if exist)
*/

static int
etherReadMIIReg(unsigned int portNumber, unsigned int MIIReg,
		unsigned int *value)
{
	SMI_REG smiReg;
	unsigned int phyAddr;
	unsigned int timeOut = 1000;
	int i;
	
	phyAddr = PHY_ADD0 + portNumber;

	/* first check that it is not busy */
#ifdef CONFIG_ARCH_FIREFOX
	smiReg = firefox_reg_read(FIREFOX_ENET_ESMIR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
	smiReg = libertas_reg_read(LIBERTAS_ENET_ESMIR);
#endif
	while (smiReg & SMI_BUSY) {
		if (timeOut-- < 1) {
			printk("TimeOut Passed Phy is busy\n");
			return 0;
		}
		for (i = 0; i < 1000; i++) ;
#ifdef CONFIG_ARCH_FIREFOX
		smiReg = firefox_reg_read(FIREFOX_ENET_ESMIR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
		smiReg = libertas_reg_read(LIBERTAS_ENET_ESMIR);
#endif
	}
	/* not busy */

#ifdef CONFIG_ARCH_FIREFOX
	firefox_reg_write(FIREFOX_ENET_ESMIR,
		 (SMI_OP_CODE_BIT_READ << 26) | (MIIReg << 21) | (phyAddr <<
								  16));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
	libertas_reg_write(LIBERTAS_ENET_ESMIR,
		 (SMI_OP_CODE_BIT_READ << 26) | (MIIReg << 21) | (phyAddr <<
								  16));
#endif

	timeOut = 1000;		/* initialize the time out var again */

	for (i = 0; i < 1000; i++) ;
#ifdef CONFIG_ARCH_FIREFOX
	smiReg = firefox_reg_read(FIREFOX_ENET_ESMIR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
	smiReg = libertas_reg_read(LIBERTAS_ENET_ESMIR);
#endif
	while (!(smiReg & READ_VALID)) {
		for (i = 0; i < 1000; i++) ;
#ifdef CONFIG_ARCH_FIREFOX
		smiReg = firefox_reg_read(FIREFOX_ENET_ESMIR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
		smiReg = libertas_reg_read(LIBERTAS_ENET_ESMIR);
#endif
		if (timeOut-- < 1) {
			printk("TimeOut Passed Read is not valid\n");
			return 0;
		}
	}

	*value = (unsigned int) (smiReg & 0xffff);

	return 1;
}


/*****************************************************************************
* 
* int etherWriteMIIReg (unsigned int portNumber , unsigned int MIIReg,
* unsigned int value)
* 
* Description
* This function will access the MII registers and will write the value
* to the MII register.
* Inputs
* portNumber - one of the 2 possiable Ethernet ports (0-1).
* MIIReg - the MII register offset.
* value -the value that will be written.
* Outputs
* Returns Value
* 1 if success.
* 0 if fail to make the assignment.
* Error types (and exceptions if exist)
*/

static int
etherWriteMIIReg(unsigned int portNumber, unsigned int MIIReg,
		 unsigned int value)
{
	SMI_REG smiReg;
	unsigned int phyAddr;
	unsigned int timeOut = 10;	/* in 100MS units */
	int i;

	/* first check that it is not busy */
#ifdef CONFIG_ARCH_FIREFOX
	(unsigned int) smiReg = firefox_reg_read(FIREFOX_ENET_ESMIR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
	(unsigned int) smiReg = libertas_reg_read(LIBERTAS_ENET_ESMIR);
#endif
	if (smiReg & SMI_BUSY) {
		for (i = 0; i < 10000000; i++) ;
		do {
#ifdef CONFIG_ARCH_FIREFOX
			(unsigned int) smiReg = firefox_reg_read(FIREFOX_ENET_ESMIR);
#endif
#ifdef CONFIG_ARCH_LIBERTAS
			(unsigned int) smiReg = libertas_reg_read(LIBERTAS_ENET_ESMIR);
#endif
			if (timeOut-- < 1) {
				printk("TimeOut Passed Phy is busy\n");
				return 0;
			}
		} while (smiReg & SMI_BUSY);
	}
	/* not busy */

	phyAddr = PHY_ADD0 + portNumber;

	smiReg = 0;		/* make sure no garbage value in reserved bits */
	smiReg = smiReg | (phyAddr << 16) | (SMI_OP_CODE_BIT_WRITE << 26) |
	    (MIIReg << 21) | (value & 0xffff);

#ifdef CONFIG_ARCH_FIREFOX
	firefox_reg_write(FIREFOX_ENET_ESMIR, *((unsigned int *) &smiReg));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
	libertas_reg_write(LIBERTAS_ENET_ESMIR, *((unsigned int *) &smiReg));
#endif

	return (1);
}


/*********************************************************************/
/* port manipulation                                                 */
/*********************************************************************/

/*
 * GT_STATUS  mvFF_eth_remove_port_from_vlan 
 *
 * remove a port from a given vlan.
 * this function will remove the port from the vlan, update its trailer/header
 * if this is the last port in this vlan we will signal the link of this vid to be down
 * the interface will stop its RUNNING flag in this case
 *
 * Inputs:
 * port - the port number to remove.
 * vid  - the vid to remove the port from.
 * newPortsBitMask - the new bit mask of this vid after a logical removal of this port.
 *
 * Outputs:
 * GT_STATUS - operation status.
 */
static GT_STATUS  mvFF_eth_remove_port_from_vlan( unsigned int port, MV_UNM_VID vid, int newPortsBitMask) {
  if( vid < 1 || vid > mvUnmGetNumOfVlans() ) {
    ASSERT(0); /*  that's a bug */
    return GT_FAIL;
  }
  ASSERT(vid <= mvUnmGetNumOfVlans() );
  ASSERT(vid>0); /*  ypu cannot remove from vid zero */
  ASSERT(mvBindings[vid]->dev);
  
  /* if the new bit mask is zero it means we have left without any ports in this vlan,
   * and the link state should be updated (to DOWN)
   */	
  if( newPortsBitMask != 0 ) { /*  not last port in this vlan */
#if defined (HEADERS)
    mvBindings[vid]->header[1] = (unsigned char)newPortsBitMask;
#elif defined (TRAILERS)
    mvBindings[vid]->trailer[1] = (unsigned char)newPortsBitMask;
#else 
#error "no trailers and no headers?"
#endif
  }
  else { /*  last port in this vlan */
#if defined (HEADERS)
    mvBindings[vid]->header[0] = 0;
    mvBindings[vid]->header[1] = 0;
#elif defined (TRAILERS)
    mvBindings[vid]->trailer[0] = 0;
    mvBindings[vid]->trailer[1] = 0;
#else 
#error "no trailers and no headers?"
#endif
    netif_stop_queue(mvBindings[vid]->dev);
    (mvBindings[vid]->dev)->flags &= ~IFF_RUNNING;
    netif_carrier_off(mvBindings[vid]->dev);    
    mvBindings[vid]->boLinkUp = false;
  }
  return GT_OK;
}

/*
 * GT_STATUS  mvFF_eth_add_port_to_vlan 
 *
 * add a port to a given vlan.
 * this function will add the port to the vlan, update its trailer/header
 * if this is the first port in this vlan we will signal the link of this vid to be up
 * the interface will start and its RUNNING flag will be set in this case
 *
 * Inputs:
 * port - the port number to add.
 * vid  - the vid to add the port to.
 * newPortsBitMask - the new bit mask of this vid after a logical removal of this port.
 * numOfPorts - the number of ports in the vlan after a logical add of this (being added) port
 *
 * Outputs:
 * GT_STATUS - operation status.
 */
static GT_STATUS  mvFF_eth_add_port_to_vlan( unsigned int port, MV_UNM_VID vid, int newPortsBitMask, int numOfPorts) {
  ASSERT(vid);
  ASSERT(mvBindings[vid]->dev);

  if( vid < 1 || vid > mvUnmGetNumOfVlans() ) {
    ASSERT(0); /*  that's a bug */
    return GT_FAIL;
  }

  /* if number of ports is exactly one, then this port (being added now) is the fisrt
   * port in this vlan, and the link state should be updated
   */
  if(numOfPorts != 1 ) { /*  not the first port, just update trailer/header */
#if defined (HEADERS)
    mvBindings[vid]->header[1] = (unsigned char)newPortsBitMask;
#elif defined (TRAILERS)
    mvBindings[vid]->trailer[1] = (unsigned char)newPortsBitMask;
#else
#error "no trailers and no headers?"
#endif
  }
  else { /*  first port in this vlan */
#if defined (HEADERS)
    mvBindings[vid]->header[0] = (vid - 1) << 4;
    mvBindings[vid]->header[1] = (unsigned char)newPortsBitMask;
#elif defined (TRAILERS)
    mvBindings[vid]->trailer[0] = (1<<7); /*  INGRESS_TRAILER_OVERRIDE */
    mvBindings[vid]->trailer[1] = (unsigned char)newPortsBitMask;
#else 
#error "no trailers and no headers?"
#endif   
    mvBindings[vid]->boLinkUp = true;
    netif_carrier_on(mvBindings[vid]->dev);  
    netif_start_queue(mvBindings[vid]->dev);
    (mvBindings[vid]->dev)->flags |= IFF_RUNNING;
  }
  return GT_OK;
}

GT_STATUS mvDisassociatePort(int qdPort, int fromVlanId, int newPortsBitMask) {
  return mvFF_eth_remove_port_from_vlan(qdPort, fromVlanId, newPortsBitMask);
}
GT_STATUS mvAssociatePort(int qdPort, int toVlanId, int newPortsBitMask, int numOfPorts) {
  return mvFF_eth_add_port_to_vlan( qdPort, toVlanId, newPortsBitMask, numOfPorts);
}


/*********************************************************************/
/* module start                                                      */
/*********************************************************************/
extern int qdEntryPoint(void);
extern void qdExitPoint(void);
extern int qdModuleStart(void);


// Fulfill the "General note" at the beginning of the file.
// Code stripped from official bootloader @ D03C0B10
static void resetEth()
{
//	 ((volatile GT_U32)*((volatile GT_U32*)(0x80006050))) = 0x7FE83080;
//	 ((volatile GT_U32)*((volatile GT_U32*)(0x80006050))) = 0x7FE83080 + 0x8000;
	 ((volatile GT_U32)*((volatile GT_U32*)(0x80006050))) = 0x7FE87800;
	 ((volatile GT_U32)*((volatile GT_U32*)(0x80006050))) = 0x7FE83080;
	 ((volatile GT_U32)*((volatile GT_U32*)(0x80006050))) = 0x2072B080;



	 //ETHER0_SDMA_CONFIG_REG = SDCR_RETX_COUNT_15 | SDCR_BLMR_LITTLE | SDCR_BLMT_LITTLE | SDCR_RIFB | SDCR_BSZ_8x64
	 //((volatile GT_U32)*((volatile GT_U32*)(0x80008440))) = 0x32FC;		
	 //ETHER0_PORT_CONFIG_REG = ETH_PROMISCUOUS_MODE | REJECT_BROADCAST | PORT_ENABLE | ETH_HASH_SIZE_500B
	 //((volatile GT_U32)*((volatile GT_U32*)(0x80008400))) |= 0x1083;	
	 //ETHER0_PORT_CONFIG_EXT_REG = AUTO_NEG_DUPLEX_DIS | AUTO_NEG_FLOW_DIS | FLOW_CTRL_DISABLE | MFL_1536_BYTES | AUTO_NEG_SPEED_DIS
	 //((volatile GT_U32)*((volatile GT_U32*)(0x80008408))) = 0x85600;	
}

/* 
 * int mvFF_eth_start(void)
 *
 * the start function is to enable to split the real initialization of QD and networking 
 * from the module initialization (either threw kernel boot or threw insmod command).
 */
int mvFF_eth_start(void) {
  int cards = 0;
  int status;
  int iNumOfVlans,i;
  BINDING * pBinding;
  struct net_device * pNetDev = NULL;

  resetEth(); // Reset the eth hardware

  memset( mvBindings, 0, sizeof(mvBindings) );
  memset( &mvEthHw, 0, sizeof(mvEthHw) );


  if( qdModuleStart() != 0 ) {
    printk("Error in QD init\n");
  }

  if( (status=mvUnmInitialize()) != GT_OK ) {
    printk("Error - NetConfig is invalid, can't start network device\n");
    return -ENODEV;
  }

#ifdef ETH_DBG_INFO
  printk("mvFF_eth_start UNM is initialized\n");
#endif
  
  mvEthHw.hwState = HW_UNKNOWN;

  iNumOfVlans = mvUnmGetNumOfVlans();
#ifdef ETH_DBG_INFO
  printk ("mvFF init: iNumOfVlans = %d\n", iNumOfVlans);
  mv_nc_printConf();
#endif
  for( i = 1 ; i <= iNumOfVlans ; i++) 
  {
    pBinding = (BINDING*) kmalloc(sizeof (BINDING), GFP_KERNEL);
    
    pNetDev = (struct net_device *) kmalloc(sizeof (struct net_device), GFP_KERNEL); 
    if (!pNetDev || !pBinding ) {
      panic("mvFF_eth_module_init : kmalloc failed\n");
    }
    memset( pBinding, 0, sizeof(BINDING) );
    memset( pNetDev, 0, sizeof(struct net_device) );
    /* initialize structs */
    mvBindings[i] = pBinding;
    pBinding->dev = pNetDev;
    pBinding->pmvEthHw = &mvEthHw;
   
    pNetDev->base_addr = PLATFORM_GTREG_BASE + FAST_ETH_MAC_OFFSET;
    pNetDev->init = mvFF_eth_init;
    
    
    if (register_netdev(mvBindings[i]->dev) == 0) {
      cards++;
    }
    else {
      status = -1;
      break;
    }
  }

  /* CleanUP - deregister net devices */
  if(status != 0 )
  {
    panic("mvFF_eth_module_init : bad status\n");
    for( i = 1 ; i <= cards+1 ; i++) {
      if(pNetDev)
		kfree(pNetDev);
      if(mvBindings[i])
		kfree(mvBindings[i]);
    } 
    memset( mvBindings, 0, sizeof(mvBindings) ); 
    cards = 0;
  }
#ifdef ETH_DBG_INFO
  printk("mvFF_eth module inited with %d devices\n",cards);
#endif
  return cards > 0 ? 0 : -ENODEV;
}


/*********************************************************************/
/* module initialization and deinitialization                        */
/*********************************************************************/
int __init mvFF_eth_module_init(void) {

#ifdef ETH_DBG_INFO
  printk("mvFF_eth module init\n");
#endif
  /* init the QD hidden module */
  qdEntryPoint();
  return 0;
}

void
mvFF_eth_module_exit(void)
{
  /* exit for the QD hidden module */
  qdExitPoint();
#ifdef ETH_DBG_INFO
  printk("mvFF_eth module exited\n");
#endif

}

module_init(mvFF_eth_module_init);
module_exit(mvFF_eth_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marvell");
MODULE_DESCRIPTION("Ethernet driver for Firefox/Libertas");
