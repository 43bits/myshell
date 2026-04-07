// =============================================
// myshell.c — Custom Unix Shell from Scratch
// Phase 1: Prompt Loop
// =============================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 1024

void print_prompt() {
    printf("myshell> ");
    fflush(stdout);  // Force prompt to show immediately
}

int main() {
    char input[MAX_INPUT];

    printf("Welcome to MyShell! Type 'exit' to quit.\n");

    while (1) {
        print_prompt();

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nBye!\n");
            break;
        }

        // Remove the newline character at the end
        input[strcspn(input, "\n")] = 0;

        // Skip empty input
        if (strlen(input) == 0) continue;

        // Exit condition
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        // For now, just echo back what user typed
        printf("You typed: %s\n", input);
    }

    return 0;
}