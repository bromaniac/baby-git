#
#  All documentation (comments) unless explicitly noted:
#
#      Copyright 2018, AnalytixBar LLC, Jacob Stopak
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>.
#
#**************************************************************************
#
#  This is the original Makefile for git.
#
#  It is used to invoke the gcc C compiler to build binary executable
#  files for each of the original 7 git commands:
#
#   1) init-db
#   2) update-cache
#   3) cat-file
#   4) show-diff
#   5) write-tree
#   6) read-tree
#   7) commit-tree
#
#  This Makefile can be invoked in 3 variations, by running the 3 following
#  commands from the command line inside the same directory as the Makefile:
#
#  1) `make clean`: This removes all previously built executables and build
#     files, from the working directory.
#
#  2) `make backup`: This first runs `make clean` and then backs up the
#     current directory into a tar archive.
#
#  3) `make`: The builds the codebase and creates the 7 git executables.
#

CFLAGS=-g # The `-g` compiler flag tells gcc to generate source-level debug information.
CC=gcc # Use the `gcc` C compiler.

# Specify all the executables names to make.
PROG=update-cache show-diff init-db write-tree read-tree commit-tree cat-file
all: $(PROG)

install: $(PROG)
	install $(PROG) $(HOME)/bin/

# Include the following dependencies in the build.
LIBS= -lcrypto -lboost_iostreams -lz

# Specify which compiled output (.o files) to use for each executable.
init-db: init-db.o

update-cache: update-cache.o read-cache.o
	$(CC) $(CFLAGS) -o update-cache update-cache.o read-cache.o $(LIBS)

show-diff: show-diff.o read-cache.o
	$(CC) $(CFLAGS) -o show-diff show-diff.o read-cache.o $(LIBS)

write-tree: write-tree.o read-cache.o
	$(CC) $(CFLAGS) -o write-tree write-tree.o read-cache.o $(LIBS)

read-tree: read-tree.o read-cache.o
	$(CC) $(CFLAGS) -o read-tree read-tree.o read-cache.o $(LIBS)

commit-tree: commit-tree.o read-cache.o
	$(CC) $(CFLAGS) -o commit-tree commit-tree.o read-cache.o $(LIBS)

cat-file: cat-file.o read-cache.o
	$(CC) $(CFLAGS) -o cat-file cat-file.o read-cache.o $(LIBS)

# Specify which C header files to include in compilation/linking.
read-cache.o: cache.h
show-diff.o: cache.h

# Define the steps to run during the `make clean` command.
clean:
	rm -f *.o $(PROG) temp_git_file_* # Remove these files from the current directory.

# Define the steps to run during the `make backup` command.
backup: clean
	cd .. ; tar czvf babygit.tar.gz baby-git # Backup the current directory into a tar archive.
