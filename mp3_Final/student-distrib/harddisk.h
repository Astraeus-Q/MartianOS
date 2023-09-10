#ifndef HARDDISK_H
#define HARDDISK_H

#include "types.h"

/* PIC specific */
#define HD_IRQ 0xE

/* primary ATA bus system config */
#define ATA_PRIMARY_IO_BASE      0x1F0
#define ATA_PRIMARY_CRTL_BASE    0x3F6
#define ATA_PRIMARY_IRQ          14

/* second ATA bus system config */
#define ATA_SECONDARY_IO_BASE    0x170
#define ATA_SECONDARY_CRTL_BASE  0x376
#define ATA_SECONDARY_IRQ        15

/* ATA IO config (both hold for primary bus and secondary bus) */
#define DATA_REGISTER_OFF           0
#define ERROR_REGISTER_OFF          1    /* read */
#define FEATURE_REGISTER_OFF        1  /* write */
#define SECTOR_COUNT_REGISTER_OFF   2
#define SECTOR_NUMBER_REGISTER_OFF  3
#define LBA_LOW_REGISTER_OFF        3
#define CYLINDER_LOW_REGISTER_OFF   4  /* sector address low */
#define LBA_MIDDLE_REGISTER_OFF     4
#define CYLINDER_HIGH_REGISTER_OFF  5 /* sector address high */
#define LBA_HIGH_REGISTER_OFF       5
#define DRIVE_REGISTER_OFF          6
#define STATUS_REGISTER_OFF         7   /* read */
#define COMMAND_REGISTER_OFF        7  /* write */

/* ATA CRTL config (both hold for primary bus and secondary bus) */
#define ALTERNATE_STATUS_REGISTER_OFF  0   /* read */
#define DEVICE_CRTL_REGISTER_OFF       0   /* write */
#define DRIVE_ADDR_REGISTER_OFF        1 

/* mask in the status register */
#define STATUS_BSY_MASK 0x80
#define STATUS_RDY_MASK 0x40
#define STATUS_DRQ_MASK 0x08

/* ATA command*/
#define WRITE_SECTORS 0x30
#define READ_SECTORS  0x20

/* macros for bit operations */
#define SET_BIT(val, pos) (val |= (1 << pos))
#define CLEAR_BIT(val, pos) (val &= ~(1 << pos))
#define CHECK_BIT(val, pos) ((val >> pos) & 0x1)

/* Basic parameters */
#define SECTOR_SIZE 512

/* error checking */
#define CHECK_ERROR() \
do {                  \
    if (1 == (inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & 0x1)) {                              \
        printf("ERR BIT set at function %s line %d\n", __FUNCTION__, __LINE__);                     \
        printf("The Error register has value %x\n", inb(ATA_PRIMARY_IO_BASE + ERROR_REGISTER_OFF)); \
        for (;;);                                                                                   \
    }                                                                                               \
}while(0)

extern int32_t Disk_SectorRead(uint16_t *buf, uint8_t sector_read, uint32_t LBA_28);

extern int32_t Disk_SectorWrite(uint16_t* buf, uint8_t sector_write, uint32_t LBA_28);

extern void do_hdIRQ();

extern int32_t Disk_init();

#endif
