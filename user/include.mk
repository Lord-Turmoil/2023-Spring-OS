lab-ge = $(shell [ "$$(echo $(lab)_ | cut -f1 -d_)" -ge $(1) ] && echo true)

INITAPPS             := tltest.x fktest.x pingpong.x

USERLIB              := entry.o \
			syscall_wrap.o \
			debugf.o \
			libos.o \
			fork.o \
			syscall_lib.o \
			ipc.o \
			conio.o \
			ctype.o \
			arguments.o

ifeq ($(call lab-ge,5), true)
	INITAPPS     += devtst.x fstest.x
	USERLIB      += fd.o \
			pageref.o \
			file.o \
			fsipc.o \
			console.o \
			printf.o
endif

ifeq ($(call lab-ge,6), true)
	INITAPPS     += icode.x

	USERLIB      += wait.o spawn.o pipe.o
	USERAPPS     := num.b  \
			echo.b \
			halt.b \
			ls.b \
			sh.b  \
			cat.b \
			testarg.b \
			pingpong.b \
			init.b \
			testconio.b \
			tokentest.b \
			pash.b \
			ampersand.b
endif

USERLIB      += lib.o
USERLIB := $(addprefix lib/, $(USERLIB)) $(wildcard ../lib/*.o)
