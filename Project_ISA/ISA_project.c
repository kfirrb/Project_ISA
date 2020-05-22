#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define IO_REGISTER_SIZE 18
#define REGISTER_SIZE 16
#define MAX_LINE_SIZE 8
#define MEM_SIZE 4096

//defined the struct that will help us cross the river of the project
typedef struct _command {
	unsigned int opcode;
	unsigned int rd;
	unsigned int rs;
	unsigned int rt;
	unsigned int immiediate;
}Command;

int main(int argc, char* argv[])
{
	int regs[REGISTER_SIZE] = { 0 };// initialize register
	int io_regs[IO_REGISTER_SIZE] = { 0 };// initialize input output register
	int counter = 0; //initialize counter
	int pc = 0; // initialize pc
	unsigned int mem[MEM_SIZE] = { 0 }; // initialize memory
	unsigned int disk[MEM_SIZE] = { 0 };// initialize disk
	unsigned int irq2[MEM_SIZE] = { 0 };// initialize irq 2
	if (read_memin(mem, argv[1]) != 0|| read_diskin(disk, argv[2]) != 0 || read_irq2in(irq2, argv[3]) != 0) //open memin
	{
		printf("An Error Occured - Exiting Simulator.\n");
		exit(1);
	}
	FILE * fp_trace; // define pointer for writing trace file
	FILE * fp_hwregtrace;//// define pointer for writing hwregtrace file
	FILE* fp_leds;//define pointer for writing leds file
	FILE* fp_display;//define pointer for writing display file
	fp_trace = fopen(argv[6], "w");
	fp_hwregtrace = fopen(argv[7], 'w');
	fp_leds = fopen(argv[9], 'w');
	fp_display = fopen(argv[10], 'w');
	if (fp_trace == NULL|| fp_hwregtrace==NULL|| fp_leds==NULL|| fp_display==NULL)
	{
		printf("Error opening file");
		exit(1);
	}
	// Execution
	unsigned int inst; // define instruction number
	while (pc != -1)
	{
		io_regs[8]=counter++;//clk cycle counter
		inst = mem[pc];									
		char line_for_trace[200] = { 0 };//create line for trace file
		char line_for_leds[20] = { 0 };//create line for trace file
		char line_for_display[20] = { 0 };//create line for trace file
		Command cmd = line_to_command(inst); // create Command struct
		regs[1] = sign_extend(cmd.immiediate);//first we do sign extend to immiediate
		create_line_for_trace(line_for_trace, regs, pc, inst,cmd.immiediate);//append to trace file
		fprintf(fp_trace, "%s\n", line_for_trace);
		if ((regs[cmd.rs] + regs[cmd.rt]) == 9) {
			create_line_for_leds(line_for_leds, regs, io_regs, counter, cmd);//append to leds file
			fprintf(fp_leds, "%s\n", line_for_leds);
		}
		if ((regs[cmd.rs] + regs[cmd.rt]) == 10) {
			create_line_for_display(line_for_display, regs, io_regs, counter, cmd);//append to display file
			fprintf(fp_display, "%s\n", line_for_display);
		}
		pc = execution(regs,io_regs, pc, cmd, mem); // execute instruction
	}
}
// a function that reads memin.txt and store it's content into an array.returns 1 if error occured, else returns 0.
int read_memin(unsigned int* mem, char * address)
{
	FILE *fp = fopen(address, "r"); // open memin file
	if (!fp) { // handle error
		printf("Error opening memin file\n");
		return 1;
	}

	// read memin file line by line and turn it into array
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

// a function that reads diskin.txt and store it's content into an array.returns 1 if error occured, else returns 0.
int read_diskin(unsigned int* disk, char * address)
{
	FILE *fp = fopen(address, "r"); // open diskin file
	if (!fp) { // handle error
		printf("Error opening memin file\n");
		return 1;
	}

	// read diskin file line by line and turn it into array
	char line[MAX_LINE_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, MAX_LINE_SIZE, fp))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore white spaces
			continue;
		disk[i] = strtol(line, NULL, 16);
		i++;
	}
	fclose(fp); // close file
	return 0;
}

