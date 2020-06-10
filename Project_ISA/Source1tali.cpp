#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> 
#include <malloc.h>
#include <stdbool.h>

//this struct will represent a command 
typedef struct cmd {
	char opcode;
	char rd;
	char rs;
	char rt;
	int immediate;
	char *immediate_label; // if the immediate is a label
	char *line_start_with_label; // if there is a label at the beggining of the line
}*Command;

typedef struct label_and_line { // mapping between line number and its label
	int line;
	char *label;
}*LabelNLine;

//this struct will hold a .word command 
typedef struct word_cmd {
	int address;
	int data;
}*Word;

//functions declarations
int assign_register(char *reg);
int assign_opcode(char *opcode);
int is_negative(char* str);
int neg_to_pos(signed int num);
int convert_str_num_to_int(char *str);
int convert_immediate_num_to_int(char *strr);
Command scaning_line(char* line);
int give_address_of_a_label(LabelNLine *label_to_line, int n, char* label);
Word scaning_word_line(char* line);
void free_cmds(Command* cmds, int n);
void free_labels(LabelNLine* labels, int n);
void free_words(Word* words, int n);

int main(int argc, char *argv[])
{
	char line[500];
	char *LAB = NULL;
	int i = 0, j = 0, l = 0, w = 0, k;
	int line_num_in_mem = 0;
	char memory[4096][8];//size of memin
	int last_mem_line = 0;
	char transition_char[9];
	char* reg1;
	FILE *f;
	FILE *output;
	Command *cmds;
	Word *words;
	LabelNLine *labels;

	cmds = (Command*)malloc(4096 * sizeof(Command));
	if (cmds == NULL) {
		printf("error: memory not allocated.");
		return 1;
	}
	labels = (LabelNLine*)malloc(4096 * sizeof(LabelNLine));
	if (labels == NULL) {
		free(cmds);
		printf("error: memory not allocated.");
		return 1;
	}
	words = (Word*)malloc(4096 * sizeof(Word));
	if (words == NULL) {
		free(cmds);
		free(labels);
		printf("error: memory not allocated.");
		return 1;
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		free(cmds);
		free(labels);
		free(words);
		printf("The file was not opened successfully.");
		return 1;
	}

	//This loop goes over the asm file and for each line, if it's not empty, it converts it to a command/word command

	while (fgets(line, 500, f) != NULL)
	{
		cmds[i] = scaning_line(line);
		Command curr_cmd = cmds[i];
		if (cmds[i] == NULL)
		{
			free_cmds(cmds, i);
			free_labels(labels, j);
			free_words(words, w);
			fclose(f);
			printf("ERROR!");
			return 1;
		}
		if (cmds[i]->opcode == -2) // check if this line is .word line 
		{
			words[w] = scaning_word_line(line);
			if (words[w] == NULL)
			{
				free_cmds(cmds, i);
				free_labels(labels, j);
				free_words(words, w);
				fclose(f);
				printf("ERROR!");
				return 1;
			}
			w++;
			continue;
		}
		if (cmds[i]->opcode > -1) //this means that the line is a command line 
		{
			if ((LAB != NULL))
			{
				cmds[i]->line_start_with_label = LAB;
				LAB = NULL;
			}

			if (cmds[i]->line_start_with_label != NULL)
			{
				labels[j] = (LabelNLine)malloc(sizeof(struct label_and_line));
				if (labels[j] == NULL)
				{
					free_cmds(cmds, i);
					free_labels(labels, j);
					free_words(words, w);
					fclose(f);
					printf("ERROR!");
					return 1;
				}

				labels[j]->label = cmds[i]->line_start_with_label;
				labels[j]->line = i;//line_num_in_mem
				j++;
			}

			i++;
		}
		else if (cmds[i]->line_start_with_label != NULL)
			LAB = cmds[i]->line_start_with_label;
	}



	output = fopen(argv[2], "w");
	if (output == NULL) {
		free_cmds(cmds, i);
		free_labels(labels, j);
		free_words(words, w);
		fclose(f);
		printf("ERROR!");
		return 1;
	}
	//this loop print the memory array into the memin file
	//if it encounters an imm label it will first conver it to a real line 

	for (k = 0; k < i; k++)
	{
		if (cmds[k]->immediate_label != NULL)
		{
			cmds[k]->immediate = give_address_of_a_label(labels, j, cmds[k]->immediate_label);
			sprintf(memory[l], "%02X%X%X%X%03X", cmds[k]->opcode, cmds[k]->rd, cmds[k]->rs, cmds[k]->rt, cmds[k]->immediate);
			if (cmds[k]->rd == 1) //not sure we need this maybe assembly
			{
				reg1 = cmds[k]->immediate_label;//save the address of the label in reg1
			}
			if (cmds[k]->rs == 1)
			{
				reg1 = cmds[k]->immediate_label;//save the address of the label in reg1
			}
			if (cmds[k]->rt == 1)
			{
				reg1 = cmds[k]->immediate_label;//save the address of the label in reg1
			}
		}
		else if (cmds[k]->immediate_label == NULL)
		{
			sprintf(memory[l], "%02X%X%X%X%03X", cmds[k]->opcode, cmds[k]->rd, cmds[k]->rs, cmds[k]->rt, cmds[k]->immediate);
		}
		l++;
	}
	last_mem_line = l - 1;

	//this loop executes the .word commands - saving the values into the memory according the destination
	for (k = 0; k < w; k++)
	{
		if (words[k]->address > last_mem_line + 1)
		{
			for (l = last_mem_line + 1; l < words[k]->address; l++)// fill with zeros untill we get to the address of the word
			{
				sprintf(memory[l], "%08X", 0);
			}
			last_mem_line = words[k]->address;
		}
		else if (words[k]->address > last_mem_line)
			last_mem_line = words[k]->address;
		sprintf(transition_char, "%08X", words[k]->data);
		strncpy(memory[words[k]->address], transition_char, 8);
	}

	//this loop prints the memory array to the memin file 
	for (k = 0; k <= last_mem_line; k++)
	{
		fprintf(output, "%c%c%c%c%c%c%c%c\n", memory[k][0], memory[k][1], memory[k][2], memory[k][3], memory[k][4], memory[k][5], memory[k][6], memory[k][7]);
	}

	free_cmds(cmds, i);
	free_labels(labels, j);
	free_words(words, w);
	fclose(f);

	return 0; //succeed
}


