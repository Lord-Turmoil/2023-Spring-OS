#include <lib.h>

struct
{
	char msg1[5000];
	char msg2[1000];
} data = { "this is initialized data", "so is this" };

char bss[6000];

static int sum(char* s, int n);
static int self_diagnose();

int main(int argc, char** argv)
{
	int r;

	debugf("init: running\n");

	self_diagnose();

	debugf("init: args:");
	for (int i = 0; i < argc; i++)
	{
		debugf(" '%s'", argv[i]);
	}
	debugf("\n");

	debugf("init: running sh\n");

	// stdin should be 0, because no file descriptors are open yet
	if ((r = opencons()) != 0)
		user_panic("opencons: %d", r);
	// stdout
	if ((r = dup(0, 1)) < 0)
		user_panic("dup: %d", r);

	char username[64];
	char home[MAXPATHLEN];

	strcpy(username, "mos");
	strcpy(home, "/home/mos");
	panic_on(profile(username, home, 1));	// create if not exists

	debugf("\nUsername: %s\n", username);
	debugf("Home Dir: %s\n\n", home);
	panic_on(chdir(home));

	char pwd[MAXPATHLEN];
	getcwd(pwd);
	if (!is_the_same(home, pwd))
	{
		user_panic("Present directory error: %s\n", pwd);
	}

	while (1)
	{
		debugf("init: starting sh\n");
		// r = spawnl("sh.b", "sh", NULL);
		r = spawnl("/bin/pash.b", "pash", "-i", NULL);
		if (r < 0)
		{
			debugf("init: spawn sh: %d\n", r);
			return r;
		}
		wait(r);
	}
}

static int sum(char* s, int n)
{
	int i, tot;

	tot = 0;
	for (i = 0; i < n; i++)
	{
		tot ^= i * s[i];
	}
	return tot;
}

static int self_diagnose()
{
	int x, want;

	want = 0xf989e;
	if ((x = sum((char*)&data, sizeof data)) != want)
	{
		debugf("init: data is not initialized: got sum %08x wanted %08x\n", x, want);
	}
	else
	{
		debugf("init: data seems okay\n");
	}
	if ((x = sum(bss, sizeof bss)) != 0)
	{
		debugf("bss is not initialized: wanted sum 0 got %08x\n", x);
	}
	else
	{
		debugf("init: bss seems okay\n");
	}

	return 0;
}