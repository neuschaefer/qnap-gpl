/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 1 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"


#include "syslog-ng.h"
#include "cfg.h"
#include "sgroup.h"
#include "dgroup.h"
#include "center.h"
#include "filter.h"
#include "templates.h"
#include "logreader.h"

#include "affile.h"
#include "afinter.h"
#include "afsocket.h"
#include "afinet.h"
#include "afunix.h"
#include "afstreams.h"
#include "afuser.h"
#include "afprog.h"

#include "messages.h"

#include "syslog-names.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int lookup_parse_flag(char *flag);

void yyerror(char *msg);
int yylex();

LogDriver *last_driver;
LogReaderOptions *last_reader_options;
LogWriterOptions *last_writer_options;
LogTemplate *last_template;
SocketOptions *last_sock_options;
gint last_addr_family = AF_INET;

#if ! ENABLE_IPV6
#undef AF_INET6
#define AF_INET6 0; g_assert_not_reached()

#endif



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 49 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
{
	guint num;
	char *cptr;
	void *ptr;
	FilterExprNode *node;
}
/* Line 187 of yacc.c.  */
#line 375 "cfg-grammar.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 388 "cfg-grammar.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   758

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  110
/* YYNRULES -- Number of rules.  */
#define YYNRULES  278
/* YYNRULES -- Number of states.  */
#define YYNSTATES  721

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   367

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     116,   117,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   113,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   114,     2,   115,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    10,    13,    16,    19,    22,
      25,    28,    33,    38,    43,    47,    48,    54,    58,    59,
      64,    69,    74,    79,    84,    89,    91,    96,   101,   105,
     106,   108,   110,   112,   114,   118,   123,   128,   129,   133,
     134,   138,   143,   145,   150,   155,   156,   162,   163,   169,
     170,   176,   177,   183,   184,   188,   189,   193,   196,   197,
     202,   207,   212,   217,   219,   221,   223,   224,   227,   230,
     231,   233,   238,   243,   248,   253,   255,   257,   258,   261,
     264,   265,   267,   272,   277,   279,   284,   289,   294,   295,
     299,   302,   303,   308,   311,   312,   317,   322,   327,   332,
     337,   342,   347,   352,   357,   360,   361,   365,   366,   368,
     370,   372,   374,   376,   381,   382,   386,   389,   390,   392,
     397,   402,   407,   412,   417,   422,   427,   432,   437,   442,
     447,   448,   452,   455,   456,   458,   463,   468,   473,   478,
     483,   484,   490,   491,   497,   498,   504,   505,   511,   512,
     516,   517,   521,   524,   525,   527,   529,   530,   534,   537,
     538,   543,   545,   547,   549,   554,   559,   564,   569,   570,
     574,   577,   578,   580,   585,   590,   595,   600,   605,   606,
     610,   613,   614,   619,   624,   629,   634,   639,   644,   649,
     654,   659,   662,   663,   665,   669,   670,   675,   680,   685,
     691,   692,   695,   696,   698,   700,   702,   704,   708,   709,
     714,   719,   724,   729,   734,   739,   744,   749,   754,   759,
     764,   769,   774,   779,   784,   789,   794,   799,   804,   809,
     814,   819,   824,   829,   834,   839,   844,   849,   854,   859,
     864,   869,   874,   879,   884,   889,   894,   899,   904,   909,
     915,   917,   920,   924,   928,   932,   937,   942,   947,   952,
     957,   962,   967,   972,   975,   977,   979,   982,   984,   988,
     990,   992,   994,   996,   998,  1000,  1002,  1004,  1006
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     119,     0,    -1,   120,    -1,   121,   113,   120,    -1,    -1,
       3,   122,    -1,     4,   123,    -1,     5,   124,    -1,     7,
     217,    -1,    71,   126,    -1,     6,   125,    -1,   226,   114,
     132,   115,    -1,   226,   114,   170,   115,    -1,   114,   210,
     212,   115,    -1,   114,   215,   115,    -1,    -1,   226,   127,
     114,   128,   115,    -1,   129,   113,   128,    -1,    -1,    71,
     116,   226,   117,    -1,    72,   116,   224,   117,    -1,    85,
     116,   108,   117,    -1,    86,   116,   108,   117,    -1,    83,
     116,   224,   117,    -1,    87,   116,   224,   117,    -1,   130,
      -1,    82,   116,   108,   117,    -1,    84,   116,   108,   117,
      -1,   133,   113,   132,    -1,    -1,   134,    -1,   135,    -1,
     141,    -1,   162,    -1,     8,   116,   117,    -1,     9,   116,
     136,   117,    -1,    10,   116,   138,   117,    -1,    -1,   226,
     137,   167,    -1,    -1,   226,   139,   140,    -1,    63,   116,
     224,   117,    -1,   167,    -1,    12,   116,   146,   117,    -1,
      11,   116,   148,   117,    -1,    -1,    14,   142,   116,   152,
     117,    -1,    -1,    13,   143,   116,   157,   117,    -1,    -1,
      16,   144,   116,   152,   117,    -1,    -1,    15,   145,   116,
     157,   117,    -1,    -1,   226,   147,   150,    -1,    -1,   226,
     149,   150,    -1,   151,   150,    -1,    -1,    65,   116,   227,
     117,    -1,    66,   116,   227,   117,    -1,    67,   116,   108,
     117,    -1,    63,   116,   224,   117,    -1,   161,    -1,   168,
      -1,   130,    -1,    -1,   153,   154,    -1,   155,   154,    -1,
      -1,   156,    -1,    79,   116,   227,   117,    -1,    80,   116,
     227,   117,    -1,    77,   116,   226,   117,    -1,    78,   116,
     226,   117,    -1,   168,    -1,   131,    -1,    -1,   158,   159,
      -1,   160,   159,    -1,    -1,   156,    -1,    79,   116,   227,
     117,    -1,    80,   116,   227,   117,    -1,   161,    -1,    75,
     116,   224,   117,    -1,    76,   116,   108,   117,    -1,    19,
     116,   163,   117,    -1,    -1,   226,   164,   165,    -1,   166,
     165,    -1,    -1,    18,   116,   226,   117,    -1,   168,   167,
      -1,    -1,    50,   116,   169,   117,    -1,    26,   116,   108,
     117,    -1,    48,   116,   108,   117,    -1,    47,   116,   108,
     117,    -1,    49,   116,   226,   117,    -1,    55,   116,   108,
     117,    -1,    73,   116,   108,   117,    -1,    56,   116,   226,
     117,    -1,    34,   116,   224,   117,    -1,   107,   169,    -1,
      -1,   171,   113,   170,    -1,    -1,   172,    -1,   177,    -1,
     182,    -1,   202,    -1,   203,    -1,     9,   116,   173,   117,
      -1,    -1,   226,   174,   175,    -1,   176,   175,    -1,    -1,
     207,    -1,    63,   116,   224,   117,    -1,    65,   116,   227,
     117,    -1,    66,   116,   227,   117,    -1,    67,   116,   108,
     117,    -1,    68,   116,   227,   117,    -1,    69,   116,   227,
     117,    -1,    70,   116,   108,   117,    -1,    64,   116,   224,
     117,    -1,    74,   116,   108,   117,    -1,    21,   116,   224,
     117,    -1,    10,   116,   178,   117,    -1,    -1,   226,   179,
     180,    -1,   181,   180,    -1,    -1,   207,    -1,    65,   116,
     227,   117,    -1,    66,   116,   227,   117,    -1,    67,   116,
     108,   117,    -1,    12,   116,   187,   117,    -1,    11,   116,
     189,   117,    -1,    -1,    14,   183,   116,   193,   117,    -1,
      -1,    13,   184,   116,   198,   117,    -1,    -1,    16,   185,
     116,   193,   117,    -1,    -1,    15,   186,   116,   198,   117,
      -1,    -1,   226,   188,   191,    -1,    -1,   226,   190,   191,
      -1,   191,   192,    -1,    -1,   207,    -1,   130,    -1,    -1,
     226,   194,   195,    -1,   195,   197,    -1,    -1,    77,   116,
     226,   117,    -1,   131,    -1,   207,    -1,   196,    -1,    79,
     116,   227,   117,    -1,    80,   116,   227,   117,    -1,    81,
     116,   227,   117,    -1,    88,   116,   224,   117,    -1,    -1,
     226,   199,   200,    -1,   200,   201,    -1,    -1,   196,    -1,
      79,   116,   227,   117,    -1,    80,   116,   227,   117,    -1,
      81,   116,   227,   117,    -1,    17,   116,   226,   117,    -1,
      20,   116,   204,   117,    -1,    -1,   226,   205,   206,    -1,
     207,   206,    -1,    -1,    50,   116,   208,   117,    -1,    46,
     116,   108,   117,    -1,    24,   116,   108,   117,    -1,    25,
     116,   108,   117,    -1,    71,   116,   226,   117,    -1,    72,
     116,   224,   117,    -1,    56,   116,   226,   117,    -1,    44,
     116,   226,   117,    -1,    45,   116,   108,   117,    -1,   209,
     208,    -1,    -1,    62,    -1,   211,   113,   210,    -1,    -1,
       3,   116,   226,   117,    -1,     7,   116,   226,   117,    -1,
       4,   116,   226,   117,    -1,    50,   116,   213,   117,   113,
      -1,    -1,   214,   213,    -1,    -1,    51,    -1,    52,    -1,
      53,    -1,    54,    -1,   216,   113,   215,    -1,    -1,    22,
     116,   108,   117,    -1,    23,   116,   108,   117,    -1,    24,
     116,   108,   117,    -1,    25,   116,   108,   117,    -1,    29,
     116,   224,   117,    -1,    30,   116,   224,   117,    -1,    31,
     116,   224,   117,    -1,    32,   116,   224,   117,    -1,    33,
     116,   109,   117,    -1,    89,   116,   224,   117,    -1,    36,
     116,   224,   117,    -1,    35,   116,   225,   117,    -1,    59,
     116,   108,   117,    -1,    60,   116,   108,   117,    -1,    61,
     116,   108,   117,    -1,    46,   116,   108,   117,    -1,    48,
     116,   108,   117,    -1,    47,   116,   108,   117,    -1,    26,
     116,   108,   117,    -1,    34,   116,   224,   117,    -1,    44,
     116,   226,   117,    -1,    45,   116,   108,   117,    -1,   101,
     116,   108,   117,    -1,   100,   116,   108,   117,    -1,    64,
     116,   224,   117,    -1,    65,   116,   227,   117,    -1,    66,
     116,   227,   117,    -1,    67,   116,   108,   117,    -1,    68,
     116,   227,   117,    -1,    69,   116,   227,   117,    -1,    70,
     116,   108,   117,    -1,    37,   116,   224,   117,    -1,    38,
     116,   108,   117,    -1,    39,   116,   108,   117,    -1,    40,
     116,   108,   117,    -1,    41,   116,   226,   117,    -1,    27,
     116,   226,   117,    -1,    28,   116,   226,   117,    -1,    57,
     116,   226,   117,    -1,    58,   116,   226,   117,    -1,   226,
     114,   218,   113,   115,    -1,   219,    -1,   112,   218,    -1,
     218,   110,   218,    -1,   218,   111,   218,    -1,   116,   218,
     117,    -1,    90,   116,   220,   117,    -1,    90,   116,   108,
     117,    -1,    91,   116,   222,   117,    -1,    20,   116,   226,
     117,    -1,    92,   116,   226,   117,    -1,    93,   116,   226,
     117,    -1,     7,   116,   226,   117,    -1,    94,   116,   226,
     117,    -1,   221,   220,    -1,   221,    -1,   107,    -1,   223,
     222,    -1,   223,    -1,   107,   106,   107,    -1,   107,    -1,
      95,    -1,    96,    -1,   108,    -1,   224,    -1,    42,    -1,
     107,    -1,   109,    -1,   226,    -1,   108,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   197,   197,   201,   202,   206,   207,   208,   209,   210,
     211,   215,   219,   223,   227,   232,   231,   240,   241,   245,
     246,   250,   251,   252,   253,   257,   258,   259,   263,   264,
     268,   269,   270,   271,   275,   279,   280,   285,   284,   295,
     294,   304,   305,   309,   310,   311,   311,   312,   312,   313,
     313,   314,   314,   319,   318,   332,   331,   345,   346,   350,
     351,   352,   353,   354,   355,   356,   361,   361,   372,   373,
     377,   378,   379,   383,   384,   385,   386,   391,   391,   402,
     403,   407,   408,   409,   410,   414,   415,   419,   424,   423,
     432,   433,   437,   441,   442,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   458,   459,   464,   465,   469,   470,
     471,   472,   473,   477,   482,   481,   492,   493,   497,   498,
     503,   504,   505,   506,   507,   508,   509,   510,   511,   515,
     520,   519,   530,   531,   535,   536,   537,   538,   543,   544,
     545,   545,   546,   546,   547,   547,   548,   548,   553,   552,
     564,   563,   574,   575,   579,   580,   585,   584,   597,   598,
     603,   604,   605,   609,   610,   611,   612,   613,   618,   617,
     630,   631,   635,   636,   637,   638,   648,   652,   657,   656,
     666,   667,   671,   672,   673,   674,   675,   684,   685,   686,
     687,   691,   692,   696,   701,   702,   706,   707,   708,   712,
     713,   718,   719,   723,   724,   725,   726,   730,   731,   735,
     736,   737,   738,   739,   740,   741,   742,   743,   744,   745,
     746,   747,   748,   749,   758,   759,   760,   761,   762,   763,
     764,   765,   766,   767,   768,   769,   770,   771,   772,   773,
     774,   775,   776,   777,   779,   780,   781,   782,   783,   787,
     791,   792,   793,   794,   795,   799,   800,   801,   802,   803,
     804,   805,   806,   810,   811,   815,   832,   833,   837,   857,
     874,   875,   876,   880,   881,   885,   886,   890,   891
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "KW_SOURCE", "KW_DESTINATION", "KW_LOG",
  "KW_OPTIONS", "KW_FILTER", "KW_INTERNAL", "KW_FILE", "KW_PIPE",
  "KW_UNIX_STREAM", "KW_UNIX_DGRAM", "KW_TCP", "KW_UDP", "KW_TCP6",
  "KW_UDP6", "KW_USER", "KW_DOOR", "KW_SUN_STREAMS", "KW_PROGRAM",
  "KW_FSYNC", "KW_MARK_FREQ", "KW_STATS_FREQ", "KW_FLUSH_LINES",
  "KW_FLUSH_TIMEOUT", "KW_LOG_MSG_SIZE", "KW_FILE_TEMPLATE",
  "KW_PROTO_TEMPLATE", "KW_CHAIN_HOSTNAMES", "KW_NORMALIZE_HOSTNAMES",
  "KW_KEEP_HOSTNAME", "KW_CHECK_HOSTNAME", "KW_BAD_HOSTNAME",
  "KW_KEEP_TIMESTAMP", "KW_USE_DNS", "KW_USE_FQDN", "KW_DNS_CACHE",
  "KW_DNS_CACHE_SIZE", "KW_DNS_CACHE_EXPIRE", "KW_DNS_CACHE_EXPIRE_FAILED",
  "KW_DNS_CACHE_HOSTS", "KW_PERSIST_ONLY", "KW_TZ_CONVERT", "KW_TS_FORMAT",
  "KW_FRAC_DIGITS", "KW_LOG_FIFO_SIZE", "KW_LOG_FETCH_LIMIT",
  "KW_LOG_IW_SIZE", "KW_LOG_PREFIX", "KW_FLAGS", "KW_CATCHALL",
  "KW_FALLBACK", "KW_FINAL", "KW_FLOW_CONTROL", "KW_PAD_SIZE",
  "KW_TIME_ZONE", "KW_RECV_TIME_ZONE", "KW_SEND_TIME_ZONE",
  "KW_TIME_REOPEN", "KW_TIME_REAP", "KW_TIME_SLEEP", "KW_TMPL_ESCAPE",
  "KW_OPTIONAL", "KW_CREATE_DIRS", "KW_OWNER", "KW_GROUP", "KW_PERM",
  "KW_DIR_OWNER", "KW_DIR_GROUP", "KW_DIR_PERM", "KW_TEMPLATE",
  "KW_TEMPLATE_ESCAPE", "KW_FOLLOW_FREQ", "KW_OVERWRITE_IF_OLDER",
  "KW_KEEP_ALIVE", "KW_MAX_CONNECTIONS", "KW_LOCALIP", "KW_IP",
  "KW_LOCALPORT", "KW_PORT", "KW_DESTPORT", "KW_IP_TTL", "KW_SO_BROADCAST",
  "KW_IP_TOS", "KW_SO_SNDBUF", "KW_SO_RCVBUF", "KW_SO_KEEPALIVE",
  "KW_SPOOF_SOURCE", "KW_USE_TIME_RECVD", "KW_FACILITY", "KW_LEVEL",
  "KW_HOST", "KW_MATCH", "KW_NETMASK", "KW_YES", "KW_NO", "KW_REQUIRED",
  "KW_ALLOW", "KW_DENY", "KW_GC_IDLE_THRESHOLD", "KW_GC_BUSY_THRESHOLD",
  "KW_COMPRESS", "KW_MAC", "KW_AUTH", "KW_ENCRYPT", "DOTDOT", "IDENTIFIER",
  "NUMBER", "STRING", "KW_OR", "KW_AND", "KW_NOT", "';'", "'{'", "'}'",
  "'('", "')'", "$accept", "start", "stmts", "stmt", "source_stmt",
  "dest_stmt", "log_stmt", "options_stmt", "template_stmt", "@1",
  "template_items", "template_item", "socket_option", "inet_socket_option",
  "source_items", "source_item", "source_afinter", "source_affile",
  "source_affile_params", "@2", "source_afpipe_params", "@3",
  "source_afpipe_options", "source_afsocket", "@4", "@5", "@6", "@7",
  "source_afunix_dgram_params", "@8", "source_afunix_stream_params", "@9",
  "source_afunix_options", "source_afunix_option",
  "source_afinet_udp_params", "@10", "source_afinet_udp_options",
  "source_afinet_udp_option", "source_afinet_option",
  "source_afinet_tcp_params", "@11", "source_afinet_tcp_options",
  "source_afinet_tcp_option", "source_afsocket_stream_params",
  "source_afstreams", "source_afstreams_params", "@12",
  "source_afstreams_options", "source_afstreams_option",
  "source_reader_options", "source_reader_option",
  "source_reader_option_flags", "dest_items", "dest_item", "dest_affile",
  "dest_affile_params", "@13", "dest_affile_options", "dest_affile_option",
  "dest_afpipe", "dest_afpipe_params", "@14", "dest_afpipe_options",
  "dest_afpipe_option", "dest_afsocket", "@15", "@16", "@17", "@18",
  "dest_afunix_dgram_params", "@19", "dest_afunix_stream_params", "@20",
  "dest_afunix_options", "dest_afunix_option", "dest_afinet_udp_params",
  "@21", "dest_afinet_udp_options", "dest_afinet_option",
  "dest_afinet_udp_option", "dest_afinet_tcp_params", "@22",
  "dest_afinet_tcp_options", "dest_afinet_tcp_option", "dest_afuser",
  "dest_afprogram", "dest_afprogram_params", "@23", "dest_writer_options",
  "dest_writer_option", "dest_writer_options_flags",
  "dest_writer_options_flag", "log_items", "log_item", "log_flags",
  "log_flags_items", "log_flags_item", "options_items", "options_item",
  "filter_stmt", "filter_expr", "filter_simple_expr", "filter_fac_list",
  "filter_fac", "filter_level_list", "filter_level", "yesno", "dnsmode",
  "string", "string_or_number", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,    59,   123,   125,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   118,   119,   120,   120,   121,   121,   121,   121,   121,
     121,   122,   123,   124,   125,   127,   126,   128,   128,   129,
     129,   130,   130,   130,   130,   131,   131,   131,   132,   132,
     133,   133,   133,   133,   134,   135,   135,   137,   136,   139,
     138,   140,   140,   141,   141,   142,   141,   143,   141,   144,
     141,   145,   141,   147,   146,   149,   148,   150,   150,   151,
     151,   151,   151,   151,   151,   151,   153,   152,   154,   154,
     155,   155,   155,   156,   156,   156,   156,   158,   157,   159,
     159,   160,   160,   160,   160,   161,   161,   162,   164,   163,
     165,   165,   166,   167,   167,   168,   168,   168,   168,   168,
     168,   168,   168,   168,   169,   169,   170,   170,   171,   171,
     171,   171,   171,   172,   174,   173,   175,   175,   176,   176,
     176,   176,   176,   176,   176,   176,   176,   176,   176,   177,
     179,   178,   180,   180,   181,   181,   181,   181,   182,   182,
     183,   182,   184,   182,   185,   182,   186,   182,   188,   187,
     190,   189,   191,   191,   192,   192,   194,   193,   195,   195,
     196,   196,   196,   197,   197,   197,   197,   197,   199,   198,
     200,   200,   201,   201,   201,   201,   202,   203,   205,   204,
     206,   206,   207,   207,   207,   207,   207,   207,   207,   207,
     207,   208,   208,   209,   210,   210,   211,   211,   211,   212,
     212,   213,   213,   214,   214,   214,   214,   215,   215,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   217,
     218,   218,   218,   218,   218,   219,   219,   219,   219,   219,
     219,   219,   219,   220,   220,   221,   222,   222,   223,   223,
     224,   224,   224,   225,   225,   226,   226,   227,   227
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     0,     2,     2,     2,     2,     2,
       2,     4,     4,     4,     3,     0,     5,     3,     0,     4,
       4,     4,     4,     4,     4,     1,     4,     4,     3,     0,
       1,     1,     1,     1,     3,     4,     4,     0,     3,     0,
       3,     4,     1,     4,     4,     0,     5,     0,     5,     0,
       5,     0,     5,     0,     3,     0,     3,     2,     0,     4,
       4,     4,     4,     1,     1,     1,     0,     2,     2,     0,
       1,     4,     4,     4,     4,     1,     1,     0,     2,     2,
       0,     1,     4,     4,     1,     4,     4,     4,     0,     3,
       2,     0,     4,     2,     0,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     2,     0,     3,     0,     1,     1,
       1,     1,     1,     4,     0,     3,     2,     0,     1,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       0,     3,     2,     0,     1,     4,     4,     4,     4,     4,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     3,
       0,     3,     2,     0,     1,     1,     0,     3,     2,     0,
       4,     1,     1,     1,     4,     4,     4,     4,     0,     3,
       2,     0,     1,     4,     4,     4,     4,     4,     0,     3,
       2,     0,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     2,     0,     1,     3,     0,     4,     4,     4,     5,
       0,     2,     0,     1,     1,     1,     1,     3,     0,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     5,
       1,     2,     3,     3,     3,     4,     4,     4,     4,     4,
       4,     4,     4,     2,     1,     1,     2,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,     0,     0,     0,     2,     0,
     275,   276,     5,     0,     6,     0,   195,     7,   208,    10,
       8,     0,     9,    15,     1,     4,    29,   107,     0,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     3,     0,     0,
       0,     0,     0,    47,    45,    51,    49,     0,     0,     0,
      30,    31,    32,    33,     0,     0,     0,     0,   142,   140,
     146,   144,     0,     0,     0,     0,   108,   109,   110,   111,
     112,     0,     0,     0,     0,     0,   195,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    14,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   250,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    29,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,   107,     0,     0,     0,   202,    13,
     194,     0,     0,     0,     0,     0,     0,     0,   270,   271,
     272,     0,     0,     0,     0,     0,     0,   274,   273,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   278,   277,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   207,     0,
       0,     0,     0,     0,     0,     0,   251,     0,     0,     0,
       0,     0,     0,     0,     0,    34,     0,    37,     0,    39,
       0,    55,     0,    53,    77,    66,    77,    66,     0,    88,
      28,     0,   114,     0,   130,     0,   150,     0,   148,     0,
       0,     0,     0,     0,     0,   178,   106,   196,   198,   197,
     203,   204,   205,   206,     0,   202,   209,   210,   211,   212,
     227,   245,   246,   213,   214,   215,   216,   217,   228,   220,
     219,   240,   241,   242,   243,   244,   229,   230,   224,   226,
     225,   247,   248,   221,   222,   223,   233,   234,   235,   236,
     237,   238,   239,   218,   232,   231,     0,     0,   265,     0,
       0,   264,   269,     0,   267,     0,     0,     0,   254,   252,
     253,   249,     0,     0,    16,    18,    35,    94,    36,    94,
      44,    58,    43,    58,     0,    80,     0,    69,     0,     0,
      87,    91,   113,   117,   129,   133,   139,   153,   138,   153,
       0,   168,     0,   156,     0,     0,   176,   177,   181,     0,
     201,   261,   258,   256,   255,   263,     0,   257,   266,   259,
     260,   262,     0,     0,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    38,    94,     0,    40,    42,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    65,
      56,    58,    63,    64,    54,    48,     0,     0,     0,     0,
       0,     0,    25,    76,    81,    78,    80,    84,    75,    46,
       0,     0,    67,    69,    70,    52,    50,     0,    89,    91,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   115,
     117,   118,     0,     0,     0,   131,   133,   134,   151,   149,
     143,   171,   141,   159,   147,   145,   179,   181,   199,   268,
      19,    20,     0,     0,     0,     0,     0,   105,     0,     0,
       0,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    57,     0,     0,     0,     0,     0,     0,
      79,     0,     0,    68,     0,    90,     0,     0,     0,     0,
       0,     0,   192,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   116,     0,     0,     0,   132,
     155,   152,   154,   169,   157,   180,     0,     0,     0,     0,
       0,   105,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   193,     0,   192,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   161,   172,   170,   162,     0,     0,
       0,     0,   163,   158,    96,   103,    98,    97,    99,   104,
      95,   100,   102,   101,    41,    62,    59,    60,    61,    85,
      86,    23,    21,    22,    24,    73,    74,    82,    83,    26,
      27,    71,    72,    92,   128,   184,   185,   189,   190,   183,
     182,   191,   188,   119,   126,   120,   121,   122,   123,   124,
     125,   186,   187,   127,   135,   136,   137,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   160,   173,   174,   175,   164,   165,   166,
     167
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,    12,    14,    17,    19,    22,    76,
     263,   264,   452,   453,    88,    89,    90,    91,   266,   367,
     268,   369,   427,    92,   177,   176,   179,   178,   272,   373,
     270,   371,   440,   441,   376,   377,   462,   463,   454,   374,
     375,   455,   456,   442,    93,   278,   381,   468,   469,   424,
     458,   582,   104,   105,   106,   281,   383,   489,   490,   107,
     283,   385,   495,   496,   108,   188,   187,   190,   189,   287,
     389,   285,   387,   498,   571,   392,   503,   574,   635,   643,
     390,   501,   573,   636,   109,   110,   294,   398,   506,   491,
     613,   614,    31,    32,   115,   304,   305,    73,    74,    20,
     168,   169,   350,   351,   353,   354,   211,   219,   238,   239
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -455
static const yytype_int16 yypact[] =
{
      36,   -49,   -49,   -77,   -60,   -49,   -49,    59,  -455,   -45,
    -455,  -455,  -455,   -39,  -455,   -36,    49,  -455,   235,  -455,
    -455,   -34,  -455,  -455,  -455,    36,   149,   184,   -31,   -25,
     -17,    32,     0,     1,     7,     8,     9,    12,    14,    17,
      18,    21,    22,    23,    27,    28,    29,    33,    34,    35,
      37,    38,    50,    51,    54,    62,    64,    70,    72,    73,
      76,    89,    93,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   115,    39,     2,    13,  -455,   116,   117,
     118,   119,   120,  -455,  -455,  -455,  -455,   121,   123,    56,
    -455,  -455,  -455,  -455,   124,   125,   126,   127,  -455,  -455,
    -455,  -455,   129,   130,   132,   137,  -455,  -455,  -455,  -455,
    -455,   -49,   -49,   -49,   135,   140,    49,    40,   148,   169,
     170,   176,   -49,   -49,   -51,   -51,   -51,   -51,   122,   -51,
     -22,   -51,   -51,   177,   189,   190,   -49,   -49,   201,   202,
     203,   205,   -49,   -49,   206,   207,   208,   -51,   -61,   -61,
     209,   -61,   -61,   211,   -51,   212,   213,  -455,   235,   210,
     216,   224,   237,   238,   239,   244,     2,     2,   -44,  -455,
      26,   247,   -49,   -49,   -49,   -49,   249,   250,   251,   252,
     -49,  -455,   149,   -49,   -49,   -49,   -49,   254,   255,   256,
     257,   -49,   -49,  -455,   184,   258,   259,   263,    11,  -455,
    -455,   264,   265,   266,   269,   270,   271,   274,  -455,  -455,
    -455,   275,   276,   278,   280,   288,   292,  -455,  -455,   294,
     295,   297,   298,   299,   300,   302,   303,   311,   312,   316,
     317,   320,   321,   326,   328,   329,   332,  -455,  -455,   333,
     334,   335,   336,   337,   338,   339,   340,   341,  -455,   -49,
     -49,    -3,   215,   -49,   -49,   -49,  -455,   -84,     2,     2,
     214,   307,   314,   344,   261,  -455,   343,  -455,   345,  -455,
     346,  -455,   347,  -455,  -455,  -455,  -455,  -455,   349,  -455,
    -455,   356,  -455,   364,  -455,   365,  -455,   366,  -455,   -49,
     -49,   -49,   -49,   367,   368,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,   369,    11,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,   370,   372,  -455,   373,
     374,   218,   133,   375,   215,   376,   377,   378,  -455,   217,
    -455,  -455,   -49,   -51,  -455,    26,  -455,    53,  -455,   595,
    -455,   454,  -455,   454,   379,   392,   380,   523,   381,   382,
    -455,   305,  -455,   567,  -455,     5,  -455,  -455,  -455,  -455,
     383,  -455,   388,  -455,   389,   390,  -455,  -455,   306,   348,
    -455,  -455,  -455,  -455,  -455,  -455,   220,  -455,  -455,  -455,
    -455,  -455,   391,   394,  -455,   396,   397,   398,   400,   406,
     407,   408,   409,   410,  -455,    53,   412,  -455,  -455,   415,
     416,   419,   420,   422,   426,   428,   429,   430,   431,  -455,
    -455,   454,  -455,  -455,  -455,  -455,   434,   435,   437,   438,
     439,   440,  -455,  -455,  -455,  -455,   392,  -455,  -455,  -455,
     442,   443,  -455,   523,  -455,  -455,  -455,   444,  -455,   305,
     446,   453,   458,   459,   460,   461,   464,   465,   466,   467,
     468,   470,   471,   473,   474,   477,   478,   479,   481,  -455,
     567,  -455,   482,   483,   488,  -455,     5,  -455,   313,   313,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,   306,  -455,  -455,
    -455,  -455,   506,   -51,   507,   508,   -49,   272,   510,   -49,
     511,  -455,   -51,   -51,   -61,   -61,   512,   -51,   514,   -51,
     516,   517,   -51,  -455,   -49,   -49,   -61,   -61,   518,   519,
    -455,   -61,   -61,  -455,   -49,  -455,   -51,   520,   532,   -49,
     538,   539,   586,   -49,   -51,   -51,   -61,   -61,   541,   -61,
     -61,   544,   -49,   -51,   545,  -455,   -61,   -61,   546,  -455,
    -455,  -455,  -455,   262,   131,  -455,   540,   542,   543,   547,
     548,   272,   549,   550,   552,   553,   554,   555,   556,   557,
     558,   559,   560,   561,   562,   563,   564,   565,   566,   568,
     569,   570,   571,   572,   573,   574,   575,   576,   577,   578,
     580,   581,  -455,   582,   586,   583,   584,   585,   587,   588,
     589,   590,   591,   592,   593,   594,   596,   597,   598,   599,
     601,   602,   603,   604,  -455,  -455,  -455,  -455,   605,   606,
     607,   608,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,   -49,   -61,   -61,
     -61,   -61,   -61,   -61,   -51,   609,   610,   611,   612,   613,
     614,   615,   616,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -455,  -455,   630,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
     291,  -455,  -352,  -454,   480,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -358,  -455,   384,  -455,   200,  -455,  -375,   427,
    -455,   228,  -455,  -372,  -455,  -455,  -455,   243,  -455,  -344,
    -335,   144,   551,  -455,  -455,  -455,  -455,   245,  -455,  -455,
    -455,  -455,   240,  -455,  -455,  -455,  -455,  -455,  -455,  -455,
    -455,  -455,  -455,   350,  -455,   445,  -455,  -455,   160,  -455,
     447,  -455,  -455,  -455,  -455,  -455,  -455,  -455,   233,  -367,
     128,  -455,   625,  -455,  -455,   441,  -455,   600,  -455,  -455,
    -143,  -455,   393,  -455,   395,  -455,  -119,  -455,    -1,  -135
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      13,    15,   464,   457,    21,    23,   212,   213,   214,   159,
     216,   218,   220,   221,   240,   444,   242,   243,   497,   439,
     217,   439,   160,   256,   257,   428,   258,   259,   236,   471,
     472,   507,   425,   358,   425,   245,   443,    16,   443,     1,
       2,     3,     4,     5,   208,   209,    10,   237,    11,   473,
     474,   475,    28,    29,    18,   476,    30,   210,    10,    24,
      11,   477,   300,   301,   302,   303,   258,   259,    25,   260,
     492,   493,   494,   208,   209,    26,   486,   487,    27,   415,
      75,   521,   114,   533,   457,   111,   210,   416,   464,   439,
     425,   112,   161,   162,   163,   164,   165,   261,   262,   113,
     417,   418,   419,   420,   348,   349,   443,     6,   421,   422,
     195,   196,   197,   116,   166,   359,   360,   117,   167,   634,
     634,   206,   207,   118,   119,   120,   423,   170,   121,   497,
     122,   572,   572,   123,   124,   225,   226,   125,   126,   127,
     507,   231,   232,   128,   129,   130,   570,   570,   201,   131,
     132,   133,   158,   134,   135,   471,   472,    78,    79,    80,
      81,    82,    83,    84,    85,    86,   136,   137,    87,   182,
     138,   267,   269,   271,   273,   473,   474,   475,   139,   279,
     140,   476,   282,   284,   286,   288,   141,   477,   142,   143,
     293,   295,   144,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   486,   487,   103,   145,   637,   637,   630,   146,
     638,   639,   640,   450,   435,   451,   436,   437,   438,   641,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   215,   171,   172,   173,   174,   175,   180,   181,   406,
     183,   184,   185,   186,   413,   191,   192,   193,   346,   347,
     194,   198,   355,   356,   357,   199,   202,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,   203,   204,    53,
      54,    55,    56,    57,   205,   222,   471,   472,   391,   393,
     391,   393,    58,    59,    60,    61,    62,   223,   224,    63,
      64,    65,    66,    67,    68,    69,   473,   474,   475,   227,
     228,   229,   476,   230,   233,   234,   235,   241,   477,   244,
     246,   247,   352,   467,    70,   348,   249,   509,   259,   361,
     471,   472,   250,   486,   487,    71,    72,   471,   472,   630,
     251,   631,   632,   633,   450,   435,   451,   436,   437,   438,
     473,   474,   475,   252,   253,   254,   476,   473,   474,   475,
     255,   412,   477,   476,   265,   274,   275,   276,   277,   477,
     289,   290,   291,   292,   365,   297,   298,   486,   487,   581,
     299,   306,   307,   308,   486,   487,   309,   310,   311,   588,
     589,   312,   313,   314,   577,   315,   435,   316,   436,   437,
     438,   599,   600,   586,   587,   317,   603,   604,   591,   318,
     593,   319,   320,   596,   321,   322,   323,   324,   415,   325,
     326,   618,   619,   362,   621,   622,   416,   606,   327,   328,
     363,   627,   628,   329,   330,   616,   617,   331,   332,   417,
     418,   419,   420,   333,   625,   334,   335,   421,   422,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   364,
     366,   508,   368,   370,   372,   423,   380,   433,   434,   446,
     447,   448,   449,   382,   450,   435,   451,   436,   437,   438,
     415,   384,   386,   388,   396,   397,   399,   401,   416,   402,
     403,   404,   407,   409,   410,   411,   445,   459,   465,   466,
     500,   417,   418,   419,   420,   502,   504,   505,   510,   421,
     422,   511,   512,   513,   514,   580,   515,   429,   584,   430,
     431,   432,   516,   517,   518,   519,   520,   423,   522,   433,
     434,   523,   524,   597,   598,   525,   526,   435,   527,   436,
     437,   438,   528,   605,   529,   530,   531,   532,   609,   415,
     534,   535,   615,   536,   537,   538,   539,   416,   541,   542,
     544,   624,   546,   706,   707,   708,   709,   710,   711,   547,
     417,   418,   419,   420,   548,   549,   550,   551,   421,   422,
     552,   553,   554,   555,   556,   712,   557,   558,   470,   559,
     560,   471,   472,   561,   562,   563,   423,   564,   566,   567,
     446,   447,   460,   461,   568,   450,   435,   451,   436,   437,
     438,   473,   474,   475,   576,   578,   579,   476,   583,   585,
     590,   415,   592,   477,   594,   595,   601,   602,   607,   416,
     478,   479,   480,   481,   482,   483,   484,   485,   486,   487,
     608,   488,   417,   418,   419,   420,   610,   611,   612,   620,
     421,   422,   623,   626,   629,    77,   414,   644,   426,   645,
     646,   379,   280,   543,   647,   648,   650,   651,   423,   652,
     653,   654,   655,   656,   657,   658,   659,   660,   661,   662,
     663,   664,   665,   666,   540,   667,   668,   669,   670,   671,
     672,   673,   674,   675,   676,   677,   705,   678,   679,   680,
     682,   683,   684,   378,   685,   686,   687,   688,   689,   690,
     691,   692,   545,   693,   694,   695,   696,   697,   698,   699,
     700,   701,   702,   703,   704,   649,   713,   714,   715,   716,
     717,   718,   719,   720,   642,   565,   569,   395,   394,   499,
     575,   200,   681,     0,   405,   296,   400,     0,     0,   408,
       0,     0,     0,     0,     0,     0,     0,     0,   248
};

