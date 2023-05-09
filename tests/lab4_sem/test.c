#include <lib.h>

static void os_assert(int cond, const char *err) {
	if (!cond) {
		user_halt("%s\n", err);
	}
}

int main() {
	int items_id = sem_init("items", 5, 0);
	int lock_id = sem_init("lock", 0, 0);
	int r = fork();
	if (r < 0) {
		user_halt("OSTEST_FORK");
	}
	if (r == 0) {
		sem_wait(lock_id);
		os_assert(sem_getvalue(items_id) == 4, "WRONG_RETURN_VALUE_WAIT 1");
		os_assert(sem_getvalue(lock_id) == 0, "WRONG_RETURN_VALUE_WAIT 2");
		debugf("OSTEST_OK\n");
		return 0;
	} else {
		os_assert(sem_getvalue(items_id) == 5, "WRONG_RETURN_VALUE 3");
		os_assert(sem_getvalue(lock_id) == 0, "WRONG_RETURN_VALUE 4");

		sem_wait(items_id);

		os_assert(sem_getvalue(items_id) == 4, "WRONG_RETURN_VALUE_WAIT 5");
		os_assert(sem_getvalue(lock_id) == 0, "WRONG_RETURN_VALUE_WAIT 6");

		sem_post(lock_id);
		debugf("OSTEST_OK\n");
		return 0;
	}
}
