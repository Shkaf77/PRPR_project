#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_MAX 1024
#define MAX_FIELDS 16
#define SAMPLE_LIMIT 10

typedef struct {
    char gid[64];
    char line[LINE_MAX];
} HItem;

static void chomp(char *s) {
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

static int split_hash_inplace(char *line, char *out[], int max_out) {
    int cnt = 0;
    char *p = line;
    char *h;
    while (cnt < max_out) {
        out[cnt++] = p;
        h = strchr(p, '#');
        if (!h) break;
        *h = '\0';
        p = h + 1;
        if (*p == '\0') {
            if (cnt < max_out) out[cnt++] = p;
            break;
        }
    }
    return cnt;
}

int v1(FILE **fileSudoku, FILE **filePlayers, FILE **fileSolutions, const char *fnSudoku, const char *fnPlayers, const char *fnSolutions) {
    char lineP[LINE_MAX];
    int first_block = 1;
    char buf[LINE_MAX];
    char *fld[MAX_FIELDS];
    int nf, printed, ns;
    long pos;
    char lineS[LINE_MAX];
    char tmp[LINE_MAX];
    char *sf[MAX_FIELDS];
    const char *pid;
    const char *meno;
    const char *krajina;
    const char *rok;
    const char *pid_s;
    int i;

    if (*fileSudoku == NULL) *fileSudoku = fopen(fnSudoku, "r");
    if (*filePlayers == NULL) *filePlayers = fopen(fnPlayers, "r");
    if (*fileSolutions == NULL) *fileSolutions = fopen(fnSolutions, "r");

    if (*fileSudoku == NULL || *filePlayers == NULL || *fileSolutions == NULL) {
        printf("V1: Neotvorene txt subory.\n");
        return 0;
    }

    rewind(*filePlayers);
    rewind(*fileSolutions);

    while (fgets(lineP, sizeof(lineP), *filePlayers)) {
        chomp(lineP);
        if (lineP[0] == '\0') continue;

        strncpy(buf, lineP, sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';

        for (i = 0; i < MAX_FIELDS; i++) fld[i] = NULL;
        nf = split_hash_inplace(buf, fld, MAX_FIELDS);

        pid = (nf >= 1) ? fld[0] : "";
        meno = (nf >= 2) ? fld[1] : "";
        krajina = (nf >= 3) ? fld[2] : "";
        rok = (nf >= 4) ? fld[3] : "";

        if (!first_block) printf("\n");
        first_block = 0;

        printf("Identifikator: %s\n", pid);
        printf("Meno a priezvisko: %s\n", meno);
        printf("Krajina: %s\n", krajina);
        printf("Rok: %s\n", rok);
        printf("Vzorka:\n");

        printed = 0;
        pos = ftell(*fileSolutions);
        rewind(*fileSolutions);

        while (printed < SAMPLE_LIMIT && fgets(lineS, sizeof(lineS), *fileSolutions)) {
            chomp(lineS);
            if (lineS[0] == '\0') continue;

            strncpy(tmp, lineS, sizeof(tmp));
            tmp[sizeof(tmp) - 1] = '\0';

            for (i = 0; i < MAX_FIELDS; i++) sf[i] = NULL;
            ns = split_hash_inplace(tmp, sf, MAX_FIELDS);
            pid_s = (ns >= 2) ? sf[1] : "";

            if (strcmp(pid_s, pid) == 0) {
                printf("\t%s\n", lineS);
                printed++;
            }
        }

        fseek(*fileSolutions, pos, SEEK_SET);
    }

    rewind(*fileSudoku);
    rewind(*filePlayers);
    rewind(*fileSolutions);

    return 0;
}

static void v(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol, int choice) {
    switch (choice) {
        case 1:
            v1(fSud, fPlr, fSol, fnSud, fnPlr, fnSol);
            break;

        case 2:
            printf("V2: Funkcia este nie je implementovana.\n");
            break;

        case 3:
            printf("V3: Funkcia este nie je implementovana.\n");
            break;

        default:
            printf("V: Nespravna volba vypisu.\n");
            break;
    }
}


static int is_valid_sid_format(const char *sid) {
    int i;

    if (!sid) return 0;
    if (strlen(sid) != 8) return 0;
    if (sid[0] != 'S' || sid[1] != 'I' || sid[2] != 'D') return 0;
    if (sid[3] < 'A' || sid[3] > 'Z') return 0;

    for (i = 4; i < 8; i++) {
        if (sid[i] < '0' || sid[i] > '9') return 0;
    }
    return 1;
}

static int sid_exists_in_sudoku(FILE *fSudoku, const char *sid) {
    char line[LINE_MAX];
    char buf[LINE_MAX];
    char *fld[MAX_FIELDS];
    int n;
    int i;

    if (!fSudoku || !sid || !*sid) return 0;
    rewind(fSudoku);

    while (fgets(line, sizeof(line), fSudoku)) {
        chomp(line);
        if (!line[0]) continue;

        strncpy(buf, line, sizeof(buf));
        buf[sizeof(buf) - 1] = '\0';

        for (i = 0; i < MAX_FIELDS; i++) fld[i] = NULL;
        n = split_hash_inplace(buf, fld, MAX_FIELDS);

        for (i = 0; i < n; ++i) {
            if (fld[i] && strcmp(fld[i], sid) == 0) return 1;
        }
    }
    return 0;
}

static int cmp_hitem_gid(const void *a, const void *b) {
    const HItem *x = (const HItem*)a;
    const HItem *y = (const HItem*)b;
    return strcmp(x->gid, y->gid);
}

static void trim_spaces(char *s) {
    char *p = s;
    size_t n;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = '\0';
}

static void to_upper_ascii(char *s) {
    for (; *s; ++s) {
        if (*s >= 'a' && *s <= 'z') *s = (char)(*s - 'a' + 'A');
    }
}

int cmd_h(FILE **fSudoku, FILE **fPlayers, FILE **fSolutions, const char *fnSudoku, const char *fnPlayers, const char *fnSolutions, const char *sid_input_line) {
    char sid[32];
    char lineS[LINE_MAX];
    char tmp[LINE_MAX];
    char *sf[MAX_FIELDS];
    char sid_s_norm[32];
    const char *gid;
    int ns;
    int i;

    HItem *arr = NULL;

    size_t n = 0, cap = 0;

    FILE *fo;
    HItem *tmpArr;

    (void)fPlayers;
    (void)fnPlayers;

    if (*fSudoku == NULL) *fSudoku = fopen(fnSudoku, "r");
    if (*fSolutions == NULL) *fSolutions = fopen(fnSolutions, "r");
    if (*fSudoku == NULL || *fSolutions == NULL) {
        printf("H: Neotvoreny txt subor.\n");
        return 0;
    }

    strncpy(sid, sid_input_line, sizeof(sid));
    sid[sizeof(sid) - 1] = '\0';
    chomp(sid);

    if (!is_valid_sid_format(sid) || !sid_exists_in_sudoku(*fSudoku, sid)) {
        printf("H: Nespravny vstup.\n");
        return 0;
    }

    rewind(*fSolutions);

    while (fgets(lineS, sizeof(lineS), *fSolutions)) {
        chomp(lineS);
        if (lineS[0] == '\0') continue;

        strncpy(tmp, lineS, sizeof(tmp));
        tmp[sizeof(tmp) - 1] = '\0';

        for (i = 0; i < MAX_FIELDS; i++) sf[i] = NULL;
        ns = split_hash_inplace(tmp, sf, MAX_FIELDS);

        sid_s_norm[0] = '\0';
        for (i = 0; i < ns; i++) {
            if (sf[i] == NULL || sf[i][0] == '\0') continue;
            {
                char candidate[32];

                strncpy(candidate, sf[i], sizeof(candidate));

                candidate[sizeof(candidate) - 1] = '\0';

                trim_spaces(candidate);

                to_upper_ascii(candidate);

                if (strlen(candidate) == 8 && strncmp(candidate, "SID", 3) == 0) {
                    strcpy(sid_s_norm, candidate);
                    break;
                }
            }
        }

        if (sid_s_norm[0] == '\0') continue;
        if (strcmp(sid_s_norm, sid) != 0) continue;

        gid = (ns >= 1) ? sf[0] : "";

        if (n == cap) {
            cap = cap ? cap * 2 : 64;
            tmpArr = (HItem*)realloc(arr, cap * sizeof(HItem));
            if (!tmpArr) {
                free(arr);
                printf("H: Neotvoreny txt subor.\n");
                return 0;
            }
            arr = tmpArr;
        }

        strncpy(arr[n].gid, gid, sizeof(arr[n].gid));
        arr[n].gid[sizeof(arr[n].gid) - 1] = '\0';
        strncpy(arr[n].line, lineS, sizeof(arr[n].line));
        arr[n].line[sizeof(arr[n].line) - 1] = '\0';
        n++;
    }

    if (n > 1) qsort(arr, n, sizeof(HItem), cmp_hitem_gid);

    fo = fopen("Vystup_H.txt", "w");
    if (!fo) {
        free(arr);
        printf("H: Neotvoreny txt subor.\n");
        return 0;
    }

    for (i = 0; i < (int)n; ++i) {
        fputs(arr[i].line, fo);
        fputc('\n', fo);
    }

    fclose(fo);
    free(arr);
    printf("H: Uspešne vytvoreny sumar.\n");
    return 0;
}

static int read_next_nonempty_line(char *buf, size_t bufsz) {
    while (fgets(buf, bufsz, stdin)) {
        chomp(buf);
        if (buf[0] != '\0') return 1;
    }
    return 0;
}

static void handle_command_loop(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol) {
    char cmdline[256];
    char c;
    int choice;
    char sidline[128];

    while (fgets(cmdline, sizeof(cmdline), stdin)) {
        chomp(cmdline);
        if (cmdline[0] == '\0') continue;

        c = 0;
        choice = -1;
        memset(sidline, 0, sizeof(sidline));

        if (sscanf(cmdline, " %c %d", &c, &choice) == 2 && (c == 'v' || c == 'V')) {
            v(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, choice);
            continue;
        }

        if (sscanf(cmdline, " %c %127s", &c, sidline) == 2 && (c == 'h' || c == 'H')) {
            cmd_h(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, sidline);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'h' || c == 'H')) {
            printf("Zadajte identifikátor Sudoku (SID): ");
            if (!read_next_nonempty_line(sidline, sizeof(sidline))) {
                printf("H: Nespravny vstup.\n");
                continue;
            }
            cmd_h(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, sidline);
            continue;
        }
    }
}


int main(void) {
    FILE *f1 = NULL, *f2 = NULL, *f3 = NULL;
    char *fname1 = "./Sudoku.txt";
    char *fname2 = "./RegisterHracov.txt";
    char *fname3 = "./RegisterRieseni.txt";

    handle_command_loop(&f1, &f2, &f3, fname1, fname2, fname3);

    if (f1 != NULL) fclose(f1);
    if (f2 != NULL) fclose(f2);
    if (f3 != NULL) fclose(f3);
    return 0;
}
