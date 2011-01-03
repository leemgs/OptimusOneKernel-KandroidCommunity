

#include <sys/cdefs.h>

#ifndef lint
#if 0
static const char copyright[] =
"@(#) Copyright (c) 1985, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif
#ifdef __IDSTRING
__IDSTRING(Berkeley, "@(#)unifdef.c	8.1 (Berkeley) 6/6/93");
__IDSTRING(NetBSD, "$NetBSD: unifdef.c,v 1.8 2000/07/03 02:51:36 matt Exp $");
__IDSTRING(dotat, "$dotat: things/unifdef.c,v 1.171 2005/03/08 12:38:48 fanf2 Exp $");
#endif
#endif 
#ifdef __FBSDID
__FBSDID("$FreeBSD: /repoman/r/ncvs/src/usr.bin/unifdef/unifdef.c,v 1.20 2005/05/21 09:55:09 ru Exp $");
#endif



#include <ctype.h>
#include <err.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

size_t strlcpy(char *dst, const char *src, size_t siz);


typedef enum {
	LT_TRUEI,		
	LT_FALSEI,		
	LT_IF,			
	LT_TRUE,		
	LT_FALSE,		
	LT_ELIF,		
	LT_ELTRUE,		
	LT_ELFALSE,		
	LT_ELSE,		
	LT_ENDIF,		
	LT_DODGY,		
	LT_DODGY_LAST = LT_DODGY + LT_ENDIF,
	LT_PLAIN,		
	LT_EOF,			
	LT_COUNT
} Linetype;

static char const * const linetype_name[] = {
	"TRUEI", "FALSEI", "IF", "TRUE", "FALSE",
	"ELIF", "ELTRUE", "ELFALSE", "ELSE", "ENDIF",
	"DODGY TRUEI", "DODGY FALSEI",
	"DODGY IF", "DODGY TRUE", "DODGY FALSE",
	"DODGY ELIF", "DODGY ELTRUE", "DODGY ELFALSE",
	"DODGY ELSE", "DODGY ENDIF",
	"PLAIN", "EOF"
};


typedef enum {
	IS_OUTSIDE,
	IS_FALSE_PREFIX,	
	IS_TRUE_PREFIX,		
	IS_PASS_MIDDLE,		
	IS_FALSE_MIDDLE,	
	IS_TRUE_MIDDLE,		
	IS_PASS_ELSE,		
	IS_FALSE_ELSE,		
	IS_TRUE_ELSE,		
	IS_FALSE_TRAILER,	
	IS_COUNT
} Ifstate;

static char const * const ifstate_name[] = {
	"OUTSIDE", "FALSE_PREFIX", "TRUE_PREFIX",
	"PASS_MIDDLE", "FALSE_MIDDLE", "TRUE_MIDDLE",
	"PASS_ELSE", "FALSE_ELSE", "TRUE_ELSE",
	"FALSE_TRAILER"
};


typedef enum {
	NO_COMMENT = false,	
	C_COMMENT,		
	CXX_COMMENT,		
	STARTING_COMMENT,	
	FINISHING_COMMENT,	
	CHAR_LITERAL,		
	STRING_LITERAL		
} Comment_state;

static char const * const comment_name[] = {
	"NO", "C", "CXX", "STARTING", "FINISHING", "CHAR", "STRING"
};


typedef enum {
	LS_START,		
	LS_HASH,		
	LS_DIRTY		
} Line_state;

static char const * const linestate_name[] = {
	"START", "HASH", "DIRTY"
};


#define	MAXDEPTH        64			
#define	MAXLINE         4096			
#define	MAXSYMS         4096			


#define	EDITSLOP        10



static bool             complement;		
static bool             debugging;		
static bool             iocccok;		
static bool             killconsts;		
static bool             lnblank;		
static bool             lnnum;			
static bool             symlist;		
static bool             text;			

