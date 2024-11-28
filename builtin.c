/* COMP 530: Tar Heel SHell
 *
 * This file implements a table of builtin commands.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "thsh.h"

struct builtin {
    const char *cmd;
    int (*func)(char *args[MAX_ARGS], int stdin, int stdout);
};

static char old_path[MAX_INPUT];
static char cur_path[MAX_INPUT];
static char usr_path[MAX_INPUT];

/* Handle a cd command.  */
int handle_cd(char *args[MAX_INPUT], int stdin, int stdout) {
    // Note that you need to handle special arguments, including:
    // "-" switch to the last directory
    // "." switch to the current directory.  This should change the
    //     behavior of a subsequent "cd -"
    // ".." go up one directory
    //
    // Hint: chdir can handle "." and "..", but saving
    //       these results may not be the desired outcome...

    // XXX: Test for errors in the output if a cd fails

    // Lab 2: Your code here
    //

    // Remove the following two lines once implemented.  These
    // just suppress the compiler warning around an unused variable

    if (strlen(*args) != 2) {
        if (strlen(*args) < 2) {
            perror("Directory argument missing");
        } else {
            perror("Only supports 1 directory argument");
        }
        return 1;
    }

    if (strcmp(args[1], "-") == 0) {
        if (strlen(old_path) == 0) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return 1;
        }
        strcpy(args[1], old_path);
    }

    char *temp = strdup(cur_path);
    if (chdir(args[1]) != 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
                 "-thsh: cd: %s: invalid option\n", args[1]);
        write(1, error_msg, strlen(error_msg));
        return 1;
    }
    strcpy(old_path, temp);
    if (getcwd(cur_path, sizeof(cur_path)) == 0) {
        return 1;
    }

    return 0;
}

/* Handle an exit command. */
int handle_exit(char *args[MAX_ARGS], int stdin, int stdout) {
    exit(0);
    return 0;  // Does not actually return
}

int init_cwd() {
    if (getcwd(usr_path, sizeof(usr_path)) != NULL) {
        strcpy(cur_path, usr_path);
    } else {
        return 1;
    }

    return 0;
}

int print_prompt(void) {
    int ret = 0;
    // Print the prompt
    // file descriptor 1 -> writing to stdout
    // print the whole prompt string (write number of
    // bytes/chars equal to the length of prompt)
    //
    const char *prompt = "thsh> ";

    // Lab 2: Your code here

    char full_prompt[256];
    strcpy(full_prompt, prompt);
    strcat(full_prompt, cur_path);
    strcat(full_prompt, "$ ");

    ret = write(1, full_prompt, strlen(full_prompt));
    return ret;
}

static struct builtin builtins[] = {
    {"cd", handle_cd}, {"exit", handle_exit}, {NULL, NULL}};

/* This function checks if the command (args[0]) is a built-in.
 * If so, call the appropriate handler, and return 1.
 * If not, return 0.
 *
 * stdin and stdout are the file handles for standard in and standard out,
 * respectively. These may or may not be used by individual builtin commands.
 *
 * Places the return value of the command in *retval.
 *
 * stdin and stdout should not be closed by this command.
 *
 * In the case of "exit", this function will not return.
 */
int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval) {
    // Lab 0: Your Code Here
    // Comment this line once implemented.  This just suppresses
    // the unused variable warning from the compiler.
    // (void) builtins;
    for (int i = 0; builtins[i].cmd != NULL; i++) {
        if (strcmp(args[0], builtins[i].cmd) == 0) {
            *retval = builtins[i].func(args, stdin, stdout);
            return 1;
        }
    }
    return 0;
}
