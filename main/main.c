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
    while (n && (s[n-1]=='\n' || s[n-1]=='\r')) s[--n] = '\0';
}

static int split_hash_inplace(char *line, char *out[], int max_out) {
    int cnt = 0;
    char *p = line;
    while (cnt < max_out) {
        out[cnt++] = p;    
        char *h = strchr(p, '#');             
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
    if (*fileSudoku == NULL) *fileSudoku = fopen(fnSudoku, "r");
    if (*filePlayers == NULL) *filePlayers = fopen(fnPlayers, "r");
    if (*fileSolutions == NULL) *fileSolutions = fopen(fnSolutions, "r");

    if (*fileSudoku == NULL || *filePlayers == NULL || *fileSolutions == NULL) {
        printf("V1: Neotvorene txt subory.\n");
        return 0;
    }

    rewind(*filePlayers);
    rewind(*fileSolutions);

    char lineP[LINE_MAX];
    int first_block = 1;

    while (fgets(lineP, sizeof(lineP), *filePlayers)) {
        chomp(lineP);
        if (lineP[0] == '\0') continue; 

        char buf[LINE_MAX];
        strncpy(buf, lineP, sizeof(buf));
        buf[sizeof(buf)-1] = '\0';

        char *fld[MAX_FIELDS] = {0};
        int nf = split_hash_inplace(buf, fld, MAX_FIELDS);

        const char *pid = (nf >= 1) ? fld[0] : "";
        const char *meno = (nf >= 2) ? fld[1] : "";
        const char *krajina = (nf >= 3) ? fld[2] : "";
        const char *rok = (nf >= 4) ? fld[3] : "";

        if (!first_block) printf("\n");
        first_block = 0;

        printf("Identifikator: %s\n", pid);
        printf("Meno a priezvisko: %s\n", meno);
        printf("Krajina: %s\n", krajina);
        printf("Rok: %s\n", rok);
        printf("Vzorka:\n");

        int printed = 0;
        long pos = ftell(*fileSolutions);
        rewind(*fileSolutions);

        char lineS[LINE_MAX];
        while (printed < SAMPLE_LIMIT && fgets(lineS, sizeof(lineS), *fileSolutions)) {
            chomp(lineS);
            if (lineS[0] == '\0') continue;

            char tmp[LINE_MAX];
            strncpy(tmp, lineS, sizeof(tmp));
            tmp[sizeof(tmp)-1] = '\0';

            char *sf[MAX_FIELDS] = {0};
            int ns = split_hash_inplace(tmp, sf, MAX_FIELDS);
            const char *pid_s = (ns >= 2) ? sf[1] : "";

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


static int is_valid_sid_format(const char *sid) {
    if (!sid) return 0;
    if (strlen(sid) != 8) return 0;         
    if (sid[0] != 'S' || sid[1] != 'I' || sid[2] != 'D') return 0;
    if (sid[3] < 'A' || sid[3] > 'Z') return 0;

    for (int i = 4; i < 8; i++) if (sid[i] < '0' || sid[i] > '9') return 0;

    return 1;
}

static int sid_exists_in_sudoku(FILE *fSudoku, const char *sid) {
    if (!fSudoku || !sid || !*sid) return 0;
    rewind(fSudoku);

    char line[LINE_MAX];
    while (fgets(line, sizeof(line), fSudoku)) {
        chomp(line);
        if (!line[0]) continue;

        char buf[LINE_MAX];
        strncpy(buf, line, sizeof(buf)); buf[sizeof(buf)-1] = '\0';

        char *fld[MAX_FIELDS] = {0};
        int n = split_hash_inplace(buf, fld, MAX_FIELDS);
        for (int i = 0; i < n; ++i) {
            if (fld[i] && strcmp(fld[i], sid) == 0) return 1;
        }
    }
    return 0;
}

static int cmp_hitem_gid(const void *a, const void *b) {
    const HItem *x = (const HItem*)a;
    const HItem *y = (const HItem*)b;

    return strcmp(x -> gid, y -> gid);
}

static void trim_spaces(char *s) {
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t n = strlen(s);
    while (n && (s[n-1] == ' ' || s[n - 1] == '\t')) s[--n] = '\0';
}

static void to_upper_ascii(char *s) {
    for (; *s; ++s)
        if (*s >= 'a' && *s <= 'z')
            *s = (char)(*s - 'a' + 'A');
}

int cmd_h(FILE **fSudoku, FILE **fPlayers, FILE **fSolutions, const char *fnSudoku, const char *fnPlayers, const char *fnSolutions, const char *sid_input_line) {
    (void)fPlayers; (void)fnPlayers;

    if (*fSudoku == NULL) *fSudoku = fopen(fnSudoku, "r");
    if (*fSolutions == NULL) *fSolutions = fopen(fnSolutions, "r");
    if (*fSudoku == NULL || *fSolutions == NULL) {
        printf("H: Neotvoreny txt subor.\n");

        return 0;
    }

    char sid[32]; strncpy(sid, sid_input_line, sizeof(sid)); sid[sizeof(sid)-1]='\0';
    chomp(sid);

    if (!is_valid_sid_format(sid) || !sid_exists_in_sudoku(*fSudoku, sid)) { 
        printf("H: Nespravny vstup.\n");
        return 0;
    }

    rewind(*fSolutions);
    HItem *arr = NULL; size_t n = 0, cap = 0;

    char lineS[LINE_MAX];
    while (fgets(lineS, sizeof(lineS), *fSolutions)) {
        chomp(lineS);
        if (lineS[0] == '\0') continue;

        char tmp[LINE_MAX]; strncpy(tmp, lineS, sizeof(tmp)); tmp[sizeof(tmp) - 1] = '\0';
        char *sf[MAX_FIELDS] = {0}; int ns = split_hash_inplace(tmp, sf, MAX_FIELDS);

        const char *gid = (ns >= 1) ? sf[0] : "";
        char sid_s_norm[64];
        strncpy(sid_s_norm, (ns >= 3) ? sf[2] : "", sizeof(sid_s_norm));
        sid_s_norm[sizeof(sid_s_norm) - 1] = '\0';

        trim_spaces(sid_s_norm);
        to_upper_ascii(sid_s_norm);

        if (strcmp(sid_s_norm, sid) == 0) {
            if (n == cap) {
                cap = cap ? cap * 2 : 64;
                HItem *tmpArr = (HItem*)realloc(arr, cap * sizeof(HItem));
                if (!tmpArr) { 
                    free(arr); printf("H: Neotvoreny txt subor.\n"); 
                    return 0; 
                }
                arr = tmpArr;
            }

            strncpy(arr[n].gid, gid, sizeof(arr[n].gid)); arr[n].gid[sizeof(arr[n].gid) - 1] = '\0';
            strncpy(arr[n].line, lineS, sizeof(arr[n].line)); arr[n].line[sizeof(arr[n].line) - 1] = '\0';
            n++;
        }
    }

    if (n > 1) qsort(arr, n, sizeof(HItem), cmp_hitem_gid);

    const char *outname = "Vystup_H.txt";

    FILE *fo = fopen(outname, "w");

    if (!fo) {
        free(arr);

        printf("H: Neotvoreny txt subor.\n");
        return 0;
    }

    for (size_t i=0; i<n; ++i) {
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

    while (fgets(cmdline, sizeof(cmdline), stdin)) {
        chomp(cmdline);
        if (cmdline[0] == '\0') continue;

        char c = 0;
        int choice = -1;
        char sidline[128] = {0}; 

        if (sscanf(cmdline, " %c %127s", &c, sidline) == 2 && (c=='h' || c=='H')) {
            cmd_h(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, sidline);
            continue;
        }
        
        if (sscanf(cmdline, " %c", &c) == 1 && (c=='h' || c=='H')) {
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

int main() {
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