static const char      *symname[MAXSYMS];	
static const char      *value[MAXSYMS];		
static bool             ignore[MAXSYMS];	
static int              nsyms;			

static FILE            *input;			
static const char      *filename;		
static int              linenum;		

static char             tline[MAXLINE+EDITSLOP];
static char            *keyword;		

static Comment_state    incomment;		
static Line_state       linestate;		
static Ifstate          ifstate[MAXDEPTH];	
static bool             ignoring[MAXDEPTH];	
static int              stifline[MAXDEPTH];	
static int              depth;			
static int              delcount;		
static bool             keepthis;		

static int              exitstat;		

static void             addsym(bool, bool, char *);
static void             debug(const char *, ...);
static void             done(void);
static void             error(const char *);
static int              findsym(const char *);
static void             flushline(bool);
static Linetype         get_line(void);
static Linetype         ifeval(const char **);
static void             ignoreoff(void);
static void             ignoreon(void);
static void             keywordedit(const char *);
static void             nest(void);
static void             process(void);
static const char      *skipcomment(const char *);
static const char      *skipsym(const char *);
static void             state(Ifstate);
static int              strlcmp(const char *, const char *, size_t);
static void             unnest(void);
static void             usage(void);

#define endsym(c) (!isalpha((unsigned char)c) && !isdigit((unsigned char)c) && c != '_')


int
main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "i:D:U:I:cdeklnst")) != -1)
		switch (opt) {
		case 'i': 
			
			opt = *optarg++;
			if (opt == 'D')
				addsym(true, true, optarg);
			else if (opt == 'U')
				addsym(true, false, optarg);
			else
				usage();
			break;
		case 'D': 
			addsym(false, true, optarg);
			break;
		case 'U': 
			addsym(false, false, optarg);
			break;
		case 'I':
			
			break;
		case 'c': 
			complement = true;
			break;
		case 'd':
			debugging = true;
			break;
		case 'e': 
			iocccok = true;
			break;
		case 'k': 
			killconsts = true;
			break;
		case 'l': 
			lnblank = true;
			break;
		case 'n': 
			lnnum = true;
			break;
		case 's': 
			symlist = true;
			break;
		case 't': 
			text = true;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc > 1) {
		errx(2, "can only do one file");
	} else if (argc == 1 && strcmp(*argv, "-") != 0) {
		filename = *argv;
		input = fopen(filename, "r");
		if (input == NULL)
			err(2, "can't open %s", filename);
	} else {
		filename = "[stdin]";
		input = stdin;
	}
	process();
	abort(); 
}

static void
usage(void)
{
	fprintf(stderr, "usage: unifdef [-cdeklnst] [-Ipath]"
	    " [-Dsym[=val]] [-Usym] [-iDsym[=val]] [-iUsym] ... [file]\n");
	exit(2);
}


typedef void state_fn(void);


static void Eelif (void) { error("Inappropriate #elif"); }
static void Eelse (void) { error("Inappropriate #else"); }
static void Eendif(void) { error("Inappropriate #endif"); }
static void Eeof  (void) { error("Premature EOF"); }
static void Eioccc(void) { error("Obfuscated preprocessor control line"); }

static void print (void) { flushline(true); }
static void drop  (void) { flushline(false); }

static void Strue (void) { drop();  ignoreoff(); state(IS_TRUE_PREFIX); }
static void Sfalse(void) { drop();  ignoreoff(); state(IS_FALSE_PREFIX); }
static void Selse (void) { drop();               state(IS_TRUE_ELSE); }

static void Pelif (void) { print(); ignoreoff(); state(IS_PASS_MIDDLE); }
static void Pelse (void) { print();              state(IS_PASS_ELSE); }
static void Pendif(void) { print(); unnest(); }

static void Dfalse(void) { drop();  ignoreoff(); state(IS_FALSE_TRAILER); }
static void Delif (void) { drop();  ignoreoff(); state(IS_FALSE_MIDDLE); }
static void Delse (void) { drop();               state(IS_FALSE_ELSE); }
static void Dendif(void) { drop();  unnest(); }

