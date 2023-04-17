#include <env.h>
#include <pmap.h>
#include <printk.h>

/* Overview:
 *   Implement a round-robin scheduling to select a runnable env and schedule it using 'env_run'.
 *
 * Post-Condition:
 *   If 'yield' is set (non-zero), 'curenv' should not be scheduled again unless it is the only
 *   runnable env.
 *
 * Hints:
 *   1. The variable 'count' used for counting slices should be defined as 'static'.
 *   2. Use variable 'env_sched_list', which contains and only contains all runnable envs.
 *   3. You shouldn't use any 'return' statement because this function is 'noreturn'.
 */
void schedule(int yield)
{
	static int count = 0; // remaining time slices of current env
	struct Env* e = curenv;
	static int user_time[5];

	struct Env* it;	// iterator

	int busy_user[5] = { 0, 0, 0, 0, 0 };
	TAILQ_FOREACH(it, &env_sched_list, env_sched_link)
	{
		busy_user[it->env_user] = 1;
	}

	count--;
	if (yield || (count <= 0) || !e || e->env_status != ENV_RUNNABLE)
	{
		if (e && e->env_status == ENV_RUNNABLE)
		{
			TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
			TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
			user_time[e->env_user] += e->env_pri;
		}
		panic_on(TAILQ_EMPTY(&env_sched_list));

		int u = -1;
		for (int i = 0; i < 5; i++)
		{
			if (!busy_user[i])
				continue;
			if (u == -1)
				u = i;
			else
			{
				if (user_time[i] < user_time[u])
					u = i;
			}
		}

		// Get first process of user 'u'
		TAILQ_FOREACH(it, &env_sched_list, env_sched_link)
		{
			if (it->env_user == u)
			{
				count = it->env_pri;
				e = it;
				break;
			}
		}
	}

	// count--;

	env_run(e);
}
