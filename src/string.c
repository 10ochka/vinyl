
/***************************************************************************************/

#include "string.h"
#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

/***************************************************************************************/

static size_t round8(size_t x) {
    const int round_to = 8;
    return (x + (round_to - 1)) & ~(round_to - 1);
}

/***************************************************************************************/

void vnl_strbuf_reserve(Vnl_StringBuffer *self, size_t size) {
	if (self->len + size <= self->cap) {
		return;
	}
	size_t newcap = round8(self->len + size);
	vnl_strbuf_reserve_exact(self, newcap);
}

void vnl_strbuf_reserve_exact(Vnl_StringBuffer *self, size_t size) {
	size = round8(size);
	if (self->cap >= size) {
		return;
	}
	self->chars = vnl_realloc(self->chars, size * sizeof(char));
	self->cap = size;
}

void vnl_strbuf_append_s(Vnl_StringBuffer *self, Vnl_String other) {
	vnl_strbuf_reserve(self, other.len);
	memcpy(self->chars + self->len, other.chars, other.len);
	self->len += other.len;
}

void vnl_strbuf_append_c(Vnl_StringBuffer *self, Vnl_CString cstr) {
	vnl_strbuf_append_s(self, vnl_string_from_c(cstr));
}

void vnl_strbuf_free(Vnl_StringBuffer *self) {
	vnl_free(self->chars);
	self->cap = 0;
	self->len = 0;
}

/***************************************************************************************/

Vnl_String vnl_string_from_c(Vnl_CString cstr) {
	return (Vnl_String){ cstr, strlen(cstr) };
}

Vnl_String vnl_string_from_b(const Vnl_StringBuffer *sb) {
	return (Vnl_String){ sb->chars, sb->len };
}

Vnl_String vnl_string_from_f(const Vnl_FixedString *fstr) {
	return (Vnl_String){ fstr->chars, fstr->len };
}

/***************************************************************************************/

bool vnl_string_cmpeq_s(Vnl_String self, Vnl_String other) {
	return (self.len == other.len) && vnl_memcmpeq(self.chars, other.chars, self.len);
}

bool vnl_string_cmpeq_c(Vnl_String self, Vnl_CString other) {
	return vnl_string_cmpeq_s(self, vnl_string_from_c(other));
}

/***************************************************************************************/

bool vnl_string_hasprefix_s(Vnl_String self, Vnl_String prefix) {
    if (self.len < prefix.len) {
        return false;
    }
    return vnl_memcmpeq(self.chars, prefix.chars, prefix.len);
}

bool vnl_string_hasprefix_c(Vnl_String self, Vnl_CString prefix) {
    return vnl_string_hasprefix_s(self, vnl_string_from_c(prefix));
}

/***************************************************************************************/

bool vnl_string_hassuffix_c(Vnl_String self, Vnl_CString suffix) {
	return vnl_string_hassuffix_s(self, vnl_string_from_c(suffix));
}

bool vnl_string_hassuffix_s(Vnl_String self, Vnl_String suffix) {
    if (self.len < suffix.len) {
        return false;
    }
    return vnl_memcmpeq(self.chars + self.len - suffix.len, suffix.chars, suffix.len);
}

/***************************************************************************************/

Vnl_String vnl_string_remprefix_s(Vnl_String self, Vnl_String prefix) {
	if (vnl_string_hasprefix_s(self, prefix)) {
		return (Vnl_String) { self.chars + prefix.len, self.len - prefix.len };
	}
	return self;
}

Vnl_String vnl_string_remprefix_c(Vnl_String self, Vnl_CString prefix) {
	return vnl_string_remprefix_s(self, vnl_string_from_c(prefix));
}

/***************************************************************************************/

Vnl_String vnl_string_remsuffix_s(Vnl_String self, Vnl_String suffix) {
	if (vnl_string_hassuffix_s(self, suffix)) {
		return (Vnl_String) { self.chars , self.len - suffix.len };
	}
	return self;
}

Vnl_String vnl_string_remsuffix_c(Vnl_String self, Vnl_CString suffix) {
	return vnl_string_remsuffix_s(self, vnl_string_from_c(suffix));
}