static void Fdrop (void) { nest();  Dfalse(); }
static void Fpass (void) { nest();  Pelif(); }
static void Ftrue (void) { nest();  Strue(); }
static void Ffalse(void) { nest();  Sfalse(); }

static void Oiffy (void) { if (!iocccok) Eioccc(); Fpass(); ignoreon(); }
static void Oif   (void) { if (!iocccok) Eioccc(); Fpass(); }
static void Oelif (void) { if (!iocccok) Eioccc(); Pelif(); }

static void Idrop (void) { Fdrop();  ignoreon(); }
static void Itrue (void) { Ftrue();  ignoreon(); }
static void Ifalse(void) { Ffalse(); ignoreon(); }

static void Mpass (void) { strncpy(keyword, "if  ", 4); Pelif(); }
static void Mtrue (void) { keywordedit("else\n");  state(IS_TRUE_MIDDLE); }
static void Melif (void) { keywordedit("endif\n"); state(IS_FALSE_TRAILER); }
static void Melse (void) { keywordedit("endif\n"); state(IS_FALSE_ELSE); }

static state_fn * const trans_table[IS_COUNT][LT_COUNT] = {

{ Itrue, Ifalse,Fpass, Ftrue, Ffalse,Eelif, Eelif, Eelif, Eelse, Eendif,
  Oiffy, Oiffy, Fpass, Oif,   Oif,   Eelif, Eelif, Eelif, Eelse, Eendif,
  print, done },

{ Idrop, Idrop, Fdrop, Fdrop, Fdrop, Mpass, Strue, Sfalse,Selse, Dendif,
  Idrop, Idrop, Fdrop, Fdrop, Fdrop, Mpass, Eioccc,Eioccc,Eioccc,Eioccc,
  drop,  Eeof },

{ Itrue, Ifalse,Fpass, Ftrue, Ffalse,Dfalse,Dfalse,Dfalse,Delse, Dendif,
  Oiffy, Oiffy, Fpass, Oif,   Oif,   Eioccc,Eioccc,Eioccc,Eioccc,Eioccc,
  print, Eeof },

{ Itrue, Ifalse,Fpass, Ftrue, Ffalse,Pelif, Mtrue, Delif, Pelse, Pendif,
  Oiffy, Oiffy, Fpass, Oif,   Oif,   Pelif, Oelif, Oelif, Pelse, Pendif,
  print, Eeof },

{ Idrop, Idrop, Fdrop, Fdrop, Fdrop, Pelif, Mtrue, Delif, Pelse, Pendif,
  Idrop, Idrop, Fdrop, Fdrop, Fdrop, Eioccc,Eioccc,Eioccc,Eioccc,Eioccc,
  drop,  Eeof },

{ Itrue, Ifalse,Fpass, Ftrue, Ffalse,Melif, Melif, Melif, Melse, Pendif,
  Oiffy, Oiffy, Fpass, Oif,   Oif,   Eioccc,Eioccc,Eioccc,Eioccc,Pendif,
  print, Eeof },

{ Itrue, Ifalse,Fpass, Ftrue, Ffalse,Eelif, Eelif, Eelif, Eelse, Pendif,
  Oiffy, Oiffy, Fpass, Oif,   Oif,   Eelif, Eelif, Eelif, Eelse, Pendif,
  print, Eeof },

{ Idrop, Idrop, Fdrop, Fdrop, Fdrop, Eelif, Eelif, Eelif, Eelse, Dendif,
  Idrop, Idrop, Fdrop, Fdrop, Fdrop, Eelif, Eelif, Eelif, Eelse, Eioccc,
  drop,  Eeof },

{ Itrue, Ifalse,Fpass, Ftrue, Ffalse,Eelif, Eelif, Eelif, Eelse, Dendif,
  Oiffy, Oiffy, Fpass, Oif,   Oif,   Eelif, Eelif, Eelif, Eelse, Eioccc,
  print, Eeof },

{ Idrop, Idrop, Fdrop, Fdrop, Fdrop, Dfalse,Dfalse,Dfalse,Delse, Dendif,
  Idrop, Idrop, Fdrop, Fdrop, Fdrop, Dfalse,Dfalse,Dfalse,Delse, Eioccc,
  drop,  Eeof }

};


