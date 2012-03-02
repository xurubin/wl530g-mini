/*
 *  linux/include/asm-armnommu/arch-libertas/serial.h
 */
#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H

 /* Should be exported into a gloabal Libertas header file */
#define LIBERTAS_SERIAL_BASE_BAUD       (8000000 / 16)
#define LIBERTAS_SERIAL_0_BASE_ADDR     0x8000c840
#define LIBERTAS_SERIAL_0_IRQ           11 /* Bit 11 */

#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)

#define STD_SERIAL_PORT_DEFNS \
{ 0, LIBERTAS_SERIAL_BASE_BAUD, LIBERTAS_SERIAL_0_BASE_ADDR, LIBERTAS_SERIAL_0_IRQ, STD_COM_FLAGS, /* ttyS0 */\
    iomem_base: (u8 *) LIBERTAS_SERIAL_0_BASE_ADDR, /* IO mem base */ \
    iomem_reg_shift: 2, \
    io_type: SERIAL_IO_MEM },
#define RS_TABLE_SIZE 2
#define EXTRA_SERIAL_PORT_DEFNS

#endif
