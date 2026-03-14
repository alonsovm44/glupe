/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
     T_RAW_CODE = 258,
     T_IDENTIFIER = 259,
     T_VAR_VALUE = 260,
     T_INTENT_TEXT = 261,
     T_STRING_LITERAL = 262,
     T_NUMBER = 263,
     T_PERSISTENT_VAR_START = 264,
     T_CONST_VAR_START = 265,
     T_EPHEMERAL_VAR_START = 266,
     T_BLOCK_START = 267,
     T_INLINE_START = 268,
     T_ABSTRACT = 269,
     T_ARROW = 270,
     T_COMMA = 271,
     T_LPAREN = 272,
     T_RPAREN = 273,
     T_LBRACE = 274,
     T_BLOCK_END = 275,
     T_INLINE_END = 276,
     T_NEWLINE = 277,
     T_PLUS = 278,
     T_MINUS = 279,
     T_STAR = 280,
     T_SLASH = 281,
     T_CARET = 282
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 16 "src/glupe.y"

    std::string* str;
    ASTNode* node;
    ProgramNode* prog;
    std::vector<std::string>* str_list;
    bool boolean;



/* Line 1685 of yacc.c  */
#line 88 "src/glupe.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


