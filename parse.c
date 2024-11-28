/* COMP 530: Tar Heel SHell
 *
 * This module implements command parsing, following the grammar
 * in the assignment handout.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "thsh.h"

/* This function returns one line from input_fd
 *
 * buf is populated with the contents, including the newline, and
 *      including a null terminator.  Must point to a buffer
 *      allocated by the caller, and may not be NULL.
 *
 * size is the size of *buf
 *
 * Return value: the length of the string (not counting the null terminator)
 *               zero indicates the end of the input file.
 *               a negative value indicates an error (e.g., -errno)
 */
int read_one_line(int input_fd, char *buf, size_t size) {
    int count, rv;
    // pointer to next place in cmd to store a character
    char *cursor;
    // the last character that was written into cmd
    char last_char;

    assert(buf);

    /*
     * We want to continue reading characters until:
     *   - read() fails (rv will become 0) OR
     *   - count == MAX_INPUT-1 (we have no buffer space left) OR
     *   - last_char was '\n'
     * so we continue the loop while:
     *   rv is nonzero AND count < MAX_INPUT - 1 AND last_char != '\n'
     *
     * On every iteration, we:
     *   - increment cursor to advance to the next char in the cmd buffer
     *   - increment count to indicate we've stored another char in cmd
     *     (this is done during the loop termination check with "++count")
     *
     * To make the termination check work as intended, the loop starts by:
     *   - setting rv = 1 (so it's initially nonzero)
     *   - setting count = 0 (we've read no characters yet)
     *   - setting cursor = cmd (cursor at start of cmd buffer)
     *   - setting last_char = 1 (so it's initially not '\n')
     *
     * In summary:
     *   - START:
     *      set rv = 1, count = 0, cursor = cmd, last_char = 1
     *   - CONTINUE WHILE:
     *      rv (is nonzero) && count < MAX_INPUT - 1 && last_char != '\n'
     *   - UPDATE PER ITERATION:
     *      increment count and cursor
     */
    for (rv = 1, count = 0, cursor = buf, last_char = 1;
         rv && (last_char != '\n') && (++count < (size - 1)); cursor++) {
        // read one character
        // file descriptor 0 -> reading from stdin
        // writing this one character to cursor (current place in cmd buffer)
        rv = read(input_fd, cursor, 1);
        last_char = *cursor;
    }
    // null terminate cmd buffer (so that it will print correctly)
    *cursor = '\0';

    // Deal with an error from the read call
    if (!rv) {
        count = -errno;
    }

    return count;
}

/* Challenge: Check is a file matches a glob.
 *
 * This function takes in a simple file glob (such as '*.c')
 * and a file name, and returns 1 if it matches, and 0 if not.
 */
static int glob_matches(const char *glob, char *name) {
    // Lab 2 Challenge (optional): Your code here
    // Your code here (maybe)
    return 0;
}

/* Challenge: Expand a file glob.
 *
 * This function takes in a simple file glob (such as '*.c')
 * and expands it to all matching files in the current working directory.
 * If the glob does not match anything, pass the glob through to the application
 * as an argument.
 *
 * glob: The glob must be simple - only one asterisk as the first character,
 * with 0+ non-special characters following it.
 *
 * buf: A double pointer to scratch buffer allocated by the caller and good for
 * the life of the command, which one may place expanded globs in.  The double
 * pointer allows you to update the offset into the buffer.
 *
 * bufsize: _pointer_ to the remaining space in the buffer.  If too many files
 * match, this space may be exceeded, in which case, the function returns
 * -ENOSPC
 *
 * commands: Same argumetn as passed to parse_line
 *
 * pipeline_idx: Index into command array for the current pipeline
 *
 * arg_idx: _Pointer_ to the current argument index.  May be incremented as
 *         a glob is expanded.
 *
 * Returns 0 on success, -errno on error
 */
