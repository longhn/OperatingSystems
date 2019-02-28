/**
 * A simple command line interpreter called "bilshell". It has a batch mode, in which
 * it can execute commands inside an input file in order. It also has an interactive
 * mode, in which the user specifies the command to execute. "bilshell" also supports
 * composed command execution upto two commands. The code below makes use of various
 * system calls and pipes as an Inter Process Communication (IPC) mechanism.
 * @author Efe Acer
 * @version 1.0
 */

// Necessary imports
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

// Definitions
#define clearTerminal() printf("\033[H\033[J")
#define MAX_LETTERS 1000 // maximum number of letters allowed in a command
#define MAX_ARGS 10 // maximum number of arguments allowed in a command
#define READ_END 0
#define WRITE_END 1
#define STDIN_FD 0
#define STDOUT_FD 1

// Global Variable(s)
unsigned int N = 10000;

// Funtion declerations
void initInteractiveMode();
void initBatchMode();
void printCurrDir();
void runBatchMode(char fileName[]);
void runInteractiveMode();
void readInput(char command[]);
void parseCommand(char command[], char* argv1[]);
int parseComposedCommand(char command[], char* commands[]);
int parseUnknownCommand(char command[], char* argv1[], char* argv2[]);
int handleBuiltInCommands(char* argv1[]);
void executeCommand(char* argv1[]);
void executeComposedCommand(char* argv1[], char* argv2[]);
void handleCommand(char command[]);

// Main function
int main(int argc, char* argv[]) {
    if (argc > 1) {
        N = atoi(argv[1]); // get the value of N (will be used in experiments)
    }
    int isBatchMode = (argc == 3);
    if (isBatchMode) {
        initBatchMode();
        runBatchMode(argv[2]);
    } else {
        initInteractiveMode();
        runInteractiveMode();
    }
    return 0;
}

/**
 * Prints a greeting message to inform that the shell is
 * starting up in the interactive mode.
 */
void initInteractiveMode() {
    clearTerminal();
    printf("Welcome to Bilshell's interactive mode.\nYou can type system "
           "calls together with their arguments to execute them.\nType \"exit\" "
           "to quit and \"help\" to read the manual.\nHave fun!\n");
}

/**
 * Prints a greeting message to inform that the shell is
 * starting up in the interactive mode.
 */
void initBatchMode() {
    clearTerminal();
    printf("Welcome to Bilshell's batch mode.\nThe systems calls that are "
           "specified in the particular file given as\nargument will be executed "
           "in order.\nHave fun!\n");
}

/**
 * Prints the current working directory.
 */
void printCurrDir() {
    char directoryString[1024];
    // Get full pathname of the current directory using getcwd() system call
    getcwd(directoryString, sizeof(directoryString));
    printf("\nCurrent Directory: %s", directoryString);
}

/**
 * Runs the program in the batch mode, in which commands inside a file are
 * consecutively executed. Each command corresponds to a line in the file.
 * @param fileName The strinf for the name of the file containing the commands
 */
void runBatchMode(char fileName[]) {
    FILE* fp = fopen(fileName, "r");
    char buffer[MAX_LETTERS];
    printf("\nStarting execution of the commands in the file.\n");
    while (fgets(buffer, MAX_LETTERS, fp)) {
        buffer[strcspn(buffer, "\n\r")] = '\0'; // remove the newline at the end
        char command[strlen(buffer)];
        strcpy(command, buffer);
        handleCommand(command);
    }
    fclose(fp);
}

/**
 * Runs the program in the interactive mode, where the user enters the commands.
 */
void runInteractiveMode() {
    char command[MAX_LETTERS];
    while (1) {
        readInput(command);
        handleCommand(command);
    }
}

/**
 * Reads a line from stdin and saves it into the command string.
 * @param command The string into which the entered line is saved.
 */
