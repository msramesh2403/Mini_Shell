/*
NAME: M S RAMESH
SEC: 26001B_105
PROJECT: MINISHELL
DES: Implement a minimalistic shell, mini-shell(msh) as part of the Linux Internal module

*/




#include "main.h"


char prompt[50] = "minishell$";     //default prompt
Slist *head = NULL;

int main()
{
    system ("clear");

    char input_string[100];
    scan_input(prompt, input_string);
}
 
