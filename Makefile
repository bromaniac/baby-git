# Makefile to compile Baby-Git program in Unix-like systems.
#
# Synopsis:
#
# For Linux distributions and MSYS2:
#
# $ make
# $ make install
# $ make clean
#
# For FreeBSD:
#
# $ gmake
# $ gmake install
# $ gmake clean

SHELL   = /bin/sh
INSTALL = install
prefix  = $(HOME)
bindir  = $(prefix)/bin

CC      = cc
CFLAGS  = -g -Wall -O3
LDLIBS  = -lcrypto -lz
RCOBJ   = read-cache.o
OBJS    = init-db.o update-cache.o write-tree.o commit-tree.o read-tree.o \
              cat-file.o show-diff.o 
PROGS  := $(subst .o,,$(OBJS))

ifeq ($(OS),Windows_NT)
    CFLAGS += -D BGIT_WINDOWS
else
    SYSTEM := $(shell uname -s)

    ifeq ($(SYSTEM),Linux)
        CFLAGS += -D BGIT_UNIX
    else
        ifeq ($(SYSTEM),FreeBSD)
            CFLAGS += -D BGIT_UNIX
        else
            ifeq ($(SYSTEM),Darwin)
                CFLAGS += -D BGIT_DARWIN
            endif
        endif
    endif
endif

OBJS   += $(RCOBJ)

.PHONY : all install clean backup test

all    : $(PROGS)

init-db      : init-db.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

update-cache : update-cache.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

write-tree   : write-tree.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

commit-tree  : commit-tree.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

read-tree    : read-tree.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

cat-file     : cat-file.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

show-diff    : show-diff.o $(RCOBJ)
	$(CC) $(CFLAGS) -o $@ $@.o $(RCOBJ) $(LDLIBS)

$(OBJS) : cache.h


install : $(PROGS)
	$(INSTALL) $(PROGS) $(bindir)

clean   :
	rm -f $(OBJS) $(PROGS)

backup  : clean
	cd ..; tar czvf babygit.tar.gz baby-git --exclude .git* \
	    --exclude .dircache --exclude temp_git_file*

test    :
	@echo "SYSTEM = $(SYSTEM)"
	@echo "CC = $(CC)"
	@echo "PROGS = $(PROGS)" 