// a function that reads irq2in.txt and store it's content into an array.returns 1 if error occured, else returns 0.
int read_irq2in(unsigned int* irq2, char * address)
{
	FILE *fp = fopen(address, "r"); // open diskin file
	if (!fp) { // handle error
		printf("Error opening memin file\n");
		return 1;
	}

	// read diskin file line by line and turn it into array
	char line[MAX_LINE_SIZE];
	int i = 0;
	while (!feof(fp) && fgets(line, MAX_LINE_SIZE, fp))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore white spaces
			continue;
		irq2[i] = strtol(line, NULL, 10);
		i++;
	}
	fclose(fp); // close file
	return 0;
}

//this function sign extend the value of imm
int sign_extend(int imm)
{
	int x = imm;
	x = (x >> 11);
	if (x == 0)
		return imm;
	else
		return imm = 0xfffff000 |imm;
}

// this function extracts one byte from number
unsigned int get_byte(unsigned int num, int pos)
{
	unsigned int mask = 0x1f << (pos * 4);
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
void add(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] + regs[cmd.rt];
}

//sub command
void sub(int* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] - regs[cmd.rt];
}

// and command.
void and(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] & regs[cmd.rt];
}

// or command.
void or (int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] | regs[cmd.rt];
}

// sll command.
void sll(int * regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] << regs[cmd.rt];
}

//sra command
void sra(int* regs, Command cmd)
{
	regs[cmd.rd] = regs[cmd.rs] >> regs[cmd.rt];
}

//srl command*************

//beq command
int beq(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] == regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//bne command
int bne(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] != regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//blt command
int blt(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] < regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//bgt command
int bgt(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] > regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//ble command
int ble(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] <= regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//bge command
int bge(int* regs, Command cmd, int pc)
{
	if (regs[cmd.rs] >= regs[cmd.rt])
		return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
	else
		pc++;
}

//jal command
int jal(int* regs, Command cmd, int pc)
{
	regs[15] = pc + 1;
	return pc = get_byte(cmd.rd, 0) + (get_byte(cmd.rd, 1) * 16) + (get_byte(cmd.rd, 2) * 16 * 16);
}

//lw command
void lw(int * regs, Command cmd, unsigned int * mem)
{
	regs[cmd.rd] = mem[regs[cmd.rs] + regs[cmd.rt]];
}

//sw command.
void sw(int * regs, Command cmd, unsigned int * mem)
{
	mem[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

//reti command
int reti(int* io_regs, int pc)
{
	return pc = io_regs[7];
}

//in command
void in(int* io_regs, int* regs, Command cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt]<18)
		regs[cmd.rd] = io_regs[regs[cmd.rs] + regs[cmd.rt]];
}

// out command
void out(int* io_regs, int* regs, Command cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 18)
		io_regs[regs[cmd.rs] + regs[cmd.rt]]= regs[cmd.rd];
}

//halt command