void readInput(char command[]) {
    char buffer[MAX_LETTERS];
    printCurrDir();
    printf("\nbilshell-$: ");
    fgets(buffer, MAX_LETTERS, stdin);
    buffer[strcspn(buffer, "\n\r")] = '\0'; // remove the newline at the end
    strcpy(command, buffer);
}

/**
 * Given a command, parses it and constructs its argument vector.
 * @param command The command as a string
 * @param argv1 The argument vector corresponding to the command
 */
void parseCommand(char command[], char* argv1[]) {
    for (int i = 0; i < MAX_ARGS + 1; i++) {
        argv1[i] = strsep(&command, " ");
        if (argv1[i] == NULL) {
            break;
        }
        i -= (strlen(argv1[i]) == 0);
    }
}

/**
 * Tries to parse a given command into two separate commands. If the
 * command cannot be parsed in such a way, returns 0 (false), otherwise
 * returns 1 (true).
 * @param command The command as a string
 * @param commands The two separate commands that composes the first one,
 *                 if the command can be composed
 * @return 0 (false) if the command cannot be composed into two  separate
 *         commands, 1 (true) if it can be composed
 */
int parseComposedCommand(char command[], char* commands[]) {
    for (int i = 0; i < 2; i++) {
        commands[i] = strsep(&command, "|");
        if (commands[i] == NULL) {
            break;
        }
    }
    // return 1 (true) if the command is a composed one, 0 (false) otherwise
    return (commands[1] != NULL);
}

/**
 * Given an unknown command, meaning that we do not know whether it is
 * a composed command of two parts or a simple command, parses the command
 * and constructs argument vector(s). A single argument vector is formed
 * (argv1) if the command is a simple command, two argument vectors (argv1
 * and argv2) are formed if the command is a composed command. If the command
 * is "exit", quits the program. Returns a flag "isComposedCommand" that is
 * 0 (false) if the command is a simple one and 1 (true) otherwise. The value
 * of the flag, however, is -1 if the command is a built in command.
 * @param command The unknown command as a string
 * @param argv1 The argument vector for the first part of the composed command
 *              or for the simple command, depending on the particular case
 * @param argv2 The argument vector for the second part of the composed command,
 *              if, of course, the command is a composed one
 * @return isComposedCommand a flag that is 0 (false) if the command is
 *                           a simple one and 1 (true) otherwise, an exception
 *                           is that the flag is -1 if the command is a built in one
 */
int parseUnknownCommand(char command[], char* argv1[], char* argv2[]) {
    char* commands[2];
    int isComposedCommand = parseComposedCommand(command, commands);
    if (isComposedCommand) {
        parseCommand(commands[0], argv1);
        parseCommand(commands[1], argv2);
    } else {
        parseCommand(command, argv1);
    }
    int isBuiltInCommand = handleBuiltInCommands(argv1);
    if (isBuiltInCommand) {
        return -1;
    }
    return isComposedCommand;
}

/**
 * Handles the execution of the built in commands (additional ones) such as
 * exit (used to quit bilshell), cd (used to change the directory) and help
 * (used to print a manual of bilshell). Returns 0 (false) if the command
 * is not a built in command, 1 (true) otherwise.
 * @param argv1 The argument vector of the built in command
 * @return 0 (false) if the command is not a built in command, 1 (true) otherwise
 */
int handleBuiltInCommands(char* argv1[]) {
    if (strcmp(argv1[0], "exit") == 0) { // quit
        printf("\nThank you for using Bilshell :)\n");
        exit(0); // successfully exit
    } else if (strcmp(argv1[0], "cd") == 0) {
        chdir(argv1[1]); // change directory
        return 1;
    } else if (strcmp(argv1[0], "help") == 0) {
        printf("\nBILSHELL:\nA simple command line interpreter that supports "
               "the following commands:\n> exit\n> cd\n> help\n> many UNIX commands"
               "\n> composed commands of two\n");
        return 1;
    }
    return 0;
}

/**
 * Given the argument vector of a command, executes it as a separate process.
 * @param argv1 Argument vector of the command
 */
