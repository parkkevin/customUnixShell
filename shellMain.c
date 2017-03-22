
#include "shell.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

char history[HISTORY_DEPTH][COMMAND_LENGTH];
int ind = 1;

void handle_SIGINT(int mySignal)
{
    write(STDOUT_FILENO, "\n", strlen("\n"));
    printHistory();
}

int main(int argc, const char * argv[])
{
    char input_buffer[COMMAND_LENGTH];
    char* tokens[NUM_TOKENS];
    char prompt[COMMAND_LENGTH];
    int retrieveFlag = 0;
    _Bool in_background = false;
    struct timespec ns;
    ns.tv_sec = 0;
    ns.tv_nsec = 100000000L;
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    handler.sa_flags = 0;
    sigemptyset(&handler.sa_mask);
    sigaction(SIGINT, &handler, NULL);
    char tempInputBuffer[COMMAND_LENGTH];

    int isPrompt = getPrompt(prompt);
    if (!isPrompt)
        strcpy(prompt, "> ");
    while (true)
    {
        if (!retrieveFlag)
        {
            memset(input_buffer, 0, sizeof(input_buffer));
            write(STDOUT_FILENO, prompt, strlen(prompt));
            in_background = false;
            read_command(input_buffer, tokens, &in_background);
        }
        else
        {
            strcpy(tempInputBuffer, input_buffer);
            int token_count = tokenize_command(tempInputBuffer, tokens);
            extractAmpersand(token_count, tokens, &in_background);
            retrieveFlag = 0;
        }

        if (strncmp(input_buffer, "\n", 1) == 0 || tokens[0] == NULL)
            continue;

        if ((tokens[0])[0] != '!')
            add_command(input_buffer);

        if (strcmp(tokens[0], "exit") == 0)
            exit(0);
        else if (strcmp(tokens[0], "pwd") == 0)
            pwd_command();
        else if (strcmp(tokens[0], "cd") == 0)
            cd_command(tokens, prompt);
        else if (strcmp(tokens[0], "history") == 0)
            printHistory(ind-1);
        else if ((tokens[0])[0] == '!')
        {
            int n;

            if ((tokens[0])[1] == '!')  // IF input is '!!'.
            {
            	if (strlen(&(tokens[0])[1]) > 1)
					continue;
                if (retrieve_command(input_buffer, ind-1))
                    retrieveFlag = 1;
            }
            else if (!(tokens[0]+1) || (tokens[0])[1] == '\n' || (tokens[0])[1] == '-' ||
                !(n = atoi(tokens[0]+1)) || n >= ind)
            	continue;
            else
            {
                if (retrieve_command(input_buffer, n))
                    retrieveFlag = 1;
            }
        }
        else
        {
            pid_t pid = fork();

            if (pid < 0)
            {
                perror("Fork failure occurred");
                exit(EXIT_FAILURE);
            }
            if (!pid)
            {
                if (execvp(tokens[0], tokens))
                {
                    perror("Exe() failed");
                    exit(EXIT_FAILURE);
                }
            }
            if (!in_background)
                waitpid(pid, NULL, 0);
            else
                nanosleep(&ns, NULL);
        }

        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;
    }


    return 0;
}
