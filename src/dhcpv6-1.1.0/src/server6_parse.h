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

#ifndef YY_SERVER6_SERVER__PARSE_H_INCLUDED
# define YY_SERVER6_SERVER__PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int server6debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INTERFACE = 258,
    IFNAME = 259,
    PREFIX = 260,
    LINK = 261,
    RELAY = 262,
    STRING = 263,
    NUMBER = 264,
    SIGNEDNUMBER = 265,
    DECIMAL = 266,
    BOOLEAN = 267,
    IPV6ADDR = 268,
    INFINITY = 269,
    HOST = 270,
    POOL = 271,
    RANGE = 272,
    GROUP = 273,
    LINKLOCAL = 274,
    OPTION = 275,
    ALLOW = 276,
    SEND = 277,
    PREFERENCE = 278,
    RENEWTIME = 279,
    REBINDTIME = 280,
    RAPIDCOMMIT = 281,
    ADDRESS = 282,
    VALIDLIFETIME = 283,
    PREFERLIFETIME = 284,
    UNICAST = 285,
    TEMPIPV6ADDR = 286,
    DNS_SERVERS = 287,
    DUID = 288,
    DUID_ID = 289,
    IAID = 290,
    IAIDINFO = 291,
    INFO_ONLY = 292,
    INFO_REFRESH_TIME = 293,
    TO = 294,
    BAD_TOKEN = 295
  };
#endif
/* Tokens.  */
#define INTERFACE 258
#define IFNAME 259
#define PREFIX 260
#define LINK 261
#define RELAY 262
#define STRING 263
#define NUMBER 264
#define SIGNEDNUMBER 265
#define DECIMAL 266
#define BOOLEAN 267
#define IPV6ADDR 268
#define INFINITY 269
#define HOST 270
#define POOL 271
#define RANGE 272
#define GROUP 273
#define LINKLOCAL 274
#define OPTION 275
#define ALLOW 276
#define SEND 277
#define PREFERENCE 278
#define RENEWTIME 279
#define REBINDTIME 280
#define RAPIDCOMMIT 281
#define ADDRESS 282
#define VALIDLIFETIME 283
#define PREFERLIFETIME 284
#define UNICAST 285
#define TEMPIPV6ADDR 286
#define DNS_SERVERS 287
#define DUID 288
#define DUID_ID 289
#define IAID 290
#define IAIDINFO 291
#define INFO_ONLY 292
#define INFO_REFRESH_TIME 293
#define TO 294
#define BAD_TOKEN 295

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 114 "server6_parse.y" /* yacc.c:1909  */

	unsigned int	num;
	int 	snum;
	char	*str;
	int 	dec;
	int	bool;
	struct in6_addr	addr;
	struct dhcp6_addr *dhcp6addr;

#line 144 "server6_parse.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE server6lval;

int server6parse (void);

#endif /* !YY_SERVER6_SERVER__PARSE_H_INCLUDED  */
