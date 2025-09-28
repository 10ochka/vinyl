
#include "strmap.h"
#include "common.h"
#include "object.h"
#include "string.h"
#include <stddef.h>

#include <string.h>
#include <xxhash.h>


typedef struct Vnl_StringMapEntry Vnl_StringMapEntry;

struct Vnl_StringMapEntry {
	Vnl_FixedString key;
	Vnl_Object *value;
};

struct Vnl_StringMap {
	Vnl_StringMapEntry *entries;
	size_t len;
	size_t cap;
};


static const Vnl_StringMapEntry EMPTY_ENTRY = { 0 };
static const size_t INITIAL_CAPACITY = 32;
static const float LOAD_FACTOR = 0.6;
static const size_t HASH_SEED = 0;


Vnl_StringMap *vnl_strmap_new() {
	Vnl_StringMap *self = vnl_malloc(sizeof(*self));
	Vnl_StringMapEntry *entries = vnl_malloc(sizeof(*entries) * INITIAL_CAPACITY);
	*self = (Vnl_StringMap){ entries, 0, INITIAL_CAPACITY };
	return self;
}


static void vnl_strmap_drop_entry(Vnl_StringMapEntry *entry) {
	vnl_fixstr_free(&entry->key);
	vnl_object_release(entry->value);
}

static bool vnl_strmap_is_empty_entry(const Vnl_StringMapEntry *entry) {
	return vnl_memcmpeq(entry, &EMPTY_ENTRY, sizeof(*entry));
}

static void vnl_strmap_clear_entry(Vnl_StringMapEntry *entry) {
	memcpy(entry, &EMPTY_ENTRY, sizeof(*entry));
}

static void vnl_strmap_insert_entry_nocopy(Vnl_StringMap *self, Vnl_StringMapEntry *entry) {
	XXH64_hash_t hash = XXH64(entry->key.chars, entry->key.len, HASH_SEED);
	for (size_t i = 0; i < self->cap; ++i) {
		size_t idx = (hash + i) % self->cap;
		Vnl_StringMapEntry *curr_entry = &self->entries[idx];
		if (vnl_strmap_is_empty_entry(curr_entry)) {
			// UNSAFE: you really shouldn't use bit-by-bit copying of owning structures!
			// It is safe here because empty entry is indicated by having 0-sized key and null value.
			memcpy(curr_entry, entry, sizeof(*entry));
			vnl_strmap_clear_entry(entry);
			return;
		}
	}
}

static void vnl_strmap_resize(Vnl_StringMap *self) {
	size_t new_cap = self->cap * 2;
	Vnl_StringMap new_self = {
		.entries = vnl_malloc(sizeof(*self->entries) * new_cap),
		.len = 0,
		.cap = new_cap,
	};

	for (size_t i = 0; i < new_cap; ++i) {
		Vnl_StringMapEntry *entry = &self->entries[i];
		if (!vnl_strmap_is_empty_entry(entry)) {
			vnl_strmap_insert_entry_nocopy(self, entry);
		}
	}

	vnl_strmap_clear(self);
	vnl_free(self->entries);
	*self = new_self;
}

void vnl_strmap_free(Vnl_StringMap *self) {
	for (size_t i = 0; i < self->cap; ++i) {
		Vnl_StringMapEntry *entry = &self->entries[i];
		if (!vnl_strmap_is_empty_entry(entry)) {
			vnl_strmap_drop_entry(entry);
		}
	}
	vnl_free(self->entries);
	vnl_free(self);
}

void vnl_strmap_clear(Vnl_StringMap *self) {
	for (size_t i = 0; i < self->cap; ++i) {
		Vnl_StringMapEntry *entry = &self->entries[i];
		if (!vnl_strmap_is_empty_entry(entry)) {
			vnl_strmap_drop_entry(entry);
		}
	}
	self->len = 0;
}

void vnl_strmap_insert(Vnl_StringMap *self, Vnl_String key, Vnl_Object *value) {
	if (self->len * 1.0 / self->cap >= LOAD_FACTOR) {
		vnl_strmap_resize(self);
	}
	vnl_object_acquire(value);
	Vnl_StringMapEntry entry = { vnl_fixstr_from_s(key), value };
	vnl_strmap_insert_entry_nocopy(self, &entry);
}


Vnl_StringMapEntry *vnl_strmap_find_entry(Vnl_StringMap *self, Vnl_String key) {
	XXH64_hash_t hash = XXH64(key.chars, key.len, HASH_SEED);
	for (size_t i = 0; i < self->cap; ++i) {
		size_t idx = (hash + i) % self->cap;
		Vnl_StringMapEntry *curr_entry = &self->entries[idx];
		if (vnl_strmap_is_empty_entry(curr_entry)) {
			return nullptr;
		}

		Vnl_String entrykey = vnl_string_from_f(&curr_entry->key);
		if (vnl_string_cmpeq_s(entrykey, key)) {
			return curr_entry;
		}
	}
	return nullptr;
}

Vnl_Object *vnl_strmap_pop(Vnl_StringMap *self, Vnl_String key) {
	Vnl_StringMapEntry *entry = vnl_strmap_find_entry(self, key);
	if (entry) {
		Vnl_Object *obj = entry->value;
		vnl_strmap_clear_entry(entry);
		vnl_object_release(obj);
		return obj;
	} else {
		return nullptr;
	}
}

Vnl_Object *vnl_strmap_find(Vnl_StringMap *self, Vnl_String key) {
	Vnl_StringMapEntry *entry = vnl_strmap_find_entry(self, key);
	if (entry) {
		Vnl_Object *obj = entry->value;
		return obj;
	} else {
		return nullptr;
	}
}

bool vnl_strmap_contains(Vnl_StringMap *self, Vnl_String key) {
	return vnl_strmap_find(self, key) != nullptr;
}


void vnl_strmap_foreach(const Vnl_StringMap *self, Vnl_StringMapCallback callback) {
	for (size_t i = 0; i < self->cap; ++i) {
		const Vnl_StringMapEntry *entry = &self->entries[i];
		if (vnl_strmap_is_empty_entry(entry)) {
			continue;
		}

		callback(
			vnl_string_from_f(&entry->key),
			entry->value
		);

	}
}
