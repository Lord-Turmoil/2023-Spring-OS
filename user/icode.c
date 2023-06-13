#include <lib.h>

int main()
{
	int fd, n, r;
	char buf[512 + 1];

	debugf("icode: open /etc/motd\n");
	if ((fd = open("/etc/motd", O_RDONLY)) < 0)
	{
		user_panic("icode: open /etc/motd: %d", fd);
	}

	debugf("icode: read /etc/motd\n");
	while ((n = read(fd, buf, sizeof buf - 1)) > 0)
	{
		buf[n] = 0;
		debugf("%s\n", buf);
	}

	debugf("icode: close /etc/motd\n");
	close(fd);

	debugf("icode: spawn /init\n");
	if ((r = spawnl("init.b", "init", "initarg1", "initarg2", NULL)) < 0)
	{
		user_panic("icode: spawn /init: %d", r);
	}

	debugf("icode: exiting\n");
	return 0;
}