//excution function for all the relevent opcode
// after every excution we have to check that $zero doesn't change his value 
int execution(int regs[], int io_regs[], int pc, Command cmd, unsigned int * mem) {
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
		pc++;
		break;
	}
	case 18://out command
	{
		out(io_regs, regs, cmd);
		pc++;
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

//timer finction
void timer(int* io_regs)
{
	if (io_regs[11] == 1)
		if (io_regs[12] == io_regs[13]) {
			io_regs[3] = 1;
			io_regs[12] = 0;
		}
		else
			io_regs[12]++;
}

// how to handel write\read from disk
void disk(int* disk, int * io_regs)
{
	switch (io_regs[14])
	{
	case 0:
		break;
	case 1:
	{
		io_regs[16] = disk[io_regs[15]];
	}
	case 2:
	{
		disk[io_regs[15]] = io_regs[16];
	}
	}
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

// this function creates regout file
void create_regout(int regs[], char file_name[]) {
	FILE* fp_regout;

	fp_regout = fopen(file_name, "w"); // open new file
	if (fp_regout == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	for (int i = 2; i <= 15; i++) // print registers to file
	{
		fprintf(fp_regout, "%08X\n", regs[i]);
	}
	fclose(fp_regout); // close file
}

// this function creates memout file
void create_memout(unsigned int * mem, char file_name[]) {
	FILE* fp_memout;
	fp_memout = fopen(file_name, "w"); // open new file
	if (fp_memout == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	for (int i = 0; i < MEM_SIZE; i++) // print memory to file
	{
		fprintf(fp_memout, "%08X\n", *mem);
		mem++;
	}
	fclose(fp_memout); // close file
}

// this function creates diskout file
void create_diskout(unsigned int * disk, char file_name[]) {
	FILE* fp_diskout;
	fp_diskout = fopen(file_name, "w"); // open new file
	if (fp_diskout == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	for (int i = 0; i < MEM_SIZE; i++) // print memory to file
	{
		fprintf(fp_diskout, "%08X\n", *disk);
		disk++;
	}
	fclose(fp_diskout); // close file
}

//create the cycles.txt file
void create_cycles(int counter, char file_name[]) {
	FILE* fp_cycles;
	fp_cycles = fopen(file_name, "w");
	if (fp_cycles == NULL) // handle error
	{
		printf("error opening file");
		exit(1);
	}
	char c_counter[8] = { 0 };
	sprintf(c_counter, "%d", counter);//print the counter to file
	fputs(c_counter, fp_cycles);
	fclose(fp_cycles); // close file
}

// this function prepares a string to print to trace file
void create_line_for_trace(char line_for_trace[], int regs[], int pc, unsigned int inst,int imm)
{
	int i;
	char inst_line[8];
	char pc_char[8] = { 0 };
	char temp_reg_char[8] = { 0 };
	sprintf(pc_char, "%08X", pc);
	sprintf(inst_line, "%08X", inst);
	sprintf(line_for_trace, pc_char); //add pc to line
	sprintf(line_for_trace + strlen(line_for_trace), " ");
	sprintf(line_for_trace + strlen(line_for_trace), inst_line); //add opcode to line
	sprintf(line_for_trace + strlen(line_for_trace), " ");

	for (i = 0; i < 15; i++) { //add registers to line
		int temp_reg = 0;
		if (i == 1)// for imm
		{
			sprintf(temp_reg_char, "%08X", sign_extend(imm));//change to hex
			sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);//add to line
			sprintf(line_for_trace + strlen(line_for_trace), " ");
		}
		if (regs[i] < 0)
			temp_reg = neg_to_pos(regs[i]);
		else
			temp_reg = regs[i];
		sprintf(temp_reg_char, "%08X", temp_reg);//change to hex
		sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);//add to line
		sprintf(line_for_trace + strlen(line_for_trace), " ");
	}

	//add last register to line (without space at the end)
	int temp_reg = 0;
	if (regs[i] < 0)
		temp_reg = neg_to_pos(regs[i]);
	else
		temp_reg = regs[i];
	sprintf(temp_reg_char, "%.8X", temp_reg);
	sprintf(line_for_trace + strlen(line_for_trace), temp_reg_char);
}

// create function that will colect data for hwregtrace
void create_line_for_hwregtrace(char line_for_hwregtrace[], int io_regs[], int pc, unsigned int inst, int imm)
{

}

//create display.txt
void create_line_for_display(char line_for_display[],int regs[], int io_regs[], int cycles, Command cmd)
{
	char clk_cycles[4];
	char curr_display[8];
	sprintf(clk_cycles, "%08X", cycles);
	sprintf(curr_display, "%08X", regs[cmd.rd]);
	sprintf(line_for_display, clk_cycles); //add clk cycles to line
	sprintf(line_for_display + strlen(line_for_display), " ");// add space 
	sprintf(line_for_display + strlen(line_for_display), curr_display); //add current display to line
}

//create line for leds.txt file
void create_line_for_leds(char line_for_leds[], int regs[], int io_regs[], int cycles, Command cmd)
{
	char clk_cycles[4];
	char curr_leds[8];
	sprintf(clk_cycles, "%08X", cycles);
	sprintf(curr_leds, "%08X", regs[cmd.rd]);
	sprintf(line_for_leds, clk_cycles); //add clk cycles to line
	sprintf(line_for_leds + strlen(line_for_leds), " ");// add space 
	sprintf(line_for_leds + strlen(line_for_leds), curr_leds); //add leds to line
}
