/*<Cezar Craciunoiu 334CA - C Preprocessor>*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./hashtable.h"

#define MAX_BUF 256
#define MAX_HASH 64
/*The function parses a string and eliminates all delimitators*/
void eliminate_delims(char *string)
{
	char *delimitators = "()[]=+-*&^%$#@!{};:',<.>/?\\|`~", *iter, *found;

	for (iter = delimitators; *iter != '\0'; ++iter) {
		found = strchr(string, *iter);
		while (found != NULL) {
			*found = ' ';
			found = strchr(found, *iter);
		}
	}
}

/* The function parses the arguments given and structures them in an
 * easily modifiable way.
 */
int parse_args(int argc, char **argv, hashtable_t *symbols, FILE **in,
		FILE **out, char ***dirs, int *nr_dirs, char **in_path)
{
	char *value_start, *value_end, *key_start, *key_end;
	int end_D = 1;

	while (argc != end_D && memcmp(argv[end_D], "-D", 2) == 0) {
		if (argv[end_D][2] == '\0')
			end_D++;
		key_start = argv[end_D];
		if (argv[end_D][0] == '-')
			key_start += 2;
		key_end = strchr(argv[end_D], '=');
		if (key_end == NULL) {
			key_end = strchr(argv[end_D], '\0');
			hashtable_insert(symbols, key_start,
					key_end - key_start, "", 1);
			end_D++;
			continue;
		}
		value_start = key_end + 1;
		value_end = strchr(value_start, 0);
		if (value_end - value_start == 0)
			hashtable_insert(symbols, key_start,
					key_end - key_start, "", 1);
		else
			hashtable_insert(symbols, key_start,
					key_end - key_start, value_start,
					value_end - value_start);
		end_D++;
	}

	while (argc != end_D && memcmp(argv[end_D], "-I", 2) == 0) {
		if (argv[end_D][2] == '\0')
			end_D++;
		value_start = argv[end_D];
		if (argv[end_D][0] == '-')
			value_start += 2;
		if (*nr_dirs == 1)
			*dirs = malloc(sizeof(char *));
		else
			*dirs = realloc(*dirs, sizeof(char *) * (*nr_dirs));
		CLOSE(*dirs, "Malloc failed!", 12);
		(*dirs)[*nr_dirs - 1] = value_start;
		(*nr_dirs)++;
		end_D++;
	}

	if (argc != end_D && memcmp(argv[end_D], "-o", 2) != 0) {
		value_start = argv[end_D];
		*in_path = value_start;
		*in = fopen(value_start, "r");
		CLOSE(*in, "Could not open in file!", 5);
		end_D++;
	} else
		*in = stdin;

	if (argc != end_D && memcmp(argv[end_D], "-o", 2) == 0) {
		if (argc == end_D + 1) {
			value_start = argv[end_D] + 2;
			value_end = strchr(value_start, '\0');
			end_D++;
		} else {
			end_D++;
			value_start = argv[end_D];
			value_end = strchr(value_start, '\0');
		}
		*out = fopen(value_start, "w");
		CLOSE(*out, "Could not open out file!", 5);
	} else {
		if (argc != end_D) {
			*out = fopen(argv[end_D], "w");
			CLOSE(*out, "Could not open out file!", 5);
			end_D++;
		} else
			*out = stdout;
		end_D++;
	}
	if (argc != end_D - 1)
		CLOSE(NULL, "Too many parameters!", 7);
	return 0;
}

/* The function receives a line a line and prints all it's characters and
 * replaces defines with their corresponding value
 */
