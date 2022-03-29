#include "loaddb.h"
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

std::string
loadCSV(std::string file, int index) {
	// Open csv file, parse schema
	FILE* file_ptr;
	std::string db_name = file.substr(0, file.length()-4) + ".db";
	std::string index_name = file.substr(0, file.length()-4) + ".db.0";

	file_ptr = fopen(&db_name[0], "r");
	if (file_ptr)
    {
        fclose(file_ptr);
    	printf("The Table already exists\n");
		return NULL;
    } 

	FILE *fp = fopen(file.c_str() , "r");
	if (!fp) {
		perror("File could not be opened");
		exit(EXIT_FAILURE);
	}

	char buf[MAX_LINE_LEN];
	char *line = fgets(buf, MAX_LINE_LEN, fp);
	if (line == NULL) {
		fprintf(stderr, "Unable to read file\n");
		exit(EXIT_FAILURE);
	}

	// Open main db file
	std::string schemaTxt = std::string(line);
	Schema *sch = parseSchema(line);
	Table *tbl;

	// ----

	int err = Table_Open(db_name, sch, false, &tbl); 	// Create a file for storing the data...
	checkerr(err);
	// Create an index for the population field

	err = AM_CreateIndex(&db_name[0], 0,'i', 4);
	checkerr(err);

	int indexFD = PF_OpenFile(&index_name[0]);
	// ----
	tbl->indexFd = indexFD;
	char *tokens[MAX_TOKENS];
	char record[MAX_PAGE_SIZE];


	while ((line = fgets(buf, MAX_LINE_LEN, fp)) != NULL) {
		int n = split(line, ",", tokens);
		if (n != sch->numColumns) {
			std::cout << "Invalid insert of row " << line << std::endl;
			continue;
		}
		int len = encode(sch, tokens, record, sizeof(record));
		
		RecId rid;

		// ----
		err = Table_Insert(tbl, record, len, &rid);	// Add the new data into the table
		checkerr(err);
		// ----

		// Indexing on the population column 
		int index_value = atoi(tokens[index]);

		// ----
		err = AM_InsertEntry(indexFD, 'i', 4, (char*)&index_value, rid);	// Add the data into the index's data structure too 
		checkerr(err);
		// ----
	}

	fclose(fp);
	Table_Close(tbl);
	err = PF_CloseFile(indexFD);
	checkerr(err);
	schemaTxt.erase(std::remove(schemaTxt.begin(), schemaTxt.end(), '\n'), schemaTxt.end());
	return schemaTxt;
}


int
insertRow(Table *tbl, Schema *sch, std::string name, std::string row, int index) {
	
	std::string index_name = name + ".db.0";
	
	// Create an index for the population field
	int indexFD = PF_OpenFile(&index_name[0]);

	char *tokens[MAX_TOKENS];
	char record[MAX_PAGE_SIZE];

	int n = split(&row[0], ";", tokens);
	
	if (n != sch->numColumns)
		return 1;
	int len = encode(sch, tokens, record, sizeof(record));
	
	RecId rid;

	// ----
	int err = Table_Insert(tbl, record, len, &rid);	// Add the new data into the table
	checkerr(err);
	// ----

	// Indexing on the population column 
	int index_value = atoi(tokens[index]);

	// ----
	err = AM_InsertEntry(indexFD, 'i', 4, (char*)&index_value, rid);	// Add the data into the index's data structure too 
	checkerr(err);
	// ----
	
	Table_Close(tbl);
	err = PF_CloseFile(indexFD);
	checkerr(err);

	return 0;


	// int indexFD = PF_OpenFile(&index_name[0]);

	// char record[MAX_PAGE_SIZE];
	// char* tokens[MAX_TOKENS];

	// int n = split(&row[0], ";", tokens);
	// if (n != sch->numColumns)
	// 	return 1;
	// int len = encode(sch, tokens, record, sizeof(record));

	// RecId rid;

	// // ----

	// int err = Table_Insert(tbl, record, len, &rid);	// Add the new data into the table
	// checkerr(err);
	// // ----

	
	// // Indexing on the population column 
	// int index_value = atoi(tokens[indexNo]);

	// for (int i = 0; i < sch->numColumns; ++i)
	// 	std::cout << tokens[i] << " ";
	// std::cout << std::endl << index_value << std::endl;

	// // ----
	// err = AM_InsertEntry(indexFD, 'i', 4, (char*)&index_value, rid);	// Add the data into the index's data structure too 
	// checkerr(err);
	// // ----
	// return 0;
}