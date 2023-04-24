// User-level IPC library routines

#include <env.h>
#include <lib.h>
#include <mmu.h>

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.
void ipc_send(u_int whom, u_int val, const void *srcva, u_int perm) {
	int r;
	while ((r = syscall_ipc_try_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV) {
		syscall_yield();
	}
	user_assert(r == 0);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int ipc_recv(u_int *whom, void *dstva, u_int *perm) {
	int r = syscall_ipc_recv(dstva);
	if (r != 0) {
		user_panic("syscall_ipc_recv err: %d", r);
	}

	if (whom) {
		*whom = env->env_ipc_from;
	}

	if (perm) {
		*perm = env->env_ipc_perm;
	}

	return env->env_ipc_value;
}

static int is_parent(u_int parent_id, u_int e_id)
{
	struct Env* e = &envs[ENVX(e_id)];

	if (e->env_status == ENV_FREE)
		return 0;
	if (e->env_id != e_id)
		return 0;
	if (e->env_id == parent_id)
		return 0;

	if (e->env_parent_id == parent_id)
		return 1;

	return is_parent(parent_id, e->env_parent_id);
}

void ipc_broadcast(u_int val, void * srcva, u_int perm)
{
	for (int i = 0; i < NENV; i++)
	{
		if (envs[i].env_status == ENV_FREE)
			continue;
		if (is_parent(env->env_id, envs[i].env_id))
			ipc_send(envs[i].env_id, val, (const void*)srcva, perm);
	}
}

