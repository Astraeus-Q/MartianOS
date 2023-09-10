#include "harddisk.h"
#include "lib.h"


int32_t Disk_init() {
    // polling based, clear nINT bit
    outb(0x02, ATA_PRIMARY_CRTL_BASE + DEVICE_CRTL_REGISTER_OFF);

    // LBA Addressing mode
    outb(inb(ATA_PRIMARY_IO_BASE + DRIVE_REGISTER_OFF) | 1 << 6, ATA_PRIMARY_IO_BASE + DRIVE_REGISTER_OFF);
    CHECK_ERROR();
    return 0;
}

/* currently does not support interrupt, use polling instead ... */
void do_hdIRQ() {
    return;
}

/* This is the lowest level function, read a sector (512 bytes) given 28-bit LBA */
/* DOES NOT PERFORM ANY TYPES OF CHECKING! */
/* Reference: OsDev */
int32_t Disk_SectorRead(uint16_t *buf, uint8_t sector_read, uint32_t LBA_28)
{
    int sec, cur_word;

    // wait for the busy flag to be cleared
	while(inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & STATUS_BSY_MASK);

    // Send 0xE0 for the "master" ORed with the highest 4 bits of the LBA to port 0x1F6
	outb(0xE0 | ((LBA_28 >> 24) & 0xF), ATA_PRIMARY_IO_BASE + DRIVE_REGISTER_OFF);
    
    // Send the sectorcount to port 0x1F2
	outb(sector_read, ATA_PRIMARY_IO_BASE + SECTOR_COUNT_REGISTER_OFF);
    
    // Send the low 8 bits of the LBA to port 0x1F3
	outb(LBA_28 & 0xFF, ATA_PRIMARY_IO_BASE + LBA_LOW_REGISTER_OFF);

    // Send the next 8 bits of the LBA to port 0x1F4
	outb((LBA_28 >> 8) & 0xFF, ATA_PRIMARY_IO_BASE + LBA_MIDDLE_REGISTER_OFF);

    // Send the next 8 bits of the LBA to port 0x1F5
    outb((LBA_28 >> 16) & 0xFF, ATA_PRIMARY_IO_BASE + LBA_HIGH_REGISTER_OFF); 

    // send command "READ SECTORS" (0x20) to the Command port
	outb(READ_SECTORS, ATA_PRIMARY_IO_BASE + COMMAND_REGISTER_OFF); //Send the write command

	for (sec = 0; sec < sector_read; sec++) {

        // wait until busy flag is cleared
		while(inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & STATUS_BSY_MASK);
       
        // wait until DRQ flag is set (ready to transfer data)
        while (!(inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & STATUS_DRQ_MASK));
        
        // write one sector
        for (cur_word = 0; cur_word < SECTOR_SIZE / 2; cur_word++)
            buf[cur_word] = inw(ATA_PRIMARY_IO_BASE + DATA_REGISTER_OFF);   // Read 2B a time.
        
        buf += SECTOR_SIZE / 2;
	}

    CHECK_ERROR();
    return 0;
}

int32_t Disk_SectorWrite(uint16_t* buf, uint8_t sector_write, uint32_t LBA_28)
{
    int sec, cur_word;
    
    // wait for the busy flag to be cleared
	while(inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & STATUS_BSY_MASK);

    // Send 0xE0 for the "master" ORed with the highest 4 bits of the LBA to port 0x1F6
	outb(0xE0 | ((LBA_28 >> 24) & 0xF), ATA_PRIMARY_IO_BASE + DRIVE_REGISTER_OFF);
    
    // Send the sectorcount to port 0x1F2
	outb(sector_write, ATA_PRIMARY_IO_BASE + SECTOR_COUNT_REGISTER_OFF);

    // Send the low 8 bits of the LBA to port 0x1F3
	outb(LBA_28 & 0xFF, ATA_PRIMARY_IO_BASE + LBA_LOW_REGISTER_OFF);

    // Send the next 8 bits of the LBA to port 0x1F4
	outb((LBA_28 >> 8) & 0xFF, ATA_PRIMARY_IO_BASE + LBA_MIDDLE_REGISTER_OFF);
	
    // Send the next 8 bits of the LBA to port 0x1F5
    outb((LBA_28 >> 16) & 0xFF, ATA_PRIMARY_IO_BASE + LBA_HIGH_REGISTER_OFF); 

    // send command "WRITE SECTORS" (0x30) to the Command port
	outb(WRITE_SECTORS, ATA_PRIMARY_IO_BASE + COMMAND_REGISTER_OFF); //Send the write command

	for (sec = 0; sec < sector_write; sec++) {
        
        // wait until busy flag is cleared
		while(inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & STATUS_BSY_MASK);

        // wait until DRQ flag is set (ready to transfer data)
        while(!(inb(ATA_PRIMARY_IO_BASE + STATUS_REGISTER_OFF) & STATUS_DRQ_MASK));

        // write one sector
        for (cur_word = 0; cur_word < SECTOR_SIZE / 2; cur_word++)
            outw(buf[cur_word], ATA_PRIMARY_IO_BASE + DATA_REGISTER_OFF); // Write 2B a time.
            
        
        buf += SECTOR_SIZE / 2;
	}
    // cache flush
    outb(0xE7, ATA_PRIMARY_IO_BASE + COMMAND_REGISTER_OFF);

    CHECK_ERROR();
    return 0;
}

void fluch_block(int32_t LBA){
    int8_t buf[SECTOR_SIZE];
    memset(buf, 0, SECTOR_SIZE);
    Disk_SectorRead((uint16_t *)buf, 1, LBA);
    return;
}
