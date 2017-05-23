#include "fs304.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void p_compare(char *filename1, char *filename2);
void p_rename(char *oldname, char *newname);
void p_copy(char *file1, char *file2);


int main(){
	char parameters[5][30];
	char paramc = 0;
	char command[30];
	char *input1, *input2;

	mountFS304();
	strcpy(command, parameters[0]);

	while(strcmp(command,"exit") != 0){
		paramc = 0;
		for (int i = 0; i < 5; i++){
			strcpy(parameters[i], "");
		}

		printf("admin@OS:");
		printPrompt();

		do{
	    if (scanf("%s", parameters[paramc]) == 1){
					paramc += 1;
	    }
		}
		while (getchar() != '\n');
		strcpy(command, parameters[0]);


		if (strcmp(command, "rename") == 0){

		} else if (strcmp(command,"compare") == 0){

		} else if (strcmp(command,"copy") == 0){

		} else if (strcmp(command,"rd") == 0){
			rd();
		} else if (strcmp(command,"cd") == 0){
			cd(parameters[1]);
		} else if (strcmp(command,"ls") == 0){
			ls();
		} else if (strcmp(command,"md") == 0){
			md(parameters[1]);
		} else if (strcmp(command,"stats") == 0){
			stats();
		} else {
				printf("%s is not recognized as a command.\n", command);
		}
	}

	return 0;
}
