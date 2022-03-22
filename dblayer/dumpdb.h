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

#include "../pflayer/pf.h"
#include "../amlayer/am.h"

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}

void printRow(void *callbackObj, RecId rid, byte *row, int len, std::vector<int> rowsToBePrinted);

void index_scan(Table *tbl, Schema *schema, int indexFD, int op, int value);
#endif