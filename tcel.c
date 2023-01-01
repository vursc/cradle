#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct section { uint16_t v; uint16_t n; uint32_t c; };
struct context { struct section *sp; char *cp; };

uint16_t load_sections(struct context ctx, char *fn) {
    FILE *f = fopen(fn, "r");
    if (f == NULL) {
        printf("error: cannot open %s: %s\n", fn, strerror(errno));
        return -1;
    }

    uint16_t m = UINT16_MAX, i = UINT16_MAX;
    for (char c = fgetc(f); !feof(f); c = fgetc(f)) {
        if (c == '[' || c == '(') {
            if (++i != 0) *(ctx.cp++) = '\0';
            ctx.sp[i].v = 0;
            ctx.sp[i].n = 0;
            ctx.sp[i].c = 0;
            if (c == '[') m = i;
            if (m != UINT16_MAX) ++(ctx.sp[m].v);
            char d = c == '[' ? ']' : ')';
            while (c = fgetc(f), c != d && !feof(f))
                ++(ctx.sp[i].n), *(ctx.cp++) = c;
            *(ctx.cp++) = '\0';
        } else if (c == '#' || c == '=' || c == '+' || c == '-' || c == '/') {
            if (i != UINT16_MAX) ++(ctx.sp[i].c), *(ctx.cp++) = c;
            else printf("warning: orphan cell: %c\n", c);
        } else if (!isspace(c)) printf("warning: unknown cell: %c\n", c);
    }
    fclose(f);
    return i + 1;
}

void dump_sections(struct context ctx, uint16_t n, char *fn) {
    FILE *f = fopen(fn, "w");
    if (f == NULL) printf("error: cannot open %s: %s\n", fn, strerror(errno));
    for (uint16_t i = 0; i < n; ++i) {
        struct section sect = ctx.sp[i];
        if (sect.v != 0) {
            fprintf(f, "[%s]",  ctx.cp);
            if (sect.c != 0) fprintf(f, "\n         ");
        } else {
            int k = fprintf(f, "(%s)", ctx.cp);
            if (k <= 9) fprintf(f, "         " + k);
            else fprintf(f, "\n         ");
        }
        ctx.cp += sect.n + 1;
        for (uint32_t j = 0, k = 0; j < sect.c; ++j) {
            if (k % 50 == 0 && k != 0) fprintf(f, "\n         ");
            if (k % 5 == 0) fputc(' ', f);
            if (ctx.cp[j] == '/') k += fprintf(f, "    /" + k % 5);
            else fputc(ctx.cp[j], f), ++k;
        }
        ctx.cp += sect.c + 1;
        fputc('\n', f);
    }
}

void show_sections(struct context ctx, uint16_t n) {
    uint16_t s = 0;
    for (uint16_t i = 0; i < n; ++i) {
        struct section sect = ctx.sp[i];
        if (sect.v != 0) {
            printf("    [%02d] > %s", ++s, ctx.cp);
            if (sect.c != 0) printf("\n         |");
        } else printf("%8.8s |", ctx.cp);
        ctx.cp += sect.n + 1;
        for (uint32_t j = 0, k = 0; j < sect.c; ++j) {
            if (k % 50 == 0 && k != 0) printf("\n         |");
            if (k % 5 == 0) putchar(' ');
            if (ctx.cp[j] == '/') k += printf("     " + k % 5);
            else putchar(ctx.cp[j]), ++k;
        }
        ctx.cp += sect.c + 1;
        putchar('\n');
    }
}

#define NS 1024
#define NC 65536

struct section ss[NS]; char cs[NC];
struct context ctx = { .sp = ss, .cp = cs };

int main(int argc, char **argv) {
    uint16_t n = load_sections(ctx, "tasks.cell");
    if (n == UINT16_MAX) return 1;
    if (argc == 1) show_sections(ctx, n);
    else dump_sections(ctx, n, "out.cell");
}
