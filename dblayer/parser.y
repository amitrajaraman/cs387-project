%{
	#include "common_headers.hh"

	std::string schemaTxt = "Country:varchar,Capital:varchar,Population:int";
	Schema *schema = parseSchema(&schemaTxt[0]);
	Table *tbl;

	int yylex();
	int yyerror(std::string);
	void yyset_in (FILE * _in_str );
	void yyset_out (FILE * _out_str);
	extern int counting;
	int stopFlag = 0;
	struct tokens {
		std::string token_name;
		std::string lexeme;
		int lineno;
	};
	extern struct tokens token_table[1000];
%}
%union {
	std::string *name;
	std::vector<std::string> *colList;
}
%token DUMP STAR WHERE QUIT HELP LT GT LEQ GEQ EQ NEQ COMMA CREATE TABLE FILE_KEYWORD INDEX
%token <name> NUM
%token <name> NAME
%token <name> FILE_NAME

%type <colList> column_list

%%

program : QUIT {
		stopFlag = 1;
	}
	| HELP {
		std::cout << "Type 'dump all' to dump all data, 'dump all where [eq/lt/gt/leq/geq/neq] num' to print all rows with population satisfying the given constraint, and 'quit' to quit." << std::endl;
	}
	| CREATE TABLE FILE_KEYWORD FILE_NAME INDEX NUM {
		loadCSV(*$4, stoi(*$6));
		std::cout << "Created database!\n";
	}
	| DUMP STAR {
		printAllRows(tbl, schema, printRow, NULL);
	}
	| DUMP column_list {
		printAllRows(tbl, schema, printRow, $2);
	}
	| DUMP STAR WHERE condition_list
	| DUMP column_list WHERE condition_list

// column_data_type_list : NAME

column_list : NAME {
		$$ = new std::vector<std::string>;
		$$->push_back(*($1));
	}
	| column_list COMMA NAME {
		$$ = $1;
		$$->push_back(*($3));
	}

condition_list : condition

condition : EQ NUM {
		index_scan(tbl, schema, tbl->indexFd, 1, stoi(*($2)));
	}
	| LT NUM {
		index_scan(tbl, schema, tbl->indexFd, 2, stoi(*($2)));
	}
	| GT NUM {
		index_scan(tbl, schema, tbl->indexFd, 3, stoi(*($2)));
	}
	| LEQ NUM {
		index_scan(tbl, schema, tbl->indexFd, 4, stoi(*($2)));
	}
	| GEQ NUM {
		index_scan(tbl, schema, tbl->indexFd, 5, stoi(*($2)));
	}
	| NEQ NUM {
		index_scan(tbl, schema, tbl->indexFd, 6, stoi(*($2)));
	}

%% 

int
main(int argc, char **argv) {

	yyset_in(stdin);
	yyset_out(stdout);


	std::cout << "Welcome. Type `help` for help." << std::endl;

	std::string input;
	while(true) {
		std::cout << std::endl;
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