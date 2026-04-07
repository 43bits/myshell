#include "shell.h"
void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;

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

void sigint_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

void sigtstp_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

void setup_signal_handlers(void) {
    struct sigaction sa_chld, sa_int, sa_stp, sa_ign;

    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags   = SA_RESTART | SA_NOCLDSTOP;
    sa_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sa_chld, NULL);

    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags   = SA_RESTART;
    sa_int.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa_int, NULL);

    sigemptyset(&sa_stp.sa_mask);
    sa_stp.sa_flags   = SA_RESTART;
    sa_stp.sa_handler = sigtstp_handler;
    sigaction(SIGTSTP, &sa_stp, NULL);

    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags   = 0;
    sa_ign.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa_ign, NULL);
    sigaction(SIGTTOU, &sa_ign, NULL);
    sigaction(SIGTTIN, &sa_ign, NULL);
}