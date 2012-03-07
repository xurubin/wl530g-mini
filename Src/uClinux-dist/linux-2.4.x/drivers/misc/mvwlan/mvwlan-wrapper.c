#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

struct module __this_mvwlan = 
{
    size_of_struct : sizeof(struct module),
    name           :  "mvWLAN driver",
    size           : 150*1024,
    uc             : {ATOMIC_INIT(1)},
    flags          : MOD_RUNNING,
};

extern int init_mvwlan();

void ap_check_tx_rates()
{
    printk("mvwlan-wrapper: ap_check_tx_rates() called!\n");
}

static int __init mvwlan_init(void)
{
    init_mvwlan();
}

static int __init mvwlan_exit(void)
{
}

module_init(mvwlan_init);
module_exit(mvwlan_exit);

MODULE_LICENSE("Proprietary");
