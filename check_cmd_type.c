#include "main.h"

extern char prompt[50];
pid_t pid = 0;

char *get_command(char *input_string)
{
    // int i = 0;

    // // get the first word of input_string and return it
    // while(input_string[i] != ' ' && input_string[i] != '\0')
    // {
    //     i++;
    // }

    // input_string[i] = '\0';

    // return input_string;

    static char command[100];
    int i = 0;

    // get the first word of input_string and return it
    while (input_string[i] != ' ' && input_string[i] != '\0')
    {
        command[i] = input_string[i];
        i++;
    }

    command[i] = '\0';

    return command;
}

int check_command_type(char *command)
{
    // either external or internal
    // List of builtin commands
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
                        "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
                        "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "fg", "bg", "jobs", NULL};

    // run a loop
    int i = 0;

    // use strcmp and compare command with bulitins 2D array
    while (builtins[i] != NULL)
    {
        // if match found return => return BUILTIN
        if (strcmp(command, builtins[i]) == 0)
        {
            return BUILTIN;
        }
        i++;
    }

    // if cmd is not builtin
    char *external_commands[200] = {NULL};

    extract_external_commands(external_commands);

    // run a loop
    i = 0;

    // use strcmp and compare command with external_commands 2D pointer
    while (external_commands[i] != NULL)
    {
       // printf("DEBUG: command = [%s], external = [%s]\n", command, external_commands[i]);

        // if match found => return EXTERNAL
        if (strcmp(command, external_commands[i]) == 0)
        {
            return EXTERNAL;
        }
        i++;
    }

    // if cmd is not external also, return NO_COMMAND
    return NO_COMMAND;
}

void extract_external_commands(char **external_commands)
{
    char buff[100];
    char ch;

    int i = 0;
    int j = 0;

    int fd = open("externalcmds.txt", O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        return;
    }

    while (read(fd, &ch, 1) > 0)
    {
        // Ignore Windows carriage return
        if (ch == '\r')
        {
            continue;
        }

        // End of one command
        if (ch == '\n')
        {
            if (j > 0)
            {
                buff[j] = '\0';

                external_commands[i] = malloc(j + 1);

                strcpy(external_commands[i], buff);

                i++;
                j = 0;
            }
        }
        else
        {
            buff[j] = ch;
            j++;
        }
    }

    // Important: handle last command if no newline at EOF
    if (j > 0)
    {
        buff[j] = '\0';

        external_commands[i] = malloc(j + 1);

        strcpy(external_commands[i], buff);

        i++;
    }

    external_commands[i] = NULL;

    close(fd);
}

void execute_external_cmds(char *input_string, int *last_status)
{
    // convert input_string from 1D array into 2D array
    char *argv[20];

    int i = 0;

    argv[i] = strtok(input_string, " ");

    while (argv[i] != NULL)
    {
        i++;
        argv[i] = strtok(NULL, " ");
    }

    // create a child
    pid = fork();

    if (pid == 0)
    {
        // child executes external command
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        execvp(argv[0], argv);
    }
    else if (pid > 0)
    {
        // parent waits for child
        int status;

        waitpid(pid, &status, WUNTRACED);

        if (WIFEXITED(status))
        {
            *last_status = WEXITSTATUS(status);
            pid = 0;
        }
        else if (WIFSTOPPED(status))
        {
            printf("DEBUG: Process stopped, PID = %d\n", pid);

            // Add stopped process to linked list
            insert_first(&head, pid, input_string);

            printf("DEBUG: Job inserted\n");
            
            pid = 0;
        }
    }
}

void n_pipe(char *input_string)
{
    // Implementation
    char *commands[20];
    int command_count = 0;

    // Step 1: Split the complete input using '|'
    commands[command_count] = strtok(input_string, "|");

    while (commands[command_count] != NULL)
    {
        command_count++;
        commands[command_count] = strtok(NULL, "|");
    }

    // Stores the read end of the previous pipe
    int previous_read = -1;

    // Store all child PIDs
    pid_t child_pid[20];

    // Step 2: Create processes for every command
    for (int i = 0; i < command_count; i++)
    {
        int fd[2];

        // Create pipe except for last command
        if (i < command_count - 1)
        {
            if (pipe(fd) == -1)
            {
                perror("pipe");
                return;
            }
        }

        // Create child
        pid = fork();

        if (pid == -1)
        {
            perror("fork");
            return;
        }

        if (pid == 0)
        {
            // Child should handle signals normally
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // Input from previous pipe
            if (previous_read != -1)
            {
                dup2(previous_read, STDIN_FILENO);

                close(previous_read);
            }

            // Output to next pipe
            if (i < command_count - 1)
            {
                dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                close(fd[1]);
            }

            // Convert current command into argv
            char *argv[20];
            int j = 0;

            argv[j] = strtok(commands[i], " ");

            while (argv[j] != NULL)
            {
                j++;
                argv[j] = strtok(NULL, " ");
            }

            // Execute command
            execvp(argv[0], argv);

            perror("execvp");
            exit(1);
        }
        else
        {
            // Parent stores child PID
            child_pid[i] = pid;

            // Parent no longer needs previous pipe
            if (previous_read != -1)
            {
                close(previous_read);
            }

            // Save current pipe read end
            // for next command
            if (i < command_count - 1)
            {
                close(fd[1]);

                previous_read = fd[0];
            }
        }
    }

    // Step 3: Wait for all child processes
    for (int i = 0; i < command_count; i++)
    {
        waitpid(child_pid[i], NULL, 0);
    }

    // Shell is waiting for next command
    pid = 0;
}

void signal_handler(int sig_num)
{

    if (sig_num == SIGINT || sig_num == SIGTSTP)
    {
        if (pid == 0)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
    }
    else if (sig_num == SIGCHLD)
    {
        int status;

        // collect terminated background child
        while (waitpid(-1, &status, WNOHANG) > 0)
        {
            // Child Process Collected
        }
    }
}
