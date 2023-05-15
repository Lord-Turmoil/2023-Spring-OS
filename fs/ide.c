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
struct SSDMap ssdmap[SSD_BLOCK_NUM];
struct SSDBlock ssdblocks[SSD_BLOCK_NUM];
u_char dumb[BY2SECT];

void ssd_init()
{
	memset(ssdmap, 0, sizeof(ssdmap));
	for (int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		ssdblocks[i].erase = 0;
		ssdblocks[i].writable = 1;
	}
}

int ssd_read(u_int logic_no, void *dst)
{
	if (!ssdmap[logic_no].valid)
		return -1;
	ide_read(0, ssdmap[logic_no].pno, dst, 1);
	return 0;
}

void ssd_write(u_int logic_no, void *src)
{
	if (ssdmap[logic_no].valid)
		ssd_erase(logic_no);
	// allocate
	int min_erase = 2147483647;
	int pno = -1;
	for (int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		if (!ssdblocks[i].writable)
			continue;
		if (ssdblocks[i].erase < min_erase)
		{
			min_erase = ssdblocks[i].erase;
			pno = i;
		}
	}
	if (pno == -1)
		return;
	if (min_erase < SSD_THRESHOLD)
	{
		ssdmap[logic_no].pno = pno;
		ssdmap[logic_no].valid = 1;
		ide_write(0, pno, src, 1);
		ssdblocks[pno].writable = 0;
		return;
	}
	
	// Here, pno -> A, rpno -> B
	u_int rpno = -1;	// replacement pno
	for (int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		if (ssdblocks[i].writable)
			continue;
		if (ssdblocks[i].erase < min_erase)
		{
			min_erase = ssdblocks[i].erase;
			rpno = i;
		}
	}
	if (rpno == -1)
		return;
	// write block B to block A
	ide_read(0, rpno, &dumb, 1);
	ide_write(0, pno, &dumb, 1);
	// ssdblocks[pno].erase++;
	ssdblocks[pno].writable = 0;
	// update map
	for (int i = 0; i < SSD_BLOCK_NUM; i++)
	{
		if (!ssdmap[i].valid)
			continue;
		if (ssdmap[i].pno == rpno)
		{
			ssdmap[i].pno = pno;
			break;
		}
	}
	
	// clear B
	ssd_erase(rpno);
	ide_write(0, rpno, src, 1);
	ssdmap[logic_no].pno = rpno;
	ssdmap[logic_no].valid = 1;
	ssdblocks[rpno].writable = 0;
}

void ssd_erase(u_int logic_no)
{
	if (!ssdmap[logic_no].valid)
		return;
	u_int pno = ssdmap[logic_no].pno;
	memset(&dumb, 0, BY2SECT);
	ide_write(0, pno, &dumb, 1);
	ssdblocks[pno].erase++;
	ssdblocks[pno].writable = 1;
}

