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

static void handle_command_loop(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol) {
    char cmdline[256];
    while (fgets(cmdline, sizeof(cmdline), stdin)) {
        chomp(cmdline);
        if (cmdline[0] == '\0') continue;

        char c = 0;
        int choice = -1;
        if (sscanf(cmdline, " %c %d", &c, &choice) == 2 && (c=='v' || c=='V')) {
            if (choice == 1) {
                v1(fSud, fPlr, fSol, fnSud, fnPlr, fnSol);
            } else {
                printf("V: Nesprávna volba vypisu.\n");
            }
        } else {

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


