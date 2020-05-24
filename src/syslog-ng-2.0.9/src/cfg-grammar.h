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
     KW_SOURCE = 258,
     KW_DESTINATION = 259,
     KW_LOG = 260,
     KW_OPTIONS = 261,
     KW_FILTER = 262,
     KW_INTERNAL = 263,
     KW_FILE = 264,
     KW_PIPE = 265,
     KW_UNIX_STREAM = 266,
     KW_UNIX_DGRAM = 267,
     KW_TCP = 268,
     KW_UDP = 269,
     KW_TCP6 = 270,
     KW_UDP6 = 271,
     KW_USER = 272,
     KW_DOOR = 273,
     KW_SUN_STREAMS = 274,
     KW_PROGRAM = 275,
     KW_FSYNC = 276,
     KW_MARK_FREQ = 277,
     KW_STATS_FREQ = 278,
     KW_FLUSH_LINES = 279,
     KW_FLUSH_TIMEOUT = 280,
     KW_LOG_MSG_SIZE = 281,
     KW_FILE_TEMPLATE = 282,
     KW_PROTO_TEMPLATE = 283,
     KW_CHAIN_HOSTNAMES = 284,
     KW_NORMALIZE_HOSTNAMES = 285,
     KW_KEEP_HOSTNAME = 286,
     KW_CHECK_HOSTNAME = 287,
     KW_BAD_HOSTNAME = 288,
     KW_KEEP_TIMESTAMP = 289,
     KW_USE_DNS = 290,
     KW_USE_FQDN = 291,
     KW_DNS_CACHE = 292,
     KW_DNS_CACHE_SIZE = 293,
     KW_DNS_CACHE_EXPIRE = 294,
     KW_DNS_CACHE_EXPIRE_FAILED = 295,
     KW_DNS_CACHE_HOSTS = 296,
     KW_PERSIST_ONLY = 297,
     KW_TZ_CONVERT = 298,
     KW_TS_FORMAT = 299,
     KW_FRAC_DIGITS = 300,
     KW_LOG_FIFO_SIZE = 301,
     KW_LOG_FETCH_LIMIT = 302,
     KW_LOG_IW_SIZE = 303,
     KW_LOG_PREFIX = 304,
     KW_FLAGS = 305,
     KW_CATCHALL = 306,
     KW_FALLBACK = 307,
     KW_FINAL = 308,
     KW_FLOW_CONTROL = 309,
     KW_PAD_SIZE = 310,
     KW_TIME_ZONE = 311,
     KW_RECV_TIME_ZONE = 312,
     KW_SEND_TIME_ZONE = 313,
     KW_TIME_REOPEN = 314,
     KW_TIME_REAP = 315,
     KW_TIME_SLEEP = 316,
     KW_TMPL_ESCAPE = 317,
     KW_OPTIONAL = 318,
     KW_CREATE_DIRS = 319,
     KW_OWNER = 320,
     KW_GROUP = 321,
     KW_PERM = 322,
     KW_DIR_OWNER = 323,
     KW_DIR_GROUP = 324,
     KW_DIR_PERM = 325,
     KW_TEMPLATE = 326,
     KW_TEMPLATE_ESCAPE = 327,
     KW_FOLLOW_FREQ = 328,
     KW_OVERWRITE_IF_OLDER = 329,
     KW_KEEP_ALIVE = 330,
     KW_MAX_CONNECTIONS = 331,
     KW_LOCALIP = 332,
     KW_IP = 333,
     KW_LOCALPORT = 334,
     KW_PORT = 335,
     KW_DESTPORT = 336,
     KW_IP_TTL = 337,
     KW_SO_BROADCAST = 338,
     KW_IP_TOS = 339,
     KW_SO_SNDBUF = 340,
     KW_SO_RCVBUF = 341,
     KW_SO_KEEPALIVE = 342,
     KW_SPOOF_SOURCE = 343,
     KW_USE_TIME_RECVD = 344,
     KW_FACILITY = 345,
     KW_LEVEL = 346,
     KW_HOST = 347,
     KW_MATCH = 348,
     KW_NETMASK = 349,
     KW_YES = 350,
     KW_NO = 351,
     KW_REQUIRED = 352,
     KW_ALLOW = 353,
     KW_DENY = 354,
     KW_GC_IDLE_THRESHOLD = 355,
     KW_GC_BUSY_THRESHOLD = 356,
     KW_COMPRESS = 357,
     KW_MAC = 358,
     KW_AUTH = 359,
     KW_ENCRYPT = 360,
     DOTDOT = 361,
     IDENTIFIER = 362,
     NUMBER = 363,
     STRING = 364,
     KW_OR = 365,
     KW_AND = 366,
     KW_NOT = 367
   };