/***************************************************************************************/

Vnl_String vnl_string_lshift(Vnl_String self) {
	if (self.len == 0) {
		return self;
	}
	return (Vnl_String){ self.chars + 1, self.len - 1 };
}

Vnl_String vnl_string_lshiftn(Vnl_String self, size_t n) {
	if (self.len <= n) {
		return (Vnl_String){ nullptr, 0 };
	}
	return (Vnl_String){ self.chars + n, self.len - n };
}

/***************************************************************************************/

Vnl_String vnl_string_rshift(Vnl_String self) {
	if (self.len == 0) {
		return self;
	}
	return (Vnl_String){ self.chars, self.len - 1 };
}

Vnl_String vnl_string_rshiftn(Vnl_String self, size_t n) {
	if (self.len <= n) {
		return (Vnl_String){ nullptr, 0 };
	}
	return (Vnl_String){ self.chars, self.len - n };
}

/***************************************************************************************/

Vnl_String vnl_string_ltrim(Vnl_String self) {
	while (self.len && strchr(" \t\n\v\f\r", *self.chars)) {
        self = vnl_string_lshift(self);
    }
    return self;
}

Vnl_String vnl_string_rtrim(Vnl_String self) {
	while (self.len && strchr(" \t\n\v\f\r", self.chars[self.len-1])) {
        self = vnl_string_rshift(self);
    }
    return self;
}

Vnl_String vnl_string_trim(Vnl_String self) {
	return vnl_string_rtrim(vnl_string_ltrim(self));
}

/***************************************************************************************/

void vnl_string_print(Vnl_String self) {
	vnl_string_fprint(self, stdout);
}

void vnl_string_println(Vnl_String self) {
	vnl_string_fprint(self, stdout);
	fputs("", stdout);
}

void vnl_string_fprint(Vnl_String self, FILE *fp) {
	for (size_t off = 0; off < self.len;) {
		int l = (self.len - off) > INT_MAX ? INT_MAX : (self.len - off);
		fprintf(fp, "%.*s", l, self.chars + off);
		off += l;
	}
}

void vnl_string_fprintln(Vnl_String self, FILE *fp) {
	vnl_string_fprint(self, fp);
	fputs("", fp);
}

void vnl_string_print_escaped(Vnl_String self) {
	vnl_string_fprint_escaped(self, stdout);
}

void vnl_string_println_escaped(Vnl_String self) {
	vnl_string_print_escaped(self);
	puts("");
}

void vnl_string_fprint_escaped(Vnl_String self, FILE *fp) {
	fputc('"', fp);
	for (size_t i = 0; i < self.len; ++i) {
		char c = self.chars[i];
		switch (c) {
			case '\a':
				fputs("\\t", fp);
			break;

			case '\n':
				fputs("\\n", fp);
			break;

			case '\t':
				fputs("\\t", fp);
			break;

			case '\b':
				fputs("\\b", fp);
			break;

			case '\r':
				fputs("\\r", fp);
			break;

			case '\f':
				fputs("\\f", fp);
			break;

			case '"':
				fputs("\\\"", fp);
			break;

			case '\\':
				fputs("\\\\", fp);
			break;

			default:
				fputc(c, fp);
			break;
		}
	}
	fputc('"', fp);
}

void vnl_string_fprintln_escaped(Vnl_String self, FILE *fp) {
	vnl_string_fprint_escaped(self, fp);
	fputs("", fp);
}

/***************************************************************************************/

Vnl_FixedString vnl_fixstr_from_s(Vnl_String str) {
	if (str.len == 0) {
		return (Vnl_FixedString){ nullptr, 0 };
	}
	char *p = vnl_malloc(str.len);
	memcpy(p, str.chars, str.len);
	return (Vnl_FixedString){
		.chars = p,
		.len = str.len
	};
}

Vnl_FixedString vnl_fixstr_from_c(Vnl_CString cstr) {
	return vnl_fixstr_from_s(vnl_string_from_c(cstr));
}

void vnl_fixstr_free(Vnl_FixedString *self) {
	void *p = (void *)self->chars;
	vnl_free(p);
}
