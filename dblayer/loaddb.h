#ifndef _LOADDB
#define _LOADDB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "codec.h"
#include "tbl.h"
#include "util.h"
#include "../pflayer/pf.h"
#include "../amlayer/am.h"

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}

#define MAX_PAGE_SIZE 4000
int encode(Schema *sch, char **fields, byte *record, int spaceLeft);
Schema * loadCSV(std::string file, int index);
#endif