static const yytype_int16 yycheck[] =
{
       1,     2,   377,   375,     5,     6,   125,   126,   127,     7,
     129,   130,   131,   132,   149,   373,   151,   152,   385,   371,
      42,   373,    20,   166,   167,   369,   110,   111,   147,    24,
      25,   398,   367,   117,   369,   154,   371,   114,   373,     3,
       4,     5,     6,     7,    95,    96,   107,   108,   109,    44,
      45,    46,     3,     4,   114,    50,     7,   108,   107,     0,
     109,    56,    51,    52,    53,    54,   110,   111,   113,   113,
      65,    66,    67,    95,    96,   114,    71,    72,   114,    26,
     114,   425,    50,   441,   456,   116,   108,    34,   463,   441,
     425,   116,    90,    91,    92,    93,    94,    71,    72,   116,
      47,    48,    49,    50,   107,   108,   441,    71,    55,    56,
     111,   112,   113,   113,   112,   258,   259,   116,   116,   573,
     574,   122,   123,   116,   116,   116,    73,   114,   116,   496,
     116,   498,   499,   116,   116,   136,   137,   116,   116,   116,
     507,   142,   143,   116,   116,   116,   498,   499,   108,   116,
     116,   116,   113,   116,   116,    24,    25,     8,     9,    10,
      11,    12,    13,    14,    15,    16,   116,   116,    19,   113,
     116,   172,   173,   174,   175,    44,    45,    46,   116,   180,
     116,    50,   183,   184,   185,   186,   116,    56,   116,   116,
     191,   192,   116,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    71,    72,    20,   116,   573,   574,    77,   116,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     115,   109,   116,   116,   116,   116,   116,   116,   115,   106,
     116,   116,   116,   116,   363,   116,   116,   115,   249,   250,
     113,   116,   253,   254,   255,   115,   108,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,   108,   108,    44,
      45,    46,    47,    48,   108,   108,    24,    25,   289,   290,
     291,   292,    57,    58,    59,    60,    61,   108,   108,    64,
      65,    66,    67,    68,    69,    70,    44,    45,    46,   108,
     108,   108,    50,   108,   108,   108,   108,   108,    56,   108,
     108,   108,   107,    18,    89,   107,   116,   107,   111,   115,
      24,    25,   116,    71,    72,   100,   101,    24,    25,    77,
     116,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      44,    45,    46,   116,   116,   116,    50,    44,    45,    46,
     116,   362,    56,    50,   117,   116,   116,   116,   116,    56,
     116,   116,   116,   116,   113,   117,   117,    71,    72,   107,
     117,   117,   117,   117,    71,    72,   117,   117,   117,   524,
     525,   117,   117,   117,   513,   117,    83,   117,    85,    86,
      87,   536,   537,   522,   523,   117,   541,   542,   527,   117,
     529,   117,   117,   532,   117,   117,   117,   117,    26,   117,
     117,   556,   557,   116,   559,   560,    34,   546,   117,   117,
     116,   566,   567,   117,   117,   554,   555,   117,   117,    47,
      48,    49,    50,   117,   563,   117,   117,    55,    56,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   115,
     117,   113,   117,   117,   117,    73,   117,    75,    76,    77,
      78,    79,    80,   117,    82,    83,    84,    85,    86,    87,
      26,   117,   117,   117,   117,   117,   117,   117,    34,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,    47,    48,    49,    50,   117,   117,   117,   117,    55,
      56,   117,   116,   116,   116,   516,   116,    63,   519,    65,
      66,    67,   116,   116,   116,   116,   116,    73,   116,    75,
      76,   116,   116,   534,   535,   116,   116,    83,   116,    85,
      86,    87,   116,   544,   116,   116,   116,   116,   549,    26,
     116,   116,   553,   116,   116,   116,   116,    34,   116,   116,
     116,   562,   116,   698,   699,   700,   701,   702,   703,   116,
      47,    48,    49,    50,   116,   116,   116,   116,    55,    56,
     116,   116,   116,   116,   116,   704,   116,   116,    21,   116,
     116,    24,    25,   116,   116,   116,    73,   116,   116,   116,
      77,    78,    79,    80,   116,    82,    83,    84,    85,    86,
      87,    44,    45,    46,   108,   108,   108,    50,   108,   108,
     108,    26,   108,    56,   108,   108,   108,   108,   108,    34,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
     108,    74,    47,    48,    49,    50,   108,   108,    62,   108,
      55,    56,   108,   108,   108,    25,   365,   117,    63,   117,
     117,   277,   182,   463,   117,   117,   117,   117,    73,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   456,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   697,   117,   117,   117,
     117,   117,   117,   276,   117,   117,   117,   117,   117,   117,
     117,   117,   469,   117,   117,   117,   117,   116,   116,   116,
     116,   116,   116,   116,   116,   581,   117,   117,   117,   117,
     117,   117,   117,   117,   574,   490,   496,   292,   291,   389,
     507,   116,   614,    -1,   351,   194,   305,    -1,    -1,   354,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   158
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,    71,   119,   120,   121,
     107,   109,   122,   226,   123,   226,   114,   124,   114,   125,
     217,   226,   126,   226,     0,   113,   114,   114,     3,     4,
       7,   210,   211,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    44,    45,    46,    47,    48,    57,    58,
      59,    60,    61,    64,    65,    66,    67,    68,    69,    70,
      89,   100,   101,   215,   216,   114,   127,   120,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    19,   132,   133,
     134,   135,   141,   162,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    20,   170,   171,   172,   177,   182,   202,
     203,   116,   116,   116,    50,   212,   113,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   115,   113,     7,
      20,    90,    91,    92,    93,    94,   112,   116,   218,   219,
     114,   116,   116,   116,   116,   116,   143,   142,   145,   144,
     116,   115,   113,   116,   116,   116,   116,   184,   183,   186,
     185,   116,   116,   115,   113,   226,   226,   226,   116,   115,
     210,   108,   108,   108,   108,   108,   226,   226,    95,    96,
     108,   224,   224,   224,   224,   109,   224,    42,   224,   225,
     224,   224,   108,   108,   108,   226,   226,   108,   108,   108,
     108,   226,   226,   108,   108,   108,   224,   108,   226,   227,
     227,   108,   227,   227,   108,   224,   108,   108,   215,   116,
     116,   116,   116,   116,   116,   116,   218,   218,   110,   111,
     113,    71,    72,   128,   129,   117,   136,   226,   138,   226,
     148,   226,   146,   226,   116,   116,   116,   116,   163,   226,
     132,   173,   226,   178,   226,   189,   226,   187,   226,   116,
     116,   116,   116,   226,   204,   226,   170,   117,   117,   117,
      51,    52,    53,    54,   213,   214,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   226,   226,   107,   108,
     220,   221,   107,   222,   223,   226,   226,   226,   117,   218,
     218,   115,   116,   116,   115,   113,   117,   137,   117,   139,
     117,   149,   117,   147,   157,   158,   152,   153,   157,   152,
     117,   164,   117,   174,   117,   179,   117,   190,   117,   188,
     198,   226,   193,   226,   198,   193,   117,   117,   205,   117,
     213,   117,   117,   117,   117,   220,   106,   117,   222,   117,
     117,   117,   226,   224,   128,    26,    34,    47,    48,    49,
      50,    55,    56,    73,   167,   168,    63,   140,   167,    63,
      65,    66,    67,    75,    76,    83,    85,    86,    87,   130,
     150,   151,   161,   168,   150,   117,    77,    78,    79,    80,
      82,    84,   130,   131,   156,   159,   160,   161,   168,   117,
      79,    80,   154,   155,   156,   117,   117,    18,   165,   166,
      21,    24,    25,    44,    45,    46,    50,    56,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    74,   175,
     176,   207,    65,    66,    67,   180,   181,   207,   191,   191,
     117,   199,   117,   194,   117,   117,   206,   207,   113,   107,
     117,   117,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   167,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   150,   116,   116,   116,   116,   116,   116,
     159,   116,   116,   154,   116,   165,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   175,   116,   116,   116,   180,
     130,   192,   207,   200,   195,   206,   108,   224,   108,   108,
     226,   107,   169,   108,   226,   108,   224,   224,   227,   227,
     108,   224,   108,   224,   108,   108,   224,   226,   226,   227,
     227,   108,   108,   227,   227,   226,   224,   108,   108,   226,
     108,   108,    62,   208,   209,   226,   224,   224,   227,   227,
     108,   227,   227,   108,   226,   224,   108,   227,   227,   108,
      77,    79,    80,    81,   131,   196,   201,   207,    79,    80,
      81,    88,   196,   197,   117,   117,   117,   117,   117,   169,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   208,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   116,   116,   116,
     116,   116,   116,   116,   116,   226,   227,   227,   227,   227,
     227,   227,   224,   117,   117,   117,   117,   117,   117,   117,
     117
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 206 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_add_source(configuration, (yyvsp[(2) - (2)].ptr)); }
    break;

  case 6:
#line 207 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_add_dest(configuration, (yyvsp[(2) - (2)].ptr)); }
    break;

  case 7:
#line 208 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_add_connection(configuration, (yyvsp[(2) - (2)].ptr)); }
    break;

  case 8:
#line 209 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_add_filter(configuration, (yyvsp[(2) - (2)].ptr)); }
    break;

  case 9:
#line 210 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_add_template(configuration, (yyvsp[(2) - (2)].ptr)); }
    break;

  case 10:
#line 211 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {  }
    break;

  case 11:
#line 215 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_source_group_new((yyvsp[(1) - (4)].cptr), (yyvsp[(3) - (4)].ptr)); free((yyvsp[(1) - (4)].cptr)); }
    break;

  case 12:
#line 219 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_dest_group_new((yyvsp[(1) - (4)].cptr), (yyvsp[(3) - (4)].ptr)); free((yyvsp[(1) - (4)].cptr)); }
    break;

  case 13:
#line 223 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_connection_new((yyvsp[(2) - (4)].ptr), (yyvsp[(3) - (4)].num)); }
    break;

  case 14:
#line 227 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = NULL; }
    break;

  case 15:
#line 232 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {
	    last_template = log_template_new((yyvsp[(1) - (1)].cptr), NULL);
	    free((yyvsp[(1) - (1)].cptr));
	  }
    break;

  case 16:
