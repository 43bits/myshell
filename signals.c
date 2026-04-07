// ================================================
// signals.c — Signal Handlers
// SIGINT (Ctrl+C), SIGCHLD (child done), SIGTSTP
// ================================================

#include "shell.h"

// ─── SIGCHLD: a child process changed state ───────
// Called when background job finishes
void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;

    // Reap all finished children without blocking
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Job *j = find_job_by_pgid(pid);
        if (!j) continue;

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            printf(COLOR_BLUE "\n[%d] Done    %s\n" COLOR_RESET, j->id, j->cmd);
            remove_job(j->pgid);
        } else if (WIFSTOPPED(status)) {
            j->state = JOB_STOPPED;
            printf(COLOR_YELLOW "\n[%d] Stopped %s\n" COLOR_RESET, j->id, j->cmd);
        }
    }
}

// ─── SIGINT: Ctrl+C pressed ───────────────────────
// Don't kill the shell itself — only foreground children
void sigint_handler(int sig) {
    (void)sig;
    // Just print a newline so prompt reappears cleanly
    write(STDOUT_FILENO, "\n", 1);
}

// ─── SIGTSTP: Ctrl+Z pressed ─────────────────────
void sigtstp_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

// ─── Setup all handlers at shell startup ─────────
void setup_signal_handlers(void) {
    struct sigaction sa_chld, sa_int, sa_stp, sa_ign;

    // SIGCHLD — reap background children
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags   = SA_RESTART | SA_NOCLDSTOP;
    sa_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sa_chld, NULL);

    // SIGINT — Ctrl+C: don't kill shell
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags   = SA_RESTART;
    sa_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa_int, NULL);

    // SIGTSTP — Ctrl+Z: don't suspend shell
    sigemptyset(&sa_stp.sa_mask);
    sa_stp.sa_flags   = SA_RESTART;
    sa_stp.sa_handler = sigtstp_handler;
    sigaction(SIGTSTP, &sa_stp, NULL);

    // SIGQUIT / SIGTTOU / SIGTTIN — ignore
    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags   = 0;
    sa_ign.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa_ign, NULL);
    sigaction(SIGTTOU, &sa_ign, NULL);
    sigaction(SIGTTIN, &sa_ign, NULL);
}