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
	u_int r;	// r is dangerous

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
		// Macro replacement will case name conflict.
		do
		{
			int r = (syscall_read_dev(&r, 0x13000000 + 0x0030, sizeof(r))); if (r != 0)
			{
				_user_panic("E:\\Program\\BUAA\\Operating System\\OS\\21371300\\fs\\ide.c", 58, "'" "syscall_read_dev(&r, DEV_DISK_ADDRESS + DEV_DISK_STATUS, sizeof(r))" "' returned %d", r);
			}
		} while (0);

		if (r == 0)	// failed	
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
