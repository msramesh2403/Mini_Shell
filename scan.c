#include "main.h"

extern char prompt[50];

void scan_input(char *prompt, char *input_string)
{
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);

    int last_status = 0;
    while(1)
    {
        printf("%s", prompt);       //to print
        scanf(" %[^\n]", input_string);     //to store

        int len = strlen(input_string);

        while(len > 0 && input_string[len-1] == ' ')    // to remove trailing spaces
        {
            input_string[len-1] = '\0';
            len--;
        }

        //Step 1 => check whether the input_string PS1  or not
        if(strncmp(input_string, "PS1=", 4) == 0)
        {
            //if yes => check whether space is present or not
            if(strchr(&input_string[4], ' ') != NULL)
            {
                //if space present =>print msg 
                printf("PS1: comnd nt found\n");
            }
            else
            {                
                strcpy(prompt, &input_string[4]);
            }
            continue;
        }
                

        //Step 2 => check cmnd type is either external or internal
        
        char *command = get_command(input_string);
        
        int ret = check_command_type(command);
        if(ret == BUILTIN)
        {
            //1.exit 2.pwd 3.cd 4. echo 

            // compare and check
            if(strcmp(command, "exit") == 0)
            {
                exit(0);
            }
            else if(strcmp(command, "pwd") == 0)
            {
                char str[100];
                getcwd(str, sizeof(str));
                printf("%s\n", str);
            }
            else if(strcmp(command, "cd") == 0)
            {
                chdir(&input_string[3]);
            }
            else if(strcmp(command, "echo") == 0)
            {
                // Get the argument after "echo "
                char *arg = &input_string[5];

                // 1. echo $SHELL
                if(strcmp(arg, "$SHELL") == 0)
                {
                    printf("%s\n", getenv("SHELL"));
                }

                // 2. echo $?
                else if(strcmp(arg, "$?") == 0)
                {
                    // Print the last executed command exit status
                    printf("%d\n", last_status);
                }

                // 3. echo $$
                else if(strcmp(arg, "$$") == 0)
                {
                    // Print current mini shell process ID
                    printf("%d\n", getpid());
                }

                // 4. Normal echo
                else
                {
                    // Print the word after echo
                    printf("%s\n", arg);
                }

            }

            // jobs
            else if(strcmp(command, "jobs") == 0)
            {
                print_list(head);
            }

            // fg
            else if(strcmp(command, "fg") == 0)
            {
                if(head != NULL)
                {
                    pid = head->pid;

                    // Continue the stopped process
                    kill(pid, SIGCONT);

                    // Wait until it exits or stops again
                    waitpid(pid, NULL, WUNTRACED);

                    // Remove from job list
                    delete_at_first(&head);

                    pid = 0;
                }
                else
                {
                    printf("fg: no current job\n");
                }
            }

            // bg
            else if(strcmp(command, "bg") == 0)
            {
                if(head != NULL)
                {
                    //pid_t job_pid = head->pid;

                    // Continue process in background
                    kill(pid, SIGCONT);

                    // Remove from stopped job list
                    delete_at_first(&head);
                }
                else
                {
                    printf("bg: no current job\n");
                }
            }
        }
        else if(ret == EXTERNAL)
        {
            //execute the external cmd using exec
            //implement n pipe

            //check pipe is present or not
            if(strchr(input_string, '|') != NULL)
            {
                //if present
                n_pipe(input_string);
            }
            else
            {
                //if not present
                execute_external_cmds(input_string, &last_status);
            }
        }
        else
        {
            //print err msg
            printf("%s: command not found\n", command);
        }
    }
}


