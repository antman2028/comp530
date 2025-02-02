/* COMP 530: Tar Heel SHell
 *
 * This file implements functions related to launching
 * jobs and job control.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "thsh.h"

static char **path_table;

typedef struct {
    int length;
    int width;
    int height;
} box;

/* Initialize the table of PATH prefixes.
 *
 * Split the result on the parenteses, and
 * remove any trailing '/' characters.
 * The last entry should be a NULL character.
 *
 * For instance, if one's PATH environment variable is:
 *  /bin:/sbin///
 *
 * Then path_table should be:
 *  path_table[0] = "/bin"
 *  path_table[1] = "/sbin"
 *  path_table[2] = '\0'
 *
 * Hint: take a look at getenv().  If you use getenv, do NOT
 *       modify the resulting string directly, but use
 *       malloc() or another function to allocate space and copy.
 *
 * Returns 0 on success, -errno on failure.
 */
int init_path(void) {
    char *path_cpy = getenv("PATH");
    if (path_cpy == NULL) {
        return EXIT_FAILURE;
    }

    char *path = strdup(path_cpy);
    if (path == NULL) {
        return EXIT_FAILURE;
    }
    path_table = (char **)malloc(sizeof(char *) * 2);
    if (path_table == NULL) {
        free(path);
        return EXIT_FAILURE;
    }

    int ind = 0;
    int cSize = 2;
    char *temp = strtok(path, ":");

    while (temp != NULL) {
        if (ind >= cSize) {
            cSize *= 2;
            char **new_table = realloc(path_table, sizeof(char *) * cSize);
            if (new_table == NULL) {
                free(path_table);
                free(path);
                return EXIT_FAILURE;
            }
            path_table = new_table;
        }
        path_table[ind] = strdup(temp);
        ind++;
        temp = strtok(NULL, ":");
    }
    if (ind >= cSize) {
        char **new_table = realloc(path_table, sizeof(char *) * (cSize + 1));
        if (new_table == NULL) {
            free(path_table);
            free(path);
            return -1;
        }
        path_table = new_table;
    }
    path_table[ind] = NULL;
    free(path);

    return 0;
}

/* Debug helper function that just prints
 * the path table out.
 */
void print_path_table() {
    if (path_table == NULL) {
        printf("XXXXXXX Path Table Not Initialized XXXXX\n");
        return;
    }

    printf("===== Begin Path Table =====\n");
    for (int i = 0; path_table[i]; i++) {
        printf("Prefix %2d: [%s]\n", i, path_table[i]);
    }
    printf("===== End Path Table =====\n");
}

// int main() {
//     init_path();
//     print_path_table();
// }

static int job_counter = 0;

struct kiddo {
    int pid;
    struct kiddo *next;  // Linked list of sibling processes
};

// A job consists of a unique numeric ID and
// one or more processes
struct job {
    int id;
    struct kiddo *kidlets;  // Linked list of child processes
    struct job *next;       // Linked list of active jobs
};

// A singly linked list of active jobs.
static struct job *jobbies = NULL;

/* Initialize a job structure
 *
 * Returns an integer ID that represents the job.
 */
int create_job(void) {
    struct job *tmp;
    struct job *j = malloc(sizeof(struct job));
    j->id = ++job_counter;
    j->kidlets = NULL;
    j->next = NULL;
    if (jobbies) {
        for (tmp = jobbies; tmp && tmp->next; tmp = tmp->next)
            ;
        assert(tmp != j);
        tmp->next = j;
    } else {
        jobbies = j;
    }
    return j->id;
}

/* Helper function to walk the job list and find
 * a given job.
 *
 * remove: If true, remove this job from the job list.
 *
 * Returns NULL on failure, a job pointer on success.
 */
static struct job *find_job(int job_id, bool remove) {
    struct job *tmp, *last = NULL;
    for (tmp = jobbies; tmp; tmp = tmp->next) {
        if (tmp->id == job_id) {
            if (remove) {
                if (last) {
                    last->next = tmp->next;
                } else {
                    assert(tmp == jobbies);
                    jobbies = tmp->next;
                }
            }
            return tmp;
        }
        last = tmp;
    }
    return NULL;
}

