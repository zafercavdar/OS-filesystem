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

	readFS304(3, buffer);
	printf("3: %s\n",buffer);

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
			p_copy(parameters[1], parameters[2]);
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
	int blocks[3];
	_directory_entry _directory_entries[4];
	int is_file_name_valid = 1;
	int f1blocks[3];
	int f1exists = 0;
	int i,j;
	int e_inode;
	int entry_count = 0;
	int newblockno;

	int empty_dblock=-1, empty_dentry=-1;;
	int empty_ientry;

	// non-empty name
	if (strlen(file2)==0) {
		printf("Usage: md <directory name>\n");
		return;
	}

	// do we have free inodes
	if (free_inode_entries == 0) {
		printf("Error: Inode table is full.\n");
		return;
	}

	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

	for (i=0; i<3; i++) {
		if (blocks[i]==0) { 	// 0 means pointing at nothing
			if (empty_dblock==-1) empty_dblock=i; // we can later add a block if needed
			continue;
		}

		readFS304(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast

		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') { // means unused entry
				if (empty_dentry==-1) { empty_dentry=j; empty_dblock=i; } // AAHA! lets keep a note of it, just in case we have to create the new directory
				continue;
			}

			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry
			entry_count += 1;

			if (_inode_table[e_inode].TT[0]=='F')  { // entry is for a file and has same filename with file2
				if (strcmp(_directory_entries[j].fname, file2) == 0){
					is_file_name_valid = 0;
				}
				if (strcmp(_directory_entries[j].fname, file1) == 0){
					f1blocks[0] = stoi(_inode_table[e_inode].XX,2);
					f1blocks[1] = stoi(_inode_table[e_inode].YY,2);
					f1blocks[2] = stoi(_inode_table[e_inode].ZZ,2);
					f1exists = 1;
				}
			}
		}
	}

	if (!f1exists) printf("Error: Source file %s does not exist in this directory!\n", file1);
	else if (!is_file_name_valid) printf("Error: %s already exists in this directory. Try different file name!\n", file2);
	else if (empty_dentry==-1 && empty_dblock==-1) {
		printf("Error: Maximum directory entries reached.\n");
	} else { // otherwise
		if (empty_dentry == -1) { // Great! didn't find an empty entry but not all three blocks have been used
			empty_dentry=0;

			if ((blocks[empty_dblock] = getBlock())==-1) {  // first get a new block using the block bitmap
				printf("Error: Disk is full.\n");
				return;
			}
			//printf("empty block: %d\n",empty_dblock);
			//printf("new empty dblock: %d\n",blocks[empty_dblock]);

			writeFS304(blocks[empty_dblock],NULL);	// write all zeros to the block (there may be junk from the past!)

			switch(empty_dblock) {	// update the inode entry of current dir to reflect that we are using a new block
				case 0: itos(_inode_table[CD_INODE_ENTRY].XX,blocks[empty_dblock],2); break;
				case 1: itos(_inode_table[CD_INODE_ENTRY].YY,blocks[empty_dblock],2); break;
				case 2: itos(_inode_table[CD_INODE_ENTRY].ZZ,blocks[empty_dblock],2); break;
			}
		}

		empty_ientry = getInode();
		//printf("empty ientry: %d\n", empty_ientry);
		readFS304(blocks[empty_dblock],(char *)_directory_entries);	// read block of current directory where info on this new directory will be written
		_directory_entries[empty_dentry].F = '1';			// remember we found which directory entry is unused; well, set it to used now
		strncpy(_directory_entries[empty_dentry].fname,file2,252);	// put the name in there
		itos(_directory_entries[empty_dentry].MMM,empty_ientry,3);	// and the index of the inode that will hold info inside this directory
		writeFS304(blocks[empty_dblock],(char *)_directory_entries);	// now write this block back to the disk

		strncpy(_inode_table[empty_ientry].TT,"FI",2);

		if (f1blocks[0] != 0){
			if ((newblockno = getBlock())==-1) {  // first get a new block using the block bitmap
				printf("Error: Disk is full.\n");
				return;
			}
			itos(_inode_table[empty_ientry].XX,newblockno,2);
			//printf("new block no for 1: %d\n",newblockno);
			readFS304(f1blocks[0],(char *)_directory_entries);
			//printf("copying block %d\n",f1blocks[0]);
			writeFS304(newblockno,(char *)_directory_entries);

		} else {
				strncpy(_inode_table[empty_ientry].XX,"00",2);
		}

		if (f1blocks[1] != 0){
			if ((newblockno = getBlock())==-1) {  // first get a new block using the block bitmap
				printf("Error: Disk is full.\n");
				return;
			}
			itos(_inode_table[empty_ientry].YY,newblockno,2);
			readFS304(f1blocks[1],(char *)_directory_entries);
			writeFS304(newblockno,(char *)_directory_entries);
			//printf("new block no for 2: %d\n",newblockno);
			//printf("copying block %d\n",f1blocks[1]);

		} else {
				strncpy(_inode_table[empty_ientry].YY,"00",2);
		}

		if (f1blocks[2] != 0){
			if ((newblockno = getBlock())==-1) {  // first get a new block using the block bitmap
				printf("Error: Disk is full.\n");
				return;
			}
			itos(_inode_table[empty_ientry].ZZ,newblockno,2);
			readFS304(f1blocks[2],(char *)_directory_entries);
			writeFS304(newblockno,(char *)_directory_entries);
			//printf("new block no for 3: %d\n",newblockno);
			//printf("copying block %d\n",f1blocks[2]);

		} else {
				strncpy(_inode_table[empty_ientry].ZZ,"00",2);
		}

		writeFS304(BLOCK_INODE_TABLE, (char *)_inode_table);	// phew!! write the inode table back to the disk
	}

}
