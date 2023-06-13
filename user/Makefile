INCLUDES    := -I../include -Iinclude

tools_dir   := ../tools

include include.mk

.PRECIOUS: %.o %.b %.b.c

# all must be the first target!!!
all: $(INITAPPS) $(USERBINS) $(USERAPPS) $(USERLIB)

%.x: %.b.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p user

%.b: %.o $(USERLIB)
	$(LD) -o $@ $(LDFLAGS) -T ./user.lds $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: lib.h

.PHONY: all clean

pash.o: ./include/pash.h

tokentest.o: ./include/pash.h

pash.b: pash.o ./lib/pashlib.o $(USERLIB)
	$(LD) -o $@ $(LDFLAGS) -T ./user.lds $^

tokentest.b: tokentest.o ./lib/pashlib.o $(USERLIB)
	$(LD) -o $@ $(LDFLAGS) -T ./user.lds $^

clean:
	rm -rf *~ *.o *.b.c *.x *.b lib/*.o
