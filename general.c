#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "general.h"


char** readFromFile(char **seq1, double *w, int *amountOfSeq2) {
	char **seq2;
	/* Open the file for reading */
	char *line_buf = NULL;
	size_t line_buf_size = 0;
	int line_count = 0;
	ssize_t line_size;
	FILE *fp = fopen(FILE_NAME, "r");
	if (!fp) {
		fprintf(stderr, "Error opening file '%s'\n", FILE_NAME);
	}

	/* Get the first line of the file. */

	fscanf(fp, "%lf %lf %lf %lf", &w[0], &w[1], &w[2], &w[3]);
	fgetc(fp);

	line_size = getline(&line_buf, &line_buf_size, fp);
	line_buf[strcspn(line_buf, "\r\n")] = '\0';
	line_count++;
	/* Loop through until we are done with the file. */
	while (line_size >= 0) {
		/* Increment our line count */
		line_count++;
		if (line_count == 2) {
			*seq1 = strdup(line_buf);
		}
		if (line_count == 3) {
			int i;
			*amountOfSeq2 = atoi(line_buf);
			seq2 = (char**) malloc(*amountOfSeq2 * sizeof(char*));
			for (i = 0; i < *amountOfSeq2; i++) {
				line_size = getline(&line_buf, &line_buf_size, fp);
				line_buf[strcspn(line_buf, "\r\n")] = '\0';
				seq2[i] = strdup(line_buf);
			}
		}

		line_size = getline(&line_buf, &line_buf_size, fp);
	}
	/* Free the allocated line buffer */
	free(line_buf);
	line_buf = NULL;
	/* Close the file now that we are done with it */
	fclose(fp);
	return seq2;

}

