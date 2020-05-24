/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

#ifndef YY_CLIENT6_CLIENT__PARSE_H_INCLUDED
# define YY_CLIENT6_CLIENT__PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int client6debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INTERFACE = 258,
    IFNAME = 259,
    IPV6ADDR = 260,
    REQUEST = 261,
    SEND = 262,
    RAPID_COMMIT = 263,
    PREFIX_DELEGATION = 264,
    DNS_SERVERS = 265,
    DOMAIN_LIST = 266,
    INFO_ONLY = 267,
    TEMP_ADDR = 268,
    ADDRESS = 269,
    PREFIX = 270,
    IAID = 271,
    RENEW_TIME = 272,
    REBIND_TIME = 273,
    V_TIME = 274,
    P_TIME = 275,
    PREFIX_DELEGATION_INTERFACE = 276,
    DEFAULT_IRT = 277,
    MAXIMUM_IRT = 278,
    NUMBER = 279,
    SLASH = 280,
    EOS = 281,
    BCL = 282,
    ECL = 283,
    STRING = 284,
    INFINITY = 285,
    COMMA = 286,
    OPTION = 287
  };
#endif
/* Tokens.  */
#define INTERFACE 258
#define IFNAME 259
#define IPV6ADDR 260
#define REQUEST 261
#define SEND 262
#define RAPID_COMMIT 263
#define PREFIX_DELEGATION 264
#define DNS_SERVERS 265
#define DOMAIN_LIST 266
#define INFO_ONLY 267
#define TEMP_ADDR 268
#define ADDRESS 269
#define PREFIX 270
#define IAID 271
#define RENEW_TIME 272
#define REBIND_TIME 273
#define V_TIME 274
#define P_TIME 275
#define PREFIX_DELEGATION_INTERFACE 276
#define DEFAULT_IRT 277
#define MAXIMUM_IRT 278
#define NUMBER 279
#define SLASH 280
#define EOS 281
#define BCL 282
#define ECL 283
#define STRING 284
#define INFINITY 285
#define COMMA 286
#define OPTION 287

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 107 "client6_parse.y" /* yacc.c:1909  */

	long long num;
	char* str;
	struct cf_list *list;
	struct in6_addr addr;
	struct dhcp6_addr *v6addr;

#line 126 "client6_parse.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE client6lval;

int client6parse (void);

#endif /* !YY_CLIENT6_CLIENT__PARSE_H_INCLUDED  */
