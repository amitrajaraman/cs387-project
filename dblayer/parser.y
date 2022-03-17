%{
	#include "dumpdb.c"

	#define DB_NAME "data.db"
	#define INDEX_NAME "data.db.0"

	std::string schemaTxt = "Country:varchar,Capital:varchar,Population:int";
	Schema *schema = parseSchema(&schemaTxt[0]);
	Table *tbl;
	int indexFD;

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
	std::string* name;
}
%token DUMP STAR WHERE QUIT HELP LT GT LEQ GEQ EQ NEQ
%token <name> NUM


%%

program : QUIT {
		stopFlag = 1;
	}
	| HELP {
		std::cout << "Type 'dump all' to dump all data, 'dump all where [eq/lt/gt/leq/geq/neq] num' to print all rows with population satisfying the given constraint, and 'quit' to quit." << std::endl;
	}
	| DUMP STAR {
		Table_Scan(tbl, schema, printRow);
	}
	| DUMP STAR WHERE condition_list

condition_list : condition

condition : EQ NUM {
		index_scan(tbl, schema, indexFD, 1, stoi(*($2)));
	}
	| LT NUM {
		index_scan(tbl, schema, indexFD, 2, stoi(*($2)));
	}
	| GT NUM {
		index_scan(tbl, schema, indexFD, 3, stoi(*($2)));
	}
	| LEQ NUM {
		index_scan(tbl, schema, indexFD, 4, stoi(*($2)));
	}
	| GEQ NUM {
		index_scan(tbl, schema, indexFD, 5, stoi(*($2)));
	}
	| NEQ NUM {
		index_scan(tbl, schema, indexFD, 6, stoi(*($2)));
	}

%% 

int
main(int argc, char **argv) {

	yyset_in(stdin);
	yyset_out(stdout);


	Table_Open(DB_NAME, schema, false, &tbl);   // Open the database and load table into memory
	
	indexFD = PF_OpenFile(INDEX_NAME);

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