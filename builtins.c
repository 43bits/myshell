// ================================================
// builtins.c — Built-in Shell Commands
// cd, exit, help, history, jobs, export, pwd, clear
// ================================================

#include "shell.h"

// ─── List of all builtin command names ───────────
static const char *builtin_names[] = {
    "cd", "exit", "help", "history",
    "jobs", "export", "pwd", "clear", NULL
};

int is_builtin(const char *cmd) {
    if (!cmd) return 0;
    for (int i = 0; builtin_names[i]; i++)
        if (strcmp(cmd, builtin_names[i]) == 0) return 1;
    return 0;
}

// ─── Dispatcher ───────────────────────────────────
int run_builtin(Command *cmd) {
    if (!cmd || !cmd->args[0]) return 1;

    if (strcmp(cmd->args[0], "cd")      == 0) { builtin_cd(cmd);      return 0; }
    if (strcmp(cmd->args[0], "exit")    == 0) { builtin_exit_shell(cmd); return 0; }
    if (strcmp(cmd->args[0], "help")    == 0) { builtin_help();        return 0; }
    if (strcmp(cmd->args[0], "history") == 0) { builtin_history();     return 0; }
    if (strcmp(cmd->args[0], "jobs")    == 0) { builtin_jobs();        return 0; }
    if (strcmp(cmd->args[0], "pwd")     == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
        return 0;
    }
    if (strcmp(cmd->args[0], "clear")   == 0) {
        printf("\033[H\033[2J");
        fflush(stdout);
        return 0;
    }
    if (strcmp(cmd->args[0], "export")  == 0) {
        if (cmd->argc < 2) {
            fprintf(stderr, "export: usage: export VAR=value\n");
            return 1;
        }
        char *eq = strchr(cmd->args[1], '=');
        if (!eq) {
            fprintf(stderr, "export: invalid format, use VAR=value\n");
            return 1;
        }
        *eq = '\0';
        setenv(cmd->args[1], eq + 1, 1);
        *eq = '=';
        return 0;
    }
    return 1;
}

// ─── cd ───────────────────────────────────────────
void builtin_cd(Command *cmd) {
    const char *dir;

    if (cmd->argc < 2 || strcmp(cmd->args[1], "~") == 0) {
        dir = getenv("HOME");
        if (!dir) { fprintf(stderr, "cd: HOME not set\n"); return; }
    } else if (strcmp(cmd->args[1], "-") == 0) {
        dir = getenv("OLDPWD");
        if (!dir) { fprintf(stderr, "cd: OLDPWD not set\n"); return; }
        printf("%s\n", dir);
    } else {
        dir = cmd->args[1];
    }

    char old[1024];
    getcwd(old, sizeof(old));

    if (chdir(dir) != 0) {
        fprintf(stderr, COLOR_RED "cd: %s: %s\n" COLOR_RESET, dir, strerror(errno));
    } else {
        setenv("OLDPWD", old, 1);
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) setenv("PWD", cwd, 1);
    }
}

// ─── help ─────────────────────────────────────────
void builtin_help(void) {
    printf(COLOR_CYAN
    "╔══════════════════════════════════════════════╗\n"
    "║         MyShell — Built-in Commands          ║\n"
    "╚══════════════════════════════════════════════╝\n"
    COLOR_RESET);
    printf(COLOR_GREEN "  cd [dir]     " COLOR_RESET "Change directory (~ for home, - for prev)\n");
    printf(COLOR_GREEN "  pwd          " COLOR_RESET "Print working directory\n");
    printf(COLOR_GREEN "  export K=V   " COLOR_RESET "Set environment variable\n");
    printf(COLOR_GREEN "  history      " COLOR_RESET "Show command history\n");
    printf(COLOR_GREEN "  jobs         " COLOR_RESET "List background jobs\n");
    printf(COLOR_GREEN "  clear         " COLOR_RESET "Clear the terminal\n");
    printf(COLOR_GREEN "  help          " COLOR_RESET "Show this help message\n");
    printf(COLOR_GREEN "  exit [code]  " COLOR_RESET "Exit the shell\n");
    printf("\n");
    printf(COLOR_YELLOW "  Features:\n" COLOR_RESET);
    printf("  Pipes:        cmd1 | cmd2 | cmd3\n");
    printf("  Redirect in:  cmd < file\n");
    printf("  Redirect out: cmd > file   or   cmd >> file\n");
    printf("  Background:   cmd &\n");
    printf("  Env vars:     echo $HOME\n");
}

// ─── history ─────────────────────────────────────
void builtin_history(void) {
    for (int i = 0; i < history_count; i++)
        printf(COLOR_YELLOW "  %3d  " COLOR_RESET "%s\n", i + 1, history[i]);
    if (history_count == 0)
        printf("  (no history)\n");
}

// ─── jobs ─────────────────────────────────────────
void builtin_jobs(void) {
    update_jobs();
    print_jobs();
}

// ─── exit ─────────────────────────────────────────
void builtin_exit_shell(Command *cmd) {
    int code = 0;
    if (cmd->argc >= 2) code = atoi(cmd->args[1]);
    printf(COLOR_CYAN "Goodbye! 👋\n" COLOR_RESET);
    exit(code);
}