#endif
/* Tokens.  */
#define KW_SOURCE 258
#define KW_DESTINATION 259
#define KW_LOG 260
#define KW_OPTIONS 261
#define KW_FILTER 262
#define KW_INTERNAL 263
#define KW_FILE 264
#define KW_PIPE 265
#define KW_UNIX_STREAM 266
#define KW_UNIX_DGRAM 267
#define KW_TCP 268
#define KW_UDP 269
#define KW_TCP6 270
#define KW_UDP6 271
#define KW_USER 272
#define KW_DOOR 273
#define KW_SUN_STREAMS 274
#define KW_PROGRAM 275
#define KW_FSYNC 276
#define KW_MARK_FREQ 277
#define KW_STATS_FREQ 278
#define KW_FLUSH_LINES 279
#define KW_FLUSH_TIMEOUT 280
#define KW_LOG_MSG_SIZE 281
#define KW_FILE_TEMPLATE 282
#define KW_PROTO_TEMPLATE 283
#define KW_CHAIN_HOSTNAMES 284
#define KW_NORMALIZE_HOSTNAMES 285
#define KW_KEEP_HOSTNAME 286
#define KW_CHECK_HOSTNAME 287
#define KW_BAD_HOSTNAME 288
#define KW_KEEP_TIMESTAMP 289
#define KW_USE_DNS 290
#define KW_USE_FQDN 291
#define KW_DNS_CACHE 292
#define KW_DNS_CACHE_SIZE 293
#define KW_DNS_CACHE_EXPIRE 294
#define KW_DNS_CACHE_EXPIRE_FAILED 295
#define KW_DNS_CACHE_HOSTS 296
#define KW_PERSIST_ONLY 297
#define KW_TZ_CONVERT 298
#define KW_TS_FORMAT 299
#define KW_FRAC_DIGITS 300
#define KW_LOG_FIFO_SIZE 301
#define KW_LOG_FETCH_LIMIT 302
#define KW_LOG_IW_SIZE 303
#define KW_LOG_PREFIX 304
#define KW_FLAGS 305
#define KW_CATCHALL 306
#define KW_FALLBACK 307
#define KW_FINAL 308
#define KW_FLOW_CONTROL 309
#define KW_PAD_SIZE 310
#define KW_TIME_ZONE 311
#define KW_RECV_TIME_ZONE 312
#define KW_SEND_TIME_ZONE 313
#define KW_TIME_REOPEN 314
#define KW_TIME_REAP 315
#define KW_TIME_SLEEP 316
#define KW_TMPL_ESCAPE 317
#define KW_OPTIONAL 318
#define KW_CREATE_DIRS 319
#define KW_OWNER 320
#define KW_GROUP 321
#define KW_PERM 322
#define KW_DIR_OWNER 323
#define KW_DIR_GROUP 324
#define KW_DIR_PERM 325
#define KW_TEMPLATE 326
#define KW_TEMPLATE_ESCAPE 327
#define KW_FOLLOW_FREQ 328
#define KW_OVERWRITE_IF_OLDER 329
#define KW_KEEP_ALIVE 330
#define KW_MAX_CONNECTIONS 331
#define KW_LOCALIP 332
#define KW_IP 333
#define KW_LOCALPORT 334
#define KW_PORT 335
#define KW_DESTPORT 336
#define KW_IP_TTL 337
#define KW_SO_BROADCAST 338
#define KW_IP_TOS 339
#define KW_SO_SNDBUF 340
#define KW_SO_RCVBUF 341
#define KW_SO_KEEPALIVE 342
#define KW_SPOOF_SOURCE 343
#define KW_USE_TIME_RECVD 344
#define KW_FACILITY 345
#define KW_LEVEL 346
#define KW_HOST 347
#define KW_MATCH 348
#define KW_NETMASK 349
#define KW_YES 350
#define KW_NO 351
#define KW_REQUIRED 352
#define KW_ALLOW 353
#define KW_DENY 354
#define KW_GC_IDLE_THRESHOLD 355
#define KW_GC_BUSY_THRESHOLD 356
#define KW_COMPRESS 357
#define KW_MAC 358
#define KW_AUTH 359
#define KW_ENCRYPT 360
#define DOTDOT 361
#define IDENTIFIER 362
#define NUMBER 363
#define STRING 364
#define KW_OR 365
#define KW_AND 366
#define KW_NOT 367




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 49 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
{
	guint num;
	char *cptr;
	void *ptr;
	FilterExprNode *node;
}
/* Line 1489 of yacc.c.  */
#line 280 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