static int expand_glob(char *glob, char **buf, size_t *bufsize,
                       char *commands[MAX_PIPELINE][MAX_ARGS], int pipeline_idx,
                       int *arg_idx) {
    // Lab 2 Challenge (optional): Your code here
    int rv = 0;

    // Your code here (maybe)

    // Suppress the compiler warning that glob_matches is not used in the starer
    // code. It is only here for a challenge problem, and this line may be
    // ignored. You may remove this line if/when you use glob_matches in your
    // code.
    (void)&glob_matches;
    return rv;
}

/* Parse one line of input.
 *
 * This function should populate a two-dimensional array of commands
 * and tokens.  The array itself should be pre-allocated by the
 * caller.
 *
 * The first level of the array is each stage in a pipeline, at most
 * MAX_PIPELINE long. The second level of the array is each argument to a given
 * command, at most MAX_ARGS entries. In each command buffer, the entry after
 * the last valid entry should be NULL. After the last valid pipeline buffer,
 * there should be one command entry with just a NULL.
 *
 * For instance, a simple command like "cd" should parse as:
 *  commands[0] = ["cd", '\0']
 *  commands[1] = ['\0']
 *
 * The first "special" character to consider is the vertical bar, or "pipe"
 * ('|'). This splits a single line into multiple sub-commands that form the
 * pipeline. We will implement pipelines in lab 2, but for now, just use this
 * character to delimit commands.
 *
 * For instance, the command: "ls | grep foo\n" should be broken into:
 *
 * commands[0] = ["ls", '\0']
 * commands[1] = ["grep", "foo", '\0']
 * commands[2] = ['\0']
 *
 * Hint: Make sure to remove the newline at the end
 *
 * Hint: Make sure the implementation is robust to extra whitespace, like: "grep
 * foo" should still be parsed as:
 *
 * commands[0] = ["grep", "foo", '\0']
 * commands[1] = ['\0']
 *
 * This function should ignore anything after the '#' character, as
 * this is a comment.
 *
 * Finally, the command should identify file redirection characters ('<' and
 * '>'). The string right after these tokens should be returned using the
 * special output parameters "infile" and "outfile".  You can assume there is at
 * most one '<' and one
 * '>' character in the entire inbuf.
 *
 * For example, in input: "ls > out.txt", the return should be:
 *   commands[0] = ["ls", '\0']
 *   commands[1] = ['\0']
 *   outfile = "out.txt"
 *
 * Hint: Be sure your implementation is robust to arbitrary (or no) space before
 * or after the '<' or '>' characters.  For instance, "ls>out.txt" and "ls >
 * out.txt" are both syntactically valid, and should parse identically to "ls >
 * out.txt". Similarly, "ls|grep foo" is also syntactically valid.
 *
 * You do not need to handle redirection of other handles (e.g., "foo 2>&1
 * out.txt").
 *
 * inbuf: a NULL-terminated buffer of input.
 *        This buffer may be changed by the function
 *        (e.g., changing some characters to \0).
 *
 * length: the length of the string in inbuf.  Should be
 *         less than the size of inbuf.
 *
 * commands: a two-dimensional array of character pointers, allocated by the
 * caller, which this function populates.
 *
 * scratch: A caller-allocated buffer that can be used for scratch space, such
 * as expanding globs in the challenge problems.  You may not need to use this
 *          for the core assignment.
 *
 * scratch_len: Size of the scratch buffer
 *
 * return value: Number of entries populated in commands (1+, not counting the
 * NULL), or -errno on failure.
 *
 *               In the case of a line with no actual commands (e.g.,
 *               a line with just comments), return 0.
 */
