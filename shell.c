
#include "shell.h"

char  *history[HISTORY_SIZE];
int    history_count = 0;
pid_t  shell_pgid;
int    shell_terminal;

void add_history(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return;

    if (history_count > 0 &&
        strcmp(history[history_count - 1], cmd) == 0) return;

    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        memmove(history, history + 1,
                (HISTORY_SIZE - 1) * sizeof(char *));
        history_count--;
    }
    history[history_count++] = strdup(cmd);
}

void print_prompt(void) {
    char cwd[1024];
    char *home = getenv("HOME");
    char *user = getenv("USER");
    if (!user) user = "user";

    if (!getcwd(cwd, sizeof(cwd))) {
        strcpy(cwd, "?");
    }

    char display_cwd[1024];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(display_cwd, sizeof(display_cwd), "~%s",
                 cwd + strlen(home));
    } else {
        strncpy(display_cwd, cwd, sizeof(display_cwd));
    }

    printf(COLOR_GREEN "%s" COLOR_RESET
           "@"
           COLOR_CYAN  "myshell" COLOR_RESET
           ":"
           COLOR_BLUE  "%s" COLOR_RESET
           " $ ",
           user, display_cwd);
    fflush(stdout);
}

void shell_init(void) {
    shell_terminal = STDIN_FILENO;

    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
        kill(-shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("setpgid");
        exit(EXIT_FAILURE);
    }

    tcsetpgrp(shell_terminal, shell_pgid);

    setup_signal_handlers();

    printf(COLOR_CYAN
    "╔══════════════════════════════════════════════╗\n"
    "║        MyShell v1.0 — Custom Unix Shell       ║\n"
    "║     Type 'help' for available commands        ║\n"
    "╚══════════════════════════════════════════════╝\n"
    COLOR_RESET "\n");
}

void shell_loop(void) {
    char input[MAX_INPUT];

    while (1) {
        update_jobs();

        print_prompt();

        if (!fgets(input, sizeof(input), stdin)) {
            if (feof(stdin)) {
                printf(COLOR_CYAN "\nGoodbye! 👋\n" COLOR_RESET);
                exit(EXIT_SUCCESS);
            }
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        char *trimmed = input;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (strlen(trimmed) == 0) continue;

        add_history(trimmed);

        Pipeline *p = parse_input(trimmed);
        if (!p) continue;

        execute_pipeline(p);

        free_pipeline(p);
    }
}

int main(void) {
    shell_init();
    shell_loop();
    return EXIT_SUCCESS;
}