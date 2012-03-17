#include <stdio.h>
#include <stdlib.h>
#include "list.h"

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
	fprintf(stderr, "[%s:%d]: ", __FILE__, __LINE__);	\
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

#define INIT_ATOM_SZ 22
char  *read_atom(FILE *f, size_t *len)
{
	size_t l = 0, m = INIT_ATOM_SZ;
	char  *p = xmalloc(m);

	for (;;) {
		int c = getc(f);
		if (c == ' ' || c == ')' || c == '}' || c == '\n' || c == '\t') {
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


#define STACK_DEF(name, type) struct name##_stack { size_t p, m; type *s; }
#define STACK_INIT(it, start_val)  ({					\
		int __r__ = 0;						\
		(it)->p = 0; (it)->m = (start_val);			\
		(it)->s = malloc(sizeof(*((it)->s)) * (it)->m);		\
		if (!(it)->s)						\
			__r__ = -1;					\
		__r__;							\
	})

#define _STACK_PUSH(it, val) do {		\
			(it)->s[(it)->p] = val;	\
			(it)->p ++;		\
	} while(0)
#define STACK_PUSH(it, val) ({									\
		int __r__ = 0;									\
		if ((it)->p < (it)->m) {							\
			_STACK_PUSH(it, val);							\
		} else {									\
			size_t __nm__ = (it)->m * 2;						\
			typeof((it)->s) __ns__ = realloc((it)->s, sizeof(*((it)->s) * __nm__));	\
			if (!__ns__)								\
				__r__ = -1;							\
			else {									\
				(it)->s = __ns__;						\
				(it)->m = __nm__;						\
				_STACK_PUSH(it, val);						\
			}									\
		}										\
		__r__;										\
	})

#define STACK_HAS_ELEM(it) (!!(it)->p)

#define _STACK_DISCARD(it) ((it)->p--)
#define _STACK_POP(it) ( (it)->s[--((it)->p)] )
#define _STACK_PEEK(it) ( (it)->s[((it)->p)-1] )

struct sexpr_callbacks {
	int (*start_list)(void *ctx);
	int (*end_list)(void *ctx);
	int (*item)(void *ctx, const char *item_val, size_t item_len);
};

#define EAL(...) ERROR_AT_LINE(lineno, charno, __VA_ARGS__)
#define NT()	 token = next_token(f)
int read_sexpr(struct sexpr_callbacks *c, void *ctx, FILE *f)
{
	enum e_s {
		P_SEXPR,
		P_SLIST,
		P_ATOM,
		P_CLOSE_SEXPR
	};

	STACK_DEF(x, enum e_s) _ss, *ss = &_ss;
	STACK_INIT(ss, 20);
	STACK_PUSH(ss, P_SEXPR);

	int token = next_token(f);

	do {
		switch(_STACK_POP(ss)) {
		case P_SEXPR:
			if (token != '(') {
				EAL("sexpr did not start with '(', has '%c'", token);
				return -1;
			}
			c->start_list(ctx);
			STACK_PUSH(ss, P_CLOSE_SEXPR);
			STACK_PUSH(ss, P_SLIST);
			NT();
			break;
		case P_SLIST:
			if (token == '(') {
				STACK_PUSH(ss, P_SLIST);
				STACK_PUSH(ss, P_SEXPR);
			} else if (token == ')') {
				/* nothing */
			} else {
				STACK_PUSH(ss, P_SLIST);
				STACK_PUSH(ss, P_ATOM);
			}
			break;
		case P_ATOM: {
			size_t len;
			char *a = read_atom(f, &len);
			if (!a) {
				EAL("allocation failed.");
				return -1;
			}
			c->item(ctx, a, len);
		} break;
		case P_CLOSE_SEXPR:
			if (token != ')') {
				EAL("sexpr did not end with ')', has '%c'", token);
				return -1;
			}
			NT();
			c->end_list(ctx);
		}
	} while(STACK_HAS_ELEM(ss));

	return 0;
}



#define _h(...) do {			\
	unsigned i;			\
	for (i = 0; i < fe->h_indent; i++) putc('\t', fe->h);	\
	fprintf(fe->h, __VA_ARGS__);	\
	putc('\n', fe->h);			\
} while(0)
#define _c(...) do {			\
	unsigned i;			\
	for (i = 0; i < c_indent; i++) putc('\t', fe->c);	\
	fprintf(fe->c, __VA_ARGS__);	\
	putc('\n', fe->c);			\
} while(0)

#define _b(...) do {	\
	_c(__VA_ARGS__);\
	_h(__VA_ARGS__);\
} while(0)

