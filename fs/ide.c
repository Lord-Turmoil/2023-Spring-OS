/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

 // Overview:
 //  read data from IDE disk. First issue a read request through
 //  disk register and then copy data from disk buffer
 //  (512 bytes, a sector) to destination array.
 //
 // Parameters:
 //  diskno: disk number.
 //  secno: start sector number.
 //  dst: destination for data read from IDE disk.
 //  nsecs: the number of sectors to read.
 //
 // Post-Condition:
 //  Panic if any error occurs. (you may want to use 'panic_on')
 //
 // Hint: Use syscalls to access device registers and buffers.
 // Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
 //  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
 //  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void* dst, u_int nsecs)
{
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	u_int read_flag = DEV_DISK_OPERATION_READ;
	u_int ret;

	for (u_int offset = 0; begin + offset < end; offset += BY2SECT)
	{
		/* Exercise 5.3: Your code here. (1/2) */
		u_int dev_offset = begin + offset;

		// Set IDE disk ID.
		panic_on(syscall_write_dev(&diskno,
								   DEV_DISK_ADDRESS + DEV_DISK_ID,
								   sizeof(diskno)));
		// Set offset from begin.
		panic_on(syscall_write_dev(&dev_offset,
								   DEV_DISK_ADDRESS + DEV_DISK_OFFSET,
								   sizeof(dev_offset)));

		// Set disk to read.
		panic_on(syscall_write_dev(&read_flag,
								   DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION,
								   sizeof(read_flag)));

		// Get disk return value (status).
		panic_on(syscall_read_dev(&ret,
								  DEV_DISK_ADDRESS + DEV_DISK_STATUS,
								  sizeof(ret)));
		if (ret == 0)	// failed
			user_panic("ide_read failed");	// not kernel 'panic'

		// Read data to device buffer.
		panic_on(syscall_read_dev(dst + offset,
								  DEV_DISK_ADDRESS + DEV_DISK_BUFFER,
								  BY2SECT));
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void* src, u_int nsecs)
{
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	u_int write_flag = DEV_DISK_OPERATION_WRITE;
	u_int ret;

	for (u_int offset = 0; begin + offset < end; offset += BY2SECT)
	{
		/* Exercise 5.3: Your code here. (2/2) */
		u_int dev_offset = begin + offset;

		// Set IDE disk ID.
		panic_on(syscall_write_dev(&diskno,
								   DEV_DISK_ADDRESS + DEV_DISK_ID,
								   sizeof(diskno)));
		// Set offset from begin.
		panic_on(syscall_write_dev(&dev_offset,
								   DEV_DISK_ADDRESS + DEV_DISK_OFFSET,
								   sizeof(dev_offset)));

		// Load data to device buffer.
		panic_on(syscall_write_dev(src + offset,
								   DEV_DISK_ADDRESS + DEV_DISK_BUFFER,
								   BY2SECT));
		// Set disk to write.
		panic_on(syscall_write_dev(&write_flag,
								   DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION,
								   sizeof(write_flag)));

		// Get disk return value (status).
		panic_on(syscall_read_dev(&ret,
								  DEV_DISK_ADDRESS + DEV_DISK_STATUS,
								  sizeof(ret)));

		if (ret == 0)	// failed
			user_panic("ide_write failed");
	}
}

//////////////////////////////////////////////////////////////
static u_int ssdmap[SSD_BLOCK_NUM];
static struct SSDBlock ssdblocks[SSD_BLOCK_NUM];
static const u_char DUMB_BLOCK[BY2SECT];
static u_char temp_block[BY2SECT];

static u_int INVALID_PNO = 0xFFFFFFFF;

void ssd_init()
{
	memset(ssdmap, 0xFF, sizeof(ssdmap));
	for (int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		ssdblocks[i].erase = 0;
		ssdblocks[i].writable = 1;
	}
}

int ssd_read(u_int logic_no, void* dst)
{
	if (ssdmap[logic_no] == INVALID_PNO)
		return -1;
	ide_read(0, ssdmap[logic_no], dst, 1);
	return 0;
}

static u_int _get_writable_block()
{
	int min_erase = 2147483647;
	u_int pno = INVALID_PNO;
	for (u_int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		if (!ssdblocks[i].writable)
			continue;
		if (ssdblocks[i].erase < min_erase)
		{
			min_erase = ssdblocks[i].erase;
			pno = i;
		}
	}

	return pno;
}

static u_int _get_non_writable_block()
{
	int min_erase = 2147483647;
	u_int pno = INVALID_PNO;
	for (u_int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		if (ssdblocks[i].writable)
			continue;
		if (ssdblocks[i].erase < min_erase)
		{
			min_erase = ssdblocks[i].erase;
			pno = i;
		}
	}

	return pno;
}

void ssd_write(u_int logic_no, void* src)
{
	if (ssdmap[logic_no] != INVALID_PNO)
		ssd_erase(logic_no);

	u_int pno = _get_writable_block();
	if (pno == INVALID_PNO)
		return;
	if (ssdblocks[pno].erase >= SSD_THRESHOLD)
	{
		u_int rpno = _get_non_writable_block();
		if (rpno == INVALID_PNO)
			return;
		ide_read(0, rpno, (void*)&temp_block, 1);
		ide_write(0, pno, (void*)&temp_block, 1);
		ssdblocks[pno].writable = 0;
		for (u_int i = 0; i < SSD_BLOCK_NUM; i++)
		{
			if (ssdmap[i] == rpno)
			{
				ssd_erase(i);
				ssdmap[i] = pno;
				break;
			}
		}
		pno = rpno;
	}

	ide_write(0, pno, src, 1);
	ssdmap[logic_no] = pno;
	ssdblocks[pno].writable = 0;
}

void ssd_erase(u_int logic_no)
{
	if (ssdmap[logic_no] == INVALID_PNO)
		return;

	u_int pno = ssdmap[logic_no];
	ide_write(0, pno, (void*)&DUMB_BLOCK, 1);

	ssdblocks[pno].erase++;
	ssdblocks[pno].writable = 1;

	ssdmap[logic_no] = INVALID_PNO;
}
