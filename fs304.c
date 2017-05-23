#include "fs304.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void p_compare(char *, char *);
void p_rename(char *, char *);
void p_copy(char *, char *);

int main(){
	char parameters[5][30];
	char paramc = 0;
	char command[30];
	char *input1, *input2;
	char buffer[1024];

	mountFS304();
	/*for (int i = 4; i < 11; i++){
		readFS304(i,buffer);
		printf("Block %d:",i);
		for (int j = 0; j < 1024; j++){
			printf("%c",buffer[j]);
		}
		printf("\n");
	}*/


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
			p_rename(parameters[1], parameters[2]);
		} else if (strcmp(command,"compare") == 0){
			p_compare(parameters[1], parameters[2]);
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

void p_compare(char *filename1, char *filename2){
	int blocks[3];
	_directory_entry _directory_entries[4];
	int i,j,z,d,t;
	int e_inode;

	int f1blocks[3];
	char f1content[3][1024];

	int f2blocks[3];
	char f2content[3][1024];

	int foundF1 = 0;
	int foundF2 = 0;
	int difference = 0;

	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

	for (i=0; i<3; i++) {
		if (blocks[i]==0) continue;	// 0 means pointing at nothing

		readFS304(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast

		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') continue;	// means unused entry

			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry

			if (_inode_table[e_inode].TT[0]=='F')  { // entry is for a file
				if (strcmp(_directory_entries[j].fname, filename1) == 0){ // found file1
						foundF1 = 1;
						f1blocks[0] = stoi(_inode_table[e_inode].XX,2);
						f1blocks[1] = stoi(_inode_table[e_inode].YY,2);
						f1blocks[2] = stoi(_inode_table[e_inode].ZZ,2);
				} else if (strcmp(_directory_entries[j].fname, filename2) == 0){ // found file2
						foundF2 = 1;
						f2blocks[0] = stoi(_inode_table[e_inode].XX,2);
						f2blocks[1] = stoi(_inode_table[e_inode].YY,2);
						f2blocks[2] = stoi(_inode_table[e_inode].ZZ,2);
				}
			}
		}
	}
	if (!foundF1) printf("%s could not be located in this directory!\n", filename1);
	if (!foundF2) printf("%s could not be located in this directory!\n", filename2);

	if (foundF1 && foundF2){
		for (d = 0; d < 3; d++){
			if (f1blocks[d] != 0 && f2blocks[d] != 0){
				readFS304(f1blocks[d], f1content[d]);
				readFS304(f2blocks[d], f2content[d]);
				for (t = 0; t < 1024; t++){
					if (f1content[d][t] != f2content[d][t]){
						difference += 1;
					}
				}
			} else {
				difference += 1024;
			}
		}
		printf("Number of differences between %s and %s: %d\n", filename1, filename2, difference);
	}

}
void p_rename(char *oldname, char *newname){
	int blocks[3];
	_directory_entry _directory_entries[4];
	int i,j;
	int e_inode;
	char fname[252];
	int file_found = 0;

	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

	for (i=0; i<3; i++) {
		if (blocks[i]==0) continue;	// 0 means pointing at nothing

		readFS304(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast

		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') continue;	// means unused entry

			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry
			strcpy(fname, _directory_entries[j].fname);

			if (strcmp(fname,oldname) == 0){
				file_found = 1;
				strcpy(_directory_entries[j].fname, newname);
				writeFS304(blocks[i], (char *) _directory_entries);
				break;
			}

		}
		if (file_found) break;
	}

	if (!file_found){
		printf("%s could not be found in this directory. Rename failed!\n", oldname);
	}

}

void p_copy(char *file1, char *file2){


}
