
#include "shell.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>



void read_command(char* buff, char* tokens[], _Bool* in_background)
{
    char* tempBuf = NULL;
    *in_background = false;

    // Read input
    long length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

    if (buff[0] != '\n' && buff[0] != '\0')
    {
        if ( (length < 0) && (errno != EINTR) )
        {
            perror("Unable to read command. Terminating.\n");
            exit(-1); /* terminate with error */
        }

        // Null terminate and strip \n.
        buff[length] = '\0';
        if (buff[strlen(buff) - 1] == '\n') {
            buff[strlen(buff) - 1] = '\0';
        }
        tempBuf = (char*)malloc(length+1);
        strcpy(tempBuf, buff);

        // Tokenize (saving original command string)
        int token_count = tokenize_command(tempBuf, tokens);
        if (token_count == 0) {
            return;
        }

        extractAmpersand(token_count, tokens, in_background);
    }
    if (buff[0] == '\0')
        memset(tokens, 0, NUM_TOKENS);
}

void extractAmpersand(int token_count, char* tokens[], _Bool* in_background)
{
    // Extract if running in background:
     if (token_count > 0 && ((strcmp(tokens[token_count - 1], "&") == 0) ||
         ((tokens[token_count-1])[strlen(tokens[token_count-1]) - 1] == '&')))
     {
         *in_background = true;

         if ((tokens[token_count-1])[strlen(tokens[token_count-1]) - 1] == '&')
             (tokens[token_count-1])[strlen(tokens[token_count-1]) - 1] = 0;
         else
             tokens[token_count - 1] = 0;
     }
}

int tokenize_command(char* buff, char* tokens[])
{
    int i = 0;
    tokens[i++] = strtok(buff, " ");
    while ((tokens[i++] = strtok(NULL, " ")) != NULL)
        ;

    return i - 1;
}

int getPrompt(char* prompt)
{
    unsigned long idx;
    if (!getcwd(prompt, COMMAND_LENGTH))
    {
        write(STDOUT_FILENO, "Can't get current directory\n", strlen("Can't get current directory\n"));
        return 0;
    }
    idx = strlen(prompt);
    prompt[idx] = '>';
    prompt[idx+1] = ' ';
    prompt[idx+2] = '\0';
    return 1;
}

void pwd_command()
{
    char buf[COMMAND_LENGTH];
    if (!getcwd(buf, COMMAND_LENGTH))
        perror("getcwd() failed");
    else
    {
        write(STDOUT_FILENO, buf, strlen(buf));
        write(STDOUT_FILENO, "\n", strlen("\n"));
    }
}

void cd_command(char* tokens[], char* prompt)
{
    if (chdir(tokens[1]) == -1)
        perror("Directory change failed");

    getcwd(prompt, COMMAND_LENGTH);
    getPrompt(prompt);
}

void add_command(char* buff)
{
    if (ind <= HISTORY_DEPTH)
        strcpy(history[ind-1], buff);
    else
    {
        int i;
        for (i = 1; i <= HISTORY_DEPTH; i++)
            strcpy(history[i-1], history[i]);
        strcpy(history[i-2], buff);
    }
    ind++;
}

int retrieve_command(char* input_buffer, int n)
{
    int i = 9, tempInd = ind-2;
    char* ch = "Invalid input\n";
    while ((n-1) != tempInd)
    {
        tempInd--;
        i--;
        if (i < 0)
        {
            write(STDOUT_FILENO, ch, strlen(ch));
            return 0;
        }
    }
    if (ind-2 >= HISTORY_DEPTH)
        strcpy(input_buffer, history[i]);
    else
        strcpy(input_buffer, history[tempInd]);

    write(STDOUT_FILENO, input_buffer, strlen(input_buffer));
    write(STDOUT_FILENO, "\n", strlen("\n"));

    return 1;
}

void printHistory()
{
    int i, j;
    char ch[5];

    if (ind-1 > HISTORY_DEPTH)
    {
        j = (ind-1)-9;  //Finds the beginning of the history list.
        for (i = 0; i < HISTORY_DEPTH; i++, j++)
        {
            sprintf(ch, "%d\t", j);
            write(STDOUT_FILENO, ch, strlen(ch));
            write(STDOUT_FILENO, history[i], strlen(history[i]));
            write(STDOUT_FILENO, "\n", strlen("\n"));
        }
    }
    else
    {
        for (i = 1; i <= ind-1; i++)
        {
            sprintf(ch, "%d\t", i);
            write(STDOUT_FILENO, ch, strlen(ch));
            write(STDOUT_FILENO, history[i-1], strlen(history[i-1]));
            write(STDOUT_FILENO, "\n", strlen("\n"));
        }
    }
}
