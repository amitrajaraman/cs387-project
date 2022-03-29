#ifndef _LOADDB
#define _LOADDB
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <ctype.h>
#include <algorithm>
#include "codec.h"
#include "tbl.h"
#include "util.h"
extern "C" {
    #include "../pflayer/pf.h"
    #include "../amlayer/am.h"
}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}

#define MAX_PAGE_SIZE 4000
int encode(Schema *sch, char **fields, byte *record, int spaceLeft);
std::string loadCSV(std::string file, int index);
int insertRow(Table *tbl, Schema *sch, std::string indexName, std::string row, int indexNo);
#endif