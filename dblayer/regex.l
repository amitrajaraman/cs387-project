%{
	#include "common_headers.hh"
	#include "parse.tab.h"
	#undef YY_INPUT
	#define YY_INPUT(b,r,s) readInputForLexer(b,&r,s)
	tokens token_table[1000];
	int counting = 0;
	void add(std::string t, std::string l, int ln);
%}

digit [0-9]
alpha [a-zA-Z]
%option noyywrap

%%

"dump" {
	add("DUMP", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return DUMP;
}
"add" {
	add("ADD", std::string(yytext), yylineno);
	return ADD;
}
"as" {
	add("AS", std::string(yytext), yylineno);
	return AS;
}
"constraint" {
	add("CONSTRAINT", std::string(yytext), yylineno);
	return CONSTRAINT;
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

"git"|"github" {
	add("GIT", std::string(yytext), yylineno);
	return GIT;
}

"insert" {
	add("INSERT", std::string(yytext), yylineno);
	return INSERT;
}

"into" {
	add("INTO", std::string(yytext), yylineno);
	return INTO;
}

"(" {
	add("LEFT_PAR", std::string(yytext), yylineno);
	return LEFT_PAR;
}

")" {
	add("RIGHT_PAR", std::string(yytext), yylineno);
	return RIGHT_PAR;
}

";" {
	add("SEMICOLON", std::string(yytext), yylineno);
	return SEMICOLON;
}


{alpha}({alpha}|{digit})* {
	add("NAME", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return NAME;
}

{digit}+ {
	add("NUM", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return NUM;
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