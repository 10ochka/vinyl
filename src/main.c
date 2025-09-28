
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "string.h"
#include "executor.h"
#include <readline/readline.h>
#include <readline/history.h>

void console_clear() {
    printf("\033[H\033[2J");
}

void console_read(Vnl_StringBuffer *buffer) {
    // buffer->len = getline(&buffer->chars, &buffer->cap, stdin);
    buffer->len = 0;
    char *line = readline("\e[34m>>\e[0m ");
   	add_history(line);
    vnl_strbuf_append_c(buffer, line);
    free(line);
}



int main() {
    Vnl_StringBuffer linebuf = {};
    Vnl_Executor *exec = vnl_exec_new();
    using_history();
    while (true) {
        console_read(&linebuf);
        Vnl_String line = vnl_string_from_b(&linebuf);
        line = vnl_string_trim(line);

        if (vnl_string_cmpeq_c(line, "")) {
            continue;
        }

        if (vnl_string_cmpeq_c(line, "exit")) {
            printf("\e[31mExiting...\e[0m\n");
            break;
        }

        if (vnl_string_cmpeq_c(line, "clear")) {
            printf("\e[1;1H\e[2J");
            continue;
        }

        vnl_exec_string(exec, line);

    }
    vnl_strbuf_free(&linebuf);

    return 0;
}
