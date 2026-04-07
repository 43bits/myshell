// ================================================
// jobs.c — Background Job Table Management
// ================================================

#include "shell.h"

Job job_table[MAX_JOBS];
int job_count = 0;

// ─── Add a new job ────────────────────────────────
int add_job(pid_t pgid, const char *cmd, JobState state) {
    // Find an empty slot
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].pgid == 0) {
            job_table[i].id    = i + 1;
            job_table[i].pgid  = pgid;
            job_table[i].state = state;
            strncpy(job_table[i].cmd, cmd, MAX_INPUT - 1);
            if (i >= job_count) job_count = i + 1;
            return i + 1;
        }
    }
    fprintf(stderr, "jobs: job table full\n");
    return -1;
}

// ─── Remove a job by pgid ────────────────────────
void remove_job(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].pgid == pgid) {
            memset(&job_table[i], 0, sizeof(Job));
        }
    }
}

// ─── Find job by pgid ─────────────────────────────
Job *find_job_by_pgid(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (job_table[i].pgid == pgid) return &job_table[i];
    return NULL;
}

// ─── Poll all jobs for state changes ─────────────
void update_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].pgid == 0) continue;

        int status;
        pid_t result = waitpid(-job_table[i].pgid, &status,
                               WNOHANG | WUNTRACED | WCONTINUED);

        if (result > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                job_table[i].state = JOB_DONE;
            } else if (WIFSTOPPED(status)) {
                job_table[i].state = JOB_STOPPED;
            } else if (WIFCONTINUED(status)) {
                job_table[i].state = JOB_RUNNING;
            }
        }
    }
}

// ─── Print all active jobs ────────────────────────
void print_jobs(void) {
    int any = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].pgid == 0) continue;
        any = 1;

        const char *state_str;
        const char *color;

        switch (job_table[i].state) {
            case JOB_RUNNING: state_str = "Running";  color = COLOR_GREEN;  break;
            case JOB_STOPPED: state_str = "Stopped";  color = COLOR_YELLOW; break;
            case JOB_DONE:    state_str = "Done";      color = COLOR_BLUE;   break;
            default:          state_str = "Unknown";   color = COLOR_RESET;  break;
        }

        printf("[%d] %s%-8s%s  (pgid %d)  %s\n",
               job_table[i].id, color, state_str,
               COLOR_RESET, job_table[i].pgid, job_table[i].cmd);

        // Clean up done jobs after displaying
        if (job_table[i].state == JOB_DONE)
            memset(&job_table[i], 0, sizeof(Job));
    }
    if (!any) printf("  (no background jobs)\n");
}