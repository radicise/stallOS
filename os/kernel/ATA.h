#ifndef __ATA_H__
#define __ATA_H__ 1
#include "types.h"
#define FAILMASK_ATA 0x00040000
#define ATA_SR_ERR 0x01
#define ATA_SR_IDX 0x02
#define ATA_SR_CORR 0x04
#define ATA_SR_DRQ 0x08
#define ATA_SR_DSC 0x10
#define ATA_SR_DWF 0x20
#define ATA_SR_DRDY 0x40
#define ATA_SR_BSY 0x80
struct ATA {
	u16 bus;
	unsigned char slaveLastUsed;
	unsigned char notUsed;
	Mutex inUse;// There is an operation
};
int ata_readBlock
int ata_PIO_transferSectors(unsigned long long blockAddr, void* dest, unsigned long long amnt, unsigned char write, struct ATAFile* ataDrive) {
	if (amnt > 256) {
		bugCheckNum(0x0002 | FAILMASK_ATA);// Amount of sectors is too large
	}
	unsigned long count = amnt;
	if (blockAddr & (~((unsigned long long) 0x0fffffff))) {

	}
	u32 LBA = blockAddr;
	struct ATA* ata = ataDrive->ata;
	unsigned char slave = ataDrive->slave;
	Mutex_acquire(&(ata->inUse));
	u16 bus = ata->bus;
	if (LBA & 0xf0000000) {
		bugCheckNum(0x0001 | FAILMASK_ATA);// Not within LBA28 range, do not assume that the drive supports LBA48
	}
	bus_out_u8(bus + 6, 0xe0 | (slave << 4) | (LBA >> 24));
	if ((ata->slaveLastUsed != slave) || ata->notUsed) {
		ata->notUsed = 0x00;
		ata->slaveLastUsed = slave;
		for (int i = 0; i < 14; i++) {// For delay, suggested
			bus_in_u8(bus + 7);
		}
	}	
	u8 status = bus_in_u8(bus + 7);
	if (status & ATA_SR_BSY) {
		kernelWarnMsg("ATA command block register was accessed when busy");// TODO What if it was set or cleared between the access and the status register read
		Mutex_release(&(ata->inUse));
		return (-1);
	}
	while (!(status & ATA_SR_DRDY)) {
		status = bus_in_u8(bus + 7);
	}
	bus_out_u8(bus + 1, 0x00);// TODO Is this necessary? Is this allowed?
	bus_out_u8(bus + 2, count);
	bus_out_u8(bus + 3, LBA);
	bus_out_u8(bus + 4, LBA >> 8);
	bus_out_u8(bus + 5, LBA >> 16);
	bus_out_u8(bus + 7, 0x20);
	for (int i = 0; i < 4; i++) {// For delay
		bus_in_u8(bus + 7);
	}
	status = bus_in_u8(bus + 7);
	if (status & ATA_SR_ERR) {
		kernelWarnMsgCode("ATA Drive: Error: ", bus_in_u8(bus + 1));
		Mutex_release(&(ata->inUse));
		return (-1);
	}
	if (status & ATA_SR_DWF) {
		kernelWarnMsg("ATA Drive: Drive Write Fault");
		Mutex_release(&(ata->inUse));
		return (-1);
	}
	while (!(status & ATA_SR_DRQ)) {
		bus_wait();
	}
	if (write) {
		// TODO write
		// TODO flush drive
	}
	else {
		// TODO read
	}
	while (1) {
		status = bus_in_u8(bus + 7);
		if (!(status & ATA_SR_BSY)) {
			break;
		}
	}
	Mutex_release(&(ata->inUse));
	return 0;
}
struct ATAFile {
	struct ATA* ata;
	unsigned char slave;
};
struct ATA ATA_0;
struct ATA ATA_1;
struct ATAFile ATA_0m;// hda
struct ATAFile ATA_0s;// hdb
struct ATAFile ATA_1m;// hdc
struct ATAFile ATA_1s;// hdd
void initATA(void) {
	ATA_0.bus = 0x01f0;
	ATA_1.bus = 0x0170;
	ATA_0.slaveLastUsed = 0x00;
	ATA_1.slaveLastUsed = 0x00;
	ATA_0.notUsed = 0x01;
	ATA_1.notUsed = 0x01;
	Mutex_initUnlocked(ATA_0.inUse);
	Mutex_initUnlocked(ATA_1.inUse);
	ATA_0m.ata = &ATA_0;
	ATA_0m.slave = 0x00;
	ATA_0s.ata = &ATA_0;
	ATA_0s.slave = 0x01;
	ATA_1m.ata = &ATA_1;
	ATA_1m.slave = 0x00;
	ATA_1s.ata = &ATA_1;
	ATA_1s.slave = 0x01;
}
struct ATA_BlockFile = {
ssize_t ATAFile_read(void* dat, size_t len, struct ATAFile* ata) {
	if (((ata->pos) | len) & 0x1ff) {
		// TODO support
		bugCheck();
	}
	
}
ssize_t ATA_read(int kfd, void* dat, size_t len) {
	struct ATAFile* file;
	switch (kfd) {
		case (2):
			file = &ATA_0m;
			break;
		case (3):
			file = &ATA_0s;
			break;
		case (4):
			file = &ATA_1m;
			break;
		case (5):
			file = &ATA_1s;
			break;
		default:
			bugCheckNum(0x0003 | FAILMASK_ATA);
	}
	return ATAFile_read(dat, len, file);
}
ssize_t ATA_write(int kfd, const void* dat, size_t len) {


}

#include "FileDriver.h"

#endif