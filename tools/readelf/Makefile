# $< : first dependency
# $@ : target name
# $^ : all dependencies

%.o: %.c
	$(CC) -c $< -m32

.PHONY: clean

readelf: main.o readelf.o
	$(CC) $^ -o $@ -m32

hello: hello.c
	$(CC) $^ -o $@ -m32 -static -g

clean:
	rm -f *.o readelf hello
