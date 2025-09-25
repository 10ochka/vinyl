#include "string.h"
#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


size_t round8(size_t x) {
    const int round_to = 8;
    return (x + (round_to - 1)) & ~(round_to - 1);
}

void strbuf_reserve(Vnl_StringBuffer *self, size_t size) {
	if (self->len + size <= self->cap) {
		return;
	}
	size_t newcap = round8(self->len + size);
	strbuf_reserve_exact(self, newcap);
}

void strbuf_reserve_exact(Vnl_StringBuffer *self, size_t size) {
	size = round8(size);
	if (self->cap >= size) {
		return;
	}
	self->chars = vnl_realloc(self->chars, size * sizeof(char));
	self->cap = size;
}

void strbuf_append_s(Vnl_StringBuffer *self, Vnl_String other) {
	strbuf_reserve(self, other.len);
	memcpy(self->chars + self->len, other.chars, other.len);
	self->len += other.len;
}

void strbuf_append_c(Vnl_StringBuffer *self, Vnl_CString cstr) {
	strbuf_append_s(self, str_from_cstr(cstr));
}

void strbuf_append_sb(Vnl_StringBuffer *self, const Vnl_StringBuffer *other) {
	strbuf_append_s(self, strbuf_string(other));
}

void strbuf_free(Vnl_StringBuffer *self) {
	vnl_free(self->chars);
	self->cap = 0;
	self->len = 0;
}

Vnl_String strbuf_string(const Vnl_StringBuffer *self) {
	return (Vnl_String){ self->chars, self->len };
}

Vnl_String str_from_cstr(Vnl_CString cstr) {
	return (Vnl_String){ cstr, strlen(cstr) };
}

bool string_cmpeq_c(Vnl_String self, Vnl_CString other) {
	return string_cmpeq_s(self, str_from_cstr(other));
}

bool string_cmpeq_s(Vnl_String self, Vnl_String other) {
	return (self.len == other.len) && memcmp(self.chars, other.chars, self.len) == 0;
}

bool string_hasprefix_c(Vnl_String self, Vnl_CString prefix) {
    return string_hasprefix_s(self, str_from_cstr(prefix));
}

bool string_hassuffix_c(Vnl_String self, Vnl_CString suffix) {
	return string_hassuffix_s(self, str_from_cstr(suffix));
}

bool string_hasprefix_s(Vnl_String self, Vnl_String prefix) {
    if (self.len < prefix.len) {
        return false;
    }
    return vnl_memcmpeq(self.chars, prefix.chars, prefix.len);
}

bool string_hassuffix_s(Vnl_String self, Vnl_String suffix) {
    if (self.len < suffix.len) {
        return false;
    }
    return vnl_memcmpeq(self.chars + self.len - suffix.len, suffix.chars, suffix.len);
}

Vnl_String string_remprefix_c(Vnl_String self, Vnl_CString prefix) {
	return string_remprefix_s(self, str_from_cstr(prefix));
}

Vnl_String string_remsuffix_c(Vnl_String self, Vnl_CString suffix) {
	return string_remsuffix_s(self, str_from_cstr(suffix));
}

Vnl_String string_remprefix_s(Vnl_String self, Vnl_String prefix) {
	if (string_hasprefix_s(self, prefix)) {
		return (Vnl_String) { self.chars + prefix.len, self.len - prefix.len };
	}
	return self;
}

Vnl_String string_remsuffix_s(Vnl_String self, Vnl_String suffix) {
	if (string_hassuffix_s(self, suffix)) {
		return (Vnl_String) { self.chars , self.len - suffix.len };
	}
	return self;
}

Vnl_String string_lshift(Vnl_String self) {
	if (self.len == 0) {
		return self;
	}
	return (Vnl_String){ self.chars + 1, self.len - 1 };
}

Vnl_String string_rshift(Vnl_String self) {
	if (self.len == 0) {
		return self;
	}
	return (Vnl_String){ self.chars, self.len - 1 };
}

Vnl_String string_lshiftn(Vnl_String self, size_t n) {
	if (self.len <= n) {
		return (Vnl_String){ nullptr, 0 };
	}
	return (Vnl_String){ self.chars + n, self.len - n };
}

Vnl_String string_rshiftn(Vnl_String self, size_t n) {
	if (self.len <= n) {
		return (Vnl_String){ nullptr, 0 };
	}
	return (Vnl_String){ self.chars, self.len - n };
}

Vnl_String string_ltrim(Vnl_String self) {
	while (self.len && strchr(" \t\n\v\f\r", *self.chars)) {
        self = string_lshift(self);
    }
    return self;
}

Vnl_String string_rtrim(Vnl_String self) {
 while (self.len && strchr(" \t\n\v\f\r", self.chars[self.len-1])) {
        self = string_rshift(self);
    }
    return self;
}

Vnl_String string_trim(Vnl_String self) {
	return string_rtrim(string_ltrim(self));
}

void string_print(Vnl_String self) {
	string_fprint(self, stdout);
}

void string_println(Vnl_String self) {
	string_fprint(self, stdout);
	fputs("", stdout);
}

void string_fprint(Vnl_String self, FILE *fp) {
	for (size_t off = 0; off < self.len;) {
		int l = (self.len - off) > INT_MAX ? INT_MAX : (self.len - off);
		fprintf(fp, "%.*s", l, self.chars + off);
		off += l;
	}
}

void string_fprintln(Vnl_String self, FILE *fp) {
	string_fprint(self, fp);
	fputs("", fp);
}

void string_print_escaped(Vnl_String self) {
	string_fprint_escaped(self, stdout);
}

void string_println_escaped(Vnl_String self) {
	string_print_escaped(self);
	puts("");
}

void string_fprint_escaped(Vnl_String self, FILE *fp) {
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

void string_fprintln_escaped(Vnl_String self, FILE *fp) {
	string_fprint_escaped(self, fp);
	fputs("", fp);
}