int parse_line(char *inbuf, size_t length,
               char *commands[MAX_PIPELINE][MAX_ARGS], char **infile,
               char **outfile, char *scratch, size_t scratch_len) {
    // Lab 1: Your code here
    // Suppress the compiler warning that expand_glob is not used in the starer
    // code. It is only here for a challenge problem, and this line may be
    // ignored. You may remove this line if/when you use expand_glob in your
    // code.

    int ind = 0;
    int arg = 0;
    char *temp = (char *)malloc(sizeof(char) * 1024);
    if (temp == NULL) {
        perror("malloc");
        return -errno;
    }

    if (length == 0) {
        return -1;
    }

    temp[0] = '\0';

    for (int i = 0; i < length; i++) {
        // printf("%c\n", inbuf[i]);
        // Logic if char is pipe;
        if (inbuf[i] == '#') {
            break;  // Ignore anything after the '#' character
        } else if (inbuf[i] == '|') {
            // Check if there was command before pipe, if not, throw error
            if (temp != NULL && strlen(temp) > 0) {
                commands[ind][arg] = strdup(temp);
                if (commands[ind][arg] == NULL) {
                    perror("strdup");
                    free(temp);
                    return -errno;
                }
                commands[ind][++arg] =
                    '\0';  // Null-terminate the current command
                arg = 0;
                ind += 1;
                temp[0] = '\0';  // Clear temp for the next command
            } else if (i != 0) {
                commands[ind][++arg] = '\0';
                arg = 0;
                ind += 1;
                temp[0] = '\0';
            }
        } else if (inbuf[i] == '>' || inbuf[i] == '<') {
            if (strlen(temp) > 0) {
                commands[ind][arg] = strdup(temp);
                if (commands[ind][arg] == NULL) {
                    perror("strdup");
                    free(temp);
                    return -errno;
                }
                commands[ind][++arg] = '\0';
                arg = 0;
                ind += 1;
                temp[0] = '\0';
            }
            // Sets temp to file input or output
            char sign = inbuf[i];
            i++;
            while (i < length && (inbuf[i] == ' ' || inbuf[i] == '\t')) {
                i++;
            }
            int j = 0;
            while (i < length && inbuf[i] != ' ' && inbuf[i] != '\t' &&
                   inbuf[i] != '\n') {
                temp[j++] = inbuf[i++];
            }
            temp[j] = '\0';
            // Sets input or output
            if (sign == '>') {
                *outfile = strdup(temp);
                if (*outfile == NULL) {
                    perror("strdup");
                    free(temp);
                    return -errno;
                }
            } else {
                *infile = strdup(temp);
                if (*infile == NULL) {
                    perror("strdup");
                    free(temp);
                    return -errno;
                }
            }
            temp[0] = '\0';

        } else if (inbuf[i] == ' ' || inbuf[i] == '\n') {
            if (strlen(temp) > 0) {
                commands[ind][arg] = strdup(temp);
                if (commands[ind][arg] == NULL) {
                    perror("strdup");
                    free(temp);
                    return -errno;
                }
                arg++;
                temp[0] = '\0';
            }
        } else {
            size_t len = strlen(temp);
            temp[len] = inbuf[i];
            temp[len + 1] = '\0';
        }
    }

    if (strlen(temp) > 0) {
        commands[ind][arg] = strdup(temp);
        if (commands[ind][arg] == NULL) {
            perror("strdup");
            free(temp);
            return -errno;
        }
        commands[ind][++arg] = '\0';
    }

    commands[ind + 1][0] = '\0';

    free(temp);

    return ind + 1;
}

// int main() {
//     char inbuf[] = "cat file.txt|grep 'search'| ls -l >output.txt";
//     char *commands[MAX_PIPELINE][MAX_ARGS] = {0};
//     char *infile = NULL;
//     char *outfile = NULL;
//     char scratch[1024];
//     size_t scratch_len = sizeof(scratch);

//     int num_commands = parse_line(inbuf, strlen(inbuf), commands, &infile,
//                                   &outfile, scratch, scratch_len);

//     printf("Number of commands: %d\n", num_commands);
//     for (int i = 0; i < num_commands; i++) {
//         printf("Command %d:\n", i);
//         for (int j = 0; commands[i][j] != NULL; j++) {
//             printf("  Arg %d: %s\n", j, commands[i][j]);
//         }
//     }
//     if (infile) {
//         printf("Input file: %s\n", infile);
//     }
//     if (outfile) {
//         printf("Output file: %s\n", outfile);
//     }

//     // Free allocated memory
//     for (int i = 0; i < num_commands; i++) {
//         for (int j = 0; commands[i][j] != NULL; j++) {
//             free(commands[i][j]);
//         }
//     }
//     if (infile) {
//         free(infile);
//     }
//     if (outfile) {
//         free(outfile);
//     }

//     return 0;
// }