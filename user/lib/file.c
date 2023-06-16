#include <fs.h>
#include <lib.h>

#define debug 0

static int file_close(struct Fd* fd);
static int file_read(struct Fd* fd, void* buf, u_int n, u_int offset);
static int file_write(struct Fd* fd, const void* buf, u_int n, u_int offset);
static int file_stat(struct Fd* fd, struct Stat* stat);

// Dot represents choosing the member within the struct declaration
// to initialize, with no need to consider the order of members.
struct Dev devfile = {
	.dev_id = 'f',
	.dev_name = "file",
	.dev_read = file_read,
	.dev_write = file_write,
	.dev_close = file_close,
	.dev_stat = file_stat,
};

static void _make_fullpath(const char* filename, char* path)
{
	*path = '\0';

	while (*filename && (*filename == ' '))
		filename++;

	if (filename[0] != '/')	// relative
	{
		getcwd(path);
		if (filename[0] == '~')	 // home dir
		{
			if (profile(NULL, path, 0) != 0)
				strcpy(path, "~/");
			filename++;
			while (*filename && (*filename == '/'))
				filename++;
		}
		if (!is_ends_with(path, "/"))
			strcat(path, "/");
	}

	strcat(path, filename);
}

// Overview:
//  Open a file (or directory).
//
// Returns:
//  the file descriptor on success,
//  the underlying error on failure.
int open(const char* path, int mode)
{
	int r;

	// Step 1: Alloc a new 'Fd' using 'fd_alloc' in fd.c.
	// Hint: return the error code if failed.
	/* Exercise 5.9: Your code here. (1/5) */
	struct Fd* fd;
	if ((r = fd_alloc(&fd)) < 0)
		return r;

	// Step 2: Prepare the 'fd' using 'fsipc_open' in fsipc.c.
	/* Exercise 5.9: Your code here. (2/5) */
	/*
	 * In this function, current process will send open fd request to
	 * our file system process, and file system will create such fd for us
	 * and send a page of fd to this fd.
	 */
	char dir[MAXPATHLEN] = { '\0' };
	_make_fullpath(path, dir);

	// debugf("open: %s\n", dir);

	if ((r = fsipc_open(dir, mode, fd)) < 0)
	{
		if (mode & O_CREAT)
			creat(dir, FTYPE_REG);
		else
			return r;

		r = fsipc_open(dir, mode, fd);
		if (r < 0)
			return r;
	}

	// Step 3: Set 'va' to the address of the page where the 'fd''s data is cached,
	// using 'fd2data'.
	// Set 'size' and 'fileid' correctly with the value in 'fd' as a 'Filefd'.
	/* Exercise 5.9: Your code here. (3/5) */
	char* va = fd2data(fd);
	struct Filefd* ffd = (struct Filefd*)fd;
	u_int fileid = ffd->f_fileid;
	u_int size = ffd->f_file.f_size;

	// Step 4: Alloc pages and map the file content using 'fsipc_map'.
	for (int i = 0; i < size; i += BY2PG)
	{
		/* Exercise 5.9: Your code here. (4/5) */
		if ((r = fsipc_map(fileid, i, va + i)) < 0)
			return r;
	}

	int fdnum = fd2num(fd);
	if (mode & O_APPEND)
		seek(fdnum, size);
	else if (mode & O_WRONLY) // clear file content
		ftruncate(fdnum, 0);

	// Step 5: Return the number of file descriptor using 'fd2num'.
	/* Exercise 5.9: Your code here. (5/5) */
	return fdnum;
}

int creat(const char* path, int mode)
{
	char dir[MAXPATHLEN] = { '\0' };
	_make_fullpath(path, dir);

	return fsipc_creat(dir, (u_int)mode);
}

int fullpath(const char* filename, char* path)
{
	char dir[MAXPATHLEN] = { '\0' };
	_make_fullpath(filename, dir);

	return fsipc_fullpath(dir, path);
}


// Overview:
//  Close a file descriptor
int file_close(struct Fd* fd)
{
	int r;

	struct Filefd* ffd = (struct Filefd*)fd;
	u_int fileid = ffd->f_fileid;
	u_int size = ffd->f_file.f_size;

	// Set the start address storing the file's content.
	void* va = fd2data(fd);

	// Tell the file server the dirty page.
	for (u_int i = 0; i < size; i += BY2PG)
		fsipc_dirty(fileid, i);

	// Request the file server to close the file with fsipc.
	if ((r = fsipc_close(fileid)) < 0)
	{
		debugf("cannot close the file\n");
		return r;
	}

	// Unmap the content of file, release memory.
	if (size == 0)
		return 0;

	for (u_int i = 0; i < size; i += BY2PG)
	{
		if ((r = syscall_mem_unmap(0, (void*)(va + i))) < 0)
		{
			debugf("cannont unmap the file.\n");
			return r;
		}
	}

	return 0;
}

