
#ifndef SHELL_h
#define SHELL_h

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2+1)
#define HISTORY_DEPTH 10
#define HISTORY_SIZE 1000

extern char history[HISTORY_DEPTH][COMMAND_LENGTH];
extern int ind;

void read_command(char* buff, char* tokens[], _Bool* in_background);

void extractAmpersand(int token_count, char* tokens[], _Bool* in_background);

int tokenize_command(char* buff, char* tokens[]);

int getPrompt(char* prompt);

void pwd_command();

void cd_command(char* tokens[], char* prompt);

void add_command(char* buff);

int retrieve_command(char* buff, int n);

void printHistory();



#endif /* SHELL_h */