static void
done(void)
{
	if (incomment)
		error("EOF in comment");
	exit(exitstat);
}
static void
ignoreoff(void)
{
	if (depth == 0)
		abort(); 
	ignoring[depth] = ignoring[depth-1];
}
static void
ignoreon(void)
{
	ignoring[depth] = true;
}
static void
keywordedit(const char *replacement)
{
	size_t size = tline + sizeof(tline) - keyword;
	char *dst = keyword;
	const char *src = replacement;
	if (size != 0) {
		while ((--size != 0) && (*src != '\0'))
			*dst++ = *src++;
		*dst = '\0';
	}
	print();
}
static void
nest(void)
{
	depth += 1;
	if (depth >= MAXDEPTH)
		error("Too many levels of nesting");
	stifline[depth] = linenum;
}
static void
unnest(void)
{
	if (depth == 0)
		abort(); 
	depth -= 1;
}
static void
state(Ifstate is)
{
	ifstate[depth] = is;
}


static void
flushline(bool keep)
{
	if (symlist)
		return;
	if (keep ^ complement) {
		if (lnnum && delcount > 0)
			printf("#line %d\n", linenum);
		fputs(tline, stdout);
		delcount = 0;
	} else {
		if (lnblank)
			putc('\n', stdout);
		exitstat = 1;
		delcount += 1;
	}
}


static void
process(void)
{
	Linetype lineval;

	for (;;) {
		linenum++;
		lineval = get_line();
		trans_table[ifstate[depth]][lineval]();
		debug("process %s -> %s depth %d",
		    linetype_name[lineval],
		    ifstate_name[ifstate[depth]], depth);
	}
}


static Linetype
get_line(void)
{
	const char *cp;
	int cursym;
	int kwlen;
	Linetype retval;
	Comment_state wascomment;

	if (fgets(tline, MAXLINE, input) == NULL)
		return (LT_EOF);
	retval = LT_PLAIN;
	wascomment = incomment;
	cp = skipcomment(tline);
	if (linestate == LS_START) {
		if (*cp == '#') {
			linestate = LS_HASH;
			cp = skipcomment(cp + 1);
		} else if (*cp != '\0')
			linestate = LS_DIRTY;
	}
	if (!incomment && linestate == LS_HASH) {
		keyword = tline + (cp - tline);
		cp = skipsym(cp);
		kwlen = cp - keyword;
		
		if (strncmp(cp, "\\\n", 2) == 0)
			Eioccc();
		if (strlcmp("ifdef", keyword, kwlen) == 0 ||
		    strlcmp("ifndef", keyword, kwlen) == 0) {
			cp = skipcomment(cp);
			if ((cursym = findsym(cp)) < 0)
				retval = LT_IF;
			else {
				retval = (keyword[2] == 'n')
				    ? LT_FALSE : LT_TRUE;
				if (value[cursym] == NULL)
					retval = (retval == LT_TRUE)
					    ? LT_FALSE : LT_TRUE;
				if (ignore[cursym])
					retval = (retval == LT_TRUE)
					    ? LT_TRUEI : LT_FALSEI;
			}
			cp = skipsym(cp);
		} else if (strlcmp("if", keyword, kwlen) == 0)
			retval = ifeval(&cp);
		else if (strlcmp("elif", keyword, kwlen) == 0)
			retval = ifeval(&cp) - LT_IF + LT_ELIF;
		else if (strlcmp("else", keyword, kwlen) == 0)
			retval = LT_ELSE;
		else if (strlcmp("endif", keyword, kwlen) == 0)
			retval = LT_ENDIF;
		else {
			linestate = LS_DIRTY;
			retval = LT_PLAIN;
		}
		cp = skipcomment(cp);
		if (*cp != '\0') {
			linestate = LS_DIRTY;
			if (retval == LT_TRUE || retval == LT_FALSE ||
			    retval == LT_TRUEI || retval == LT_FALSEI)
				retval = LT_IF;
			if (retval == LT_ELTRUE || retval == LT_ELFALSE)
				retval = LT_ELIF;
		}
		if (retval != LT_PLAIN && (wascomment || incomment)) {
			retval += LT_DODGY;
			if (incomment)
				linestate = LS_DIRTY;
		}
		
		if (linestate == LS_HASH)
			abort(); 
	}
	if (linestate == LS_DIRTY) {
		while (*cp != '\0')
			cp = skipcomment(cp + 1);
	}
	debug("parser %s comment %s line",
	    comment_name[incomment], linestate_name[linestate]);
	return (retval);
}