int parse_line(char item[MAX_BUF], char *iter, hashtable_t *symbols, FILE *out)
{
	char cleared[MAX_BUF], next_line[MAX_BUF];
	char aux_line[MAX_BUF], *step, *value;
	int return_value = 0;

	strcpy(cleared, item);
	eliminate_delims(cleared);
	memset(next_line, '\0', MAX_BUF);
	for (step = cleared, value = item; *step != '\0' && *value != '\0';
				++step, ++value) {

		if (*step == ' ') {
			fprintf(out, "%c", *value);
			continue;
		}

		if (*step == '"') {
			fprintf(out, "%c", *value);
			value++;
			step++;
			while (*step != '"') {
				fprintf(out, "%c", *value);
				step++;
				value++;
			}
			fprintf(out, "%c", *value);
			continue;
		}

		if (strchr(step, ' ') != NULL &&
			hashtable_at(symbols, step, strchr(step, ' ') - step)) {
			return_value = 1;
			strcpy(next_line, hashtable_at(symbols, step,
						strchr(step, ' ') - step));
			if (*hashtable_at(symbols, step, strchr(step, ' ') -
								step) == ' ') {
				strcpy(aux_line, next_line + 1);
				if (strchr(aux_line, '\n') != NULL)
					*strchr(aux_line, '\n') = '\0';
				strcpy(next_line, aux_line);
			}
			if (hashtable_at(symbols,
					next_line, strlen(next_line))) {
				strcpy(next_line, hashtable_at(symbols,
						next_line, strlen(next_line)));
			}
			if (next_line[strlen(next_line) - 1] == '\n') {
				next_line[strlen(next_line) - 1] = 0;
				value += strchr(step, ' ') - step - 1;
				step += strchr(step, ' ') - step - 1;
			} else {
				value += strchr(step, ' ') - step - 1;
				step += strchr(step, ' ') - step - 1;
			}
			if (next_line[0] == ' ')
				parse_line(next_line + 1, next_line + 1,
						symbols, out);
			else
				parse_line(next_line, next_line, symbols, out);
			continue;
		}
		fprintf(out, "%c", *value);
	}
	return return_value;
}

/* Receives a line, parses it, and inserts the key/value in the hashtable.
 */
int parse_define(hashtable_t *symbols, char *line, FILE **in)
{
	char *aux_iter, *iter, *value, next_line[MAX_BUF];
	int key_len, value_len;

	while (strchr(line, '\\') != NULL &&
		(*(strchr(line, '\\') + 1) - 'a' > ('z' - 'a') ||
		*(strchr(line, '\\') + 1) - 'a' < 0)) {
		for (aux_iter = strchr(line, '\0') - 1; aux_iter != NULL &&
			(*aux_iter == ' ' || *aux_iter == '\\' ||
			*aux_iter == '\n'); --aux_iter)
			*aux_iter = '\0';
		fgets(next_line, MAX_BUF, *in);
		aux_iter = next_line;
		while (*aux_iter == ' ')
			aux_iter++;
		strcat(line, " ");
		strncat(line, aux_iter, MAX_BUF);
	}
	iter = line + 8;
	value = strchr(iter, ' ');
	if (value == NULL) {
		key_len = strchr(iter, '\0') - iter;
		hashtable_insert(symbols, iter, key_len, "", 1);
	} else {
		value_len = strchr(value, '\0') - value;
		key_len = value - iter;
		hashtable_insert(symbols, iter, key_len, value, value_len);
	}
	return 0;
}

/* Parses a line and checks if the value received with it is 0 or something
 * else, and take according action.
 */
int parse_if(hashtable_t *symbols, char *if_taken, int *current_depth,
			char *line, char *read, FILE **in)
{
	char *iter;

	(*current_depth)++;
	if (*current_depth > MAX_BUF)
		CLOSE(NULL, "Too many ifs imbricated", 27);
	iter = line + 4;
	*(iter + strlen(iter) - 1) = '\0';
	if (hashtable_at(symbols, iter, strlen(iter)) != NULL) {
		if (!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				"0\n") ||
			!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				"0") ||
			!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				" 0") ||
			!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				" 0\n")) {
			do {
				if (fgets(line, MAX_BUF, *in) == NULL)
					break;
			} while (strncmp(line, "#", 1) != 0);
			*read = 0;
		} else
			if_taken[*current_depth] = 1;
	} else {
		if (strcmp(iter, "0") == 0 || strcmp(iter, " 0") == 0) {
			do {
				if (fgets(line, MAX_BUF, *in) == NULL)
					break;
			} while (strncmp(line, "#", 1) != 0);
			*read = 0;
		} else {
			if (*iter - '0' < 9) {
				if_taken[*current_depth] = 1;
			} else {
				do {
					if (fgets(line, MAX_BUF, *in) == NULL)
						break;
				} while (strncmp(line, "#", 1) != 0);
				*read = 0;
			}
		}
	}
	return 0;
}

/* Similar to if, but does not increase the current_depth, just the value in it
 */
