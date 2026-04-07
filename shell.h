// ================================================
// shell.h — MyShell Header File
// All structs, constants, and function declarations
// ================================================

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>

// ─── Constants ───────────────────────────────────
#define MAX_INPUT       4096
#define MAX_ARGS        256
#define MAX_PIPES       16
#define MAX_JOBS        64
#define HISTORY_SIZE    100
#define SHELL_NAME      "myshell"

// ─── ANSI Color Codes ────────────────────────────
#define COLOR_RESET     "\033[0m"
#define COLOR_GREEN     "\033[1;32m"
#define COLOR_CYAN      "\033[1;36m"
#define COLOR_RED       "\033[1;31m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_BLUE      "\033[1;34m"
#define COLOR_BOLD      "\033[1m"

// ─── Job States ──────────────────────────────────
typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobState;

// ─── Single Command Struct ───────────────────────
// Represents one command in a pipeline e.g. "grep foo"
typedef struct {
    char  *args[MAX_ARGS];   // argument vector, NULL-terminated
    int    argc;             // argument count
    char  *input_file;       // input redirection  <
    char  *output_file;      // output redirection >
    int    append_output;    // append mode        >>
} Command;

// ─── Pipeline Struct ─────────────────────────────
// Represents a full pipeline e.g. "ls | grep .c | wc -l"
typedef struct {
    Command  cmds[MAX_PIPES];  // array of commands
    int      num_cmds;         // how many commands
    int      background;       // run in background &
} Pipeline;

// ─── Job Table Entry ─────────────────────────────
typedef struct {
    int       id;               // job number [1], [2]...
    pid_t     pgid;             // process group id
    char      cmd[MAX_INPUT];   // original command string
    JobState  state;
} Job;

// ─── Global Shell State ──────────────────────────
extern Job     job_table[MAX_JOBS];
extern int     job_count;
extern char   *history[HISTORY_SIZE];
extern int     history_count;
extern pid_t   shell_pgid;
extern int     shell_terminal;

// ─── Function Declarations ───────────────────────

// shell.c
void shell_init(void);
void shell_loop(void);
void print_prompt(void);

// parser.c
Pipeline *parse_input(char *input);
void      free_pipeline(Pipeline *p);
void      debug_pipeline(Pipeline *p);

// executor.c
int  execute_pipeline(Pipeline *p);
int  execute_single(Command *cmd, int in_fd, int out_fd);

// builtins.c
int  is_builtin(const char *cmd);
int  run_builtin(Command *cmd);
void builtin_cd(Command *cmd);
void builtin_help(void);
void builtin_history(void);
void builtin_jobs(void);
void builtin_exit_shell(Command *cmd);

// jobs.c
int   add_job(pid_t pgid, const char *cmd, JobState state);
void  remove_job(pid_t pgid);
void  update_jobs(void);
void  print_jobs(void);
Job  *find_job_by_pgid(pid_t pgid);

// signals.c
void setup_signal_handlers(void);
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

// history
void add_history(const char *cmd);
char *get_history(int index);

#endif // SHELL_H