static int op_lt(int a, int b) { return (a < b); }
static int op_gt(int a, int b) { return (a > b); }
static int op_le(int a, int b) { return (a <= b); }
static int op_ge(int a, int b) { return (a >= b); }
static int op_eq(int a, int b) { return (a == b); }
static int op_ne(int a, int b) { return (a != b); }
static int op_or(int a, int b) { return (a || b); }
static int op_and(int a, int b) { return (a && b); }


struct ops;

typedef Linetype eval_fn(const struct ops *, int *, const char **);

static eval_fn eval_table, eval_unary;


static const struct ops {
	eval_fn *inner;
	struct op {
		const char *str;
		int (*fn)(int, int);
	} op[5];
} eval_ops[] = {
	{ eval_table, { { "||", op_or } } },
	{ eval_table, { { "&&", op_and } } },
	{ eval_table, { { "==", op_eq },
			{ "!=", op_ne } } },
	{ eval_unary, { { "<=", op_le },
			{ ">=", op_ge },
			{ "<", op_lt },
			{ ">", op_gt } } }
};


static Linetype
eval_unary(const struct ops *ops, int *valp, const char **cpp)
{
	const char *cp;
	char *ep;
	int sym;

	cp = skipcomment(*cpp);
	if (*cp == '!') {
		debug("eval%d !", ops - eval_ops);
		cp++;
		if (eval_unary(ops, valp, &cp) == LT_IF) {
			*cpp = cp;
			return (LT_IF);
		}
		*valp = !*valp;
	} else if (*cp == '(') {
		cp++;
		debug("eval%d (", ops - eval_ops);
		if (eval_table(eval_ops, valp, &cp) == LT_IF)
			return (LT_IF);
		cp = skipcomment(cp);
		if (*cp++ != ')')
			return (LT_IF);
	} else if (isdigit((unsigned char)*cp)) {
		debug("eval%d number", ops - eval_ops);
		*valp = strtol(cp, &ep, 0);
		cp = skipsym(cp);
	} else if (strncmp(cp, "defined", 7) == 0 && endsym(cp[7])) {
		cp = skipcomment(cp+7);
		debug("eval%d defined", ops - eval_ops);
		if (*cp++ != '(')
			return (LT_IF);
		cp = skipcomment(cp);
		sym = findsym(cp);
		cp = skipsym(cp);
		cp = skipcomment(cp);
		if (*cp++ != ')')
			return (LT_IF);
		if (sym >= 0)
			*valp = (value[sym] != NULL);
		else {
			*cpp = cp;
			return (LT_IF);
		}
		keepthis = false;
	} else if (!endsym(*cp)) {
		debug("eval%d symbol", ops - eval_ops);
		sym = findsym(cp);
		if (sym < 0)
			return (LT_IF);
		if (value[sym] == NULL)
			*valp = 0;
		else {
			*valp = strtol(value[sym], &ep, 0);
			if (*ep != '\0' || ep == value[sym])
				return (LT_IF);
		}
		cp = skipsym(cp);
		keepthis = false;
	} else {
		debug("eval%d bad expr", ops - eval_ops);
		return (LT_IF);
	}

	*cpp = cp;
	debug("eval%d = %d", ops - eval_ops, *valp);
	return (*valp ? LT_TRUE : LT_FALSE);
}