/* Given the command listed in args,
 * try to execute it and create a job structure.
 *
 * This function does NOT wait on the child to complete,
 * nor does it return an exit code from the child.
 *
 * If the first argument starts with a '.'
 * or a '/', it is an absolute path and can
 * execute as-is.
 *
 * Otherwise, search each prefix in the path_table
 * in order to find the path to the binary.
 *
 * Then fork a child and pass the path and the additional arguments
 * to execve() in the child.  Wait for exeuction to complete
 * before returning.
 *
 * stdin is a file handle to be used for standard in.
 * stdout is a file handle to be used for standard out.
 *
 * If stdin and stdout are not 0 and 1, respectively, they will be
 * closed in the parent process before this function returns.
 *
 * job_id is the job_id allocated in create_job
 *
 * Returns 0 on success, -errno on failure to create the child.
 *
 */
int run_command(char *args[MAX_ARGS], int stdin, int stdout, int job_id) {
    /* Lab 2: Your code here */
    int rv = 0;
    struct job *s = find_job(job_id, false);
    if (s == NULL) {
        return -errno;
    }
    extern char **environ;
    if (args[0] == NULL) {
        return -errno;
    }
    char *cmd = NULL;
    if (args[0] == "/" || args[0] == ".") {
        cmd = strdup(args[0]);
    } else {
        int i = 0;
        while (path_table[i] != NULL) {
            // Calculate required buffer size
            size_t path_len = strlen(path_table[i]) + strlen(args[0]) + 2;
            char *tmp = malloc(path_len);

            if (!tmp) {
                return -ENOMEM;
            }

            // Build path safely
            snprintf(tmp, path_len, "%s/%s", path_table[i], args[0]);

            // Check if executable exists and is accessible
            if (access(tmp, X_OK) == 0) {
                cmd = strdup(tmp);
                free(tmp);
                break;
            }

            free(tmp);
            i++;
        }
    }
    if (cmd == NULL) {
        return -ENOENT;
    }

    pid_t pid = fork();

    if (pid < 0) {
        return -errno;
    }

    if (pid == 0) {
        if (stdin != STDIN_FILENO) {
            dup2(stdin, STDIN_FILENO);
            close(stdin);
        }

        if (stdout != STDOUT_FILENO) {
            dup2(stdout, STDOUT_FILENO);
            close(stdout);
        }

        execve(cmd, args, environ);
        _exit(-errno);
    }

    free(cmd);

    struct kiddo *k = (struct kiddo *)malloc(sizeof(struct kiddo));
    k->pid = pid;
    k->next = s->kidlets;
    s->kidlets = k;

    if (stdin != STDIN_FILENO) close(stdin);
    if (stdout != STDOUT_FILENO) close(stdout);

    // Suppress the compiler warning that find_job is not used in the starer
    // code. You may remove this line if/when you use find_job in your code.
    //(void)&find_job;

    return 0;
}

/* Wait for the job to complete and free internal bookkeeping
 *
 * job_id is the job_id allocated in create_job
 *
 * exit_code is the exit code from the last child process, if it executed.
 *           This parameter may be NULL, and is only set if the return
 *           value is zero.  This is the same as the wstatus parameter
 *           to waitpid variants, and can be used with functions such
 *           as WIFEXITED.  If this job includes multiple
 *           processes, the exit code will be the last process.
 *
 * Returns zero on success, -errno on error.
 */
int wait_on_job(int job_id, int *exit_code) {
    struct job *s = find_job(job_id, true);
    if (s == NULL) {
        return -errno;
    }

    int last_status = 0;
    while (s->kidlets != NULL) {
        int status;
        pid_t res = waitpid(s->kidlets->pid, &status, 0);
        if (res < 0) {
            return -errno;
        }

        last_status = status;
        struct kiddo *next = s->kidlets->next;
        free(s->kidlets);
        s->kidlets = next;
    }

    *exit_code = last_status;
    free(s);
    return 0;
}