// User-level IPC library routines

#include <env.h>
#include <lib.h>
#include <mmu.h>

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.
void ipc_send(u_int whom, u_int val, const void* srcva, u_int perm)
{
	int r;
	while ((r = syscall_ipc_try_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV)
	{
		syscall_yield();
	}
	user_assert(r == 0);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int ipc_recv(u_int* whom, void* dstva, u_int* perm)
{
	int r = syscall_ipc_recv(dstva);	// blocked until receive anything
	if (r != 0)
		user_panic("syscall_ipc_recv err: %d", r);

	if (whom)
		*whom = env->env_ipc_from;

	if (perm)
		*perm = env->env_ipc_perm;

	return env->env_ipc_value;
}

u_int get_time(u_int *us)
{
	u_int write = 1;
	panic_on(syscall_write_dev(&write,
				DEV_RTC_ADDRESS + DEV_RTC_TRIGGER_READ 
				sizeof(u_int)));
	u_int sec;
	u_int usec;
	panic_on(syscall_read_dev(&sec,
				DEV_RTC_ADDRESS + DEV_RTC_SEC,
				sizeof(u_int)));
	panic_on(syscall_read_dev(&usec,
				DEV_RTC_ADDRESS + DEV_RTC_USEC,
				sizeof(u_int)));
	if (us)
		*us = usec;
	return sec;
}

void usleep(u_int us)
{
	u_int old_sec, old_usec;
	u_int new_sec, new_usec;
	u_int elapsed;

	old_sec = get_time(&old_usec);
	for (; ; )
	{
		new_sec = get_time(&new_usec);
		if (new_usec < old_usec)
		{
			new_usec += 1000;
			new_sec--;
		}
		elapsed = (new_sec - old_sec) * 1000u + (new_usec - old_usec);
		if (elapsed < us)
			syscall_yield();
		else
			return;
	}
}