#line 236 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_template;  }
    break;

  case 19:
#line 245 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_template->template = g_string_new((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 20:
#line 246 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { log_template_set_escape(last_template, (yyvsp[(3) - (4)].num)); }
    break;

  case 21:
#line 250 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_sock_options->sndbuf = (yyvsp[(3) - (4)].num); }
    break;

  case 22:
#line 251 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_sock_options->rcvbuf = (yyvsp[(3) - (4)].num); }
    break;

  case 23:
#line 252 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_sock_options->broadcast = (yyvsp[(3) - (4)].num); }
    break;

  case 24:
#line 253 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_sock_options->keepalive = (yyvsp[(3) - (4)].num); }
    break;

  case 26:
#line 258 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { ((InetSocketOptions *) last_sock_options)->ttl = (yyvsp[(3) - (4)].num); }
    break;

  case 27:
#line 259 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { ((InetSocketOptions *) last_sock_options)->tos = (yyvsp[(3) - (4)].num); }
    break;

  case 28:
#line 263 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { log_drv_append((yyvsp[(1) - (3)].ptr), (yyvsp[(3) - (3)].ptr)); log_drv_unref((yyvsp[(3) - (3)].ptr)); (yyval.ptr) = (yyvsp[(1) - (3)].ptr); }
    break;

  case 29:
