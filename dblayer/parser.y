%{
	#include "common_headers.hh"
	Table *tbl = NULL;
	static int globalReadOffset;
	// Text to read:
	char *globalInputText;
	int yylex();
	int yyerror(std::string);
	void yyset_in (FILE * _in_str );
	void yyset_out (FILE * _out_str);
	int readInputForLexer(char* buffer,int *numBytesRead,int maxBytesToRead);
	extern int counting;
	int stopFlag = 0;
	int qc = -1;
	Schema* schema;

	extern struct tokens token_table[1000];
	std::map<std::string, std::string> schema_meta_data;
	std::map<std::string, int> index_meta_data;
	std::map<std::string, std::vector<Constraint*> > constr_meta_data;
	std::vector<std::string> q;
	std::vector<std::string> cols;
	std::vector<int> cond;
	std::string table = ""; // $ would be a reserved keyword for database
	int lock_type = -1;  // 0 for X-lock and 1 for S-lock

	int load_meta_data() {
		schema_meta_data.clear();
		index_meta_data.clear();
		constr_meta_data.clear();

		std::vector<std::vector<std::string> > content, constr;
		std::vector<std::string> row, rc;
		std::string line, word;
	
		std::fstream file2 ("meta_data.db",  std::fstream::in | std::fstream::out | std::fstream::app );
		file2.close();
		std::fstream file ("meta_data.db",  std::fstream::in | std::fstream::out | std::fstream::ate );

		file.seekg(0, std::ios::beg);
		if(file.is_open())
		{	
			while(getline(file, line))
			{
				// Schema information
				if(line.at(0)=='$'){
					line = line.substr(1, line.length());
					row.clear();
					std::stringstream str(line);
					while(getline(str, word, ';')) {
						row.push_back(word);
					}
					content.push_back(row);
				}
				// Constraint Information
				else if(line.at(0)=='#'){
					line = line.substr(1, line.length());
					rc.clear();
					std::stringstream str(line);
					while(getline(str, word, ';')){
						rc.push_back(word);
					}
					constr.push_back(rc);
				}
			}
		} 
		else {
			std::cout << "Error Occured\n";
		}
		for(int i=0;i<content.size();i++)
		{
			schema_meta_data[content[i][0]] = content[i][1];
			index_meta_data[content[i][0]] = stoi(content[i][2]);
		}
		for(int i=0; i<constr.size(); i++){
			Constraint *con = new Constraint;
			con->constr_name = constr[i][1];
			con->op = stoi(constr[i][2]);
			con->val = stoi(constr[i][3]);
			constr_meta_data[constr[i][0]].push_back(con);
		}
		file.close();
		return 1;

	}
%}
%union {
	std::string *name;
	std::vector<std::string> *colList;
	Condition *condition;
}
%token DUMP STAR WHERE QUIT HELP LT GT LEQ GEQ EQ NEQ COMMA CREATE TABLE FILE_KEYWORD INDEX GIT INSERT INTO LEFT_PAR RIGHT_PAR SEMICOLON ADD CONSTRAINT AS
%token <name> NUM
%token <name> NAME
// %token <name> DUMP
// %token <name> STAR
// %token <name> WHERE
// working without these now for some reason, not sure why
%token <name> FILE_NAME
%type <name> row
%type <condition> condition
%type <colList> column_list

%%

program
	: QUIT {
		qc = 1;
	}
	| HELP {
		qc = 2;
	}
	| GIT {
		qc = 3;
	}
	| CREATE TABLE FILE_KEYWORD FILE_NAME INDEX NUM {
		qc = 0;
		q.insert(q.end(),{*$4,*$6});
		table = "$";
		lock_type = 0; 

	}
	| DUMP STAR NAME {
		qc = 6;
		q.insert(q.end(),{*$3});
		table = *$3;
		lock_type = 1;
	}
	| DUMP column_list NAME {
		qc = 8;
		q.insert(q.end(),{*$3});
		cols = *$2;
		table = *$3;
		lock_type = 1;
	}
	| DUMP STAR NAME WHERE condition {
		qc = 7;
		q.insert(q.end(),{*$3});
		cond.insert(cond.end(),{*($5->op),*($5->num)});
		table = *$3;
		lock_type = 1;
	}
	| DUMP column_list NAME WHERE condition {
		qc = 9;
		q.insert(q.end(),{*$3});
		cond.insert(cond.end(),{*($5->op),*($5->num)});
		cols = *$2;
		table = *$3;
		lock_type = 1;
	}
	| INSERT LEFT_PAR row RIGHT_PAR INTO NAME {
		qc = 4;
		q.insert(q.end(),{*$3,*$6});
		table = *$6;
		lock_type = 0;
	}
	| ADD CONSTRAINT condition AS NAME INTO NAME {
		qc = 5;
		q.insert(q.end(),{*$5,*$7});
		cond.insert(cond.end(),{*($3->op),*($3->num)});
		table = "$";
		lock_type = 0;
 	}
	| DUMP CONSTRAINT NAME {
		qc = 10;
		q.insert(q.end(),{*$3});
	}
	| error 

