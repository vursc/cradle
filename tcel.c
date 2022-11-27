#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct section {
    char *name, *cells;
    uint32_t count;
    uint16_t dount;
    uint16_t index;
};

char pool[65536];
struct section sections[4096];

uint16_t load_sections(const char *const fn) {
    FILE *f = fopen(fn, "r"); if (f == NULL) {
        fprintf(stderr, "error: cannot open %s\n", fn);
        goto fail;
    }

    uint16_t i = 1;
    char *p = pool;
    struct section *c = NULL, *s = NULL;
    char ch; while (ch = fgetc(f), !feof(f)) {
        if (ch == '[') {
            c = s = s ? ++s : sections;
            s->name = p;
            while ((ch = fgetc(f)) != ']')
                if (feof(f)) {
                    fputs("error: unexpected EOF\n", stderr);
                    goto fail;
                }
                else *(p++) = ch; *(p++) = '\0';
            s->cells = p;
            s->count = 0;
            s->dount = 1;
            s->index = i++;
        } else if (ch == '(') {
            if (s == NULL) {
                fputs("error: unexpected section\n", stderr);
                goto fail;
            }
            ++s;
            s->name = p;
            while((ch = fgetc(f)) != ')')
                if (feof(f)) {
                    fputs("error: unexpected EOF\n", stderr);
                    goto fail;
                }
                else *(p++) = ch; *(p++) = 0;
            s->cells = p;
            s->count = 0;
            s->dount = 0;
            s->index = s - c;
            ++c->dount;
        } else if (strchr("-#+=/", ch)) {
            if (s == NULL) {
                fputs("error: unexpected cell\n", stderr);
                goto fail;
            }
            *(p++) = ch; ++s->count;
        }
    }
    fclose(f);
    return s - sections;

fail:
    exit(EXIT_FAILURE);
}

void print_section(const uint16_t idx) {
    const struct section s = sections[idx];

    if (s.dount != 0) {
        printf("        [%02d] > %s", s.index, s.name);
        if (s.count != 0) fputs("\n             | ", stdout);
    } else printf("%12.12s | ", s.name);

    for (uint32_t i = 0, j = 0; i < s.count; ++i, ++j) {
        if (j != 0) if (j % 50 == 0) fputs("\n             | ", stdout);
               else if (j % 10 == 0) putchar(' '), putchar(' ');
               else if (j %  5 == 0) putchar(' ');
        if (s.cells[i] == '/') switch (j % 5) {
            case 0: putchar(' '); ++j; case 1: putchar(' '); ++j;
            case 2: putchar(' '); ++j; case 3: putchar(' '); ++j;
            case 4: putchar(' ');
        } else putchar(s.cells[i]);
    }
    putchar('\n');
}

int main(int argc, char **argv) {
    char *fn = argc > 1 ? argv[1] : "tasks.cell";
    uint16_t scnt = load_sections(fn);
    for (uint16_t i = 0; i < scnt; ++i) print_section(i);
    return EXIT_SUCCESS;
}