#define _h_in() (fe->h_indent ++)
#define _h_out() (fe->h_indent --)
#define _c_in() (fe->c_indent++)
#define _c_out() (fe->c_indent--)
#define _b_in() do { _h_in(); _c_in(); } while(0)
#define _b_out() do { _h_out(); _c_out() } while(0)

#define BPGEN_VER_STR "bpgen-[i have to set this up]"

typedef struct file_emitter {
	FILE *h, c;
	unsigned h_indent, c_indent;
} fe_t;

int g_enum_term(fe_t *fe, char *proto_name, char *enum_name, sexpr_t *et)
{
	if (!is_atom(fst(et)))
		ERROR_EXIT("enum must begin with name");

	char *e_name = as_atom(fst(et)).val;

	if (has_snd(et) && is_atom(snd(et))) {
		_h("%s = %s,", e_name, snd(et).val);
	} else {
		_h("%s,", e_name);
	}

	return 0;
}


int g_struct_member(fe_t *fe, char *proto_name, char *struct_name, sexpr_t *sm)
{

}

int g_enum(fe_t *fe, char *proto_name, sexpr_t *ep)
{
	/* ep = (enum <name> (<elem 1> [elem 1 val]) (<elem 2> [elem2 val]) ... ) */
	if (!sexpr_snd_term_is_atom(ep))
		ERROR_EXIT("`enum` needs name");

	char *enum_name = sexpr_snd_term_as_atom(ep);
	_h("enum %s_%s {", proto_name, enum_name);
	_h_in();
	term_t *se;
	sexpr_for_each_term(se, ep->next->next) {
		g_enum_term(fe, proto_name, enum_name, se);
	}

	_h_out();
	_h("}");
}

int g_struct(fe_t *fe, char *proto_name, sexpr_t *ep)
{
	if (!len_eq(ep, 3))
		ERROR_EXIT("struct needs 3 terms: (struct <name> <field-list>)");



}

int generate_proto(sexpr_t *p)
{

	unsigned c_indent = 0, h_indent = 0;

	if (!sexpr_fst_atom_has_value(p, "proto"))
		ERROR_EXIT("protocol must begin with `proto` definition");

	if (!sexpr_snd_term_is_atom(p))
		ERROR_EXIT("protocol needs name");

	char *proto_name = sexpr_snd_term_as_atom(p).val;

	DEBUG("Generating protocol \"%s\"", proto_name);

	char *header_name = asprintf("%s.h", proto_name);
	char *source_name = asprintf("%s.c", proto_name);

	FILE *h = fopen(header_name, "w");
	FILE *c = fopen(source_name, "w");


	struct file_emitter _fe = {
		.h = h, .c = h
	}, *fe = &_fe;

	_h("#ifndef _%s_H", proto_name);
	_h("#define _%s_H", proto_name);

	_b("/* Generated by " BPGEN_VER_STR " */");

	term_t *s;
	sexpr_for_each_term(s, p->next->next) {
		if (term_is_atom(s))
			ERROR_EXIT("don't know how to handle atom, expected a sexpr '(enum ...)' or '(struct ...)'");

		sexpr_t *thing = term_as_sexpr(s);

		if (!sexpr_fst_term_is_atom(thing))
			ERROR_EXIT("Need `enum` or `struct`");

		if (sexpr_fst_term_as_atom_has_value(thing, "enum")) {
			g_enum(fe, proto_name, thing);
		} else if (sexpr_fst_term_as_atom_has_value(thing, "struct")) {
			g_struct(fe, proto_name, thing);
		}
	}


	_h("#endif /* _%s_H */", proto_name);
}

struct proto {
	char *name;
	struct list_head struct_list;
	struct list_head enum_list;
};

struct proto_struct {
	struct list_head l;
	char *name;
	struct list_head members;
};

struct proto_enum {
	struct list_head l;
	char *name;
	struct list_head evals;
};

enum pr_state {
	S_INIT,
	S_PROTO,
	S_STRUCT,
	S_ENUM,
	S_,
	S_
};

struct cb_state {
	struct proto p;
	STACK_DEF(z, struct list_head) head_stack;
	STACK_DEF(w, enum pr_state)    st_stack;
};

int main(int argc, char **argv)
{
	struct sexpr_callbacks cb = {
		.start_list = bp_sl,
		.end_list   = bp_el,
		.item       = bp_item
	};

	sexpr_t *s = read_sexpr(&cb, stdin);
	print_sexpr(s, stdout, 0);
	putc('\n', stdout);
	free_sexpr(s);
	return 0;
}