static Linetype
eval_table(const struct ops *ops, int *valp, const char **cpp)
{
	const struct op *op;
	const char *cp;
	int val;
	Linetype lhs, rhs;

	debug("eval%d", ops - eval_ops);
	cp = *cpp;
	lhs = ops->inner(ops+1, valp, &cp);
	for (;;) {
		cp = skipcomment(cp);
		for (op = ops->op; op->str != NULL; op++)
			if (strncmp(cp, op->str, strlen(op->str)) == 0)
				break;
		if (op->str == NULL)
			break;
		cp += strlen(op->str);
		debug("eval%d %s", ops - eval_ops, op->str);
		rhs = ops->inner(ops+1, &val, &cp);
		if (op->fn == op_and && (lhs == LT_FALSE || rhs == LT_FALSE)) {
			debug("eval%d: and always false", ops - eval_ops);
			if (lhs == LT_IF)
				*valp = val;
			lhs = LT_FALSE;
			continue;
		}
		if (op->fn == op_or && (lhs == LT_TRUE || rhs == LT_TRUE)) {
			debug("eval%d: or always true", ops - eval_ops);
			if (lhs == LT_IF)
				*valp = val;
			lhs = LT_TRUE;
			continue;
		}
		if (rhs == LT_IF)
			lhs = LT_IF;
		if (lhs != LT_IF)
			*valp = op->fn(*valp, val);
	}

	*cpp = cp;
	debug("eval%d = %d", ops - eval_ops, *valp);
	if (lhs != LT_IF)
		lhs = (*valp ? LT_TRUE : LT_FALSE);
	return lhs;
}


static Linetype
ifeval(const char **cpp)
{
	const char *cp = *cpp;
	int ret;
	int val;

	debug("eval %s", *cpp);
	keepthis = killconsts ? false : true;
	ret = eval_table(eval_ops, &val, &cp);
	if (ret != LT_IF)
		*cpp = cp;
	debug("eval = %d", val);
	return (keepthis ? LT_IF : ret);
}


