%{
	#include <string>
	#include <iostream>
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
	yylval.name = new std::string(yytext);
	return QUIT;
}

"LESS_THAN"|"lt" {
	add("LT", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return LT;
}

"GREATER_THAN"|"gt" {
	add("GT", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return GT;
}

"GREATER_THAN_EQUAL"|"geq" {
	add("GEQ", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return GEQ;
}

"LESS_THAN_EQUAL"|"leq" {
	add("LEQ", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return LEQ;
}

"EQUAL"|"eq" {
	add("EQ", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return EQ;
}

"NOT_EQUAL"|"neq" {
	add("NEQ", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return NEQ;
}

"help" {
	add("HELP", std::string(yytext), yylineno);
	yylval.name = new std::string(yytext);
	return HELP;
}

[\n] {
	return -1;
}

%%

void add(std::string t, std::string l, int ln) {
	token_table[counting].token_name = t;
	token_table[counting].lexeme = l;
	token_table[counting].lineno = ln;
	counting++;
}