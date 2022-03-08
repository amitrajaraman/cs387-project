#include <stdio.h>
#include <stdlib.h>
#include "codec.h"
#include "tbl.h"
#include "util.h"
#include "../pflayer/pf.h"
#include "../amlayer/am.h"
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

int
main(int argc, char **argv) {
	char *schemaTxt = "Country:varchar,Capital:varchar,Population:int";
	Schema *schema = parseSchema(schemaTxt);
	Table *tbl;

	// UNIMPLEMENTED;
	// ----
	Table_Open(DB_NAME, schema, false, &tbl);   // Open the database and load table into memory
	// ----

	if (argc == 2 && *(argv[1]) == 's') {
		// UNIMPLEMENTED ;
		// invoke Table_Scan with printRow, which will be invoked for each row in the table.
		// ----
		Table_Scan(tbl, schema, printRow);  // Sequentially scan the table
		// ----
	} 
	else if(argc == 4 && *(argv[1]) == 'i') {
		// index scan by default
		int indexFD = PF_OpenFile(INDEX_NAME);
		checkerr(indexFD);

		// Ask for populations less than 100000, then more than 100000. Together they should
		// yield the complete database.
		int op = -1;

		if(stricmp(argv[2], "EQUAL")==0) op = 1;
		if(stricmp(argv[2], "LESS_THAN")==0) op = 2;
		if(stricmp(argv[2], "GREATER_THAN")==0) op = 3;
		if(stricmp(argv[2], "LESS_THAN_EQUAL")==0) op = 4;
		if(stricmp(argv[2], "GREATER_THAN_EQUAL")==0) op = 5;
		if(stricmp(argv[2], "NOT_EQUAL")==0) op = 6;
		
		if(op == -1)
			printf("Invalid operation\n");
		else
			index_scan(tbl, schema, indexFD, op, atoi(argv[3]));
	}
	else if(argc == 2 && *(argv[1]) == 'i') {
		// index scan by default
		int indexFD = PF_OpenFile(INDEX_NAME);
		checkerr(indexFD);

		// Ask for populations less than 100000, then more than 100000. Together they should
		// yield the complete database.
		index_scan(tbl, schema, indexFD, LESS_THAN_EQUAL, 100000);
		index_scan(tbl, schema, indexFD, GREATER_THAN, 100000);
	}
	else{
		printf("Invalid arguments!\n");
	}
	Table_Close(tbl);
}