//this function converts a register word to an integer which is represented by the index in the array
int assign_register(char *reg)
{
	const char *registers_lst[] = { "$zero","$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra" };

	for (int i = 0; i < 15; i++)
	{
		
		if (strcmp(reg, registers_lst[i]) == 0)
		{
			return i;
		}
	}
}


//this function converts a command word to an integer which is represented by the index in the array
int assign_opcode(char *opcode)
{
	
	const char *opcodes[] = { "add", "sub", "and", "or", "sll", "sra", "srl", "beq", "bne", "blt", "bgt","ble","bge","jal","lw","sw","reti","in","out","halt" };
	for (int i = 0; i < 19; i++)
	{
		
		if (strcmp(opcode, opcodes[i]) == 0)
		{
			return i;
		}
	}

}

int is_negative(char* str)
{
	int res;
	res = strncmp(str, "-", 1);
	return !res;
}
int neg_to_pos(signed int num)
{

	num = abs(num);
	signed int mask = 0xfff;
	num = num ^ mask;
	num++;
	return num;
}
//this function takes a string that can be hex or decimal (positve or negative) and convert it into a 32 bit integer. 
int convert_str_num_to_int(char *str)
{
	int res;

	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))//need to convert hexa to int
	{
		{
			return res=(int)strtol(str, NULL,0);//string to long integer
		}
	if (is_negative(str)) //negative number
		{
			
			int res = atoi(str);
			return res= neg_to_pos(res);
		}
	}
	else
		return res = atoi(str); //need to convert str to int

}

int convert_immediate_num_to_int(char *imm)
{
	int res;

	if (imm[0] == '0' && (imm[1] == 'x' || imm[1] == 'X'))//need to convert hexa to int
	
		{

			return res = (int)strtol(imm, NULL, 0);//string to long integer
		}
	if (is_negative(imm)) //negative number
		{

			int res = atoi(imm);
			return res = neg_to_pos(res);

		}
	else return atoi(imm);

}

