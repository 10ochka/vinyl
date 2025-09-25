
#ifndef __VINYL_STRING_H__
#define __VINYL_STRING_H__

#include "common.h"
#include <stddef.h>
#include <stdio.h>

typedef const char *Vnl_CString;
typedef struct Vnl_String Vnl_String;
typedef struct Vnl_StringBuffer Vnl_StringBuffer;

struct Vnl_String {
	const char *chars;
	size_t len;
};

struct Vnl_StringBuffer {
	char *chars;
	size_t len;
	size_t cap;
};


void strbuf_reserve(Vnl_StringBuffer *, size_t);
void strbuf_reserve_exact(Vnl_StringBuffer *, size_t);
void strbuf_append_s(Vnl_StringBuffer *, Vnl_String);
void strbuf_append_c(Vnl_StringBuffer *, Vnl_CString);
void strbuf_append_sb(Vnl_StringBuffer *, const Vnl_StringBuffer *);
void strbuf_free(Vnl_StringBuffer *);
Vnl_String strbuf_string(const Vnl_StringBuffer *);


Vnl_String str_from_cstr(Vnl_CString);

bool string_cmpeq_c(Vnl_String, Vnl_CString);
bool string_cmpeq_s(Vnl_String, Vnl_String);

bool string_hasprefix_c(Vnl_String, Vnl_CString);
bool string_hassuffix_c(Vnl_String, Vnl_CString);
bool string_hasprefix_s(Vnl_String, Vnl_String);
bool string_hassuffix_s(Vnl_String, Vnl_String);

Vnl_String string_remprefix_c(Vnl_String, Vnl_CString);
Vnl_String string_remsuffix_c(Vnl_String, Vnl_CString);
Vnl_String string_remprefix_s(Vnl_String, Vnl_String);
Vnl_String string_remsuffix_s(Vnl_String, Vnl_String);

Vnl_String string_lshift(Vnl_String);
Vnl_String string_rshift(Vnl_String);
Vnl_String string_lshiftn(Vnl_String, size_t);
Vnl_String string_rshiftn(Vnl_String, size_t);

Vnl_String string_ltrim(Vnl_String);
Vnl_String string_rtrim(Vnl_String);
Vnl_String string_trim(Vnl_String);


void string_print(Vnl_String);
void string_println(Vnl_String);
void string_fprint(Vnl_String, FILE *);
void string_fprintln(Vnl_String, FILE *);
void string_print_escaped(Vnl_String);
void string_println_escaped(Vnl_String);
void string_fprint_escaped(Vnl_String, FILE *);
void string_fprintln_escaped(Vnl_String, FILE *);



#endif //__VINYL_STRING_H__
