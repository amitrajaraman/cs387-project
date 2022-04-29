#ifndef _DUMPDB
#define _DUMPDB

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include "codec.h"
#include "tbl.h"
#include "util.h"

extern "C" {
    #include "../pflayer/pf.h"
    #include "../amlayer/am.h"
}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}

void printRow(void *callbackObj, RecId rid, byte *row, int len, std::vector<int> rowsToBePrinted, std::string &output, int, int, int);

// void index_scan(Table *tbl, Schema *schema, int indexFD, int op, int value, std::vector<std::string> *colList, std::string &output);
#endif