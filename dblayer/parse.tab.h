/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DUMP = 258,
     STAR = 259,
     WHERE = 260,
     QUIT = 261,
     HELP = 262,
     LT = 263,
     GT = 264,
     LEQ = 265,
     GEQ = 266,
     EQ = 267,
     NEQ = 268,
     COMMA = 269,
     CREATE = 270,
     TABLE = 271,
     FILE_KEYWORD = 272,
     INDEX = 273,
     NUM = 274,
     NAME = 275,
     FILE_NAME = 276
   };
#endif
/* Tokens.  */
#define DUMP 258
#define STAR 259
#define WHERE 260
#define QUIT 261
#define HELP 262
#define LT 263
#define GT 264
#define LEQ 265
#define GEQ 266
#define EQ 267
#define NEQ 268
#define COMMA 269
#define CREATE 270
#define TABLE 271
#define FILE_KEYWORD 272
#define INDEX 273
#define NUM 274
#define NAME 275
#define FILE_NAME 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 23 "parser.y"
{
	std::string *name;
	std::vector<std::string> *colList;
}
/* Line 1529 of yacc.c.  */
#line 96 "parse.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

