#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "codec.h"
#include "tbl.h"
#include "util.h"
extern "C" {
	#include "../pflayer/pf.h"
	#include "../amlayer/am.h"
}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}

#define MAX_PAGE_SIZE 4000


#define DB_NAME "data.db"
#define INDEX_NAME "data.db.0"
#define CSV_NAME "data.csv"


/*
Takes a schema, and an array of strings (fields), and uses the functionality
in codec.c to convert strings into compact binary representations
 */
int
encode(Schema *sch, char **fields, byte *record, int spaceLeft) {
	// UNIMPLEMENTED; 
	// for each field
	//    switch corresponding schema type is
	//        VARCHAR : EncodeCString
	//        INT : EncodeInt
	//        LONG: EncodeLong
	// return the total number of bytes encoded into record
	// ----

	int n = sch->numColumns;       // Get the number of attributes
	int bytes_encoded = 0;                          // Bytes encoded
	for(int i=0; i<n; i++){
		char uhg[999];
		int x = 0;
		switch (sch->columns[i]->type)
		{
		case 1:
			x = EncodeCString(fields[i], record+bytes_encoded, spaceLeft-bytes_encoded);
			DecodeCString(record+bytes_encoded, uhg, 999);
			bytes_encoded += x;
			break;
			
		case 2:
			EncodeInt(atoi(fields[i]), record+bytes_encoded);
			bytes_encoded += 4;
			break;
			
		case 3:
			EncodeLong(atoi(fields[i]), record+bytes_encoded);
			bytes_encoded += 8;
			break;
		
		default:
			fprintf(stderr, "Schema data type unknown, custom error!\n");
			exit(EXIT_FAILURE);
			break;									// Not really needed, but return error if schema type is not one of these three
		}
		if(bytes_encoded > spaceLeft){
			fprintf(stderr, "Not enough place in record, custom error!\n");
			exit(EXIT_FAILURE);									// Throw error if not enough space present
		}
	}
	return bytes_encoded;	// Return the total bytes encoded into record
	// ----

}

Schema *
loadCSV() {
	// Open csv file, parse schema
	FILE *fp = fopen(CSV_NAME, "r");
	if (!fp) {
	perror("data.csv could not be opened");
		exit(EXIT_FAILURE);
	}

	char buf[MAX_LINE_LEN];
	char *line = fgets(buf, MAX_LINE_LEN, fp);
	if (line == NULL) {
		fprintf(stderr, "Unable to read data.csv\n");
		exit(EXIT_FAILURE);
	}

	// Open main db file
	Schema *sch = parseSchema(line);
	Table *tbl;

	// UNIMPLEMENTED;
	// ----
	int err = Table_Open(DB_NAME, sch, false, &tbl); 	// Create a file for storing the data...
	checkerr(err);
	
	// Create an index for the population field
	err = AM_CreateIndex(DB_NAME, 0,'i', 4);
	checkerr(err);
	int indexFD = PF_OpenFile(INDEX_NAME);

	// ----

	char *tokens[MAX_TOKENS];
	char record[MAX_PAGE_SIZE];

	while ((line = fgets(buf, MAX_LINE_LEN, fp)) != NULL) {
		int n = split(line, ",", tokens);
		assert (n == sch->numColumns);
		int len = encode(sch, tokens, record, sizeof(record));
		
		RecId rid;

		// UNIMPLEMENTED;
		// Implemented, need to check!
		// ----
		err = Table_Insert(tbl, record, len, &rid);	// Add the new data into the table
			checkerr(err);
		// ----

		// Indexing on the population column 
		int population = atoi(tokens[2]);

		// UNIMPLEMENTED;
		// ----
		err = AM_InsertEntry(indexFD, 'i', 4, (char*)&population, rid);	// Add the data into the index's data structure too 
			checkerr(err);
		// ----
	}

	fclose(fp);
	Table_Close(tbl);
	err = PF_CloseFile(indexFD);
	checkerr(err);
	return sch;
}

int
main() {
	loadCSV();
}
