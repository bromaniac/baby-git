/*
 *  GIT - The information manager from hell
 *
 *  Copyright (C) Linus Torvalds, 2005
 *
 *  The purpose of this file is to be compiled into an executable
 *  called `commit-tree`. The syntax of the command is:
 *      
 *      commit-tree <sha1> [-p <sha1>]
 *
 *  When `commit-tree` is run from the command line it takes at
 *  least 1 command line argument which is the SHA1 hash of the
 *  tree to commit. If that is the only parameter supplied, git
 *  will assume you are creating the the initial commit in the
 *  repository, i.e. a commit with no parents. If the `-p <sha1>`
 *  is specified as a second parameter, that SHA1 hash should
 *  correspond to the parent commit of the new one about to be
 *  created. Technically in a multi-commit merge, multiple
 *  parent commit SHA1 hashes can be passed in.
 *
 *  The `commit-tree` command creates a new commit object
 *  in the object store, either as the initial commit if no
 *  parents are specified, or as a new commit with the parent
 *  specified by the second parameter.
 *
 *  Everything in the main function in this file will run
 *  when ./commit-tree executable is run from the command line.
 *
 *  Note: If the `commit-tree` command appears to be hanging
 *  after you press ENTER at the command line, it is not. It is
 *  waiting for you to type in a commit message and then press
 *  CTRL-D. This will add the commit message into the newly
 *  created commit and write it to the object store.
 */

#include "cache.h"
/*  The above 'include' allows use of the following functions and
    variables from <cache.h> header file, ranked in order of first use
    in this file. Most are functions/macros from standard C libraries
    that are `#included` in <cache.h>. Function names are followed by
    parenthesis whereas variable/struct names are not:

	-malloc(size): Allocate unused space for an object whose
                   size in bytes is specified by `size` and whose
                   value is unspecified. Sourced from <stdlib.h>.

    -memset(void *s, int c, size_t n): Copies `c` (converted to an unsigned
                                       char) into each of the first `n` bytes
                                       of the object pointed to by `s`.

	-va_start(va_list ap, parm_n): Macro enabling access to the variable
								   arguments following the named argument.

    -vsnprintf(char *s, size_t n, const char *format, va_list arg):
        Composes a string with the same text that would be printed if format
        was used on printf, but using the elements in the variable argument
        list identified by `arg` instead of additional function arguments and
        storing the resulting content as a string in the buffer pointed by
        `s` (taking `n` as the maximum buffer capacity to fill).

	-va_end(va_list ap): Marco that performs cleanup for an ap object
						 initialized by a call to va_start().

	-realloc(pointer, size): Update the size of the memory object pointed
                             to by `pointer` to `size`. Sourced from <stdlib.h>.

    -memcpy(s1, s2, n): Copy n bytes from the object pointed to
                        by s2 into the object pointed to by s1.

	-strlen(string): Return the length of `string` in bytes.

    -get_sha1_hex(): Convert the SHA1 passed in as a command line
                     argument to the proper hexadecimal format.
                     Sourced from <cache.h> and defined in
                     <read-cache.c>.

    -usage(): Print a usage message in the event of incorrect usage.
              Sourced from <cache.h>.

    -strcmp(s1, s2): Compares the string pointed to by `s1` to the
                     string pointed to by `s2`.

    -fprintf(stream, message): Place `message` on the named output
                               `stream`. Sourced from <stdio.h>.

    -getuid(): Return the real user ID of the calling process.

    -getpwuid(): Search the user database for an entry with a matching uid.

    -gethostname(name, namelen): Return the standard host name for the
                                 current machine.

    -time(time_t *tloc): Return the value of time in seconds since the Epoch.

    -ctime(const time_t *clock): Convert time value to a date and time string.

    -getenv(name): Get value of the environment variable `name`.
                   Sourced from <stdlib.h>.

    -sha1_to_hex: TODO

    -fgets(char *restrict s, int n, FILE *restrict stream):
        Read bytes from `stream` into the array pointed to by `s`, until `n`-1
        bytes are read, or a <newline> is read and transferred to `s`, or an
        end-of-file condition is encountered. The string is then terminated with
        a null byte.

    -sizeof(datatype): Generates the size of a variable or datatype,
                       measured in the number of char size storage
                       units required for the type.

    -write_sha1_file(buf, len): Compress file content stored in `buf`, hash
                                compressed output, and write to object store.

****************************************************************

The following variables and functions are defined locally.

    -main(): The main function runs each time the ./commit-tree
             command is run.

    -init_buffer(): Allocate memory and offset size for buffer.

    -add_buffer(): Adds a line of content into the buffer to be committed into
                   the object store.

    -prepend_integer(): Populate the buffer offset.

    -finish_buffer(): Final prep for the buffer so that the new commit ready
                      to be added to object store.

    -ORIG_OFFSET: The default offset for the buffer to be committed.

    -BLOCKING: The initial memory size to allocate for the buffer.

*/

/* Include header file for working with user account structures (like `passwd`) */
#include <pwd.h>

