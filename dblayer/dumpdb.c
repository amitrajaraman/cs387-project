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

// std::vector<std::string> split(const std::string &s, char delim) {
//     std::vector<std::string> elems;
//     split(s, delim, elems);
//     return elems;
// }

bool BothAreSpaces(char lhs, char rhs) { return (isspace(lhs)) && (isspace(rhs)); }

int
main(int argc, char **argv) {
	std::string schemaTxt = "Country:varchar,Capital:varchar,Population:int";
	Schema *schema = parseSchema(&schemaTxt[0]);
	Table *tbl;

	// UNIMPLEMENTED;
	// ----
	Table_Open(DB_NAME, schema, false, &tbl);   // Open the database and load table into memory
	// ----
	
	std::cout << "Welcome. Type `help` for help." << std::endl;

	std::string input;
	while(std::getline(std::cin, input)) {
		std::string::iterator new_end = std::unique(input.begin(), input.end(), BothAreSpaces);
		input.erase(new_end, input.end());   

		std::vector<std::string> splitInput{};

	    size_t pos = 0;
	    std::string tmpInput = input;
	    std::string space_delimiter = " ";
	    while ((pos = input.find(space_delimiter)) != std::string::npos) {
	        splitInput.push_back(input.substr(0, pos));
	        input.erase(0, pos + space_delimiter.length());
	    }
	    splitInput.push_back(input);
	    input = tmpInput;

		if(input == "quit" or input == "exit")
			break;
		// if (argc == 2 && *(argv[1]) == 's')
		else if(input == "dump all") {
			// UNIMPLEMENTED ;
			// invoke Table_Scan with printRow, which will be invoked for each row in the table.
			// ----
			Table_Scan(tbl, schema, printRow);  // Sequentially scan the table
			// ----
		} 
		else if(splitInput.size() == 3 && splitInput[0] == "dump") {
			// index scan by default
			int indexFD = PF_OpenFile(INDEX_NAME);
			checkerr(indexFD);

			// Ask for populations less than 100000, then more than 100000. Together they should
			// yield the complete database.
			int op = -1;

			if(splitInput[1] == "EQUAL" || splitInput[1] == "eq") op = 1;
			if(splitInput[1] == "LESS_THAN" || splitInput[1] == "lt") op = 2;
			if(splitInput[1] == "GREATER_THAN" || splitInput[1] == "gt") op = 3;
			if(splitInput[1] == "LESS_THAN_EQUAL" || splitInput[1] == "leq") op = 4;
			if(splitInput[1] == "GREATER_THAN_EQUAL" || splitInput[1] == "geq") op = 5;
			if(splitInput[1] == "NOT_EQUAL" || splitInput[1] == "neq") op = 6;
			
			if(op == -1)
				printf("Invalid operation\n");
			else
				index_scan(tbl, schema, indexFD, op, stoi(splitInput[2]));
		}
		else if(input == "help") {
			std::cout << "Type 'dump all' to dump all data, 'dump [eq/lt/gt/leq/geq/neq] num' to print all rows with population satisfying the given constraint, and 'quit' to quit." << std::endl;
		}
		else {
			printf("Invalid arguments!\n");
		}
		std::cout << std::endl;
		
	}
	Table_Close(tbl);
}
