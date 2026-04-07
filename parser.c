// ================================================
// parser.c — Input Tokenizer & Parser
// Handles: pipes, redirection, background, quotes
// ================================================

#include "shell.h"

// ─── Helper: strip leading/trailing whitespace ───
static char *trim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t')) *end-- = '\0';
    return s;
}

// ─── Expand environment variables ($HOME, $PATH) ─
static char *expand_env(char *token) {
    if (token[0] != '$') return strdup(token);
    char *val = getenv(token + 1);
    return val ? strdup(val) : strdup("");
}

// ─── Parse a single command segment ──────────────
// e.g. "grep -n foo < input.txt > out.txt"
static void parse_command(char *segment, Command *cmd) {
    cmd->argc         = 0;
    cmd->input_file   = NULL;
    cmd->output_file  = NULL;
    cmd->append_output = 0;

    char *token;
    char *rest = segment;
    char  buf[MAX_INPUT];
    strncpy(buf, segment, MAX_INPUT - 1);
    buf[MAX_INPUT - 1] = '\0';
    rest = buf;

    while ((token = strtok_r(rest, " \t", &rest))) {
        // Handle >> (append) redirection
        if (strcmp(token, ">>") == 0) {
            char *file = strtok_r(rest, " \t", &rest);
            if (file) {
                cmd->output_file   = strdup(file);
                cmd->append_output = 1;
            }
        }
        // Handle > (output) redirection
        else if (strcmp(token, ">") == 0) {
            char *file = strtok_r(rest, " \t", &rest);
            if (file) cmd->output_file = strdup(file);
        }
        // Handle < (input) redirection
        else if (strcmp(token, "<") == 0) {
            char *file = strtok_r(rest, " \t", &rest);
            if (file) cmd->input_file = strdup(file);
        }
        // Handle inline >> within token e.g. "file>>"
        else if (strstr(token, ">>")) {
            char *pos = strstr(token, ">>");
            *pos = '\0';
            if (strlen(token) > 0)
                cmd->args[cmd->argc++] = expand_env(token);
            cmd->output_file   = strdup(pos + 2);
            cmd->append_output = 1;
        }
        // Handle inline > within token
        else if (strchr(token, '>')) {
            char *pos = strchr(token, '>');
            *pos = '\0';
            if (strlen(token) > 0)
                cmd->args[cmd->argc++] = expand_env(token);
            cmd->output_file = strdup(pos + 1);
        }
        // Handle inline < within token
        else if (strchr(token, '<')) {
            char *pos = strchr(token, '<');
            *pos = '\0';
            if (strlen(token) > 0)
                cmd->args[cmd->argc++] = expand_env(token);
            cmd->input_file = strdup(pos + 1);
        }
        // Normal argument
        else {
            if (cmd->argc < MAX_ARGS - 1)
                cmd->args[cmd->argc++] = expand_env(token);
        }
    }
    cmd->args[cmd->argc] = NULL;  // NULL-terminate for execvp
}

// ─── Main Parser ─────────────────────────────────
Pipeline *parse_input(char *raw_input) {
    if (!raw_input || strlen(raw_input) == 0) return NULL;

    Pipeline *p = calloc(1, sizeof(Pipeline));
    if (!p) { perror("calloc"); return NULL; }

    char input[MAX_INPUT];
    strncpy(input, raw_input, MAX_INPUT - 1);
    input[MAX_INPUT - 1] = '\0';

    // Check for background operator &
    char *amp = strrchr(input, '&');
    if (amp) {
        p->background = 1;
        *amp = '\0';
    }

    // Split by pipe '|'
    char *segment;
    char *rest = input;
    p->num_cmds = 0;

    while ((segment = strtok_r(rest, "|", &rest))) {
        segment = trim(segment);
        if (strlen(segment) == 0) continue;
        if (p->num_cmds >= MAX_PIPES) break;

        parse_command(segment, &p->cmds[p->num_cmds]);
        p->num_cmds++;
    }

    if (p->num_cmds == 0) {
        free(p);
        return NULL;
    }

    return p;
}

// ─── Free all allocated memory in pipeline ───────
void free_pipeline(Pipeline *p) {
    if (!p) return;
    for (int i = 0; i < p->num_cmds; i++) {
        Command *cmd = &p->cmds[i];
        for (int j = 0; j < cmd->argc; j++)
            free(cmd->args[j]);
        if (cmd->input_file)  free(cmd->input_file);
        if (cmd->output_file) free(cmd->output_file);
    }
    free(p);
}

// ─── Debug: print parsed pipeline ────────────────
void debug_pipeline(Pipeline *p) {
    if (!p) return;
    printf(COLOR_YELLOW "[DEBUG] %d command(s), background=%d\n" COLOR_RESET,
           p->num_cmds, p->background);
    for (int i = 0; i < p->num_cmds; i++) {
        Command *c = &p->cmds[i];
        printf("  [cmd %d] ", i);
        for (int j = 0; j < c->argc; j++) printf("%s ", c->args[j]);
        if (c->input_file)  printf("< %s ", c->input_file);
        if (c->output_file) printf("%s %s ", c->append_output ? ">>" : ">", c->output_file);
        printf("\n");
    }
}