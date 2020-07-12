#include <stdio.h>
#include <stdlib.h>
#include <io.h>
//#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

#define true 1
#define false 0

#define MINBUFLEN 32
#define BUFLEN 256
#define MAXCNT 100

#define OUTFILE "test.out"

typedef struct token {
	int number;            // token number
	char value[MINBUFLEN];   // token value
}token;

char* typeName[] = {
   "LITERALSTR", "ID",   // literal string, identifier
   "INTEGER", "BOOLEAN", "FLOAT",         // signed integer, boolean string, floating-point number
   "INT", "CHAR", "BOOL", "FLOATTYPE",         // variable type
   "IF", "ELSE", "WHILE", "FOR", "RETURN",   // keyword
   "OP",         // +, -, *, /
   "BITWISE",            // <<, >>, &, |
   "ASSIGN",         // =
   "COMPARISON",         // <, >, ==, !=, <=, >=
   "TERMINATE",         // ;
   "LBRACE", "RBRACE",      // {}
   "LPAREN", "RPAREN",      // ()
   "COMMA",             // ,
   "WHITESPACE"			// \n, \t, blanks
};

enum typeNumber {
	LITERALSTR, ID,
	INTEGER, BOOLEAN, FLOAT,
	INT, CHAR, BOOL, FLOATTYPE,
	IF, ELSE, WHILE, FOR, RETURN,
	OP,
	BITWISE,
	ASSIGN,
	COMPARISON,
	TERMINATE,
	LBRACE, RBRACE,
	LPAREN, RPAREN,
	COMMA,
	WHITESPACE
};

char inputfile[BUFLEN];
int make_tokens(char* str, token* result);
void printResult(token* result, char* line);

