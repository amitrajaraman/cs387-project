#include <iostream>

#define MAX_TOKENS 100
#define MAX_LINE_LEN   1000

int stricmp(char const *a, char const *b);
char *trim(char *str);

int split(char *buf, std::string delim, char **tokens);

Schema *parseSchema(char* buf);
