
#include "common.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void *vnl_malloc(size_t size) {
	void *ptr = malloc(size);
	if (ptr == nullptr) {
		printf(VNL_ANSICOL_RED "Fatal error: unable to malloc!" VNL_ANSICOL_RESET);
		abort();
	}
	return ptr;
}


void *vnl_realloc(void *ptr, size_t new_size) {
	if (new_size == 0) {
		vnl_free(ptr);
		return nullptr;
	}
	void *new_ptr = realloc(ptr, new_size);
	if (new_ptr == nullptr) {
		free(ptr);
		printf(VNL_ANSICOL_RED "Fatal error: Unable to realloc!" VNL_ANSICOL_RESET);
		abort();
	}
	return new_ptr;
}


void vnl_free(void *ptr) {
	if (ptr == nullptr) {
		return;
	}
	free(ptr);
}

bool vnl_memcmpeq(const void *mema, const void *memb, size_t size) {
	return memcmp(mema, memb, size) == 0;
}
