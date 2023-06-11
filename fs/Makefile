user_dir    := ../user
tools_dir   := ../tools
INCLUDES    := -I../include -I$(user_dir)/include

include $(user_dir)/include.mk
USERLIB     := $(addprefix $(user_dir)/, $(USERLIB))
USERAPPS    := $(addprefix $(user_dir)/, $(USERAPPS))

FSLIB       := fs.o ide.o

.PRECIOUS: %.b %.b.c
%.x: %.b.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p fs

%.b: %.o $(USERLIB) $(FSLIB)
	$(LD) -o $@ $(LDFLAGS) -T $(user_dir)/user.lds $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: $(user_dir)/lib.h

.PHONY: all clean image

all: serv.x check.x $(FSLIB)

clean:
	rm -rf *~ *.o *.b.c *.b *.x


# FSIMGFILES  := rootfs/motd rootfs/newmotd $(USERAPPS) $(fs-files)

FSIMGFILES :=	rootfs/init.b  \
				rootfs/motd    \
				rootfs/newmotd \
				rootfs/home/   \
				rootfs/bin/    \
				$(fs-files)

image: $(tools_dir)/fsformat
	dd if=/dev/zero of=../target/fs.img bs=4096 count=1024 2>/dev/null
	# using awk to remove paths with identical basename from FSIMGFILES
	cp -vf $(USERAPPS) -t rootfs/bin/
	mv -vf rootfs/bin/init.b rootfs/init.b

	# $(tools_dir)/fsformat ../target/fs.img \
	#	$$(printf '%s\n' $(FSIMGFILES) | awk -F/ '{ ns[$$NF]=$$0 } END { for (n in ns) { print ns[n] } }')
	$(tools_dir)/fsformat ../target/fs.img $$(printf '%s\n' $(FSIMGFILES))