/* Include header file for workin with date and time structures. */
#include <time.h>

#define BLOCKING (1ul << 14) /* The initial memory size to allocate for the buffer. */
#define ORIG_OFFSET (40) /* The default offset for the buffer to be committed. */

/*
 * Function: `init_buffer`
 * Parameters:
 *      -bufp: Buffer string to initialize.
 *      -sizep: Size of the buffer string.
 * Purpose: Allocate memory and offset size for buffer.
 */
static void init_buffer(char **bufp, unsigned int *sizep)
{
	char *buf = malloc(BLOCKING); /* Allocate memory for buffer. */
	memset(buf, 0, ORIG_OFFSET); /* Copies '0' (null byte) into first 40 byes of buffer. */
	*sizep = ORIG_OFFSET; /* Set buffer size to default offset value. */
	*bufp = buf; /* Set the passed-in buffer. */
}

/*
 * Function: `add_buffer`
 * Parameters:
 *      -bufp: The initialized buffer to add content to.
 *      -sizep: The size of the buffer.
 *      -fmt: Variable argument list.
 * Purpose: Adds a line of content into the buffer to be committed into the object store.
 */
static void add_buffer(char **bufp, unsigned int *sizep, const char *fmt, ...)
{
	char one_line[2048]; /* Represents a line to add to buffer. */
	va_list args; /* References variable argument list. */
	int len; /* Length of variable argument list. */
	unsigned long alloc, size, newsize; /* Memory sizes. */
	char *buf; /* The buffer. */

	va_start(args, fmt); /* Handle variable argument list. */

    /* Populate `one_line` with the variable argument list parameters. */
	len = vsnprintf(one_line, sizeof(one_line), fmt, args);

	va_end(args); /* End parsing variable argument list. */
	size = *sizep; /* Set passed-in size. */
	newsize = size + len; /* Combine passed-in size and length of `one_line`. */
	alloc = (size + 32767) & ~32767; /* Set memory to allocate for the buffer. */
	buf = *bufp; /* Set passed-in buffer. */

    /* If the new buffer size will be greater that default memory to allocate... */
	if (newsize > alloc) {
		alloc = (newsize + 32767) & ~32767; /* Increase the size to allocate. */
		buf = realloc(buf, alloc); /* Updated memory allocated for buffer. */
		*bufp = buf; /* Set the buffer. */
	}
	*sizep = newsize; /* Update buffer size to new value */
	memcpy(buf + size, one_line, len); /* Copy `one_line` into the buffer. */
}

/*
 * Function: `prepend_integer`
 * Parameters:
 *      -buffer: The buffer to be committed to the object store.
 *      -val: The value to use to populate the buffer offset.
 *      -i: The size of the buffer offset to populate.
 * Purpose: Populate the buffer offset.
 */
static int prepend_integer(char *buffer, unsigned val, int i)
{
	buffer[--i] = '\0'; /* Set the ith entry in buffer to the null byte and decrement i. */

    /* Populate the buffer offset. */
	do {
		buffer[--i] = '0' + (val % 10);
		val /= 10;
	} while (val);
	return i;
}

/*
 * Function: `finish_buffer`
 * Parameters:
 *      -tag: Label to add at the beginning of the buffer.
 *      -bufp: The buffer to be committed.
 *      -sizep: The size of the buffer.
 * Purpose: Final prep for the buffer so that the new commit ready to be added to object store.
 */
static void finish_buffer(char *tag, char **bufp, unsigned int *sizep)
{
	int taglen; /* The length of the tag. */
	int offset; /* The buffer offset. */
	char *buf = *bufp; /* Set the passed-in buffer. */
	unsigned int size = *sizep; /* Set the passed-in buffer size. */

    /* Populate buffer offset. */
	offset = prepend_integer(buf, size - ORIG_OFFSET, ORIG_OFFSET);
	taglen = strlen(tag); /* Length of tag. */
	offset -= taglen; /* Decrement offset by taglen. */
	buf += offset; /* Add offset to buffer. */
	size -= offset; /* Decrement size by offset. */
	memcpy(buf, tag, taglen); /* Copy the tag into the buffer. */

	*bufp = buf; /* Set the final buffer. */
	*sizep = size; /* Set the final buffer size. */
}

/*
 * Function: `remove_special`
 * Parameters:
 *      -p: String to remove special characters from. 
 * Purpose: Remove newline and angle bracket characters from string.
 */
static void remove_special(char *p)
{
	char c; /* Used to iterate through string characters. */
	char *dst = p; /* Pointer to the string. */

    /* Run infinitely until break statement. */
	for (;;) {
		c = *p; /* Set `c` to first character in string. */
		p++; /* Increment pointer to next character. */

        /* If character is newline or angle bracket, ignore it. */
		switch(c) {
            case '\n': case '<': case '>':
                continue;
		}

        /* Add character to string. */
		*dst++ = c;

        /* If we've gone off end of string, end. */
		if (!c)
			break;
	}
}

