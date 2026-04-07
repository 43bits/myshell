// ================================================
// executor.c — Fork / Exec / Pipe Logic
// Handles: single commands, full pipelines, I/O redirect
// ================================================

#include "shell.h"

// ─── Apply I/O redirection for a command ─────────
static void apply_redirections(Command *cmd) {
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, COLOR_RED "%s: %s: %s\n" COLOR_RESET,
                    SHELL_NAME, cmd->input_file, strerror(errno));
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT | (cmd->append_output ? O_APPEND : O_TRUNC);
        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, COLOR_RED "%s: %s: %s\n" COLOR_RESET,
                    SHELL_NAME, cmd->output_file, strerror(errno));
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

// ─── Execute a single command (child process) ────
int execute_single(Command *cmd, int in_fd, int out_fd) {
    if (cmd->argc == 0) return 0;

    // Wire up pipe file descriptors
    if (in_fd != STDIN_FILENO) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }
    if (out_fd != STDOUT_FILENO) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }

    // Apply file redirections (overrides pipe if both present)
    apply_redirections(cmd);

    // Reset signals to default in child
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    execvp(cmd->args[0], cmd->args);

    // execvp only returns on error
    fprintf(stderr, COLOR_RED "%s: %s: %s\n" COLOR_RESET,
            SHELL_NAME, cmd->args[0], strerror(errno));
    exit(EXIT_FAILURE);
}

// ─── Execute a full pipeline ──────────────────────
// e.g.  ls -la | grep .c | wc -l
int execute_pipeline(Pipeline *p) {
    if (!p || p->num_cmds == 0) return 0;

    // Single command — handle builtins here
    if (p->num_cmds == 1) {
        if (is_builtin(p->cmds[0].args[0]))
            return run_builtin(&p->cmds[0]);
    }

    int     num   = p->num_cmds;
    pid_t   pids[MAX_PIPES];
    int     pipes[MAX_PIPES][2];   // pipes[i] connects cmd[i] -> cmd[i+1]
    pid_t   pgid  = 0;

    // Create all pipes upfront
    for (int i = 0; i < num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return -1;
        }
    }

    // Fork each command
    for (int i = 0; i < num; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            // ── Child ──────────────────────────────
            // Determine stdin: pipe from previous or default
            int in_fd  = (i == 0)       ? STDIN_FILENO  : pipes[i-1][0];
            // Determine stdout: pipe to next or default
            int out_fd = (i == num - 1) ? STDOUT_FILENO : pipes[i][1];

            // Close all unneeded pipe ends in child
            for (int j = 0; j < num - 1; j++) {
                if (j != i - 1) close(pipes[j][0]);
                if (j != i)     close(pipes[j][1]);
            }

            execute_single(&p->cmds[i], in_fd, out_fd);
            // execute_single calls exit() — never returns
        }

        // ── Parent ─────────────────────────────────
        pids[i] = pid;

        // Put all processes in same process group
        if (pgid == 0) pgid = pid;
        setpgid(pid, pgid);
    }

    // Parent closes all pipe ends
    for (int i = 0; i < num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Background job — register and don't wait
    if (p->background) {
        // Build command string for job table
        char cmd_str[MAX_INPUT] = {0};
        for (int i = 0; i < num; i++) {
            for (int j = 0; j < p->cmds[i].argc; j++) {
                strncat(cmd_str, p->cmds[i].args[j], MAX_INPUT - strlen(cmd_str) - 2);
                strncat(cmd_str, " ",                MAX_INPUT - strlen(cmd_str) - 1);
            }
            if (i < num - 1) strncat(cmd_str, "| ", MAX_INPUT - strlen(cmd_str) - 1);
        }
        int jid = add_job(pgid, cmd_str, JOB_RUNNING);
        printf("[%d] %d\n", jid, pgid);
        return 0;
    }

    // Foreground — wait for all children
    int status = 0;
    for (int i = 0; i < num; i++) {
        int s;
        waitpid(pids[i], &s, WUNTRACED);
        if (i == num - 1) status = WEXITSTATUS(s);
    }

    return status;
}