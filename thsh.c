/* COMP 530: Tar Heel SHell */

#include "thsh.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv, char **envp) {
    // flag that the program should end
    bool finished = 0;
    int input_fd = 0;  // Default to stdin
    int ret = 0;

    // Lab 2:
    // Add support for parsing the -d option from the command line
    // and handling the case where a script is passed as input to your shell

    // Lab 2: Your code here

    ret = init_cwd();
    if (ret) {
        dprintf(2, "Error initializing the current working directory: %d\n",
                ret);
        return ret;
    }

    ret = init_path();
    if (ret) {
        dprintf(2, "Error initializing the path table: %d\n", ret);
        return ret;
    }

    while (!finished) {
        int length;
        // Buffer to hold input
        char cmd[MAX_INPUT];
        // Buffer for scratch space - optional, only necessary for challenge
        // problems
        char scratch[MAX_INPUT];
        // Get a pointer to cmd that type-checks with char *
        char *buf = &cmd[0];
        char *parsed_commands[MAX_PIPELINE][MAX_ARGS];
        char *infile = NULL;
        char *outfile = NULL;
        int pipeline_steps = 0;

        if (!input_fd) {
            ret = print_prompt();
            if (ret <= 0) {
                // if we printed 0 bytes, this call failed and the program
                // should end -- this will likely never occur.
                finished = true;
                break;
            }
        }

        // Reset memory from the last iteration
        for (int i = 0; i < MAX_PIPELINE; i++) {
            for (int j = 0; j < MAX_ARGS; j++) {
                parsed_commands[i][j] = NULL;
            }
        }

        // Read a line of input
        length = read_one_line(input_fd, buf, MAX_INPUT);
        if (length <= 0) {
            ret = length;
            break;
        }

        // Add it to the history
        // add_history_line(buf);

        // Pass it to the parser
        pipeline_steps = parse_line(buf, length, parsed_commands, &infile,
                                    &outfile, scratch, MAX_INPUT);
        if (pipeline_steps < 0) {
            dprintf(2, "Parsing error.  Cannot execute command. %d\n",
                    -pipeline_steps);
            continue;
        }

        // Just echo the command line for now
        // file descriptor 1 -> writing to stdout
        // print the whole cmd string (write number of
        // chars/bytes equal to the length of cmd, or MAX_INPUT,
        // whichever is less)
        //
        // Comment this line once you implement
        // command handling
        dprintf(1, "%s\n", cmd);
        int res = 0;
        handle_builtin((char **)parsed_commands, 0, 1, &res);

        // In Lab 2, you will need to add code to actually run the commands,
        // add debug printing, and handle redirection and pipelines, as
        // explained in the handout.
        //
        // For now, ret will be set to zero; once you implement command
        // handling, ret should be set to the return from the command.
        ret = 0;

        // Do NOT change this if/printf - it is used by the autograder.
        if (ret) {
            char buf[100];
            int rv =
                snprintf(buf, 100, "Failed to run command - error %d\n", ret);
            if (rv > 0)
                write(1, buf, strlen(buf));
            else
                dprintf(2,
                        "Failed to format the output (%d).  This shouldn't "
                        "happen...\n",
                        rv);
        }
    }

    // Only return a non-zero value from main() if the shell itself
    // has a bug.  Do not use this to indicate a failed command.
    return 0;
}