void executeCommand(char* argv1[]) {
    pid_t pid = fork(); // fork a child
    if (pid < 0) { // error case
        fprintf(stderr, "\nFork failed.");
        exit(1);
    } else if (pid == 0) { // child process
        if (execvp(argv1[0], argv1) < 0) {
            fprintf(stderr, "\nCommand execution failed.");
            exit(1);
        }
        exit(0); // successfully exit
    } else { // parent process
        wait(NULL); // wait for the child to complete
    }
}

/**
 * Given the argument vectors for the two parts of a composed commands, executes the
 * parts in order such that the output of the first part is fed as input to the second
 * part via a pipe. Additionaly, prints some statistics about number of bytes transferred
 * between the pipes, number of calls to read and number of calls to write.
 * @param argv1 Argument vector of the first part of the composed command
 * @param argv2 Argument vector of the second part of the composed command
 */
void executeComposedCommand(char* argv1[], char* argv2[]) {
    // some statistics
    int bytesTransferred = 0;
    int readCount = 0;
    int writeCount = 0;
    int fd1[2]; // pipe 1
    if (pipe(fd1) < 0) {
        fprintf(stderr, "\nPipe 1 failed.");
        exit(1);
    }
    int fd2[2]; // pipe 2
    if (pipe(fd2) < 0) {
        fprintf(stderr, "\nPipe 2 failed.");
        exit(1);
    }
    pid_t pid1 = fork(); // fork child 1
    if (pid1 < 0) {
        fprintf(stderr, "\nFork failed for child 1.");
        exit(1);
    } else if (pid1 == 0) { // child 1
        close(fd2[READ_END]); // close unused ends
        close(fd2[WRITE_END]);
        close(fd1[READ_END]);
        dup2(fd1[WRITE_END], STDOUT_FD);
        if (execvp(argv1[0], argv1) < 0) {
            fprintf(stderr, "\nExecution of the first command failed.");
            exit(1);
        }
    } else {
        pid_t pid2 = fork(); // fork child 2
        if (pid2 < 0) {
            fprintf(stderr, "\nFork failed for child 2.");
            exit(1);
        } else if (pid2 == 0) { // child 2
            close(fd1[READ_END]); // close unused ends
            close(fd1[WRITE_END]);
            close(fd2[WRITE_END]);
            dup2(fd2[READ_END], STDIN_FD);
            if (execvp(argv2[0], argv2) < 0) {
                fprintf(stderr, "\nExecution of the second command failed.");
                exit(1);
            }
        } else { // parent
            close(fd1[WRITE_END]); // close unused ends
            close(fd2[READ_END]);
            // Transfer the bytes written to pipe 1 by child 1 to pipe 2
            int bytesRead; // statistic
            char buffer[N];
            while ((bytesRead = read(fd1[READ_END], buffer, N)) > 0) {
                int bytesWritten = write(fd2[WRITE_END], buffer, bytesRead);
                bytesTransferred += bytesRead + bytesWritten;
                readCount++;
                writeCount++;
            }
            readCount++;
            close(fd1[READ_END]); // close unused ends
            close(fd2[WRITE_END]);
            wait(NULL);
            wait(NULL);
            printf("\ncharacter-count: %d\nread-call-count: %d\nwrite-call-count: %d\n",
                   bytesTransferred, readCount, writeCount);
        }
    }
}

/*
 * Given any command (simple, composed or built in), parses and executes
 * it using the helper functions implemented above.
 * @param command The string representing the command.
 */
void handleCommand(char command[]) {
    char* argv1[MAX_ARGS + 1];
    char* argv2[MAX_ARGS + 1];
    int isComposedCommand = parseUnknownCommand(command, argv1, argv2);
    if (isComposedCommand == 1) {
        executeComposedCommand(argv1, argv2);
    } else if (isComposedCommand == 0) {
        executeCommand(argv1);
    }
}
