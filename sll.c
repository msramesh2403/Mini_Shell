#include "main.h"


void insert_first(Slist **head, pid_t pid, char *command)
{
    Slist *new = malloc(sizeof(Slist));

    if(new == NULL)
    {
        printf("memory allocation failed\n");
    }

    new->pid = pid;

    strcpy(new->command, command);

    new->link = *head;

    *head = new;
}


void delete_at_first(Slist **head)
{
    if(*head == NULL)
    {
        return;
    }

    Slist *temp = *head;

    *head = (*head)->link;

    free(temp);
}


void print_list(Slist *head)
{
    if(head == NULL)
    {
        printf("No Jobs\n");
        return;
    }

    while(head != NULL)
    {
        printf("[%d] %s\n", head->pid, head->command);

        head = head->link;
    }

}