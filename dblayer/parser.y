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
	int *glbl_res;
	Schema* schema;

	extern struct tokens token_table[1000];
	std::vector<std::string> q;
	std::vector<std::string> cols;
	std::vector<int> cond;
	std::string table = ""; // $ would be a reserved keyword for database
	std::string created_table = ""; 
	int lock_type = -1;  // 0 for X-lock and 1 for S-lock
	int global_stat = 1;
	std::map<std::string, std::string> schema_meta_data;
	std::map<std::string, int> index_meta_data;
	std::map<std::string, std::vector<Constraint*> > constr_meta_data;

	int load_meta_data(std::map<std::string, std::string> &schema_meta_data, std::map<std::string, int> &index_meta_data, std::map<std::string, std::vector<Constraint*> > &constr_meta_data, std::string file_name = "meta_data.db") {
		schema_meta_data.clear();
		index_meta_data.clear();
		constr_meta_data.clear();

		std::vector<std::vector<std::string> > content, constr;
		std::vector<std::string> row, rc;
		std::string line, word;
	
		std::fstream file2 (file_name,  std::fstream::in | std::fstream::out | std::fstream::app );
		file2.close();
		std::fstream file (file_name,  std::fstream::in | std::fstream::out | std::fstream::ate );

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
		created_table = *$4;
		created_table = created_table.substr(0, created_table.length()-4);

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

// i is the client_id here
int executeQuery(int i, std::vector<std::string>q, std::vector<std::string>col,std::vector<int>cond,int client_id, int a, int b, int *res, std::string &output){
	// a is 1 if we are supposed to use local metadata else it is 0, b is 1 if we are supposed to use local table
    if(i == 0){
        //create table data_<i>.db and index file data_<i>.db.o
        std::string schemaTxt = loadCSV(q[0], stoi(q[1]), client_id);
		std::ofstream outfile;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
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
        std::string s1 = "Implemented commands:\n";
		std::string s2 = "'create table file <file_name> index <col_number>' creates a table from the csv file file_name with the indexing column being the col_number column. The name of the table is the name of the file without the csv extension.\n";
		std::string s3 = "'insert (<col0>;<col1>;...) into <table_name>' inserts the specified row into the table";
		std::string s4 = "'dump all <table_name>' to dump all data in the specified table\n";
		std::string s5 = "'dump all <table_name> where [eq/lt/gt/leq/geq/neq] num' to print all rows in the specified table with indexing row satisfying the given constraint\n";
		std::string s6 = "'dump <col_list> <table_name>' to dump the named columns in all the rows of the specified table\n";
		std::string s7 = "'dump <col_list> where [eq/lt/gt/leq/geq/neq] num' to print the named columns from all rows with indexing row satisfying the given constraint\n";
		std::string s8 = "'add constraint [eq/lt/gt/leq/geq/neq] <num> as <constraint_name> into <table_name>' to add a new constraint which checks all subsequent additions\n";
		std::string s9 = "'help' for help :)\n";
		std::string s10 = "'git' to show the git repository of this project.\n";
		std::string s11 = "'quit' to quit.\n";
		output = s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11;
    }
    else if(i == 3){
        //git
        output = "Head to https://github.com/amitrajaraman/cs387-project/ for the Git repository of this project!\n";
    }
    else if(i == 4){
        //insert
		std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[1]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		std::cout << "Reached here" << std::endl;
		if(b)
			local = q[1] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[1] + ".db";
		
		int ret = Table_Open(local, schema, false, &tbl);
		if(ret < 0){
			std::cout << "Result not available" << std::endl;
			*res = 0;
		}

		std::string index_name = "";

		if(insertRow(tbl, schema, index_name, q[0], index_meta_data[q[1]], constr_meta_data[q[1]]) != 0){
			std::cout << "Invalid insert of row!" << std::endl;
			*res = 0;
		}
		else
			std::cout << "Inserted successfully!" << std::endl;
    }
    else if(i == 5){
        //add constraint
        // assuming input query is of the form ADD CONSTRAINT op num AS NAME INTO NAME
        std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[1]];
		Schema *schema = parseSchema(&schemaTxt[0]);

		if(b)
			local = q[0] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[0] + ".db";
		
		int ret = Table_Open(local, schema, false, &tbl);
		if(ret<0){
			std::cout << "Result not available!" << std::endl;
			*res = 0;
		}
		
		for(int i=0; i<constr_meta_data[q[1]].size(); i++)
			if(constr_meta_data[q[1]][i]->constr_name == q[0]){
				std::cout << "constraint with same name already exists for this table!" << std::endl;
				*res = 0;
				return -1;
			}
		
		std::ofstream outfile;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
		outfile.open(local, std::ios_base::app);
		outfile << "#" + q[1] + ";" + q[0] + ";" + std::to_string(cond[0]) + ";" + std::to_string(cond[1]) << std::endl; 
		std::cout << "Added Contraint!" << std::endl;
    }
    else if(i == 6){
        //dump all table_name
        std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);

		if(b)
			local = q[0] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[0] + ".db";
		std::cout << "inside dump " << local << std::endl;
		int ret = Table_Open(local, schema, false, &tbl);

		if(ret < 0) {
			std::cout << "Result not available" << std::endl;
			*res = 0;
		}
		printAllRows(tbl, schema, printRow, NULL, output);
		Table_Close(tbl);
    }
    else if(i == 7){
        //dump all table_name where constraint
        // assuming input query is of the form DUMP STAR NAME WHERE op num
        //condition
        std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);

		if(b)
			local = q[0] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[0] + ".db";

		int ret = Table_Open(local, schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available" << std::endl;
			*res = 0;
		}
		std::string index_name;

		printAllRows(tbl, schema, printRow, NULL, output, index_meta_data[q[0]], cond[0], cond[1]);
    }
    else if(i == 8){
        //dump col-list table_name
        std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		if(b)
			local = q[0] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[0] + ".db";
		int ret = Table_Open(local, schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available" << std::endl;
			*res = 0;
		}
		printAllRows(tbl, schema, printRow, &col, output);
		Table_Close(tbl);
    }
    else if(i == 9){
        //dump col-list table_name where op num
        std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[0]];

		Schema *schema = parseSchema(&schemaTxt[0]);
		if(b)
			local = q[0] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[0] + ".db";
		int ret = Table_Open(local, schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available" << std::endl;
			*res = 0;
		}
		std::string index_name;


		printAllRows(tbl, schema, printRow, &col, output, index_meta_data[q[0]], cond[0], cond[1]);
    }
    else if(i == 10){
        //dump constraint name
        std::map<std::string, std::string> schema_meta_data;
		std::map<std::string, int> index_meta_data;
		std::map<std::string, std::vector<Constraint*> > constr_meta_data;
		std::string local;
		if(a)
			local = "meta_data_" + std::to_string(client_id) + ".db";
		else 
			local = "meta_data.db";
        load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, local);
		std::string schemaTxt = schema_meta_data[q[0]];
		Schema *schema = parseSchema(&schemaTxt[0]);
		
		if(b)
			local = q[0] + "_" + std::to_string(client_id) + ".db";
		else 
			local = q[0] + ".db";

		int ret = Table_Open(local, schema, false, &tbl);
		if(ret<0){
			std::cout << "Result not available!" << std::endl;
			*res = 0;
		}
		std::string index_name;

		if(b)
			index_name = q[0] + "_" + std::to_string(client_id) + ".db.0";
		else 
			index_name = q[0] + ".db.0";

		if(constr_meta_data[q[0]].size() == 0)
			output = "No constraints exist for this table!";
		else{
			output = output + "Constraint Name\tCondition\n";
			for(int i=0; i<constr_meta_data[q[0]].size(); i++){
				output = output + constr_meta_data[q[0]][i]->constr_name + "\t";
			}
		}
    }
	return 0;
}

int parse_query(std::string input, int *res) {

	//std::cout << "Welcome. Type `help` for help." << std::endl;

	// Schema and index meta data is in file meta_data.db
	std::map<std::string, std::string> schema_meta_data;
	std::map<std::string, int> index_meta_data;
	std::map<std::string, std::vector<Constraint*> > constr_meta_data;
    load_meta_data(schema_meta_data, index_meta_data, constr_meta_data, "meta_data.db");
		
	globalReadOffset = 0;
	globalInputText = new char[input.size() + 1];

	for (int i = 0; i < input.size(); i++) {
		globalInputText[i] = input[i];
	}
	globalInputText[input.size()] = '\0';

	global_stat = 1;
	yyparse();
	std::cout << "after yyparse, get value: " << global_stat << std::endl;
	*(res) = global_stat;

	if(stopFlag)
		return 0;

	delete[] globalInputText;	
	if(tbl)
		Table_Close(tbl);
	return 1;
}

int yyerror(std::string msg) {
	fprintf(stderr, "%s\n", msg.c_str());
	global_stat = 0;
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