#line 264 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = NULL; }
    break;

  case 30:
#line 268 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 31:
#line 269 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 32:
#line 270 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 33:
#line 271 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 34:
#line 275 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = afinter_sd_new(); }
    break;

  case 35:
#line 279 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 36:
#line 280 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 37:
#line 285 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {
	    last_driver = affile_sd_new((yyvsp[(1) - (1)].cptr), 0); 
	    free((yyvsp[(1) - (1)].cptr)); 
	    last_reader_options = &((AFFileSourceDriver *) last_driver)->reader_options;
	  }
    break;

  case 38:
#line 290 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 39:
#line 295 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {
	    last_driver = affile_sd_new((yyvsp[(1) - (1)].cptr), AFFILE_PIPE); 
	    free((yyvsp[(1) - (1)].cptr)); 
	    last_reader_options = &((AFFileSourceDriver *) last_driver)->reader_options;
	  }
    break;

  case 40:
#line 300 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 41:
#line 304 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_driver->optional = (yyvsp[(3) - (4)].num); }
    break;

  case 42:
#line 305 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {}
    break;

  case 43:
#line 309 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 44:
#line 310 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 45:
#line 311 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 46:
#line 311 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 47:
#line 312 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 48:
#line 312 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 49:
#line 313 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 50:
#line 313 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 51:
#line 314 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 52:
#line 314 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 53:
#line 319 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afunix_sd_new(
		(yyvsp[(1) - (1)].cptr),
		AFSOCKET_DGRAM | AFSOCKET_LOCAL); 
	    free((yyvsp[(1) - (1)].cptr)); 
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFUnixSourceDriver *) last_driver)->sock_options;
	  }
    break;

  case 54:
#line 327 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 55:
#line 332 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afunix_sd_new(
		(yyvsp[(1) - (1)].cptr),
		AFSOCKET_STREAM | AFSOCKET_KEEP_ALIVE | AFSOCKET_LOCAL);
	    free((yyvsp[(1) - (1)].cptr));
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFUnixSourceDriver *) last_driver)->sock_options;
	  }
    break;

  case 56:
#line 340 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 59:
#line 350 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afunix_sd_set_uid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 60:
#line 351 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afunix_sd_set_gid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 61:
#line 352 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afunix_sd_set_perm(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 62:
#line 353 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_driver->optional = (yyvsp[(3) - (4)].num); }
    break;

  case 63:
#line 354 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {}
    break;

  case 64:
#line 355 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {}
    break;

  case 65:
#line 356 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {}
    break;

  case 66:
#line 361 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afinet_sd_new(last_addr_family,
			NULL, 514,
			AFSOCKET_DGRAM);
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFInetSourceDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 67:
#line 368 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 71:
#line 378 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, (yyvsp[(3) - (4)].cptr), "udp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 72:
#line 379 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, (yyvsp[(3) - (4)].cptr), "udp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 73:
#line 383 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_sd_set_localip(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 74:
#line 384 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_sd_set_localip(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 77:
#line 391 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afinet_sd_new(last_addr_family,
			NULL, 514,
			AFSOCKET_STREAM);
	    last_reader_options = &((AFSocketSourceDriver *) last_driver)->reader_options;
	    last_sock_options = &((AFInetSourceDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 78:
#line 398 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 82:
#line 408 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, (yyvsp[(3) - (4)].cptr), "tcp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 83:
#line 409 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_sd_set_localport(last_driver, (yyvsp[(3) - (4)].cptr), "tcp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 84:
#line 410 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    {}
    break;

  case 85:
#line 414 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afsocket_sd_set_keep_alive(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 86:
#line 415 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afsocket_sd_set_max_connections(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 87:
#line 419 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 88:
#line 424 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afstreams_sd_new((yyvsp[(1) - (1)].cptr)); 
	    free((yyvsp[(1) - (1)].cptr)); 
	  }
    break;

  case 89:
#line 428 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 92:
#line 437 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afstreams_sd_set_sundoor(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 95:
#line 446 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->options = (yyvsp[(3) - (4)].num); }
    break;

  case 96:
#line 447 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->msg_size = (yyvsp[(3) - (4)].num); }
    break;

  case 97:
#line 448 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->source_opts.init_window_size = (yyvsp[(3) - (4)].num); }
    break;

  case 98:
#line 449 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->fetch_limit = (yyvsp[(3) - (4)].num); }
    break;

  case 99:
#line 450 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->prefix = (yyvsp[(3) - (4)].cptr); }
    break;

  case 100:
#line 451 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->padding = (yyvsp[(3) - (4)].num); }
    break;

  case 101:
#line 452 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->follow_freq = (yyvsp[(3) - (4)].num); }
    break;

  case 102:
#line 453 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_timezone_value((yyvsp[(3) - (4)].cptr), &last_reader_options->zone_offset); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 103:
#line 454 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_reader_options->keep_timestamp = (yyvsp[(3) - (4)].num); }
    break;

  case 104:
#line 458 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = lookup_parse_flag((yyvsp[(1) - (2)].cptr)) | (yyvsp[(2) - (2)].num); free((yyvsp[(1) - (2)].cptr)); }
    break;

  case 105:
#line 459 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 0; }
    break;

  case 106:
#line 464 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { log_drv_append((yyvsp[(1) - (3)].ptr), (yyvsp[(3) - (3)].ptr)); log_drv_unref((yyvsp[(3) - (3)].ptr)); (yyval.ptr) = (yyvsp[(1) - (3)].ptr); }
    break;

  case 107:
#line 465 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = NULL; }
    break;

  case 108:
#line 469 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 109:
#line 470 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 110:
#line 471 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 111:
#line 472 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 112:
#line 473 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (1)].ptr); }
    break;

  case 113:
#line 477 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 114:
#line 482 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = affile_dd_new((yyvsp[(1) - (1)].cptr), 0); 
	    free((yyvsp[(1) - (1)].cptr)); 
	    last_writer_options = &((AFFileDestDriver *) last_driver)->writer_options;
	  }
    break;

  case 115:
#line 488 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 119:
#line 498 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_driver->optional = (yyvsp[(3) - (4)].num); }
    break;

  case 120:
#line 503 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_file_uid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 121:
#line 504 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_file_gid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 122:
#line 505 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_file_perm(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 123:
#line 506 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_dir_uid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 124:
#line 507 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_dir_gid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 125:
#line 508 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_dir_perm(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 126:
#line 509 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_create_dirs(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 127:
#line 510 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_overwrite_if_older(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 128:
#line 511 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_fsync(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 129:
#line 515 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 130:
#line 520 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = affile_dd_new((yyvsp[(1) - (1)].cptr), AFFILE_NO_EXPAND | AFFILE_PIPE);
	    free((yyvsp[(1) - (1)].cptr)); 
	    last_writer_options = &((AFFileDestDriver *) last_driver)->writer_options;
	    last_writer_options->flush_lines = 0;
	  }
    break;

  case 131:
#line 526 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 135:
#line 536 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_file_uid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 136:
#line 537 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_file_gid(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 137:
#line 538 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { affile_dd_set_file_perm(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 138:
#line 543 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 139:
#line 544 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 140:
#line 545 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 141:
#line 545 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 142:
#line 546 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET; }
    break;

  case 143:
#line 546 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 144:
#line 547 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 145:
#line 547 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 146:
#line 548 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_addr_family = AF_INET6; }
    break;

  case 147:
#line 548 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(4) - (5)].ptr); }
    break;

  case 148:
#line 553 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afunix_dd_new((yyvsp[(1) - (1)].cptr), AFSOCKET_DGRAM);
	    free((yyvsp[(1) - (1)].cptr));
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFUnixDestDriver *) last_driver)->sock_options;
	  }
    break;

  case 149:
#line 559 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 150:
#line 564 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afunix_dd_new((yyvsp[(1) - (1)].cptr), AFSOCKET_STREAM);
	    free((yyvsp[(1) - (1)].cptr));
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFUnixDestDriver *) last_driver)->sock_options;
	  }
    break;

  case 151:
#line 570 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 156:
#line 585 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afinet_dd_new(last_addr_family,
			(yyvsp[(1) - (1)].cptr), 514,
			AFSOCKET_DGRAM);
	    free((yyvsp[(1) - (1)].cptr));
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFInetDestDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 157:
#line 593 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 160:
#line 603 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_localip(last_driver, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 164:
#line 610 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_localport(last_driver, (yyvsp[(3) - (4)].cptr), "udp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 165:
#line 611 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, (yyvsp[(3) - (4)].cptr), "udp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 166:
#line 612 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, (yyvsp[(3) - (4)].cptr), "udp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 167:
#line 613 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_spoof_source(last_driver, (yyvsp[(3) - (4)].num)); }
    break;

  case 168:
#line 618 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afinet_dd_new(last_addr_family,
			(yyvsp[(1) - (1)].cptr), 514,
			AFSOCKET_STREAM); 
	    free((yyvsp[(1) - (1)].cptr));
	    last_writer_options = &((AFSocketDestDriver *) last_driver)->writer_options;
	    last_sock_options = &((AFInetDestDriver *) last_driver)->sock_options.super;
	  }
    break;

  case 169:
#line 626 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 173:
#line 636 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_localport(last_driver, (yyvsp[(3) - (4)].cptr), "tcp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 174:
#line 637 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, (yyvsp[(3) - (4)].cptr), "tcp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 175:
#line 638 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { afinet_dd_set_destport(last_driver, (yyvsp[(3) - (4)].cptr), "tcp"); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 176:
#line 648 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = afuser_dd_new((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 177:
#line 652 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(3) - (4)].ptr); }
    break;

  case 178:
#line 657 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    last_driver = afprogram_dd_new((yyvsp[(1) - (1)].cptr)); 
	    free((yyvsp[(1) - (1)].cptr)); 
	    last_writer_options = &((AFProgramDestDriver *) last_driver)->writer_options;
	  }
    break;

  case 179:
#line 662 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = last_driver; }
    break;

  case 182:
#line 671 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_writer_options->options = (yyvsp[(3) - (4)].num); }
    break;

  case 183:
#line 672 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_writer_options->fifo_size = (yyvsp[(3) - (4)].num); }
    break;

  case 184:
#line 673 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_writer_options->flush_lines = (yyvsp[(3) - (4)].num); }
    break;

  case 185:
#line 674 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_writer_options->flush_timeout = (yyvsp[(3) - (4)].num); }
    break;

  case 186:
#line 675 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	                                          last_writer_options->template = cfg_lookup_template(configuration, (yyvsp[(3) - (4)].cptr));
	                                          if (last_writer_options->template == NULL)
	                                            {
	                                              last_writer_options->template = log_template_new(NULL, (yyvsp[(3) - (4)].cptr)); 
	                                              last_writer_options->template->def_inline = TRUE;
	                                            }
	                                          free((yyvsp[(3) - (4)].cptr));
	                                        }
    break;

  case 187:
#line 684 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { log_writer_options_set_template_escape(last_writer_options, (yyvsp[(3) - (4)].num)); }
    break;

  case 188:
#line 685 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_timezone_value((yyvsp[(3) - (4)].cptr), &last_writer_options->zone_offset); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 189:
#line 686 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_writer_options->ts_format = cfg_ts_format_value((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 190:
#line 687 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { last_writer_options->frac_digits = (yyvsp[(3) - (4)].num); }
    break;

  case 191:
#line 691 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (2)].num) | (yyvsp[(2) - (2)].num); }
    break;

  case 192:
#line 692 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 0; }
    break;

  case 193:
#line 696 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = LWO_TMPL_ESCAPE; }
    break;

  case 194:
#line 701 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { log_endpoint_append((yyvsp[(1) - (3)].ptr), (yyvsp[(3) - (3)].ptr)); (yyval.ptr) = (yyvsp[(1) - (3)].ptr); }
    break;

  case 195:
#line 702 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = NULL; }
    break;

  case 196:
#line 706 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_endpoint_new(EP_SOURCE, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 197:
#line 707 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_endpoint_new(EP_FILTER, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 198:
#line 708 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_endpoint_new(EP_DESTINATION, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 199:
#line 712 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(3) - (5)].num); }
    break;

  case 200:
#line 713 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 0; }
    break;

  case 201:
#line 718 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) |= (yyvsp[(2) - (2)].num); }
    break;

  case 202:
#line 719 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 0; }
    break;

  case 203:
#line 723 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = LC_CATCHALL; }
    break;

  case 204:
#line 724 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = LC_FALLBACK; }
    break;

  case 205:
#line 725 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = LC_FINAL; }
    break;

  case 206:
#line 726 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = LC_FLOW_CONTROL; }
    break;

  case 207:
#line 730 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = (yyvsp[(1) - (3)].ptr); }
    break;

  case 208:
#line 731 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = NULL; }
    break;

  case 209:
#line 735 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->mark_freq = (yyvsp[(3) - (4)].num); }
    break;

  case 210:
#line 736 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->stats_freq = (yyvsp[(3) - (4)].num); }
    break;

  case 211:
#line 737 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->flush_lines = (yyvsp[(3) - (4)].num); }
    break;

  case 212:
#line 738 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->flush_timeout = (yyvsp[(3) - (4)].num); }
    break;

  case 213:
#line 739 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->chain_hostnames = (yyvsp[(3) - (4)].num); }
    break;

  case 214:
#line 740 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->normalize_hostnames = (yyvsp[(3) - (4)].num); }
    break;

  case 215:
#line 741 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->keep_hostname = (yyvsp[(3) - (4)].num); }
    break;

  case 216:
#line 742 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->check_hostname = (yyvsp[(3) - (4)].num); }
    break;

  case 217:
#line 743 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_bad_hostname_set(configuration, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 218:
#line 744 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->use_time_recvd = (yyvsp[(3) - (4)].num); }
    break;

  case 219:
#line 745 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->use_fqdn = (yyvsp[(3) - (4)].num); }
    break;

  case 220:
#line 746 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->use_dns = (yyvsp[(3) - (4)].num); }
    break;

  case 221:
#line 747 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->time_reopen = (yyvsp[(3) - (4)].num); }
    break;

  case 222:
#line 748 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->time_reap = (yyvsp[(3) - (4)].num); }
    break;

  case 223:
#line 750 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
		  configuration->time_sleep = (yyvsp[(3) - (4)].num); 
		  if ((yyvsp[(3) - (4)].num) > 500) 
		    { 
		      msg_notice("The value specified for time_sleep is too large", evt_tag_int("time_sleep", (yyvsp[(3) - (4)].num)), NULL);
		      configuration->time_sleep = 500;
		    }
		}
    break;

  case 224:
#line 758 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->log_fifo_size = (yyvsp[(3) - (4)].num); }
    break;

  case 225:
#line 759 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->log_iw_size = (yyvsp[(3) - (4)].num); }
    break;

  case 226:
#line 760 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->log_fetch_limit = (yyvsp[(3) - (4)].num); }
    break;

  case 227:
#line 761 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->log_msg_size = (yyvsp[(3) - (4)].num); }
    break;

  case 228:
#line 762 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->keep_timestamp = (yyvsp[(3) - (4)].num); }
    break;

  case 229:
#line 763 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->ts_format = cfg_ts_format_value((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 230:
#line 764 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->frac_digits = (yyvsp[(3) - (4)].num); }
    break;

  case 231:
#line 765 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { /* ignored */; }
    break;

  case 232:
#line 766 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { /* ignored */; }
    break;

  case 233:
#line 767 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->create_dirs = (yyvsp[(3) - (4)].num); }
    break;

  case 234:
#line 768 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_file_owner_set(configuration, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 235:
#line 769 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_file_group_set(configuration, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 236:
#line 770 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_file_perm_set(configuration, (yyvsp[(3) - (4)].num)); }
    break;

  case 237:
#line 771 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_dir_owner_set(configuration, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 238:
#line 772 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_dir_group_set(configuration, (yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 239:
#line 773 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_dir_perm_set(configuration, (yyvsp[(3) - (4)].num)); }
    break;

  case 240:
#line 774 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->use_dns_cache = (yyvsp[(3) - (4)].num); }
    break;

  case 241:
#line 775 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->dns_cache_size = (yyvsp[(3) - (4)].num); }
    break;

  case 242:
#line 776 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->dns_cache_expire = (yyvsp[(3) - (4)].num); }
    break;

  case 243:
#line 778 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->dns_cache_expire_failed = (yyvsp[(3) - (4)].num); }
    break;

  case 244:
#line 779 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->dns_cache_hosts = (yyvsp[(3) - (4)].cptr); }
    break;

  case 245:
#line 780 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->file_template_name = (yyvsp[(3) - (4)].cptr); }
    break;

  case 246:
#line 781 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { configuration->proto_template_name = (yyvsp[(3) - (4)].cptr); }
    break;

  case 247:
#line 782 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_timezone_value((yyvsp[(3) - (4)].cptr), &configuration->recv_zone_offset); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 248:
#line 783 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { cfg_timezone_value((yyvsp[(3) - (4)].cptr), &configuration->send_zone_offset); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 249:
#line 787 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.ptr) = log_filter_rule_new((yyvsp[(1) - (5)].cptr), (yyvsp[(3) - (5)].node)); free((yyvsp[(1) - (5)].cptr)); }
    break;

  case 250:
#line 791 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); if (!(yyvsp[(1) - (1)].node)) return 1; }
    break;

  case 251:
#line 792 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyvsp[(2) - (2)].node)->comp = !((yyvsp[(2) - (2)].node)->comp); (yyval.node) = (yyvsp[(2) - (2)].node); }
    break;

  case 252:
#line 793 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = fop_or_new((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 253:
#line 794 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = fop_and_new((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); }
    break;

  case 254:
#line 795 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); }
    break;

  case 255:
#line 799 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_facility_new((yyvsp[(3) - (4)].num));  }
    break;

  case 256:
#line 800 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_facility_new(0x80000000 | (yyvsp[(3) - (4)].num)); }
    break;

  case 257:
#line 801 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_level_new((yyvsp[(3) - (4)].num)); }
    break;

  case 258:
#line 802 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_prog_new((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 259:
#line 803 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_host_new((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 260:
#line 804 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_match_new((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 261:
#line 805 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_call_new((yyvsp[(3) - (4)].cptr), configuration); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 262:
#line 806 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.node) = filter_netmask_new((yyvsp[(3) - (4)].cptr)); free((yyvsp[(3) - (4)].cptr)); }
    break;

  case 263:
#line 810 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (2)].num) | (yyvsp[(2) - (2)].num); }
    break;

  case 264:
#line 811 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 265:
#line 816 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    int n = syslog_name_lookup_facility_by_name((yyvsp[(1) - (1)].cptr));
	    if (n == -1)
	      {
	        msg_error("Warning: Unknown facility", 
	                  evt_tag_str("facility", (yyvsp[(1) - (1)].cptr)),
	                  NULL);
	        (yyval.num) = 0;
	      }
	    else
	      (yyval.num) = (1 << n); 
	    free((yyvsp[(1) - (1)].cptr)); 
	  }
    break;

  case 266:
#line 832 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (2)].num) | (yyvsp[(2) - (2)].num); }
    break;

  case 267:
#line 833 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 268:
#line 838 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    int r1, r2;
	    r1 = syslog_name_lookup_level_by_name((yyvsp[(1) - (3)].cptr));
	    if (r1 == -1)
	      msg_error("Warning: Unknown priority level",
                        evt_tag_str("priority", (yyvsp[(1) - (3)].cptr)),
                        NULL);
	    r2 = syslog_name_lookup_level_by_name((yyvsp[(3) - (3)].cptr));
	    if (r2 == -1)
	      msg_error("Warning: Unknown priority level",
                        evt_tag_str("priority", (yyvsp[(1) - (3)].cptr)),
                        NULL);
	    if (r1 != -1 && r2 != -1)
	      (yyval.num) = syslog_make_range(r1, r2); 
	    else
	      (yyval.num) = 0;
	    free((yyvsp[(1) - (3)].cptr)); 
	    free((yyvsp[(3) - (3)].cptr)); 
	  }
    break;

  case 269:
#line 858 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { 
	    int n = syslog_name_lookup_level_by_name((yyvsp[(1) - (1)].cptr)); 
	    if (n == -1)
	      {
	        msg_error("Warning: Unknown priority level",
                          evt_tag_str("priority", (yyvsp[(1) - (1)].cptr)),
                          NULL);
	        (yyval.num) = 0;
	      }
	    else
	      (yyval.num) = 1 << n;
	    free((yyvsp[(1) - (1)].cptr)); 
	  }
    break;

  case 270:
#line 874 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 1; }
    break;

  case 271:
#line 875 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 0; }
    break;

  case 272:
#line 876 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 273:
#line 880 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 274:
#line 881 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.num) = 2; }
    break;

  case 277:
#line 890 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { (yyval.cptr) = (yyvsp[(1) - (1)].cptr); }
    break;

  case 278:
#line 891 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"
    { char buf[16]; snprintf(buf, sizeof(buf), "%d", (yyvsp[(1) - (1)].num)); (yyval.cptr) = strdup(buf); }
    break;


/* Line 1267 of yacc.c.  */
#line 3548 "cfg-grammar.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 893 "/home/bazsi/zwa/git//syslog-ng/syslog-ng-ose--mainline--2.0/src/cfg-grammar.y"


extern int linenum;

void 
yyerror(char *msg)
{
  fprintf(stderr, "%s at %d\n", msg, linenum);
}

void
yyparser_reset(void)
{
  last_driver = NULL;
  last_reader_options = NULL;
  last_writer_options = NULL;
  last_template = NULL;
}