//this function handles a single line and returns a command. 
//it returns NULL in case of error
//it return opcode=-1 in case of a line without a command but only a label
//it return opcode=-2 in case of a .word command 
Command scaning_line(char* line)
{
	int j = 0;
	int len = strlen(line);
	int m;
	char *curr_word;
	char word_count = 0; // this will represent how many words we have found
	Command cm;
	char is_label_line = 0;


	curr_word = (char*)malloc(500);
	if (curr_word == NULL) {
		printf("error: memory not allocated.");
		return NULL;
	}
	cm = (Command)malloc(sizeof(*cm));
	if (cm == NULL) {
		free(curr_word);
		printf("error: memory not allocated.");
		return NULL;
	}
	cm->immediate_label = NULL;
	cm->line_start_with_label = NULL;
	cm->opcode = -1;
	for (int i = 0; i < len; i++)
	{
		if ((line[i] == ' ' || line[i] == '\n' || line[i] == '\t') && word_count != 1 && (word_count != 5 || (word_count == 5 && j == 0)))
			continue;
		if (is_label_line == 1 && line[i] == '#')//if we get to the end of the assembly command 
			break;
		if ((line[i] == ' ' || line[i] == '\n' || line[i] == '\t' ) && word_count == 1)
		{
			curr_word[j] = '\0';
			if (strcmp(curr_word, ".word") == 0)
			{
				free(curr_word);
				cm->opcode = -2; // -2 will represent a .word command
				return cm;
			}
			j = 0;
			cm->opcode = assign_opcode(curr_word);
			word_count++;
			continue;
		}
		if (strchr(line, ':') != NULL)
		{
			is_label_line = 1;
			if (j > 0)
				curr_word[j] = '\0';
			j = 0;
			cm->line_start_with_label = (char*)malloc(strlen(curr_word + 1));
			if (cm->line_start_with_label == NULL) {
				free(curr_word);
				free(cm);
				printf("error: memory not allocated.");
				return NULL;
			}
			//int len = strlen(line);
			strcpy(cm->line_start_with_label,line);
			cm->line_start_with_label[len - 2] = '\0';
			word_count = 0;
			cm->opcode = -1;
			continue;
		}
		if (line[i] == ',')
		{
			curr_word[j] = '\0';
			j = 0;

			if (word_count == 2)
				cm->rd = assign_register(curr_word);


			if (word_count == 3)
				cm->rs = assign_register(curr_word);


			if (word_count == 4)
				cm->rt = assign_register(curr_word);

			word_count++;
			continue;
		}
		if (word_count == 5 && (line[i] == ' ' || line[i] == '\n' || line[i] == '\t' || line[i] == '#'))// if we get to the last item in the line
		{
			curr_word[j] = '\0';
			j = 0;
			m = strlen(curr_word);
			if (isalpha(curr_word[0]))
				cm->immediate_label = curr_word;
			else if (!isalpha(curr_word[0]))
				cm->immediate =convert_immediate_num_to_int(curr_word); //Convert.ToInt32("curr_word", 32);//
			break;
		}
		if (j == 0 && word_count == 0) {
			word_count++;
		}
		curr_word[j] = line[i];
		j++;
	}

	return cm;
}

//this function takes a list of label to line mappings and a label to find, and returns the line of the label
int give_address_of_a_label(LabelNLine *label_to_line, int n, char* label)
{
	int i=0;
	while (strcmp(label_to_line[i]->label, label) != 0)
	{
		i++;
	}
	return label_to_line[i]->line;
			

}

//this functions takes a single line and returns a Word command 
Word scaning_word_line(char* line)
{
	Word wr;
	char *curr_word;
	unsigned int j = 0;
	int word_count = 0;

	curr_word = (char*)malloc(500);
	if (curr_word == NULL)
	{
		printf("error: memory not allocated.");
		return NULL;
	}
	wr = (Word)malloc(sizeof(*wr));
	if (wr == NULL) {
		free(curr_word);
		printf("error: memory not allocated.");
		return NULL;
	}
	for (unsigned int i = 0; i < strlen(line); i++)
	{
		if ((line[i] == ' ' || line[i] == '\n' || line[i] == '\t') && (word_count == 0 || word_count > 3))
			continue;

		if ((line[i] == ' ' || line[i] == '\n' || line[i] == '\t') && word_count >= 1)
		{
			curr_word[j] = '\0';
			j = 0;
			if (word_count == 2)
				wr->address = convert_str_num_to_int(curr_word);
			else if (word_count == 3)
				wr->data = convert_str_num_to_int(curr_word);
			word_count++;
			continue;
		}

		if (j == 0 && word_count == 0) {
			word_count++;
		}
		curr_word[j] = line[i];
		j++;
	}
	free(curr_word);
	return wr;
}
//recursive freeing of memory 
void free_cmds(Command* cmds, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (cmds[i] != NULL)
		{
			free(cmds[i]->immediate_label);
			free(cmds[i]);
		}
	}
	free(cmds);
}

//recursive freeing of memory 
void free_labels(LabelNLine* labels, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (labels[i] != NULL)
		{
			free(labels[i]);
		}
	}
	free(labels);
}

//recursive freeing of memory 
void free_words(Word* words, int n)
{

	for (int i = 0; i < n; i++)
	{
		if (words[i] != NULL)
		{
			free(words[i]);
		}
	}
	free(words);
}
