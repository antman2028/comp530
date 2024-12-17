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
        // dprintf(1, "%s\n", cmd);
        int handled = handle_builtin((char **)parsed_commands, 0, 1, &ret);
        if (handled == 0) {
            int pipefd[2] = {-1, -1};
            int prev_read_fd = -1;
            int *jobList = (int *)malloc(sizeof(int) * 50);
            int numOfJobs = 0;
            for (int i = 0; parsed_commands[i][0] != '\0'; i++) {
                int job_id = create_job();
                if (job_id < 0) {
                    free(jobList);
                    dprintf(2, "Error creating command %d\n", job_id);
                }
                jobList[numOfJobs++] = job_id;
                // Last command in pipeline
                if (parsed_commands[i + 1][0] == '\0') {
                    ret = run_command(
                        parsed_commands[i],
                        prev_read_fd == -1 ? STDIN_FILENO : prev_read_fd,
                        STDOUT_FILENO, job_id);
                    if (prev_read_fd != -1) close(prev_read_fd);
                    break;
                }

                // Create new pipe for next command
                if (pipe(pipefd) == -1) {
                    if (prev_read_fd != -1) close(prev_read_fd);
                    dprintf(2, "failed to generate pipeline - %d\n", -errno);
                }

                // Run command with appropriate fds
                ret = run_command(
                    parsed_commands[i],
                    prev_read_fd == -1 ? STDIN_FILENO : prev_read_fd, pipefd[1],
                    job_id);

                // Cleanup current pipe iteration
                if (prev_read_fd != -1) close(prev_read_fd);
                close(pipefd[1]);
                prev_read_fd = pipefd[0];  // Save read end for next iteration
            }
            printf("Ret code: %d\n", ret);
            for (int i = 0; i < numOfJobs; i++) {
                wait_on_job(jobList[i], &ret);
                if (ret < 0) {
                    dprintf(2, "Job failed: %d", jobList[i]);
                    free(jobList);
                }
            }
        }
        // In Lab 2, you will need to add code to actually run the commands,
        // add debug printing, and handle redirection and pipelines, as
        // explained in the handout.
        //
        // For now, ret will be set to zero; once you implement command
        // handling, ret should be set to the return from the command.

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
