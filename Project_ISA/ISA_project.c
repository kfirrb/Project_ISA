#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGISTER_SIZE 32
#define MAX_LINE_SIZE 500
#define MEM_SIZE 4096

// a function that reads memin.text and store it's content into an array.returns 1 if error occured, else returns 0.
int read_memin(unsigned short* mem, char * address)
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
