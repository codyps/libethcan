#include <stdio.h>
#include <stdlib.h>

typedef char    atom_t;
typedef struct sexpr_t sexpr_t;
typedef sexpr_t slist_t;

typedef struct term_t {
	int type;
	union {
		atom_t  *atom;
		sexpr_t *sexpr;
	};
	unsigned lineno;
} term_t;
typedef struct sexpr_t {
	struct sexpr_t *next;
	struct term_t  term;
} sexpr_t;

enum {
	ATOM,
	SEXPR
};

#define ERROR_EXIT(...) do {		\
	ERROR(__VA_ARGS__);		\
	exit(1);			\
} while(0)

#define ERROR_AT_LINE(line, charno, ...) do {		\
	fflush(stdout);					\
	fprintf(stderr, "%zu,%zu: ", line, charno);	\
	fprintf(stderr, __VA_ARGS__);			\
	putc('\n', stderr);				\
	fflush(stderr);					\
} while(0)

#define ERROR(...) do {					\
	fflush(stdout);					\
	fprintf(stderr, "[%s:%zu]: ", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);			\
	putc('\n', stderr);				\
	fflush(stderr);					\
} while(0)

void *xmalloc(size_t s)
{
	void *x = malloc(s);
	if (x == NULL)
		ERROR_EXIT("allocation of %zu bytes failed.", s);
	return x;
}

void *xrealloc(void *x, size_t s)
{
	void *n = realloc(x, s);
	if (n == NULL)
		ERROR_EXIT("allocation of %zu bytes failed.", s);
	return n;
}


/*
term  := ATOM
       | sexpr
sexpr := '('  slist ')'
slist := ws
       | ws term slist ws
*/

static size_t lineno, charno;

int next_token(FILE *f)
{
	int c;
	for(;;) {
		c = getc(f);
		charno++;

		if (c == EOF)
			return c;

		if (c == '\n') {
			charno = 0;
			lineno++;
		} else if (!(c == ' ' || c == '\t')) {
			return c;
		}
	}
}


void push_token(int c, FILE *f)
{
	ungetc(c, f);
}

int peek_token(FILE *f)
{
	int c = next_token(f);
	push_token(c, f);
	return c;
}

#define INIT_ATOM_SZ 22
atom_t  *read_atom(FILE *f)
{
	char  *p = xmalloc(INIT_ATOM_SZ);
	size_t l = 0, m = INIT_ATOM_SZ;

	for (;;) {
		int c = getc(f);
		if (c == ' ' || c == ')' || c == '\n' || c == '\t') {
			ungetc(c, f);
			p[l] = '\0';
			return p;
		}

		if (l + 2 > m) {
			m *= 2;
			p = xrealloc(p, m);
		}

		p[l] = c;
		l++;
	}
}

sexpr_t *read_sexpr(FILE *f);
int  read_term(FILE *f, term_t *res)
{
	int c = next_token(f);

	if (c == '(') {
		push_token(c, f);
		res->type  = SEXPR;
		res->sexpr = read_sexpr(f);
		res->lineno = lineno;
	} else if (c == ')') {
		push_token(c, f);
		return 1;
	} else {
		push_token(c, f);
		res->type   = ATOM;
		res->atom   = read_atom(f);
		res->lineno = lineno;
	}

	return 0;
}

slist_t *read_slist(FILE *f)
{
	slist_t *res = xmalloc(sizeof(*res));
	int r = read_term(f, &res->term);

	if (r == 1) {
		free(res);
		return NULL;
	}

	res->next = read_slist(f);
	return res;
}

sexpr_t *read_sexpr(FILE *f)
{
	int c = next_token(f);
	if (c != '(') {
		ERROR_AT_LINE(lineno, charno, "sexpr did not start with '(', has '%c'", c);
		return NULL;
	}

	sexpr_t *s = read_slist(f);

	c = next_token(f);
	if (c != ')') {
		ERROR_AT_LINE(lineno, charno, "sexpr lacks ')', has '%c'", c);
		return NULL;
	}

	return s;
}

void print_sexpr(sexpr_t *s, FILE *f);
void print_term(term_t *t, FILE *f)
{
	if (t->type == ATOM) {
		fprintf(f, "<%s>", t->atom);
	} else if (t->type == SEXPR) {
		print_sexpr(t->sexpr, f);
	} else {
		ERROR("unk term type.");
	}
}

void print_sexpr(sexpr_t *s, FILE *f)
{
	putc('[', f);
	while(s) {
		print_term(&s->term, f);
		s = s->next;
	}
	putc(']', f);
}

void free_sexpr(sexpr_t *s);
void free_term(term_t *t)
{
	if (t->type == ATOM)
		free(t->atom);
	else if (t->type == SEXPR)
		free_sexpr(t->sexpr);
}

void free_sexpr(sexpr_t *s)
{
	while(s) {
		sexpr_t *t = s->next;
		free_term(&s->term);
		free(s);
		s = t;
	}
}

int main(int argc, char **argv)
{
	sexpr_t *s = read_sexpr(stdin);
	print_sexpr(s, stdout);
	putc('\n', stdout);
	free_sexpr(s);
	return 0;
}
