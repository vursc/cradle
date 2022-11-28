#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct section {
    char *name;
    char *cells;
    uint32_t count;
    uint16_t dount;
    uint16_t index;
};

char name_pool[4096];
char cell_pool[32768];
struct section sect_pool[1024];

uint16_t load_sections(const char *const fn) {
    uint16_t i = 0;
    char ch;
    char *np = name_pool;
    char *cp = cell_pool;
    struct section *c = sect_pool;
    struct section *s = sect_pool;
    sect_pool[0] = (struct section){ "(default)", cp, 0, 1, i++ };

    FILE *f = fopen(fn, "r");
    if (f == NULL) { fputs("error: cannot open file\n", stderr); goto fail; }
    while (ch = fgetc(f), !feof(f))
        if (ch == '[') {
            c = ++s;
            *s = (struct section){ np, cp, 0, 1, i++ };
            while ((ch = fgetc(f)) != ']' && !feof(f)) *(np++) = ch;
            if (feof(f)) { fputs("error: early EOF\n", stderr); goto fail; }
            *(np++) = '\0'; 
        } else if (ch == '(') {
            ++c->dount; ++s;
            *s = (struct section){ np, cp, 0, 0, s - c };
            while ((ch = fgetc(f)) != ')' &&!feof(f)) *(np++) = ch;
            if (feof(f)) { fputs("error: early EOF\n", stderr); goto fail; }
            *(np++) = '\0';
        } else if (strchr("-#+=/", ch)) { ++(s->count); *(cp++) = ch; }
    fclose(f);
    return s - sect_pool;

fail: exit(EXIT_FAILURE);
}

void print_section(const uint16_t idx) {
    const struct section s = sect_pool[idx];
    if (s.dount != 0) {
        printf("        [%02d] > %s", s.index, s.name);
        if (s.count != 0) fputs("\n             | ", stdout);
    } else printf("%12.12s | ", s.name);

    for (uint32_t i = 0, j = 0; i < s.count; ++i) {
        if (j != 0)
            if (j % 50 == 0) fputs("\n             | ", stdout);
            else if (j % 10 == 0) fputs("  ", stdout);
            else if (j %  5 == 0) putchar(' ');
        if (s.cells[i] == '/') {
            fputs("     " + j % 5, stdout);
            j += 5 - j % 5;
        } else { putchar(s.cells[i]); ++j; }
    }
    putchar('\n');
}

int main(int argc, char **argv) {
    char *fn = argc > 1 ? argv[1] : "tasks.cell";
    uint16_t scnt = load_sections(fn);
    uint16_t x = !(sect_pool[0].count != 0 || sect_pool[0].dount != 1);
    for (uint16_t i = x; i <= scnt; ++i) {
        print_section(i);
    }
    return EXIT_SUCCESS;
}
