%{
	#include "common_headers.hh"

	Table *tbl;

	int yylex();
	int yyerror(std::string);
	void yyset_in (FILE * _in_str );
	void yyset_out (FILE * _out_str);
	extern int counting;
	int stopFlag = 0;
	Schema* schema;
	struct tokens {
		std::string token_name;
		std::string lexeme;
		int lineno;
	};

	extern struct tokens token_table[1000];
	std::map<std::string, std::string> schema_meta_data;
	std::map<std::string, int> index_meta_data;

	int load_meta_data() {
		schema_meta_data.clear();
		index_meta_data.clear();
		std::vector<std::vector<std::string>> content;
		std::vector<std::string> row;
		std::string line, word;
	
		std::fstream file2 ("meta_data.db",  std::fstream::in | std::fstream::out | std::fstream::app );
		file2.close();
		std::fstream file ("meta_data.db",  std::fstream::in | std::fstream::out | std::fstream::ate );

		file.seekg(0, std::ios::beg);
		if(file.is_open())
		{	
			std::cout << "here\n";
			while(getline(file, line))
			{
				row.clear();
				std::cout << "here1\n";
				std::stringstream str(line);
				while(getline(str, word, ';')) {
					std::cout << "here2\n";
					row.push_back(word);
				}
				content.push_back(row);
			}
		} 
		else {
			std::cout << "Error Occured\n";
		}
		for(int i=0;i<content.size();i++)
		{
			std::cout << content[i][0] << std::endl;
			std::cout << content[i][1] << std::endl;
			std::cout << content[i][2] << std::endl;
			schema_meta_data[content[i][0]] = content[i][1];
			index_meta_data[content[i][0]] = stoi(content[i][2]);
		}
		file.close();
		std::cout << schema_meta_data["data"] << std::endl;
		return 1;

	}
%}
%union {
	std::string *name;
	std::vector<std::string> *colList;
	Condition *condition;
}
%token DUMP STAR WHERE QUIT HELP LT GT LEQ GEQ EQ NEQ COMMA CREATE TABLE FILE_KEYWORD INDEX
%token <name> NUM
%token <name> NAME
%token <name> FILE_NAME
%type <condition> condition
%type <colList> column_list

%%

program : QUIT {
		stopFlag = 1;
	}
	| HELP {
		std::cout << "Type 'dump all' to dump all data, 'dump all where [eq/lt/gt/leq/geq/neq] num' to print all rows with population satisfying the given constraint, and 'quit' to quit." << std::endl;
	}
	| CREATE TABLE FILE_KEYWORD FILE_NAME INDEX NUM {
		std::string schemaTxt = loadCSV(*$4, stoi(*$6));
		std::ofstream outfile;
		outfile.open("meta_data.db", std::ios_base::app);
		std::string s = *$4;
		s = s.substr(0, s.length()-4);
		outfile << s + ";" + schemaTxt + ";" + *$6; 
		std::cout << "Created table!\n";
	}
	| DUMP STAR NAME{
		load_meta_data();
		std::string schemaTxt = schema_meta_data[*$3];
		Schema *schema = parseSchema(&schemaTxt[0]);
		int ret = Table_Open(*$3 + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		printAllRows(tbl, schema, printRow, NULL);
		Table_Close(tbl);
	}
	| DUMP column_list NAME {
		load_meta_data();
		std::string schemaTxt = schema_meta_data[*$3];
		Schema *schema = parseSchema(&schemaTxt[0]);
		int ret = Table_Open(*$3 + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		printAllRows(tbl, schema, printRow, $2);
		Table_Close(tbl);
	}
	| DUMP STAR NAME WHERE condition {
		load_meta_data();
		std::string schemaTxt = schema_meta_data[*$3];

		Schema *schema = parseSchema(&schemaTxt[0]);
		std::cout << schemaTxt << " " << *$3 + ".db" << std::endl;

		int ret = Table_Open(*$3 + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		std::string index_name = *$3 + ".db.0";
		char *index_name_c = new char[index_name.length() + 1];
		strcpy(index_name_c, index_name.c_str());
		int indexFD = PF_OpenFile(index_name_c);

		index_scan(tbl, schema, indexFD, *($5->op), *($5->num));
	}
	| DUMP column_list NAME WHERE condition {
		load_meta_data();
		std::string schemaTxt = schema_meta_data[*$3];
		Schema *schema = parseSchema(&schemaTxt[0]);
		int ret = Table_Open(*$3 + ".db", schema, false, &tbl);
		if(ret < 0) {
			std::cout << "Result not available";
		}
		std::string index_name = *$3 + ".db.0";
		char *index_name_c = new char[index_name.length() + 1];
		strcpy(index_name_c, index_name.c_str());
		int indexFD = PF_OpenFile(index_name_c);
		index_scan(tbl, schema, indexFD, *($5->op), *($5->num));
	}

// column_data_type_list : NAME

column_list : NAME {
		$$ = new std::vector<std::string>;
		$$->push_back(*($1));
	}
	| column_list COMMA NAME {
		$$ = $1;
		$$->push_back(*($3));
	}


condition : EQ NUM {
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

int
main(int argc, char **argv) {

	yyset_in(stdin);
	yyset_out(stdout);


	std::cout << "Welcome. Type `help` for help." << std::endl;

	// Schema and index meta data is in file meta_data.db
	load_meta_data();
	std::string input;
	while(true) {
		std::cout << std::endl << ">";
		yyparse();
		if(stopFlag)
			break;	
	}
	Table_Close(tbl);
}

int yyerror(std::string msg) {
	fprintf(stderr, "%s\n", msg.c_str());
	exit(1);
}