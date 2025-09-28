
#ifndef __VINYL_STRING_H__
#define __VINYL_STRING_H__

#include "common.h"


typedef const char *Vnl_CString;
typedef struct Vnl_String Vnl_String;
typedef struct Vnl_StringBuffer Vnl_StringBuffer;
typedef struct Vnl_FixedString Vnl_FixedString;

struct Vnl_String {
	const char *chars;
	size_t len;
};

struct Vnl_StringBuffer {
	char *chars;
	size_t len;
	size_t cap;
};

struct Vnl_FixedString {
	const char * const chars;
	const size_t len;
};


void vnl_strbuf_reserve(Vnl_StringBuffer *, size_t);
void vnl_strbuf_reserve_exact(Vnl_StringBuffer *, size_t);
void vnl_strbuf_append_s(Vnl_StringBuffer *, Vnl_String);
void vnl_strbuf_append_c(Vnl_StringBuffer *, Vnl_CString);
void vnl_strbuf_free(Vnl_StringBuffer *);

Vnl_String vnl_string_from_c(Vnl_CString);
Vnl_String vnl_string_from_b(const Vnl_StringBuffer *);
Vnl_String vnl_string_from_f(const Vnl_FixedString *);

bool vnl_string_cmpeq_s(Vnl_String, Vnl_String);
bool vnl_string_cmpeq_c(Vnl_String, Vnl_CString);

bool vnl_string_hasprefix_s(Vnl_String, Vnl_String);
bool vnl_string_hasprefix_c(Vnl_String, Vnl_CString);

bool vnl_string_hassuffix_s(Vnl_String, Vnl_String);
bool vnl_string_hassuffix_c(Vnl_String, Vnl_CString);

Vnl_String vnl_string_remprefix_s(Vnl_String, Vnl_String);
Vnl_String vnl_string_remprefix_c(Vnl_String, Vnl_CString);

Vnl_String vnl_string_remsuffix_s(Vnl_String, Vnl_String);
Vnl_String vnl_string_remsuffix_c(Vnl_String, Vnl_CString);

Vnl_String vnl_string_lshift(Vnl_String);
Vnl_String vnl_string_lshiftn(Vnl_String, size_t);

Vnl_String vnl_string_rshift(Vnl_String);
Vnl_String vnl_string_rshiftn(Vnl_String, size_t);

Vnl_String vnl_string_ltrim(Vnl_String);
Vnl_String vnl_string_rtrim(Vnl_String);
Vnl_String vnl_string_trim(Vnl_String);

void vnl_string_print(Vnl_String);
void vnl_string_println(Vnl_String);
void vnl_string_fprint(Vnl_String, FILE *);
void vnl_string_fprintln(Vnl_String, FILE *);
void vnl_string_print_escaped(Vnl_String);
void vnl_string_println_escaped(Vnl_String);
void vnl_string_fprint_escaped(Vnl_String, FILE *);
void vnl_string_fprintln_escaped(Vnl_String, FILE *);

Vnl_FixedString vnl_fixstr_from_s(Vnl_String);
Vnl_FixedString vnl_fixstr_from_c(Vnl_CString);
void vnl_fixstr_free(Vnl_FixedString *);

#endif //__VINYL_STRING_H__
