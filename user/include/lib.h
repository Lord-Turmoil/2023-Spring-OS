#ifndef LIB_H
#define LIB_H
#include <args.h>
#include <env.h>
#include <fd.h>
#include <mmu.h>
#include <pmap.h>
#include <syscall.h>
#include <trap.h>

#define vpt ((volatile Pte *)UVPT)
#define vpd ((volatile Pde *)(UVPT + (PDX(UVPT) << PGSHIFT)))
#define envs ((volatile struct Env *)UENVS)
#define pages ((volatile struct Page *)UPAGES)

// libos
void exit(void) __attribute__((noreturn));

extern volatile struct Env* env;

#define USED(x) (void)(x)

// debugf
void debugf(const char* fmt, ...);

void _user_panic(const char*, int, const char*, ...) __attribute__((noreturn));
void _user_halt(const char*, int, const char*, ...) __attribute__((noreturn));

#define user_panic(...) _user_panic(__FILE__, __LINE__, __VA_ARGS__)
#define user_halt(...) _user_halt(__FILE__, __LINE__, __VA_ARGS__)

#undef panic_on
#define panic_on(expr)                                  \
	do {                                                \
		int __r = (expr);                               \
		if (__r != 0) {                                 \
			user_panic("'" #expr "' returned %d", __r); \
		}                                               \
	} while (0)

/// fork, spawn
int spawn(char* prog, char** argv);
int spawnl(char* prot, char* args, ...);
int fork(void);

/// syscalls
extern int msyscall(int, ...);

void syscall_putchar(int ch);
int syscall_print_cons(const void* str, u_int num);
u_int syscall_getenvid(void);
void syscall_yield(void);
int syscall_env_destroy(u_int envid);
int syscall_set_tlb_mod_entry(u_int envid, void (*func)(struct Trapframe*));
int syscall_mem_alloc(u_int envid, void* va, u_int perm);
int syscall_mem_map(u_int srcid, void* srcva, u_int dstid, void* dstva, u_int perm);
int syscall_mem_unmap(u_int envid, void* va);

__attribute__((always_inline)) inline static int syscall_exofork(void)
{
	/*
	 * fork will create a new process, and when it is called, a new child
	 * process will be created USING PARENT'S Trapframe! So when the child
	 * starts to run, it will start right here (actually in `msyscall`).
	 * Since we altered the return value of the new process to 0 (value of $v0),
	 * the child process will then return 0, instead of its process id returned
	 * in parent process.
	 */
	return msyscall(SYS_exofork, 0, 0, 0, 0, 0);
}

int syscall_set_env_status(u_int envid, u_int status);
int syscall_set_trapframe(u_int envid, struct Trapframe* tf);
void syscall_panic(const char* msg) __attribute__((noreturn));
int syscall_ipc_try_send(u_int envid, u_int value, const void* srcva, u_int perm);
int syscall_ipc_recv(void* dstva);
int syscall_cgetc();
int syscall_write_dev(void*, u_int, u_int);
int syscall_read_dev(void*, u_int, u_int);
int syscall_getch(void);
void syscall_get_pwd(char* path);
void syscall_set_pwd(const char* path);

// ipc.c
void ipc_send(u_int whom, u_int val, const void* srcva, u_int perm);
u_int ipc_recv(u_int* whom, void* dstva, u_int* perm);

u_int get_time(u_int* us);
void usleep(u_int us);
void sleep(u_int ms);

// wait.c
void wait(u_int envid);

// console.c
int opencons(void);
int iscons(int fdnum);

// pipe.c
int pipe(int pfd[2]);
int pipe_is_closed(int fdnum);

// pageref.c
int pageref(void*);

// printf.c
int printf(const char* fmt, ...);
int fprintf(int fd, const char* fmt, ...);
int sprintf(char* buffer, const char* fmt, ...);
int printfc(int color, const char* fmt, ...);

/********************************************************************
** Console colors. Adapted from PassBash.
*/
#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7

#define FOREGROUND(COLOR) ((COLOR) + 30)
#define BACKGROUND(COLOR) ((COLOR) + 40)

#define FOREGROUND_INTENSE(COLOR) (FOREGROUND(COLOR) + 60)
#define BACKGROUND_INTENSE(COLOR) (BACKGROUND(COLOR) + 60)

#define MSG_COLOR   FOREGROUND(YELLOW)
#define ERROR_COLOR FOREGROUND_INTENSE(RED)


// fsipc.c
int fsipc_open(const char*, u_int, struct Fd*);
int fsipc_creat(const char*, u_int);
int fsipc_fullpath(const char*, char*);
int fsipc_map(u_int, u_int, void*);
int fsipc_set_size(u_int, u_int);
int fsipc_close(u_int);
int fsipc_dirty(u_int, u_int);
int fsipc_remove(const char*);
int fsipc_sync(void);
int fsipc_incref(u_int);

// fd.c
int close(int fd);
int read(int fd, void* buf, u_int nbytes);
int write(int fd, const void* buf, u_int nbytes);
int seek(int fd, u_int offset);
int ftell(int fd);
void close_all(void);
int readn(int fd, void* buf, u_int nbytes);
int dup(int oldfd, int newfd);
int dup1(int oldfdnum);
int fstat(int fdnum, struct Stat* stat);
int stat(const char* path, struct Stat* stat);
int readline(int fd, char* buffer);

// file.c
int open(const char* path, int mode);
int creat(const char* path, int mode);
int fullpath(const char* filename, char* path);
int read_map(int fd, u_int offset, void** blk);
int remove(const char* path);
int ftruncate(int fd, u_int size);
int sync(void);
int basename(const char* path, char* basename);
int parentpath(const char* path, char* parent);

#define user_assert(x)                                                                             \
	do {                                                                                       \
		if (!(x))                                                                          \
			user_panic("assertion failed: %s", #x);                                    \
	} while (0)

// File open modes
#define O_RDONLY  0x0000  /* open for reading only */
#define O_WRONLY  0x0001  /* open for writing only */
#define O_RDWR    0x0002  /* open for reading and writing */
#define O_ACCMODE 0x0003  /* mask for above modes */

// Unimplemented open modes
#define O_CREAT  0x0100  /* create if nonexistent */
#define O_TRUNC  0x0200  /* truncate to zero length */
#define O_APPEND 0x0400  /* open for append writing or reading? */
#define O_EXCL   0x0800  /* error if already exists */
#define O_MKDIR  0x1000  /* create directory, not regular file */

// lib.c

int access(const char* path, int type);
void getcwd(char* path);
int chdir(const char* path);
int isdir(const char* path);
int isreg(const char* path);

int atoi(const char* str);

int is_null_or_empty(const char* str);
int is_no_content(const char* str);

int is_the_same(const char* str1, const char* str2);

int is_begins_with(const char* str, const char* prefix);
int is_ends_with(const char* str, const char* suffix);

int execl(const char* path, ...);
int execv(const char* path, char* argv[]);

int profile(char* username, char* path, int create);

#endif
