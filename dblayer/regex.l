%{
	#include "common_headers.hh"
	#include "parse.tab.h"
	struct tokens {
		std::string token_name;
		std::string lexeme;
		int lineno;
	} token_table[1000];
	int counting = 0;
	void add(std::string t, std::string l, int ln);
%}
%option noyywrap

digit [0-9]
alpha [a-zA-Z]

%%

{digit}+ {
	add("NUM", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return NUM;
}

"dump" {
	add("DUMP", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return DUMP;
}
"create" {
	add("CREATE", std::string(yytext), yylineno);
	return CREATE;
}

"all" {
	add("STAR", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return STAR;
}

"where" {
	add("WHERE", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return WHERE;
}

"exit"|"quit" {
	add("QUIT", std::string(yytext), yylineno);
	return QUIT;
}

"LESS_THAN"|"lt" {
	add("LT", std::string(yytext), yylineno);
	return LT;
}

"GREATER_THAN"|"gt" {
	add("GT", std::string(yytext), yylineno);
	return GT;
}

"GREATER_THAN_EQUAL"|"geq" {
	add("GEQ", std::string(yytext), yylineno);
	return GEQ;
}

"LESS_THAN_EQUAL"|"leq" {
	add("LEQ", std::string(yytext), yylineno);
	return LEQ;
}

"EQUAL"|"eq" {
	add("EQ", std::string(yytext), yylineno);
	return EQ;
}

"NOT_EQUAL"|"neq" {
	add("NEQ", std::string(yytext), yylineno);
	return NEQ;
}

"help" {
	add("HELP", std::string(yytext), yylineno);
	return HELP;
}

"table" {
	add("TABLE", std::string(yytext), yylineno);
	return TABLE;
}

"index" {
	add("INDEX", std::string(yytext), yylineno);
	return INDEX;
}

"file" {
	add("FILE_KEYWORD", std::string(yytext), yylineno);
	return FILE_KEYWORD;
}

"," {
	add("COMMA", std::string(yytext), yylineno);
	return COMMA;
}

{alpha}({alpha}|{digit})* {
	add("NAME", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return NAME;
}

[\n] {
	return -1;
}

{alpha}+\.{alpha}+ {
	add("FILE_NAME", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return FILE_NAME;
}

%%


void add(std::string t, std::string l, int ln) {
	token_table[counting].token_name = t;
	token_table[counting].lexeme = l;
	token_table[counting].lineno = ln;
	// std::cout << t << " " << l << std::endl;
	counting++;
}