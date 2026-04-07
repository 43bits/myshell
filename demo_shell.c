#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 1024

void print_prompt() {
    printf("myshell> ");
    fflush(stdout);  
}

int main() {
    char input[MAX_INPUT];

    printf("Welcome to MyShell! Type 'exit' to quit.\n");

    while (1) {
        print_prompt();

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\nBye!\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        printf("You typed: %s\n", input);
    }

    return 0;
}