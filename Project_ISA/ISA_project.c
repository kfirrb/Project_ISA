#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGISTER_SIZE 32
#define MAX_LINE_SIZE 500
#define MEM_SIZE 4096

//defined the struct that will help us cross the river of the project
typedef struct _command {
	unsigned short opcode;
	unsigned short rd;
	unsigned short rs;
	unsigned short rt;
	unsigned short immiediate;
}Command;

// a function that reads memin.text and store it's content into an array.returns 1 if error occured, else returns 0.
int read_memin(unsigned long* mem, char * address)
{
	FILE *fp = fopen(address, "r"); // open memin file
	if (!fp) { // handle error
		printf("Error opening memin file\n");
		return 1;
	}

	// read memin file line by line and turn it into matrix
	char line[MAX_LINE_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, MAX_LINE_SIZE, fp))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore white spaces
			continue;
		mem[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp); // close file
	return 0;
}

// this function extracts one byte from number
unsigned short get_byte(unsigned short num, int pos)
{
	unsigned short mask = 0x1f << (pos * 4);
	return (num & mask) >> (pos * 4);
}

// this function creates a struct Command from a string in memory
Command line_to_command(unsigned int inst) // create new Command struct from code line
{
	Command cmd;
	cmd.opcode = get_byte(inst, 6)+get_byte(inst,7);
	cmd.rd = get_byte(inst, 5);
	cmd.rs = get_byte(inst, 4);
	cmd.rt = get_byte(inst, 3);
	cmd.immiediate = get_byte(inst, 2) + get_byte(inst, 1) + get_byte(inst, 0);
	return cmd;
}


//basic commands and instructions

//add command
void add(short * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] + regs[cmd.rt];
}

//sub command
void sub(short* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] - regs[cmd.rt];
}
