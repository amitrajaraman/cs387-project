#include "dumpdb.h"

extern "C" {
	#include "../pflayer/pf.h"
	#include "../amlayer/am.h"
}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}


void
printRow(void *callbackObj, RecId rid, byte *row, int len, std::vector<int> rowsToBePrinted) {
	Schema *schema = (Schema *) callbackObj;
	byte *cursor = row;

	// Iterate through the columns
	int rowIndex = 0;
	while(rowIndex < rowsToBePrinted.size()) {
		byte *tempCursor = cursor;
		for (int i = 0; schema->numColumns; ++i) {
			int x = schema->columns[i]->type;
			if (x == 1) {
				char tmp[999];
				DecodeCString(cursor,tmp,999);
				if(i == rowsToBePrinted[rowIndex]) {
					std::cout << tmp;
					++rowIndex;
					if(rowIndex == rowsToBePrinted.size()-1)
						std::cout << std::endl;
					else
						std::cout << ",";
					if(rowIndex >= rowsToBePrinted.size())
						break;
				}
				tempCursor = tempCursor + strlen(tmp)+2;
			}
			else if (x == 2) {
				int out = DecodeInt(cursor);
				if(i == rowsToBePrinted[rowIndex]) {
					std::cout << out;
					++rowIndex;
					if(rowIndex == rowsToBePrinted.size()-1)
						std::cout << std::endl;
					else
						std::cout << ",";
					if(rowIndex >= rowsToBePrinted.size())
						break;
				}
				tempCursor = tempCursor+4;
			}
			else if (x == 3) {
				long long out = DecodeLong(cursor);
				if(i == rowsToBePrinted[rowIndex]) {
					std::cout << out;
					++rowIndex;
					if(rowIndex == rowsToBePrinted.size())
						std::cout << std::endl;
					else
						std::cout << ",";
					if(rowIndex >= rowsToBePrinted.size())
						break;
				}
				tempCursor = tempCursor+8;
			}
			else {
				fprintf(stderr, "Schema column type unknown, custom error!\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}
	 
void
index_scan(Table *tbl, Schema *schema, int indexFD, int op, int value, std::vector<std::string> *colList) {
	/*
	Open index ...
	while (true) {
	find next entry in index
	fetch rid from table
		printRow(...)
	}
	close index ...
	*/
	int scan_desc = AM_OpenIndexScan(indexFD, 'i', 4, op, (char*)&value);
	
	std::vector<int> rowsToBePrinted;
	
	if(colList != NULL) {
		for (int i = 0; i < colList->size(); ++i) {
			for (int j = 0; j < tbl->schema->numColumns; ++j) {
				if(tbl->schema->columns[j]->name == (*colList)[i]) {
					rowsToBePrinted.push_back(j);
					break;
				}
			}
		}
	}
	else {
		for (int i = 0; i < tbl->schema->numColumns; ++i)
			rowsToBePrinted.push_back(i);
	}

	while(true){
		int next_rid = AM_FindNextEntry(scan_desc);         // Get the next entry...
		if(next_rid<0) break;                               // ...and break if at end
		byte record[999];
		int len = Table_Get(tbl, next_rid, record, 999);    // Get the byte array
		printRow(schema, next_rid, record, len, rowsToBePrinted);            // print the byte array
	}
	AM_CloseIndexScan(scan_desc);                           // Finally, close the scan desc at end
	// ----
}