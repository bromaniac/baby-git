/*
 *  All documentation (comments) unless explicitly noted:
 *
 *      Copyright 2018, AnalytixBar LLC, Jacob Stopak
 *
 *  All code & explicitly labelled documentation (comments):
 *      
 *      Copyright 2005, Linus Torvalds
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *************************************************************************
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
/* The above 'include' allows use of the following functions and
   variables from "cache.h" header file, ranked in order of first use
   in this file. Most are functions/macros from standard C libraries
   that are `#included` in "cache.h". Function names are followed by
   parenthesis whereas variable/struct names are not:

   -malloc(size): Allocate unused space for an object whose size in bytes is 
                  specified by `size` and whose value is unspecified. Sourced 
                  from <stdlib.h>.

   -memset(void *s, int c, size_t n): Copies `c` (converted to an unsigned
                                      char) into each of the first `n` bytes
                                      of the object pointed to by `s`.

   -va_start(va_list ap, last): In a function with a variable argument list,
                                this macro initializes the variable `ap` to
                                point to the first unnamed argument after the
                                `last` named argument.

   -vsnprintf(char *s, size_t n, const char *format, va_list arg):
        Construct `format` using the variable argument list `arg` and write 
        the formatted string into `s`, with `n` specifying the maximum number 
        of characters that can be written to `s`, including the null 
        terminating character.

   -va_end(va_list ap): Marco that performs cleanup for a va_list object that
                        was initialized through a call to va_start().

   -realloc(pointer, size): Update the size of the memory object pointed to by
                            `pointer` to `size`. Sourced from <stdlib.h>.

   -memcpy(s1, s2, n): Copy n bytes from the object pointed to by s2 into the 
                       object pointed to by s1.

   -strlen(string): Return the length of `string` in bytes.

   -get_sha1_hex(): Convert a 40-character hexadicimal representation of an 
                    SHA1 hash value to the equivalent 20-byte decimal 
                    representation.

   -usage(): Print an error message.

   -strcmp(s1, s2): Compares the string pointed to by `s1` to the string 
                    pointed to by `s2`.

   -fprintf(stream, message, ...): Write `message` to the output `stream`.
                                   Sourced from <stdio.h>.

   -getuid(): Returns the real user ID of the calling process.

   -getpwuid(uid): Returns a pointer to a structure that contains information
                   about the user with user ID `uid`.

   -gethostname(name, namelen): Get the host name of the system on which the
                                function it was called and store it in array 
                                `name`, which has length `namelen`.

   -time(time_t *timep): Return the current calendar time in number of seconds
                         elapsed since 00:00:00 on January 1, 1970 UTC.

   -ctime(const time_t *timep): Convert time pointed to by `timep` to a
                                human-readable string of the corresponding
                                local date and time.

   -getenv(name): Get value of the environment variable `name`. Sourced from
                  <stdlib.h>.

   -sha1_to_hex(): Convert a 20-byte decimal representation of an SHA1 hash
                   value to the equivalent 40-character hexadicimal 
                   representation.

   -fgets(char *s, int n, FILE *stream):
        Read bytes from `stream` into the array pointed to by `s`, until `n`-1
        bytes are read, or a <newline> is read and transferred to `s`, or an
        end-of-file condition is encountered. The string is then terminated with
        a null byte.

   -sizeof(datatype): Operator that gives the number of bytes needed to store 
                      a datatype or variable. 

   -write_sha1_file(): Deflate an object, calculate the hash value, then call
                       the write_sha1_buffer function to write the deflated
                       object to the object database.

   ****************************************************************

   The following variables and functions are defined in this source file.

   -main(): The main function runs each time the ./commit-tree
            command is run.

   -init_buffer(): Allocate space to and initialize the buffer that will
                   contain the commit data.

   -add_buffer(): Adds one item of the commit data to the buffer.

   -prepend_integer(): Prepend the size of the commit data in bytes to the
                       buffer.

   -finish_buffer(): Call the prepend_integer() function to prepend the size 
                     of the commit data to the buffer, and then prepend the
                     object tag to the buffer.

   -ORIG_OFFSET: Token that defines the number of bytes at the beginning of 
                 the buffer that are allocated for the object tag and the 
                 object data size.

   -BLOCKING: Token that defines the initial memory size to allocate to the 
              buffer. Set to 16384 bytes.
*/

#ifndef BGIT_WINDOWS
/*
 * Include header file that contains the template for the passwd structure
 * for storing a user's login account information.
 */
#include <pwd.h>
#endif
/*
 * Include header file that contains the template for the tm structure for
 * storing data and time information.
 */
#include <time.h>

