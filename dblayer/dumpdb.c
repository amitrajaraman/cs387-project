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


void
printRow(void *callbackObj, RecId rid, byte *row, int len) {
	Schema *schema = (Schema *) callbackObj;
	byte *cursor = row;

	// UNIMPLEMENTED;
	// ----
	// Iterate through the columns
	for(int i=0; i<schema->numColumns-1; i++){
		int x = schema->columns[i]->type;
		if (x == 1) {
			char tmp[999];
			DecodeCString(cursor,tmp,999);
			cursor = cursor + strlen(tmp)+2;
			printf("%s,", tmp);			
		}
		else if (x == 2) {
			int out = DecodeInt(cursor);
			printf("%d,", out);
			cursor = cursor+4;
		}
		else if (x == 3) {
			long long out = DecodeLong(cursor);
			printf("%llu,", out);
			cursor = cursor+8;
		}
		else {
			fprintf(stderr, "Schema column type unknown, custom error!\n");
			exit(EXIT_FAILURE);
		}
	}
	// Go through the last table
	int x = schema->columns[schema->numColumns-1]->type;
	if (x == 1) {
		char tmp[999];
		DecodeCString(cursor,tmp,999);
		printf("%s\n", tmp);
		cursor = cursor + strlen(tmp) + 2;
	}
	else if (x == 2) {
		int out = DecodeInt(cursor);
		printf("%d\n", out);
		cursor = cursor+4;
	}
	else if (x == 3) {
		long long out = DecodeLong(cursor);
		printf("%llu\n", out);
		cursor = cursor+8;
	}
	else {
		fprintf(stderr, "Schema column type unknown, custom error!\n");
		exit(EXIT_FAILURE);
	}
	// ----
}

#define DB_NAME "data.db"
#define INDEX_NAME "data.db.0"
	 
void
index_scan(Table *tbl, Schema *schema, int indexFD, int op, int value) {
	// UNIMPLEMENTED;
	// ----
	/*
	Open index ...
	while (true) {
	find next entry in index
	fetch rid from table
		printRow(...)
	}
	close index ...
	*/
	int scan_desc = AM_OpenIndexScan(indexFD, 'i', 4, op, (char*)&value); // Hardcoded population as index; not good imo
	while(true){
		int next_rid = AM_FindNextEntry(scan_desc);         // Get the next entry...
		if(next_rid<0) break;                               // ...and break if at end
		byte record[999];
		int len = Table_Get(tbl, next_rid, record, 999);    // Get the byte array
		printRow(schema, next_rid, record, len);            // print the byte array
	}
	AM_CloseIndexScan(scan_desc);                           // Finally, close the scan desc at end
	// ----
}