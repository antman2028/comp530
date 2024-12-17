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
    if (args[1] == NULL) {
        dprintf(2, "Directory argument missing\n");
        return 1;
    }

    char target_path[MAX_INPUT];
    if (strcmp(args[1], "-") == 0) {
        if (strlen(old_path) == 0) {
            dprintf(2, "cd: OLDPWD not set\n");
            return -errno;
        }
        strcpy(target_path, old_path);
    } else {
        strcpy(target_path, args[1]);
    }

    // Save current path before changing
    strcpy(old_path, cur_path);

    if (chdir(target_path) != 0) {
        dprintf(2, "-thsh: cd: %s: invalid option\n", target_path);
        return -errno;
    }

    if (getcwd(cur_path, sizeof(cur_path)) == 0) {
        return -errno;
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
    if (!args || !args[0] || !retval) {
        return 0;  // Not a builtin if invalid args
    }
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
