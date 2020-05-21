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
Command line_to_command(unsigned int inst)
{
	Command cmd;
	cmd.opcode = (get_byte(inst, 7)*16)+get_byte(inst,6);
	cmd.rd = get_byte(inst, 5);
	cmd.rs = get_byte(inst, 4);
	cmd.rt = get_byte(inst, 3);
	cmd.immiediate = (get_byte(inst, 2)*16*16) + (get_byte(inst, 1)*16) + get_byte(inst, 0);
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

// and command.
void and(short * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] & regs[cmd.rt];
}

// or command.
void or (short * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] | regs[cmd.rt];
}

// sll command.
void sll(short * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] << regs[cmd.rt];
}

//sra command
void sra(short* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] >> regs[cmd.rt];
}

//srl command*************

//beq command
int beq(short* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] == regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//bne command
int bne(short* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] != regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//blt command
int blt(short* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] < regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//bgt command
int bgt(short* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] > regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//ble command
int ble(short* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] <= regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//bge command
int bge(short* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] >= regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//jal command
int jal(short* regs, Command cmd, int pc)
{
	regs[15] = pc + 1;
	return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
}

//lw command
void lw(short * regs, Command cmd, unsigned short * mem)
{
	regs[cmd.rd] = mem[regs[cmd.rs] + regs[cmd.rt]];
}

//sw command.
void sw(short * regs, Command cmd, unsigned short * mem)
{
	mem[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

//reti command
int reti(short* io_regs, int pc)
{
	return pc = io_regs[7];
}

//in command
void in(short* io_regs, short* regs, Command cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt]<18)
		regs[cmd.rd] = io_regs[regs[cmd.rs] + regs[cmd.rt]];
}

// out command
void out(short* io_regs, short* regs, Command cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 18)
		io_regs[regs[cmd.rs] + regs[cmd.rt]]= regs[cmd.rd];
}

//halt command

//excution function for all the relevent opcode
// after every excution we have to check that $zero doesn't change his value 
int execution(short regs[], short io_regs[], int pc, Command cmd, unsigned short * mem) {
	switch (cmd.opcode)
	{
	case 0: //add opcode
	{
		add(regs, cmd);
		regs[0] = 0; // make sure $zero is zero
		pc++;
		break;
	}
	case 1: //sub opcode
	{
		sub(regs, cmd);
		regs[0] = 0;
		pc++;
		break;
	}
	case 2: //and opcode
	{
		and (regs, cmd);
		regs[0] = 0;
		pc++;
		break;
	}
	case 3://or opcode
	{
		or (regs, cmd);
		regs[0] = 0;
		pc++;
		break;
	}
	case 4: //sll opcode
	{
		sll(regs, cmd);
		regs[0] = 0;
		pc++;
		break;
	}
	case 5: //sra opcode
	{
		sra(regs, cmd);
		regs[0] = 0;
		pc++;
		break;
	}
	case 6: //srl opcode***************
	{
	}
	case 7: //beq opcode
	{
		pc = beq(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 8: //bne opcode
	{
		pc = bne(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 9: //blt opcode
	{
		pc = blt(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 10: //bgt opcode
	{
		pc = bgt(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 11: //ble opcode
	{
		pc = ble(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 12: //bge opcode
	{
		pc = bge(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 13: //jal opcode
	{
		pc = jal(regs, cmd, pc);
		regs[0] = 0;
		break;
	}
	case 14: //lw opcode
	{
		lw(regs, cmd, mem);
		regs[0] = 0;
		pc++;
		break;
	}
	case 15: //sw opcode
	{
		sw(regs, cmd, mem);
		regs[0] = 0;
		pc++;
		break;
	}
	case 16: //reti command
	{
		reti(io_regs, pc);
		break;
	}
	case 17://in command
	{
		in(io_regs, regs, cmd);
		break;
	}
	case 18://out command
	{
		out(io_regs, regs, cmd);
		break;
	}
	case 19: //halt command, we need to exit simulator
	{
		pc = -1; 
		break;
	}
	}
	return pc;
}

	//A function that converts a negative number to positive in 2's compliment
	int neg_to_pos(signed int num)
	{
		num = abs(num);
		signed int mask = 0xffffffff;
		num = num ^ mask; // invert all bits
		num++; // add 1 as in 2's comp
		return num;
	}