// Overview:
//  Read 'n' bytes from 'fd' at the current seek position into 'buf'. Since files
//  are memory-mapped, this amounts to a memcpy() surrounded by a little red
//  tape to handle the file size and seek pointer.
static int file_read(struct Fd* fd, void* buf, u_int n, u_int offset)
{
	struct Filefd* f = (struct Filefd*)fd;
	u_int size = f->f_file.f_size;

	// Avoid reading past the end of file.
	if (offset > size)
		return 0;
	if (offset + n > size)
		n = size - offset;

	memcpy(buf, (char*)fd2data(fd) + offset, n);

	return n;
}

// Overview:
//  Find the virtual address of the page that maps the file block
//  starting at 'offset'.
int read_map(int fdnum, u_int offset, void** blk)
{
	int r;
	void* va;
	struct Fd* fd;

	if ((r = fd_lookup(fdnum, &fd)) < 0)
		return r;

	if (fd->fd_dev_id != devfile.dev_id)
		return -E_INVAL;

	va = fd2data(fd) + offset;

	if (offset >= MAXFILESIZE)
	{
		return -E_NO_DISK;
	}

	if (!(vpd[PDX(va)] & PTE_V) || !(vpt[VPN(va)] & PTE_V))
	{
		return -E_NO_DISK;
	}

	*blk = (void*)va;
	return 0;
}

// Overview:
//  Write 'n' bytes from 'buf' to 'fd' at the current seek position.
static int file_write(struct Fd* fd, const void* buf, u_int n, u_int offset)
{
	int r;

	struct Filefd* f = (struct Filefd*)fd;
	u_int tot = offset + n;

	// Don't write more than the maximum file size.
	if (tot > MAXFILESIZE)
		return -E_NO_DISK;

	// Increase the file's size if necessary
	if (tot > f->f_file.f_size)
	{
		if ((r = ftruncate(fd2num(fd), tot)) < 0)
			return r;
	}

	// Write the data
	memcpy((char*)fd2data(fd) + offset, buf, n);
	return n;
}

static int file_stat(struct Fd* fd, struct Stat* st)
{
	struct Filefd* f = (struct Filefd*)fd;

	strcpy(st->st_name, f->f_file.f_name);
	st->st_size = f->f_file.f_size;
	st->st_isdir = f->f_file.f_type == FTYPE_DIR;

	return 0;
}

// Overview:
//  Truncate or extend an open file to 'size' bytes
int ftruncate(int fdnum, u_int size)
{
	if (size > MAXFILESIZE)
		return -E_NO_DISK;

	int r;
	struct Fd* fd;
	if ((r = fd_lookup(fdnum, &fd)) < 0)
		return r;

	if (fd->fd_dev_id != devfile.dev_id)
		return -E_INVAL;

	struct Filefd* f = (struct Filefd*)fd;
	u_int fileid = f->f_fileid;
	u_int oldsize = f->f_file.f_size;
	f->f_file.f_size = size;

	if ((r = fsipc_set_size(fileid, size)) < 0)
		return r;

	void* va = fd2data(fd);

	// Map any new pages needed if extending the file
	for (u_int i = ROUND(oldsize, BY2PG); i < ROUND(size, BY2PG); i += BY2PG)
	{
		if ((r = fsipc_map(fileid, i, va + i)) < 0)
		{
			fsipc_set_size(fileid, oldsize);
			return r;
		}
	}

	// Unmap pages if truncating the file
	for (u_int i = ROUND(size, BY2PG); i < ROUND(oldsize, BY2PG); i += BY2PG)
	{
		if ((r = syscall_mem_unmap(0, (void*)(va + i))) < 0)
			user_panic("ftruncate: syscall_mem_unmap %08x: %e", va + i, r);
	}

	return 0;
}

// Overview:
//  Delete a file or directory.
int remove(const char* path)
{
	// Your code here.
	// Call fsipc_remove.
	char dir[MAXPATHLEN] = { '\0' };
	_make_fullpath(path, dir);

	/* Exercise 5.13: Your code here. */
	return fsipc_remove(dir);
}

// Overview:
//  Synchronize disk with buffer cache
int sync(void)
{
	return fsipc_sync();
}

int basename(const char* path, char* basename)
{
	const char* p = path + (strlen(path) - 1);
	while ((p > path) && (*p == '/'))
		p--;
	while ((p > path) && (*p != '/'))
		p--;

	if (*p == '/')
		strcpy(basename, p + 1);
	else
		strcpy(basename, p);
	strstrip(basename, '/');

	if (basename[0] == '\0')
		strcpy(basename, "/");

	return 0;
}

// will not reserve last '/'
int parentpath(const char* path, char* parent)
{
	char dir[MAXPATHLEN];

	strcpy(dir, path);
	strstripr(dir, '/');
	if (dir[0] == '\0')
	{
		strcpy(parent, "/");
		return 0;
	}

	char* p = dir + strlen(dir);
	while ((p > dir) && (*p != '/'))
		p--;
	if (*p == '/')
	{
		if (p == dir)
			strcpy(parent, "/");
		else
		{
			*p = '\0';
			strcpy(parent, dir);
		}
	}
	else
		strcpy(parent, "");

	return 0;
}