int parse_elif(hashtable_t *symbols, char *line, char *if_taken,
				int *current_depth, char *read, FILE **in)
{
	char *iter;

	iter = line + 6;
	*(iter + strlen(iter) - 1) = '\0';
	if (hashtable_at(symbols, iter, strlen(iter))) {
		if (!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				"0\n") ||
			!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				"0") ||
			!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				" 0") ||
			!strcmp(hashtable_at(symbols, iter, strlen(iter)),
				" 0\n")) {
			do {
				if (fgets(line, MAX_BUF, *in) == NULL)
					break;
			} while (strncmp(line, "#", 1) != 0);
			*read = 0;
		} else {
			if_taken[*current_depth]++;
		}
	} else {
		if (strcmp(iter, "0") == 0 || strcmp(iter, " 0") == 0) {
			do {
				if (fgets(line, MAX_BUF, *in) == NULL)
					break;
			} while (strncmp(line, "#", 1) != 0);
			*read = 0;
		} else {
			if (*iter - '0' < 9)
				if_taken[*current_depth]++;
			else {
				do {
					if (fgets(line, MAX_BUF, *in) == NULL)
						break;
				} while (strncmp(line, "#", 1) != 0);
				*read = 0;
			}
		}
	}
	return 0;
}

/* If no other if at that depth was taken then it takes this
 */
int parse_else(char *line, char *if_taken, int *current_depth, FILE **in)
{
	if (if_taken[*current_depth] > 0) {
		while (fgets(line, MAX_BUF, *in) != NULL) {
			if (strlen(line) > 6 &&
				memcmp(line, "#endif", 6) == 0) {
				if_taken[*current_depth] = 0;
				current_depth--;
				break;
			}
		}
	}
	return 0;
}

/* Similar to if but only checks if the key is defined
 */
int parse_ifdef(hashtable_t *symbols, char *line, char *if_taken,
				int *current_depth, char *read, FILE **in)
{
	char *iter;

	(*current_depth)++;
	if (*current_depth > MAX_BUF)
		CLOSE(NULL, "Too many ifs imbricated", 27);
	iter = line + 7;
	if (hashtable_at(symbols, iter, strchr(iter, '\0') - iter - 1)) {
		if_taken[*current_depth] = 1;
		return 0;
	}
	do {
		if (fgets(line, MAX_BUF, *in) == NULL)
			break;
	} while (strncmp(line, "#endif", 6) != 0);
	*read = 0;
	return 0;
}

/* Similar to ifdef but the result is reversed
 */
int parse_ifndef(hashtable_t *symbols, char *line, char *if_taken,
				int *current_depth, char *read, FILE **in)
{
	char *iter;

	(*current_depth)++;
	if (*current_depth > MAX_BUF)
		CLOSE(NULL, "Too many ifs imbricated", 27);
	iter = line + 8;
	if (*iter == ' ')
		iter++;
	if (!hashtable_at(symbols, iter, strchr(iter, '\0') - iter - 1)) {
		if_taken[*current_depth] = 1;
		return 0;
	}
	do {
		if (fgets(line, MAX_BUF, *in) == NULL)
			break;
	} while (strncmp(line, "#endif", 6) != 0);
	*read = 0;
	return 0;
}

/* Searches the given folders for the header and if it doesnt find it anywhere
 * it exits with an error.
 */
int parse_include(char *line, FILE **include_taken, int *include_depth,
		char **dirs, int *nr_dirs, char *in_path, FILE **in)
{
	char *iter, *aux_iter;
	FILE *aux_file;
	int file_found = 1, curr_dir;
	char item[MAX_BUF], saved_in_path[MAX_BUF];

	iter = line + 10;
	iter[strlen(iter) - 2] = '\0';
	strcpy(saved_in_path, in_path);
	aux_iter = strchr(saved_in_path, '\0');
	while (*aux_iter != '/') {
		*aux_iter = '\0';
		aux_iter--;
	}
	strcat(saved_in_path, iter);

	aux_file = fopen(saved_in_path, "r");
	if (aux_file != NULL) {
		include_taken[*include_depth] = *in;
		(*include_depth)++;
		*in = aux_file;
	} else {
		file_found = 0;
		for (curr_dir = 0; curr_dir < *nr_dirs - 1; ++curr_dir) {
			memset(item, '\0', MAX_BUF);
			strcpy(item, dirs[curr_dir]);
			strcat(item, "/");
			strcat(item, iter);

			aux_file = fopen(item, "r");
			if (aux_file != NULL) {
				include_taken[*include_depth] = *in;
				(*include_depth)++;
				*in = aux_file;
				file_found = 1;
				break;
			}
		}
		if (file_found == 0)
			CLOSE(NULL, "Include not Found!", 2);
	}
	return 0;
}