row
	: row SEMICOLON NAME {
		$$ = $1;
		*($$) = *($$) + ";" + *($3);
	}
	| row SEMICOLON NUM {
		$$ = $1;
		*($$) = *($$) + ";" + *($3); 
	}
	| NAME {
		$$ = new std::string(*($1));
	}
	| NUM {
		$$ = new std::string(*($1));
	}

column_list
	: NAME {
		$$ = new std::vector<std::string>;
		$$->push_back(*($1));
	}
	| column_list COMMA NAME {
		$$ = $1;
		$$->push_back(*($3));
	}


condition
	: EQ NUM {
		$$->op = new int(1);
		$$->num = new int(stoi(*$2));
	}
	| LT NUM {
		$$->op = new int(2);
		$$->num = new int(stoi(*$2));
	}
	| GT NUM {
		$$->op = new int(3);
		$$->num = new int(stoi(*$2));
	}
	| LEQ NUM {
		$$->op = new int(4);
		$$->num = new int(stoi(*$2));
	}
	| GEQ NUM {
		$$->op = new int(5);
		$$->num = new int(stoi(*$2));
	}
	| NEQ NUM {
		$$->op = new int(6);
		$$->num = new int(stoi(*$2));
	}

%% 

int executeQuery(int i, std::vector<std::string>q, std::vector<std::string>col,std::vector<int>cond,int client_id){
    if(i == 0){
        //create table
        std::string schemaTxt = loadCSV(q[0], stoi(q[1]));
		std::ofstream outfile;
		std::string local = "meta_data_" + std::to_string(client_id) + ".db";
		outfile.open(local, std::ios_base::app);
		std::string s = q[0];
		s = s.substr(0, s.length()-4);
		outfile << "$" + s + ";" + schemaTxt.substr(0, schemaTxt.length()) + ";" + q[1] << std::endl; 
		std::cout << "Created table!\n";
    }
    else if(i == 1){
        //quit
        stopFlag = 1;
    }
    else if(i == 2){
        //help
        std::cout << "Implemented commands:\n"
				"'create table file <file_name> index <col_number>' creates a table from the csv file file_name with the indexing column being the col_number column. The name of the table is the name of the file without the csv extension.\n"
				"'insert (<col0>;<col1>;...) into <table_name>' inserts the specified row into the table"
				"'dump all <table_name>' to dump all data in the specified table\n"
				"'dump all <table_name> where [eq/lt/gt/leq/geq/neq] num' to print all rows in the specified table with indexing row satisfying the given constraint\n"
				"'dump <col_list> <table_name>' to dump the named columns in all the rows of the specified table\n"
				"'dump <col_list> where [eq/lt/gt/leq/geq/neq] num' to print the named columns from all rows with indexing row satisfying the given constraint\n"
				"'add constraint [eq/lt/gt/leq/geq/neq] <num> as <constraint_name> into <table_name>' to add a new constraint which checks all subsequent additions\n"
				"'help' for help :)\n"
				"'git' to show the git repository of this project.\n"
				"'quit' to quit." << std::endl;
    }
    else if(i == 3){
        //git
        std::cout << "Head to https://github.com/amitrajaraman/cs387-project/ for the Git repository of this project!" << std::endl;
    }
    else if(i == 4){
        //insert
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[1]];
		Schema *schema = parseSchema(&schemaTxt[0]);

		int ret = Table_Open(q[1] + "_" + std::to_string(client_id) + ".db", schema, false, &tbl);
		if(ret < 0)
			std::cout << "Result not available";
		std::string index_name = q[1] + "_" + std::to_string(client_id) + ".db.0";

		if(insertRow(tbl, schema, q[1], q[0], index_meta_data[q[1]], constr_meta_data[q[1]]) != 0)
			std::cout << "Invalid insert of row!" << std::endl;
		else
			std::cout << "Inserted successfully!" << std::endl;
    }
    else if(i == 5){
        //add constraint
        // assuming input query is of the form ADD CONSTRAINT op num AS NAME INTO NAME
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[1]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		
		int ret = Table_Open(q[0] + "_" + std::to_string(client_id) + ".db", schema, false, &tbl);
		if(ret<0)
			std::cout << "Result not available!" << std::endl;
		std::string index_name = q[1] + "_" + std::to_string(client_id) + "db.0";
		
		for(int i=0; i<constr_meta_data[q[1]].size(); i++)
			if(constr_meta_data[q[1]][i]->constr_name == q[0]){
				std::cout << "constraint with same name already exists for this table!" << std::endl;
				return -1;
			}
		
		std::ofstream outfile;
		std::string local = "meta_data_" + std::to_string(client_id) + ".db";
		outfile.open(local, std::ios_base::app);
		outfile << "#" + q[1] + ";" + q[0] + ";" + std::to_string(cond[0]) + ";" + std::to_string(cond[1]) << std::endl; 
		std::cout << "Added Contraint!" << std::endl;
    }
    else if(i == 6){
        //dump all table_name
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		int ret = Table_Open(q[0] + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		printAllRows(tbl, schema, printRow, NULL);
		Table_Close(tbl);
    }
    else if(i == 7){
        //dump all table_name where constraint
        // assuming input query is of the form DUMP STAR NAME WHERE op num
        //condition
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[0]];

		Schema *schema = parseSchema(&schemaTxt[0]);

		int ret = Table_Open(q[0] + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		std::string index_name = q[0] + ".db.0";
		char *index_name_c = new char[index_name.length() + 1];
		strcpy(index_name_c, index_name.c_str());
		int indexFD = PF_OpenFile(index_name_c);

		index_scan(tbl, schema, indexFD, cond[0], cond[1], NULL);
    }
    else if(i == 8){
        //dump col-list table_name
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		int ret = Table_Open(q[0] + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		printAllRows(tbl, schema, printRow, &col);
		Table_Close(tbl);
    }
    else if(i == 9){
        //dump col-list table_name where op num
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[0]];

		Schema *schema = parseSchema(&schemaTxt[0]);

		int ret = Table_Open(q[0] + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		std::string index_name = q[0] + ".db.0";
		char *index_name_c = new char[index_name.length() + 1];
		strcpy(index_name_c, index_name.c_str());
		int indexFD = PF_OpenFile(index_name_c);

		index_scan(tbl, schema, indexFD, cond[0], cond[1], &col);
    }
    else if(i == 10){
        //dump constraint name
        load_meta_data();
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		
		int ret = Table_Open(q[0] + "_" + std::to_string(client_id) + ".db", schema, false, &tbl);
		if(ret<0)
			std::cout << "Result not available!" << std::endl;
		std::string index_name = q[0] + "_" + std::to_string(client_id) + "db.0";

		if(constr_meta_data[q[0]].size() == 0)
			std::cout << "No constraints exist for this table!" << std::endl;
		else{
			std::cout << "Constraint Name\tCondition" << std::endl;
			for(int i=0; i<constr_meta_data[q[0]].size(); i++){
				std::cout << constr_meta_data[q[0]][i]->constr_name << "\t" << std::endl;
			}
		}
    }
	return 0;
}

int parse_query(std::string input) {

	//std::cout << "Welcome. Type `help` for help." << std::endl;

	// Schema and index meta data is in file meta_data.db
	load_meta_data();
		
	globalReadOffset = 0;
	globalInputText = new char[input.size() + 1];

	for (int i = 0; i < input.size(); i++) {
		globalInputText[i] = input[i];
	}
	globalInputText[input.size()] = '\0';

	yyparse();

	if(stopFlag)
		return 0;

	delete[] globalInputText;	
	if(tbl)
		Table_Close(tbl);
	return 1;
}

int yyerror(std::string msg) {
	fprintf(stderr, "%s\n", msg.c_str());
}

int readInputForLexer( char *buffer, int *numBytesRead, int maxBytesToRead ) {
    int numBytesToRead = maxBytesToRead;
    int bytesRemaining = strlen(globalInputText)-globalReadOffset;
    int i;
    if ( numBytesToRead > bytesRemaining ) { numBytesToRead = bytesRemaining; }
    for ( i = 0; i < numBytesToRead; i++ ) {
        buffer[i] = globalInputText[globalReadOffset+i];
    }
    *numBytesRead = numBytesToRead;
    globalReadOffset += numBytesToRead;
    return 0;
}