/*
 * Having more than two parents may be strange, but hey, there's
 * no conceptual reason why the file format couldn't accept multi-way
 * merges. It might be the "union" of several packages, for example.
 *
 * I don't really expect that to happen, but this is here to make
 * it clear that _conceptually_ it's ok..
 */
#define MAXPARENT (16)

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the command itself.
 *      -argv: An array of the command line arguments, including the command itself.
 * Purpose: Standard `main` function definition. Runs when the executable `commit-tree` is
 *          run from the command line.
 */
int main(int argc, char **argv)
{
	int i; /* For loop counter. */
    int len; /* Used to store the length of the OS user name. */
	int parents = 0; /* The number of passed-in parent commits. */
	unsigned char tree_sha1[20]; /* The SHA1 of the tree to be committed. */
	unsigned char parent_sha1[MAXPARENT][20]; /* An array of passed-in parent commit SHA1 hashes. */
	char *gecos, *realgecos;
	char *email, realemail[1000]; /* Used to store the user's email address. */
	char *date, *realdate; /* Used to store the date. */
	char comment[1000]; /* Used to store the commit message. */

    /* The `passwd` struct is used for storing user account information. */
	struct passwd *pw;

	time_t now; /* Used to store the time of the commit. */
	char *buffer; /* Used to store and build up the content to be added to the new commit before it is written to the object store. */
	unsigned int size; /* The size of `buffer`. */

    /*
     * Show usage message if less than 2 args are passed or if the tree
     * SHA1 hash is not in the object store.
     */
	if (argc < 2 || get_sha1_hex(argv[1], tree_sha1) < 0)
		usage("commit-tree <sha1> [-p <sha1>]* < changelog");

    /*
     * Loop thru all passed-in parent commit SHA1 hashes and show usage
     * message if any of them don't exist in the object store.
     */
	for (i = 2; i < argc; i += 2) {
		char *a, *b;
		a = argv[i]; b = argv[i+1];
		if (!b || strcmp(a, "-p") || get_sha1_hex(b, parent_sha1[parents]))
			usage("commit-tree <sha1> [-p <sha1>]* < changelog");
		parents++;
	}

    /* If no parent commit SHA1 hashes are passed in, assume this is the first commit. */
	if (!parents)
		fprintf(stderr, "Committing initial tree %s\n", argv[1]);

    /* Get a `passwd` struct object for the current user running this process. */
	pw = getpwuid(getuid());

    /* Print an error message if the user isn't read properly. */
	if (!pw)
		usage("You don't exist. Go away!");

    /* Get the user's full name. */
	realgecos = pw->pw_gecos;

    /* Get the length of the user's login id. */
	len = strlen(pw->pw_name);

    /* Contruct an email address for the user. */
	memcpy(realemail, pw->pw_name, len);
	realemail[len] = '@';

    /* Get the hostname. */
	gethostname(realemail+len+1, sizeof(realemail)-len-1);

    /* Get the current date and time. */
	time(&now);
	realdate = ctime(&now);

    /*
     * Set user name, email, & date from either environment
     * variables or the previously calculated values.
     */
	gecos = getenv("COMMITTER_NAME") ? : realgecos;
	email = getenv("COMMITTER_EMAIL") ? : realemail;
	date = getenv("COMMITTER_DATE") ? : realdate;

    /* Remove special characters from user property variables. */
	remove_special(gecos); remove_special(realgecos);
	remove_special(email); remove_special(realemail);
	remove_special(date); remove_special(realdate);

    /* Initialize the buffer to hold the new commit info. */
	init_buffer(&buffer, &size);

    /* Add the static text 'tree' and the tree SHA1 hash to the buffer. */
	add_buffer(&buffer, &size, "tree %s\n", sha1_to_hex(tree_sha1));

	/*
     * For each passed-in parent commit SHA1 hash, add the static text
     * 'parent' and the parent commit SHA1 hash to the buffer.
     *
	 * NOTE! This ordering means that the same exact tree merged with a
	 * different order of parents will be a _different_ changeset even
	 * if everything else stays the same.
	 */
	for (i = 0; i < parents; i++)
		add_buffer(&buffer, &size, "parent %s\n", sha1_to_hex(parent_sha1[i]));

	/* Add the commit auther, email, and date info to the buffer. */
	add_buffer(&buffer, &size, "author %s <%s> %s\n", gecos, email, date);
	add_buffer(&buffer, &size, "committer %s <%s> %s\n\n", realgecos, realemail, realdate);

	/* Add the commit message to the buffer. This is what requires the user to type CTRL-D to finish the `commit-tree` command. */
	while (fgets(comment, sizeof(comment), stdin) != NULL)
		add_buffer(&buffer, &size, "%s", comment);

    /* Finalize the buffer content to get ready to write the new commit file to the object store. */
	finish_buffer("commit ", &buffer, &size);

    /* Write the new commit object to the object store. */
	write_sha1_file(buffer, size);

	return 0; /* Return success. */
}