static const char *
skipcomment(const char *cp)
{
	if (text || ignoring[depth]) {
		for (; isspace((unsigned char)*cp); cp++)
			if (*cp == '\n')
				linestate = LS_START;
		return (cp);
	}
	while (*cp != '\0')
		
		if (strncmp(cp, "\\\n", 2) == 0)
			cp += 2;
		else switch (incomment) {
		case NO_COMMENT:
			if (strncmp(cp, "/\\\n", 3) == 0) {
				incomment = STARTING_COMMENT;
				cp += 3;
			} else if (strncmp(cp, "/*", 2) == 0) {
				incomment = C_COMMENT;
				cp += 2;
			} else if (strncmp(cp, "//", 2) == 0) {
				incomment = CXX_COMMENT;
				cp += 2;
			} else if (strncmp(cp, "\'", 1) == 0) {
				incomment = CHAR_LITERAL;
				linestate = LS_DIRTY;
				cp += 1;
			} else if (strncmp(cp, "\"", 1) == 0) {
				incomment = STRING_LITERAL;
				linestate = LS_DIRTY;
				cp += 1;
			} else if (strncmp(cp, "\n", 1) == 0) {
				linestate = LS_START;
				cp += 1;
			} else if (strchr(" \t", *cp) != NULL) {
				cp += 1;
			} else
				return (cp);
			continue;
		case CXX_COMMENT:
			if (strncmp(cp, "\n", 1) == 0) {
				incomment = NO_COMMENT;
				linestate = LS_START;
			}
			cp += 1;
			continue;
		case CHAR_LITERAL:
		case STRING_LITERAL:
			if ((incomment == CHAR_LITERAL && cp[0] == '\'') ||
			    (incomment == STRING_LITERAL && cp[0] == '\"')) {
				incomment = NO_COMMENT;
				cp += 1;
			} else if (cp[0] == '\\') {
				if (cp[1] == '\0')
					cp += 1;
				else
					cp += 2;
			} else if (strncmp(cp, "\n", 1) == 0) {
				if (incomment == CHAR_LITERAL)
					error("unterminated char literal");
				else
					error("unterminated string literal");
			} else
				cp += 1;
			continue;
		case C_COMMENT:
			if (strncmp(cp, "*\\\n", 3) == 0) {
				incomment = FINISHING_COMMENT;
				cp += 3;
			} else if (strncmp(cp, "*/", 2) == 0) {
				incomment = NO_COMMENT;
				cp += 2;
			} else
				cp += 1;
			continue;
		case STARTING_COMMENT:
			if (*cp == '*') {
				incomment = C_COMMENT;
				cp += 1;
			} else if (*cp == '/') {
				incomment = CXX_COMMENT;
				cp += 1;
			} else {
				incomment = NO_COMMENT;
				linestate = LS_DIRTY;
			}
			continue;
		case FINISHING_COMMENT:
			if (*cp == '/') {
				incomment = NO_COMMENT;
				cp += 1;
			} else
				incomment = C_COMMENT;
			continue;
		default:
			abort(); 
		}
	return (cp);
}


static const char *
skipsym(const char *cp)
{
	while (!endsym(*cp))
		++cp;
	return (cp);
}


static int
findsym(const char *str)
{
	const char *cp;
	int symind;

	cp = skipsym(str);
	if (cp == str)
		return (-1);
	if (symlist) {
		printf("%.*s\n", (int)(cp-str), str);
		
		return (0);
	}
	for (symind = 0; symind < nsyms; ++symind) {
		if (strlcmp(symname[symind], str, cp-str) == 0) {
			debug("findsym %s %s", symname[symind],
			    value[symind] ? value[symind] : "");
			return (symind);
		}
	}
	return (-1);
}


static void
addsym(bool ignorethis, bool definethis, char *sym)
{
	int symind;
	char *val;

	symind = findsym(sym);
	if (symind < 0) {
		if (nsyms >= MAXSYMS)
			errx(2, "too many symbols");
		symind = nsyms++;
	}
	symname[symind] = sym;
	ignore[symind] = ignorethis;
	val = sym + (skipsym(sym) - sym);
	if (definethis) {
		if (*val == '=') {
			value[symind] = val+1;
			*val = '\0';
		} else if (*val == '\0')
			value[symind] = "";
		else
			usage();
	} else {
		if (*val != '\0')
			usage();
		value[symind] = NULL;
	}
}


static int
strlcmp(const char *s, const char *t, size_t n)
{
	while (n-- && *t != '\0')
		if (*s != *t)
			return ((unsigned char)*s - (unsigned char)*t);
		else
			++s, ++t;
	return ((unsigned char)*s);
}


static void
debug(const char *msg, ...)
{
	va_list ap;

	if (debugging) {
		va_start(ap, msg);
		vwarnx(msg, ap);
		va_end(ap);
	}
}

static void
error(const char *msg)
{
	if (depth == 0)
		warnx("%s: %d: %s", filename, linenum, msg);
	else
		warnx("%s: %d: %s (#if line %d depth %d)",
		    filename, linenum, msg, stifline[depth], depth);
	errx(2, "output may be truncated");
}