/* 16384 bytes, the initial memory size to allocate to the buffer. */
#define BLOCKING (1ul << 14) 
/*
 * Defines the number of bytes at the beginning of the buffer that are 
 * allocated for the object tag and the object data size.
 */
#define ORIG_OFFSET (40) 

/*
 * Function: `init_buffer`
 * Parameters:
 *      -bufp: Pointer to a pointer to a buffer to allocate and initialize.
 *      -sizep: Pointer to the size of filled portion of the buffer.
 * Purpose: Allocate space to and initialize the buffer that will contain the 
 *          commit data.
 */
static void init_buffer(char **bufp, unsigned int *sizep)
{
    /* Allocate memory to buffer. */
    char *buf = malloc(BLOCKING); 
    /* Copy null byte into first 40 bytes of the buffer. */
    memset(buf, 0, ORIG_OFFSET); 
    /* Size of filled portion of buffer. */
    *sizep = ORIG_OFFSET; 
    /* Set the commit object buffer to the allocated space. */
    *bufp = buf; 
}

/*
 * Function: `add_buffer`
 * Parameters:
 *      -bufp: Pointer to a pointer to the commit object buffer.
 *      -sizep: Pointer to the size of the filled portion of the buffer.
 *      -fmt: Format string.
 * Purpose: Adds one item of the commit data to the buffer.
 */
static void add_buffer(char **bufp, unsigned int *sizep, const char *fmt, ...)
{
    char one_line[2048];                /* String to add to the buffer. */
    va_list args;                       /* A variable argument list. */
    int len;                            /* Length of string in one_line. */
    unsigned long alloc, size, newsize; /* Variables to track buffer size. */
    char *buf;                          /* Pointer to commit object buffer. */

    /* Initialize args to point to the first unnamed argument after fmt. */
    va_start(args, fmt); 

    /*
     * Use argument list args to construct the fmt string and write the string 
     * to one_line. 
     */
    len = vsnprintf(one_line, sizeof(one_line), fmt, args);

    /* Clean up args variable list object. */
    va_end(args); 
    /* Size of current filled portion of commit object buffer. */
    size = *sizep; 
    /* Add length of one_line to the size of the filled buffer portion. */
    newsize = size + len; 
    /* Calculate minimum buffer size. Should be a multiple of 32768 bytes. */
    alloc = (size + 32767) & ~32767; 
    /* Set local pointer to point to the commit object buffer. */
    buf = *bufp; 

    /* 
     * Increase the buffer size if the calculated size of filled portion of 
     * the buffer is greater than the calculated minimum buffer size. 
     */
    if (newsize > alloc) {
        /*
         * Calculate minimum buffer size. Should be a multiple of 32768 
         * bytes. 
         */
        alloc = (newsize + 32767) & ~32767; 
        /* Increase the buffer size. */
        buf = realloc(buf, alloc); 
        /*
         * Set the commit object buffer pointer to point to the reallocated
         * buffer. 
         */
        *bufp = buf; 
    }
    /* New size of filled portion of commit object buffer. */
    *sizep = newsize; 
    /*
     * Append one_line after the filled portion of the commit object 
     * buffer. 
     */
    memcpy(buf + size, one_line, len); 
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
 * Linus Torvalds: Having more than two parents may be strange, but hey, there's
 * no conceptual reason why the file format couldn't accept multi-way
 * merges. It might be the "union" of several packages, for example.
 *
 * Linus Torvalds: I don't really expect that to happen, but this is here to make
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
        size_t hostname_size;
    char *date, *realdate; /* Used to store the date. */
    char comment[1000]; /* Used to store the commit message. */

        #ifndef BGIT_WINDOWS
            /* The `passwd` struct is used for storing user account information. */
            struct passwd *pw;
            char *username;
        #else
            unsigned long uname_len = UNLEN + 1;
            char username[uname_len];
        #endif

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

        #ifndef BGIT_WINDOWS
    /* Get a `passwd` struct object for the current user running this process. */
    pw = getpwuid(getuid());

    /* Print an error message if the user isn't read properly. */
    if (!pw)
        usage("You don't exist. Go away!");

    /* Get the user's full name. */
    realgecos = pw->pw_gecos;

    /* Get the length of the user's login id. */
        username = pw->pw_name;

        #else

        GetUserName( username, &uname_len );
        realgecos = username;

        #endif

    len = strlen(username);

    /* Contruct an email address for the user. */
    memcpy(realemail, username, len);
    realemail[len] = '@';

    /* Get the hostname. */
        hostname_size = sizeof(realemail) - len - 1;
        #ifndef BGIT_WINDOWS
    gethostname(realemail+len+1, hostname_size);
        #else
        GetComputerName( realemail+len+1, &hostname_size );
        #endif

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
     * Linus Torvalds: NOTE! This ordering means that the same exact tree merged with a
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