int main(int argc, char* argv[])
{
	FILE* in_fp;
	int out_fd;
	char line[BUFLEN];
	token result[MAXCNT];

	if (argc > 1)
		strcpy(inputfile, argv[1]);
	else
		strcpy(inputfile, "test.c");

	// open and read input_file
	if ((in_fp = fopen(inputfile, "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", inputfile);
		exit(1);
	}

	// open and write output file
	if ((out_fd = open(OUTFILE, O_WRONLY | O_CREAT | O_TRUNC, 0640)) < 0) {
		fprintf(stderr, "open error for %s\n", OUTFILE);
		exit(1);
	}

	// copy file descriptor, out_fd = stdout
	dup2(out_fd, 1);

	while (fgets(line, BUFLEN, in_fp) != NULL) {
		line[strlen(line) - 1] = '\0';
		printf("- Input: %s\n", line);

		if (!make_tokens(line, result))
			printResult(result, line);
	}

	fclose(in_fp);
	close(out_fd);

	return 0;
}

void printResult(token* result, char* line)
{
	char buf[BUFLEN];
	int idx = 0;
	int tmp;
	int first = true;

	printf("- Output: ");
	while (1)
	{
		tmp = result[idx].number;

		if (tmp < 0)
			break;

		if (first == true) {
			printf("<%s", typeName[tmp]);
			first = false;
		}
		else
			printf(", <%s", typeName[tmp]);

		// keyword don't need to print value
		if (tmp != INT && tmp != CHAR && tmp != BOOL && tmp != FLOATTYPE
			&& tmp != IF && tmp != ELSE && tmp != WHILE
			&& tmp != FOR && tmp != RETURN) {
			printf(", %s>", result[idx].value);
		}
		else
			printf(">");

		idx++;
	}
	printf("<WHITESPACE, >");
	printf("\n\n");
}

int make_tokens(char* str, token* result)
{
	int i, j, idx = 0;
	char error = false;
	int len = strlen(str);
	char tmp[BUFLEN];
	int dotCnt;

	for (i = 0; i < len; i++)
	{
		strcpy(result[idx].value, "\0");
		if (error)
			break;

		// remove whitespace
		if (isspace(str[i]) || str[i] == '\t' || str[i] == '\n')
			result[idx++].number = WHITESPACE;
		//continue;

		else if (isdigit(str[i])) {
			// case 1) signed integer -> single zero
			//          float-point number -> 0->.->(zero|digit)
			if (str[i] == '0' && str[i + 1] != '.') {
				result[idx].number = INTEGER;
				strcpy(result[idx++].value, "0");
				continue;
			}

			int lastzero = 0;
			int lastdigit = 0;
			int firstdot = 0;
			dotCnt = 0;
			j = 0;

			while (1) {
				tmp[j++] = str[i++];

				// case 1) float-point number = digit+.(digit|zero)+
				if (str[i] == '.') {
					dotCnt++;
					if (!isdigit(str[i + 1])) {
						error = true;
						printf("- Output: Error at .\n\n");
						break;
					}
					if (dotCnt == 1)
						firstdot = i;
				}

				if (dotCnt == 1) {
					if (str[i - 1] >= '1' && str[i - 1] <= '9')
						lastdigit = i - 1;
					else
						lastzero = i - 1;
				}

				if (dotCnt > 0 && i + 1 <= len) {
					if (str[i + 1] == '.') {
						// ex. 0.00
						if (lastdigit == 0 && lastzero != 0) {
							tmp[firstdot + 2] = '\0';
							i = firstdot + 1;
							result[idx].number = FLOAT;
						}
						strcpy(result[idx++].value, tmp);
						break;
					}
				}
				// signed integer is sequence of digits
				else if (!isdigit(str[i]) && str[i] != '.') {
					tmp[j] = '\0';

					// case 1) (digits|zero).(zero|digit)*(digit)+
					// check the number of dot
					if (dotCnt > 0 && lastdigit == 0) {
						tmp[firstdot + 2] = '\0';
						i = firstdot + 1;
						result[idx].number = FLOAT;
						strcpy(result[idx++].value, tmp);
					}
					// case 3) non-empty sequence of digits
					else {
						// case 3-1) .(zero|digit)
						if (dotCnt > 0)
							result[idx].number = FLOAT;
						else
							result[idx].number = INTEGER;
						strcpy(result[idx++].value, tmp);
						--i;
					}
					break;
				}
			}
		}

		else if (isalpha(str[i]) || str[i] == '_') {
			j = 0;
			while (1) {
				tmp[j++] = str[i++];

				if (str[i] == ' ' || (str[i] != '_' && !isalpha(str[i]) && !isdigit(str[i]))) {
					tmp[j] = '\0';

					// case 1) variable type
					// variable type doesnt need token value
					if (!strcmp(tmp, "int"))
						result[idx++].number = INT;
					else if (!strcmp(tmp, "char"))
						result[idx++].number = CHAR;
					else if (!strcmp(tmp, "bool"))
						result[idx++].number = BOOL;
					else if (!strcmp(tmp, "float"))
						result[idx++].number = FLOATTYPE;
					// case 2) keyword
					// keyword doesnt need token value
					else if (!strcmp(tmp, "if"))
						result[idx++].number = IF;
					else if (!strcmp(tmp, "else"))
						result[idx++].number = ELSE;
					else if (!strcmp(tmp, "while"))
						result[idx++].number = WHILE;
					else if (!strcmp(tmp, "for"))
						result[idx++].number = FOR;
					else if (!strcmp(tmp, "return"))
						result[idx++].number = RETURN;
					// case 3) boolean type
					else if (!strcmp(tmp, "true")) {
						result[idx].number = BOOLEAN;
						strcpy(result[idx++].value, "true");
					}
					else if (!strcmp(tmp, "false")) {
						result[idx].number = BOOLEAN;
						strcpy(result[idx++].value, "false");
					}
					else {
						// case 4) identifier
						// ID needs token value
						strcpy(result[idx].value, tmp);
						result[idx++].number = ID;
					}
					--i;
					break;
				}
			}
		}

		else if (str[i] == '"') {
			// starting from and terminating with a symbol "

			tmp[0] = '"';
			j = 1;
			while (1) {

				tmp[j++] = str[++i];

				if (str[i] == '"') {
					tmp[j] = '\0';

					strcpy(result[idx].value, tmp);
					result[idx++].number = LITERALSTR;
					break;
				}
			}
		}

		else if (str[i] == '+') {
			result[idx].number = OP;
			strcpy(result[idx++].value, "+");
		}

		else if (str[i] == '-') {
			// case 1) operand - operand -> minus is arithmetic
			// signed integer and identifier can be operand
			if (result[idx - 1].number == INTEGER || result[idx - 1].number == FLOAT || result[idx - 1].number == ID) {
				result[idx].number = OP;
				strcpy(result[idx++].value, "-");
			}
			// case 2) - -> zero -> not . => - is arithmetic
			else if (str[i + 1] == '0' && str[i + 2] != '.') {
				result[idx].number = OP;
				strcpy(result[idx++].value, "-");
			}
			// case 3) signed integer or floating-point number
			else {
				int lastzero = 0;
				int lastdigit = 0;
				int firstdot = 0;
				dotCnt = 0;
				j = 0;
				while (1) {
					tmp[j++] = str[i++];

					// case 2-1) float-point number = .(zero|digit)+
					// ex) -0.00. -> error
					// ex) -0.00.0 -> float + float
					// ex) -0. -> error
					if (str[i] == '.') {
						dotCnt++;
						if (!isdigit(str[i + 1])) {
							error = true;
							printf("- Output: Error at .\n\n");
							break;
						}
						if (dotCnt == 1)
							firstdot = i;
					}

					if (dotCnt == 1) {
						if (str[i - 1] >= '1' && str[i - 1] <= '9')
							lastdigit = i - 1;
						else
							lastzero = i - 1;
					}

					if (dotCnt > 0 && i + 1 <= len) {
						if (str[i + 1] == '.') {
							// ex. 0.00
							if (lastdigit == 0 && lastzero != 0) {
								tmp[firstdot + 2] = '\0';
								i = firstdot + 1;
								result[idx].number = FLOAT;
							}
							strcpy(result[idx++].value, tmp);
							break;
						}
					}
					// signed integer is sequence of digits
					else if (!isdigit(str[i]) && str[i] != '.') {
						tmp[j] = '\0';

						if (dotCnt > 0 && lastdigit == 0) {
							tmp[firstdot + 2] = '\0';
							i = firstdot + 1;
							result[idx].number = FLOAT;
							strcpy(result[idx++].value, tmp);
						}
						else {
							if (dotCnt > 0)
								result[idx].number = FLOAT;
							else
								result[idx].number = INTEGER;
							strcpy(result[idx++].value, tmp);
							--i;
						}
						break;
					}
				}
			}
		}

		else if (str[i] == '*') {
			result[idx].number = OP;
			strcpy(result[idx++].value, "*");
		}

		else if (str[i] == '/') {
			result[idx].number = OP;
			strcpy(result[idx++].value, "/");
		}

		// <<, >>, <=, >=
		else if (str[i] == '<' || str[i] == '>') {
			j = 0;
			while (1) {
				tmp[j++] = str[i++];

				if (str[i] != '<' && str[i] && '>' && str[i] != '=') {
					tmp[j] = '\0';

					if (j < 3) {
						// case 1) comparison
						if (j == 1) {
							result[idx].number = COMPARISON;
							strcpy(result[idx++].value, tmp);
							--i;
						}
						// case 2) bitwise
						else if (j == 2) {
							// correct. ex) <<, >>
							if (tmp[0] == tmp[1]) {
								result[idx].number = BITWISE;
								strcpy(result[idx++].value, tmp);
								--i;
							}
							else if (tmp[1] == '=') {
								tmp[j] = '\0';
								result[idx].number = COMPARISON;
								strcpy(result[idx++].value, tmp);
								--i;
							}
							else {
								tmp[j - 1] = '\0';
								result[idx].number = COMPARISON;
								strcpy(result[idx++].value, tmp);
								i -= 2;
							}
						}
					}
					else {
						error = true;
						printf("Error at %s\n", tmp);
					}
					break;
				}
			}
		}

		else if (str[i] == '=') {
			j = 0;
			while (1) {
				tmp[j++] = str[i++];

				if (str[i] != '=') {
					tmp[j] = '\0';

					if (j < 3) {
						// case 1) assignment
						if (j == 1)
							result[idx].number = ASSIGN;
						// case 2) comparison
						else if (j == 2)
							result[idx].number = COMPARISON;
						strcpy(result[idx++].value, tmp);
					}
					// case 3) error (ex.'===', '====' ..)
					else {
						error = true;
						printf("Error at %s\n", tmp);
					}

					--i;
					break;
				}
			}
		}

		else if (str[i] == '&') {
			result[idx].number = BITWISE;
			strcpy(result[idx++].value, "&");
		}

		else if (str[i] == '|') {
			result[idx].number = BITWISE;
			strcpy(result[idx++].value, "|");
		}

		else if (str[i] == '!') {
			if (str[i + 1] == '=') {
				j = 0;
				tmp[j++] = str[i++];
				tmp[j++] = str[i++];
				tmp[j] = '\0';
				result[idx].number = COMPARISON;
				strcpy(result[idx++].value, tmp);
			}
			else {
				error = true;
				printf("Error at !\n");
			}
		}

		else if (str[i] == '(') {
			result[idx].number = LPAREN;
			strcpy(result[idx++].value, "(");
		}

		else if (str[i] == ')') {
			result[idx].number = RPAREN;
			strcpy(result[idx++].value, ")");
		}

		else if (str[i] == '{') {
			result[idx].number = LBRACE;
			strcpy(result[idx++].value, "{");
		}

		else if (str[i] == '}') {
			result[idx].number = RBRACE;
			strcpy(result[idx++].value, "}");
		}

		else if (str[i] == ';') {
			result[idx].number = TERMINATE;
			strcpy(result[idx++].value, ";");
		}

		else if (str[i] == ',') {
			result[idx].number = COMMA;
			strcpy(result[idx++].value, ",");
		}
	}

	// no more tokens
	result[idx].number = -1;
	return error;
}