/* Receives all parameters in order to call specific functions and checks if
 * the received line matches any directive.
 */
int parse_directives(hashtable_t *symbols, char **line, char **if_taken,
		char *read, int *current_depth, FILE **in, FILE **include_taken,
		int *include_depth, char ***dirs, int *nr_dirs, char **in_path)
{
	char *iter;

	if (strlen(*line) > 7 && memcmp(*line, "#define", 7) == 0) {
		parse_define(symbols, *line, in);
		return 0;
	}

	if (strlen(*line) > 3 && memcmp(*line, "#if", 3) == 0 &&
				(*line)[3] == ' ') {
		parse_if(symbols, *if_taken, current_depth, *line, read, in);
		return 0;
	}

	if (strlen(*line) > 5 && memcmp(*line, "#elif", 5) == 0) {
		parse_elif(symbols, *line, *if_taken, current_depth, read, in);
		return 0;
	}

	if (strlen(*line) > 5 && memcmp(*line, "#else", 5) == 0) {
		parse_else(*line, *if_taken, current_depth, in);
		return 0;
	}

	if (strlen(*line) > 6 && memcmp(*line, "#endif", 6) == 0) {
		if (--(*current_depth) > 0)
			(*current_depth)--;
		return 0;
	}

	if (strlen(*line) > 6 && memcmp(*line, "#ifdef", 6) == 0) {
		parse_ifdef(symbols, *line, *if_taken, current_depth, read, in);
		return 0;
	}

	if (strlen(*line) > 7 && memcmp(*line, "#ifndef", 7) == 0) {
		parse_ifndef(symbols, *line, *if_taken,
				current_depth, read, in);
		return 0;
	}

	if (strlen(*line) > 8 && memcmp(*line, "#include", 8) == 0) {
		parse_include(*line, include_taken, include_depth,
				*dirs, nr_dirs, *in_path, in);
		return 0;
	}

	if (strlen(*line) > 6 && memcmp(*line, "#undef", 6) == 0) {
		iter = *line + 7;
		if (hashtable_at(symbols, iter, strchr(iter, '\0') - iter - 1))
			hashtable_pop(symbols, iter,
					strchr(iter, '\0') - iter - 1);
		return 0;
	}
	return 1;
}

int main(int argc, char **argv)
{
	hashtable_t symbols;
	FILE *out = NULL, *in = NULL, **include_taken;
	int include_depth = 0, current_depth = 0, nr_dirs = 1;
	char *iter, *line, item[MAX_BUF], *in_path, **dirs = NULL;
	char *if_taken, read = 1, ret = 1;

	line = malloc(sizeof(char) * 2 * MAX_BUF);
	CLOSE(line, "Malloc failed!", 12);
	include_taken = malloc(sizeof(FILE *) * MAX_BUF);
	CLOSE(include_taken, "Malloc failed!", 12);
	if_taken = calloc(sizeof(char), MAX_BUF);
	CLOSE(if_taken, "Malloc failed!", 12);
	hashtable_init(&symbols, MAX_HASH, MAX_HASH);
	parse_args(argc, argv, &symbols, &in, &out, &dirs, &nr_dirs, &in_path);

	while (1) {
		/*Skips a line that has already been read or reads a new one*/
		if (read == 1) {
			if (fgets(line, MAX_BUF, in) == NULL) {
				if (include_depth > 0) {
					fclose(in);
					include_depth--;
					in = include_taken[include_depth];
					continue;
				} else
					break;
			}
		} else
			read = 1;

		/*Checks for directives*/
		ret = parse_directives(&symbols, &line, &if_taken, &read,
				&current_depth, &in, include_taken,
				&include_depth, &dirs, &nr_dirs, &in_path);
		if (ret == 0)
			continue;

		/*Else it prints each character and parses the line*/
		iter = line;
		while (*iter == ' ')
			iter++;
		if (*iter == '\n')
			continue;
		iter = line;
		while (*iter == ' ') {
			fprintf(out, "%c", *iter);
			iter++;
		}
		strcpy(item, iter);
		parse_line(item, iter, &symbols, out);
	}

	free(line);
	free(if_taken);
	free(include_taken);
	if (in != stdin && in != NULL)
		fclose(in);
	if (out != stdout && out != NULL)
		fclose(out);
	if (dirs != NULL)
		free(dirs);
	hashtable_free(&symbols);
	return 0;
}
