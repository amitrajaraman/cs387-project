#ifndef _CH
#define _CH
#include <string>
#include <iostream>
#include <vector>
#include "loaddb.h"
#include "dumpdb.h"
#include "client.h"
#include <map>
#include <fstream>
#include <sstream>
struct Condition {
    int* op;
    int* num;
};
struct tokens {
    std::string token_name;
    std::string lexeme;
    int lineno;
};

extern std::queue<TransactionInstance> transaction_queue;
int readInputForLexer(char* buffer,int *numBytesRead,int maxBytesToRead);
int parse_query(std::string input);

#endif