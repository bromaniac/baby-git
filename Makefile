# Makefile to compile Baby-Git program in Unix-like systems.
#
# Synopsis:
#
# $ make
# $ make install
# $ make clean

SHELL   = /bin/sh
INSTALL = install
prefix  = $(HOME)
bindir  = $(prefix)/bin

CC      = cc
#DISTRO := $(shell uname -s)

#ifeq ( $(DISTRO), FreeBSD )
#    CC = clang
#endif

CFLAGS  = -g -Wall -O3
LDLIBS  = -lcrypto -lz
RCOBJ   = read-cache.o
OBJS    = init-db.o update-cache.o write-tree.o commit-tree.o read-tree.o \
              cat-file.o show-diff.o 
PROGS   = init-db update-cache write-tree commit-tree read-tree \
              cat-file show-diff 
#PROGS  := $(subst .o,,$(OBJS))
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
	cd .. ; tar czvf babygit.tar.gz baby-git

test    :
	@printf "DISTRO = $(DISTRO), CC = $(CC)\n"
	@printf "PROGS = $(PROGS)\n" 

