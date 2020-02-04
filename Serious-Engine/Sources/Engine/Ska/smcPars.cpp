
/*  A Bison parser, made from smcPars.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse syyparse
#define yylex syylex
#define yyerror syyerror
#define yylval syylval
#define yychar syychar
#define yydebug syydebug
#define yynerrs syynerrs
#define	c_float	258
#define	c_int	259
#define	c_string	260
#define	c_modelinstance	261
#define	k_PARENTBONE	262
#define	k_SE_SMC	263
#define	k_SE_END	264
#define	k_NAME	265
#define	k_TFNM	266
#define	k_MESH	267
#define	k_SKELETON	268
#define	k_ANIMSET	269
#define	K_ANIMATION	270
#define	k_TEXTURES	271
#define	k_OFFSET	272
#define	k_COLISION	273
#define	k_ALLFRAMESBBOX	274
#define	k_ANIMSPEED	275
#define	k_COLOR	276

#line 1 "smcPars.y"

#include <Engine/StdH.h>
#include <Engine/Ska/ModelInstance.h>
#include <Engine/Ska/AnimSet.h>
#include <Engine/Ska/StringTable.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/Shell.h>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Ska/ParsingSmbs.h>

extern BOOL bRememberSourceFN;
BOOL bOffsetAllreadySet = FALSE;
#line 15 "smcPars.y"

#define YYERROR_VERBOSE 0
// if error occurs in parsing
void syyerror(char *str)
{
  // just report the string
  _pShell->ErrorF("%s", str);
};

#line 27 "smcPars.y"
typedef union {
  int i;
  float f;
  const char *str;
  CModelInstance *pmi;
  float f6[6];
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		123
#define	YYFLAG		-32768
#define	YYNTBASE	26

#define YYTRANSLATE(x) ((unsigned)(x) <= 276 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    25,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    22,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    23,     2,    24,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,    10,    11,    14,    16,    18,    20,    22,    24,
    26,    28,    30,    31,    33,    37,    42,    43,    45,    47,
    50,    66,    67,    69,    83,    84,    87,   101,   102,   114,
   115,   122,   127,   130,   135,   137,   140,   144,   145,   147,
   151,   152,   155,   160,   161,   164,   169,   171,   173
};

static const short yyrhs[] = {    -1,
    38,    10,     5,    22,    27,    23,    28,    24,     0,     0,
    29,    28,     0,    42,     0,    44,     0,    45,     0,    48,
     0,    40,     0,    32,     0,    36,     0,    30,     0,     0,
    31,     0,    21,     4,    22,     0,    18,    23,    33,    24,
     0,     0,    34,     0,    35,     0,    34,    35,     0,     5,
    23,    54,    25,    54,    25,    54,    25,    54,    25,    54,
    25,    54,    22,    24,     0,     0,    37,     0,    19,    54,
    25,    54,    25,    54,    25,    54,    25,    54,    25,    54,
    22,     0,     0,    39,    38,     0,    17,    54,    25,    54,
    25,    54,    25,    54,    25,    54,    25,    54,    22,     0,
     0,     7,     5,    22,    38,    10,     5,    22,    41,    23,
    28,    24,     0,     0,    12,    11,     5,    22,    43,    50,
     0,    13,    11,     5,    22,     0,    14,    47,     0,    14,
    23,    46,    24,     0,    47,     0,    46,    47,     0,    11,
     5,    22,     0,     0,    49,     0,    15,     5,    22,     0,
     0,    50,    51,     0,    16,    23,    52,    24,     0,     0,
    52,    53,     0,     5,    11,     5,    22,     0,     3,     0,
     4,     0,     4,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    72,    91,    95,    96,   100,   101,   102,   103,   104,   105,
   106,   107,   111,   112,   116,   124,   130,   131,   135,   136,
   140,   150,   151,   155,   164,   174,   181,   200,   225,   234,
   240,   243,   250,   251,   255,   256,   260,   268,   269,   273,
   285,   286,   290,   294,   295,   299,   306,   310,   317
};

static const char * const yytname[] = {   "$","error","$undefined.","c_float",
"c_int","c_string","c_modelinstance","k_PARENTBONE","k_SE_SMC","k_SE_END","k_NAME",
"k_TFNM","k_MESH","k_SKELETON","k_ANIMSET","K_ANIMATION","k_TEXTURES","k_OFFSET",
"k_COLISION","k_ALLFRAMESBBOX","k_ANIMSPEED","k_COLOR","';'","'{'","'}'","','",
"parent_model","@1","components","component","mdl_color_opt","mdl_color","colision_header",
"colision_opt","colision_array","colision","all_frames_bbox_opt","all_frames_bbox",
"offset_opt","offset","child_model","@2","mesh","@3","skeleton","animset_header",
"animset_array","animset","animation_opt","animation","opt_textures","textures",
"textures_array","texture","float_const","\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735\37777777735"
};
#endif

static const short yyr1[] = {     0,
    27,    26,    28,    28,    29,    29,    29,    29,    29,    29,
    29,    29,    30,    30,    31,    32,    33,    33,    34,    34,
    35,    36,    36,    37,    38,    38,    39,    41,    40,    43,
    42,    44,    45,    45,    46,    46,    47,    48,    48,    49,
    50,    50,    51,    52,    52,    53,    54,    54,    -1
};

static const short yyr2[] = {     0,
     0,     8,     0,     2,     1,     1,     1,     1,     1,     1,
     1,     1,     0,     1,     3,     4,     0,     1,     1,     2,
    15,     0,     1,    13,     0,     2,    13,     0,    11,     0,
     6,     4,     2,     4,     1,     2,     3,     0,     1,     3,
     0,     2,     4,     0,     2,     4,     1,     1,     1
};

static const short yydefact[] = {    25,
     0,     0,    25,    47,    48,     0,     0,    26,     0,     0,
     0,     1,     0,     0,     0,     3,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     3,    12,    14,    10,
    11,    23,     9,     5,     6,     7,     8,    39,     0,     0,
     0,     0,     0,     0,    33,     0,    17,     0,     0,     2,
     4,     0,    25,     0,     0,     0,     0,    35,    40,     0,
     0,    18,    19,     0,    15,     0,     0,    30,    32,    37,
    34,    36,     0,    16,    20,     0,     0,     0,    41,     0,
     0,     0,     0,    31,     0,     0,    27,    28,     0,    42,
     0,     0,     0,    44,     0,     0,     3,     0,     0,     0,
     0,     0,    43,    45,     0,     0,    29,     0,     0,     0,
     0,     0,     0,    46,     0,    24,     0,     0,     0,    21,
     0,     0,     0
};

static const short yydefgoto[] = {   121,
    14,    26,    27,    28,    29,    30,    61,    62,    63,    31,
    32,     2,     3,    33,    93,    34,    79,    35,    36,    57,
    45,    37,    38,    84,    90,    98,   104,     6
};

static const short yypact[] = {   -12,
     6,    -4,   -12,-32768,-32768,   -18,    16,-32768,     6,   -11,
     7,-32768,     6,     8,     9,     5,     6,    23,    19,    22,
     2,    30,    13,     6,    33,    14,     5,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    15,    17,
    36,    37,    39,    34,-32768,    24,    42,    25,    26,-32768,
-32768,     6,   -12,    27,    29,    31,     3,-32768,-32768,    35,
    32,    42,-32768,     6,-32768,    38,    44,-32768,-32768,-32768,
-32768,-32768,     6,-32768,-32768,    40,     6,    52,-32768,    41,
     6,    45,    47,    43,     6,    48,-32768,-32768,    51,-32768,
    50,     6,    54,-32768,     6,    53,     5,    -2,    55,     6,
    46,    49,-32768,-32768,     6,    56,-32768,    57,    59,     6,
    60,     6,    63,-32768,    62,-32768,     6,    66,    65,-32768,
    61,    79,-32768
};

static const short yypgoto[] = {-32768,
-32768,   -26,-32768,-32768,-32768,-32768,-32768,-32768,    28,-32768,
-32768,    -1,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -28,-32768,-32768,-32768,-32768,-32768,-32768,    -9
};


#define	YYLAST		108


static const short yytable[] = {    11,
    51,     8,   102,    15,     1,     7,     9,    39,     4,     5,
    12,    18,    43,    43,    48,    58,    19,    20,    21,    22,
    10,   103,    23,    24,    44,    25,    71,    40,    72,    41,
    16,    13,    42,    17,    46,    47,    49,    50,    53,    52,
    54,    55,    66,    56,    43,    59,    60,    65,    68,    64,
    69,    67,    70,    78,    76,    74,    83,    73,    89,   108,
   122,   111,    77,    80,    81,    85,    87,    82,    88,   107,
   101,    86,    92,    94,    95,    91,    97,   100,   123,   105,
   110,   114,    96,   112,   116,    99,   117,   119,   120,    75,
   106,     0,     0,     0,     0,   109,     0,     0,     0,     0,
   113,     0,   115,     0,     0,     0,     0,   118
};

static const short yycheck[] = {     9,
    27,     3,     5,    13,    17,    10,    25,    17,     3,     4,
    22,     7,    11,    11,    24,    44,    12,    13,    14,    15,
     5,    24,    18,    19,    23,    21,    24,     5,    57,    11,
    23,    25,    11,    25,     5,    23,     4,    24,    22,    25,
     5,     5,    52,     5,    11,    22,     5,    22,    22,    25,
    22,    53,    22,    10,    64,    24,     5,    23,    16,    11,
     0,     5,    25,    73,    25,    25,    22,    77,    22,    24,
    97,    81,    25,    23,    25,    85,    23,    25,     0,    25,
    25,    22,    92,    25,    22,    95,    25,    22,    24,    62,
   100,    -1,    -1,    -1,    -1,   105,    -1,    -1,    -1,    -1,
   110,    -1,   112,    -1,    -1,    -1,    -1,   117
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#undef YYERROR_VERBOSE
#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 73 "smcPars.y"
{
  if(_yy_mi==0) {
    yyerror("_yy_mi = NULL");
  }
  // create new model instance
  // _yy_mi = CreateModelInstance($3);
  _yy_mi->SetName(yyvsp[-1].str);
  // set its offset
  _yy_mi->SetOffset(yyvsp[-3].f6);
  // mark offset as read
  bOffsetAllreadySet = FALSE;
  // check if flag to remember source file name is set
  if(bRememberSourceFN)
  {
    // remember source file name
    _yy_mi->mi_fnSourceFile = CTString(SMCGetBufferName());
  }
;
    break;}
case 15:
#line 117 "smcPars.y"
{
  COLOR c = yyvsp[-1].i;
  // _yy_mi->SetModelColor($2);
;
    break;}
case 16:
#line 125 "smcPars.y"
{

;
    break;}
case 21:
#line 141 "smcPars.y"
{
  FLOAT3D vMin = FLOAT3D(yyvsp[-12].f, yyvsp[-10].f, yyvsp[-8].f);
  FLOAT3D vMax = FLOAT3D(yyvsp[-6].f,yyvsp[-4].f,yyvsp[-2].f);
  // add new colision box to current model instance
  _yy_mi->AddColisionBox(yyvsp[-14].str,vMin,vMax);
;
    break;}
case 24:
#line 156 "smcPars.y"
{
  // add new colision box to current model instance
  _yy_mi->mi_cbAllFramesBBox.SetMin(FLOAT3D(yyvsp[-11].f, yyvsp[-9].f, yyvsp[-7].f));
  _yy_mi->mi_cbAllFramesBBox.SetMax(FLOAT3D(yyvsp[-5].f,yyvsp[-3].f,yyvsp[-1].f));
;
    break;}
case 25:
#line 165 "smcPars.y"
{
  // set offset with default offset values
  yyval.f6[0] = 0;
  yyval.f6[1] = 0;
  yyval.f6[2] = 0;
  yyval.f6[3] = 0;
  yyval.f6[4] = 0;
  yyval.f6[5] = 0;
;
    break;}
case 26:
#line 175 "smcPars.y"
{
  // return new offset
  memcpy(yyval.f6,yyvsp[-1].f6,sizeof(float)*6);
;
    break;}
case 27:
#line 182 "smcPars.y"
{
  // if offset is not set
  if(!bOffsetAllreadySet)
  {
    // set offset
    yyval.f6[0] = yyvsp[-11].f;
    yyval.f6[1] = yyvsp[-9].f;
    yyval.f6[2] = yyvsp[-7].f;
    yyval.f6[3] = yyvsp[-5].f;
    yyval.f6[4] = yyvsp[-3].f;
    yyval.f6[5] = yyvsp[-1].f;
    // mark it as set now
    bOffsetAllreadySet = TRUE;
  }
;
    break;}
case 28:
#line 201 "smcPars.y"
{
  // get parent ID
  int iParentBoneID = ska_FindStringInTable(yyvsp[-5].str);
  if(iParentBoneID<0) iParentBoneID=0;
  // remember current model instance in parent bone token
  yyvsp[-6].pmi = _yy_mi;
  // set _yy_mi as new child
  _yy_mi = CreateModelInstance(yyvsp[-1].str);
  // add child to parent model instance 
  yyvsp[-6].pmi->AddChild(_yy_mi);
  // add offset
  _yy_mi->SetOffset(yyvsp[-3].f6);
  // set its parent bone
  _yy_mi->SetParentBone(iParentBoneID);
  // 
  bOffsetAllreadySet = FALSE;
  // if flag to remember source file is set
  if(bRememberSourceFN)
  {
    // remember source name
    _yy_mi->mi_fnSourceFile = CTString(SMCGetBufferName());
  }
;
    break;}
case 29:
#line 226 "smcPars.y"
{
   // set parent model instance to _yy_mi again
  _yy_mi = yyvsp[-10].pmi;
;
    break;}
case 30:
#line 235 "smcPars.y"
{
  // add mesh to current model instance
  _yy_mi->AddMesh_t((CTString)yyvsp[-1].str);
;
    break;}
case 32:
#line 244 "smcPars.y"
{
  // add skeleton to current model instance
  _yy_mi->AddSkeleton_t((CTString)yyvsp[-1].str);
;
    break;}
case 37:
#line 261 "smcPars.y"
{
  // add animset to curent model instnce 
  _yy_mi->AddAnimSet_t((CTString)yyvsp[-1].str);
;
    break;}
case 40:
#line 274 "smcPars.y"
{
  // set new clear state in model instance
  _yy_mi->NewClearState(1);
  // get anim ID
  INDEX iAnimID = ska_GetIDFromStringTable(yyvsp[-1].str);
  // add animation to curent model instance
  _yy_mi->AddAnimation(iAnimID,AN_LOOPING,1,0);
;
    break;}
case 46:
#line 300 "smcPars.y"
{
  // add texture to current model instance
  _yy_mi->AddTexture_t((CTString)yyvsp[-1].str,yyvsp[-3].str,NULL);
;
    break;}
case 47:
#line 307 "smcPars.y"
{
  yyval.f = yyvsp[0].f;
;
    break;}
case 48:
#line 311 "smcPars.y"
{
  yyval.f = (float)yyvsp[0].i;
;
    break;}
case 49:
#line 318 "smcPars.y"
{
  yyval.i = yyvsp[0].i;
;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 323 "smcPars.y"
