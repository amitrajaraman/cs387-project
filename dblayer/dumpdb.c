#include "dumpdb.h"

extern "C" {
	#include "../pflayer/pf.h"
	// #include "../amlayer/am.h"
}

#define checkerr(err) {if (err < 0) {PF_PrintError(); exit(1);}}


void
printRow(void *callbackObj, RecId rid, byte *row, int len, std::vector<int> rowsToBePrinted, std::string &output, int indexCol, int op, int value) {
	Schema *schema = (Schema *) callbackObj;
	byte *cursor = row;

	// Iterate through the columns
	int rowIndex = 0;
	std::string currOutput;
	while(rowIndex < rowsToBePrinted.size()) {
		int flag = 0;
		byte *tempCursor = cursor;
		for (int i = 0; i < schema->numColumns; ++i) {
			int x = schema->columns[i]->type;
			if (x == 1) {
				char tmp[999];
				DecodeCString(tempCursor,tmp,999);
				if(i == rowsToBePrinted[rowIndex]) {
					currOutput += tmp;
					++rowIndex;
				}
				tempCursor = tempCursor + strlen(tmp)+2;
			}
			else if (x == 2) {
				int out = DecodeInt(tempCursor);
				if(i == indexCol) {
					if(op == 1 && out != value) {
						currOutput = "";
						flag = 1;
					}
					else if(op == 2 && out >= value) {
						currOutput = "";
						flag = 1;
					}
					else if(op == 3 && out <= value) {
						currOutput = "";
						flag = 1;
					}
					else if (op == 4 && out > value) {
						currOutput = "";
						flag = 1;
					}
					else if(op == 5 && out < value) {
						currOutput = "";
						flag = 1;
					}
					else if (op == 6 && out == value) {
						currOutput = "";
						flag = 1;
					}
				}
				if(flag != 1 && i == rowsToBePrinted[rowIndex]) {
					currOutput += std::to_string(out);
					++rowIndex;
				}
				tempCursor = tempCursor+4;
			}
			else if (x == 3) {
				long long out = DecodeLong(tempCursor);
				if(i == indexCol) {
					if(op == 1 && out != value) {
						currOutput = "";
						flag = 1;
					}
					else if(op == 2 && out >= value) {
						currOutput = "";
						flag = 1;
					}
					else if(op == 3 && out <= value) {
						currOutput = "";
						flag = 1;
					}
					else if (op == 4 && out > value) {
						currOutput = "";
						flag = 1;
					}
					else if(op == 5 && out < value) {
						currOutput = "";
						flag = 1;
					}
					else if (op == 6 && out == value) {
						currOutput = "";
						flag = 1;
					}
				}
				if(flag != 1 && i == rowsToBePrinted[rowIndex]) {
					currOutput += std::to_string(out);
					++rowIndex;
				}
				tempCursor = tempCursor+8;
			}
			else {
				fprintf(stderr, "Schema column type unknown, custom error!\n");
				exit(EXIT_FAILURE);
			}
			if(rowIndex > 0 && i == rowsToBePrinted[rowIndex-1]) {
				if(rowIndex == rowsToBePrinted.size())
					currOutput += "\n";
				else
					currOutput += ",";
			}
			if(rowIndex >= rowsToBePrinted.size())
				break;
		}
		if(flag == 1)
			break;
	}
	output += currOutput;
}

// void
// index_scan(Table *tbl, Schema *schema, int indexFD, int op, int value, std::vector<std::string> *colList, std::string &output) {
// 	/*
// 	Open index ...
// 	while (true) {
// 	find next entry in index
// 	fetch rid from table
// 		printRow(...)
// 	}
// 	close index ...
// 	*/
// 	int scan_desc = AM_OpenIndexScan(indexFD, 'i', 4, op, (char*)&value);
	
// 	std::vector<int> rowsToBePrinted;
	
// 	if(colList != NULL) {
// 		for (int i = 0; i < colList->size(); ++i) {
// 			for (int j = 0; j < tbl->schema->numColumns; ++j) {
// 				if(tbl->schema->columns[j]->name == (*colList)[i]) {
// 					rowsToBePrinted.push_back(j);
// 					break;
// 				}
// 			}
// 		}
// 	}
// 	else {
// 		for (int i = 0; i < tbl->schema->numColumns; ++i)
// 			rowsToBePrinted.push_back(i);
// 	}

// 	while(true){
// 		int next_rid = AM_FindNextEntry(scan_desc);         // Get the next entry...
// 		if(next_rid<0) break;                               // ...and break if at end
// 		byte record[999];
// 		int len = Table_Get(tbl, next_rid, record, 999);    // Get the byte array
// 		printRow(schema, next_rid, record, len, rowsToBePrinted, output);            // print the byte array
// 	}
// 	AM_CloseIndexScan(scan_desc);                           // Finally, close the scan desc at end
// 	// ----
// }