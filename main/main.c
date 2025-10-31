#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define LINE_MAX 1024
#define MAX_FIELDS 16
#define SAMPLE_LIMIT 10

typedef struct {
    char gid[64];
    char line[LINE_MAX];
} HItem;

void chomp(char *s) {
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

void trimSpaces(char *s) {
    char *p = s;
    size_t n;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t')) s[--n] = '\0';
}

int splitHashInplace(char *line, char *out[], int max_out) {
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

int isValidSIDFormat(const char *sid) {
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

int SIDExistsInSudoku(FILE *fSudoku, const char *sid) {
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
        n = splitHashInplace(buf, fld, MAX_FIELDS);

        for (i = 0; i < n; ++i) {
            if (fld[i] && strcmp(fld[i], sid) == 0) return 1;
        }
    }
    return 0;
}

int cmpHitemGID(const void *a, const void *b) {
    const HItem *x = (const HItem*)a;
    const HItem *y = (const HItem*)b;

    return strcmp(x -> gid, y -> gid);
}

void toUpperASCII(char *s) {
    for (; *s; ++s) {
        if (*s >= 'a' && *s <= 'z') *s = (char)(*s - 'a' + 'A');
    }
}

static int isGIDOk(const char *s) {
    return s && strlen(s)==7 && s[0]=='G' && s[1]=='I' && s[2]=='D' && (s[3]>='a' && s[3]<='z') && isdigit((unsigned char)s[4]) && isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]);
}

static int isPIDOk(const char *s) {
    return s && strlen(s)==9 && s[0]=='P' && s[1]=='I' && s[2]=='D' && ( (s[3]>='a' && s[3]<='z') || (s[3]>='A' && s[3]<='Z') ) && isdigit((unsigned char)s[4]) && isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]) && isdigit((unsigned char)s[7]) && isdigit((unsigned char)s[8]);
}

int isSIDOk(const char *s) {
    return s && strlen(s)==8 && s[0]=='S' && s[1]=='I' && s[2]=='D' && (s[3]>='A' && s[3]<='Z') && isdigit((unsigned char)s[4]) && isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]) && isdigit((unsigned char)s[7]);
}

int isYYYYMMDDOk(int d) {
    int y, m, day;

    if (d < 10101) return 0;

    y = d / 10000;
    m = (d / 100) % 100;
    day = d % 100;

    if (m < 1 || m > 12) return 0;
    if (day < 1 || day > 31) return 0;
    return 1;
}

int readline(char *buf, size_t n) {
    if (!fgets(buf, n, stdin)) return 0;
    chomp(buf);

    return 1;
}

// n
int n (FILE **fileSudoku, FILE **filePlayers, FILE **fileSolutions, char ***sudokuArr, int *sudokuCnt, char ***playersArr, int *playersCnt, char ***solutionsArr, int *solutionsCnt) {
    char buf[LINE_MAX];
    char **arr = NULL;
    int cnt = 0;
    int cap = 0;
    int i;

    if (!fileSudoku || !filePlayers || !fileSolutions || !*fileSudoku || !*filePlayers || !*fileSolutions) {
        printf("N: Neotvorene txt subory.\n");
        return 0;
    }

    if (sudokuArr && *sudokuArr) {
        for (i = 0; i < (sudokuCnt ? *sudokuCnt : 0); ++i) {
            free((*sudokuArr)[i]);
        }

        free(*sudokuArr);
        *sudokuArr = NULL;

        if (sudokuCnt) {
            *sudokuCnt = 0;
        }
    }
    if (playersArr && *playersArr) {
        for (i = 0; i < (playersCnt ? *playersCnt : 0); ++i) {
            free((*playersArr)[i]);
        }

        free(*playersArr);
        *playersArr = NULL;

        if (playersCnt) {
            *playersCnt = 0;
        }
    }
    if (solutionsArr && *solutionsArr) {
        for (i = 0; i < (solutionsCnt ? *solutionsCnt : 0); ++i) {
            free((*solutionsArr)[i]);
        }

        free(*solutionsArr);
        *solutionsArr = NULL;

        if (solutionsCnt) {
            *solutionsCnt = 0;
        }
    }

    arr = NULL;
    cnt = 0;
    cap = 0;

    rewind(*fileSudoku);

    while (fgets(buf, sizeof(buf), *fileSudoku)) {
        size_t nlen;
        char *s;

        chomp(buf);

        nlen = strlen(buf) + 1;
        s = (char*)malloc(nlen);

        if (!s) {
            for (i = 0; i < cnt; ++i) {
                free(arr[i]);
            }
            free(arr);

            return 0;
        }

        memcpy(s, buf, nlen);

        if (cnt == cap) {
            char **tmp;

            cap = cap ? cap * 2 : 64;
            tmp = (char**)realloc(arr, cap * sizeof(char*));

            if (!tmp) {
                free(s);

                for (i = 0; i < cnt; ++i) {
                    free(arr[i]);
                }

                free(arr);

                return 0;
            }

            arr = tmp;
        }

        arr[cnt++] = s;
    }

    if (sudokuArr) *sudokuArr = arr;
    if (sudokuCnt) *sudokuCnt = cnt;

    arr = NULL;
    cnt = 0;
    cap = 0;

    rewind(*filePlayers);

    while (fgets(buf, sizeof(buf), *filePlayers)) {
        size_t nlen;
        char *s;

        chomp(buf);

        nlen = strlen(buf) + 1;
        s = (char*)malloc(nlen);

        if (!s) {
            for (i = 0; i < cnt; ++i) {
                free(arr[i]);
            }

            free(arr);

            return 0;
        }

        memcpy(s, buf, nlen);

        if (cnt == cap) {
            char **tmp;

            cap = cap ? cap * 2 : 64;
            tmp = (char**)realloc(arr, cap * sizeof(char*));

            if (!tmp) {
                free(s);

                for (i = 0; i < cnt; ++i) {
                    free(arr[i]);
                }

                free(arr);

                return 0;
            }
            arr = tmp;
        }
        arr[cnt++] = s;
    }

    if (playersArr) *playersArr = arr;
    if (playersCnt) *playersCnt = cnt;

    arr = NULL; 
    cnt = 0;
    cap = 0;

    rewind(*fileSolutions);

    while (fgets(buf, sizeof(buf), *fileSolutions)) {
        size_t nlen;
        char *s;

        chomp(buf);

        nlen = strlen(buf) + 1;
        s = (char*)malloc(nlen);

        if (!s) {
            for (i = 0; i < cnt; ++i) {
                free(arr[i]);
            }

            free(arr);

            return 0;
        }

        memcpy(s, buf, nlen);

        if (cnt == cap) {
            char **tmp;

            cap = cap ? cap * 2 : 64;
            tmp = (char**)realloc(arr, cap * sizeof(char*));

            if (!tmp) {
                free(s);

                for (i = 0; i < cnt; ++i) {
                    free(arr[i]);
                }

                free(arr);

                return 0;
            }
            arr = tmp;
        }
        arr[cnt++] = s;
    }

    if (solutionsArr) *solutionsArr = arr;
    if (solutionsCnt) *solutionsCnt = cnt;

    return 0; 
}

// q
int q(FILE **fileSudoku, FILE **filePlayers, FILE **fileSolutions, char ***solutionsArr, int *solutionsCnt) {
    char yline[64];
    long Y;
    char gid[64], pid[64], sid[64];
    char linebuf[LINE_MAX];
    int datei, mini, seci;
    char *newline;
    int pos, i;

    (void)fileSudoku; (void)filePlayers; (void)fileSolutions;

    if (!solutionsArr || !solutionsCnt || !*solutionsArr) {
        printf("Q: Polia nie su vytvorene.\n"); 
        return 0;
    }

    if (!readline(yline, sizeof(yline))) return 0;
    Y = strtol(yline, NULL, 10);
    if (Y < 1) {
        Y = 1;
    }

    for (;;) {
        if (!readline(gid, sizeof(gid))) return 0;
        if (isGIDOk(gid)) break;
        printf("Q: nespravny format vstupu, zadaj znova: ");
        fflush(stdout);
    }

    for (;;) {
        if (!readline(pid, sizeof(pid))) return 0;
        if (isPIDOk(pid)) break;
        printf("Q: nespravny format vstupu, zadaj znova: ");
        fflush(stdout);
    }

    for (;;) {
        if (!readline(sid, sizeof(sid))) return 0;
        if (isSIDOk(sid)) break;

        printf("Q: nespravny format vstupu, zadaj znova: ");
        fflush(stdout);
    }

    for (;;) {
        char nums[128];
        int nread = 0;
        int ymd, mm, ss;
        char *p = nums, *endp;


        if (!readline(nums, sizeof(nums))) return 0;

        if (sscanf(nums, "%d %d %d", &ymd, &mm, &ss) == 3) {
            if (isYYYYMMDDOk(ymd) && mm >= 0 && ss >= 0 && ss <= 59) {
                datei = ymd; mini = mm; seci = ss;

                break;
            }
        }

        printf("Q: nespravny format vstupu, zadaj znova: ");
        fflush(stdout);
    }

    int need = (int)(strlen(gid)+1 + strlen(pid)+1 + strlen(sid)+1 + 8+1 + 10+1 + 10+1 + 8);

    (void)need; 
    sprintf(linebuf, "%s#%s#%s#%08d#%d#%d#", gid, pid, sid, datei, mini, seci);

    newline = (char*)malloc(strlen(linebuf)+1);
    if (!newline) return 0;
    strcpy(newline, linebuf);

    if (Y > *solutionsCnt) {
        Y = *solutionsCnt + 1;
    }

    pos = (int)Y - 1;

    {
        char **tmp = (char**)realloc(*solutionsArr, (*solutionsCnt + 1) * sizeof(char*));
        if (!tmp) { free(newline); return 0; }
        *solutionsArr = tmp;
    }

    for (i = *solutionsCnt; i > pos; --i) {
        (*solutionsArr)[i] = (*solutionsArr)[i-1];
    }

    (*solutionsArr)[pos] = newline;
    (*solutionsCnt)++;

    return 0;
}

// v1
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
        nf = splitHashInplace(buf, fld, MAX_FIELDS);

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
            ns = splitHashInplace(tmp, sf, MAX_FIELDS);
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

int isNdigits(const char *s, int n) {
    int i;

    if (!s) return 0;

    for (i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return s[n] == '\0';
}

void normFiled(char *dst, const char *src, size_t cap) {
    size_t len;

    strncpy(dst, src ? src : "", cap);
    dst[cap - 1] = '\0';
    trimSpaces(dst);
    len = strlen(dst);
}

int toIntDef (const char *s, int defv) {
    int v = 0;

    if (!s || !*s) return defv;
    v = atoi(s);

    return v;
}

int v2(FILE **fileSudoku, FILE **filePlayers, FILE **fileSolutions, const char *fnSudoku, const char *fnPlayers, const char *fnSolutions) {
    char lineP[LINE_MAX];
    char bufP[LINE_MAX];
    char *pf[MAX_FIELDS];
    int i, nf;

    char lineS[LINE_MAX];
    char bufS[LINE_MAX];
    char *sf[MAX_FIELDS];
    int ns;

    char pid_cur[64];
    const char *pid;
    const char *meno;
    const char *krajina;
    const char *rok;

    if (*fileSudoku == NULL) *fileSudoku = fopen(fnSudoku, "r");
    if (*filePlayers == NULL) *filePlayers = fopen(fnPlayers, "r");
    if (*fileSolutions == NULL) *fileSolutions = fopen(fnSolutions, "r");

    if (*filePlayers == NULL || *fileSolutions == NULL) {
        printf("V2: Nenaplnene polia.\n");

        return 0;
    }

    rewind(*filePlayers);

    while (fgets(lineP, sizeof(lineP), *filePlayers)) {
        int printed = 0;
        long pos;

        chomp(lineP);
        if (lineP[0] == '\0') continue;

        strncpy(bufP, lineP, sizeof(bufP));
        bufP[sizeof(bufP) - 1] = '\0';

        for (i = 0; i < MAX_FIELDS; i++) pf[i] = NULL;
        nf = splitHashInplace(bufP, pf, MAX_FIELDS);

        pid = (nf >= 1) ? pf[0] : "";
        meno = (nf >= 2) ? pf[1] : "";
        krajina = (nf >= 3) ? pf[2] : "";
        rok = (nf >= 4) ? pf[3] : "";

        strncpy(pid_cur, (nf >= 1 && pf[0]) ? pf[0] : "", sizeof(pid_cur));

        printf("PID: %s / %s / %s\n", pid, rok, krajina);
        printf("Identita: %s\n", meno);
        printf("Vysledok:\n");

        pos = ftell(*fileSolutions);

        rewind(*fileSolutions);

        while (printed < SAMPLE_LIMIT && fgets(lineS, sizeof(lineS), *fileSolutions)) {
            char gid[64] = "";
            char pid_s[64] = "";
            char sid[32] = "";
            char date[16] = "";
            char diff_sut[8] = ""; 
            char diff_hry[8] = "";
            char mins[16] = "";
            char secs[16] = "";
            int seconds = -1;

            chomp(lineS);
            if (lineS[0] == '\0') continue;

            strncpy(bufS, lineS, sizeof(bufS));
            bufS[sizeof(bufS) - 1] = '\0';

            for (i = 0; i < MAX_FIELDS; i++) sf[i] = NULL;

            ns = splitHashInplace(bufS, sf, MAX_FIELDS);

            for (i = 0; i < ns; ++i) {
                char tmp[64];

                if (!sf[i] || !*sf[i]) continue;

                normFiled(tmp, sf[i], sizeof(tmp));

                if (gid[0] == '\0' && strncmp(tmp, "GID", 3) == 0) {
                    strncpy(gid, tmp, sizeof(gid));
                    gid[sizeof(gid) - 1] = '\0';

                    continue;
                }

                if (pid_s[0] == '\0' && strncmp(tmp, "PID", 3) == 0) {
                    strncpy(pid_s, tmp, sizeof(pid_s));
                    pid_s[sizeof(pid_s) - 1] = '\0';

                    continue;
                }

                if (sid[0] == '\0' && strncmp(tmp, "SID", 3) == 0) {
                    strncpy(sid, tmp, sizeof(sid));
                    sid[sizeof(sid) - 1] = '\0';

                    continue;
                }

                if (date[0] == '\0' && strlen(tmp) == 8 && isNdigits(tmp, 8)) {
                    strncpy(date, tmp, sizeof(date));
                    date[sizeof(date) - 1] = '\0';

                    continue;
                }

                if (diff_sut[0] == '\0' && strlen(tmp) == 1 && tmp[0] >= 'a' && tmp[0] <= 'z') {
                    strncpy(diff_sut, tmp, sizeof(diff_sut));
                    diff_sut[sizeof(diff_sut) - 1] = '\0';

                    continue;
                }

                if (diff_hry[0] == '\0' && strlen(tmp) == 1 && tmp[0] >= 'A' && tmp[0] <= 'Z') {
                    strncpy(diff_hry, tmp, sizeof(diff_hry));
                    diff_hry[sizeof(diff_hry) - 1] = '\0';

                    continue;
                }
            }

            int last = -1;
            int prev = -1;

            for (i = 0; i < ns; ++i) {
                if (sf[i] && *sf[i]) {
                    char t[32];
                    normFiled(t, sf[i], sizeof(t));
                    if (t[0] && isNdigits(t, (int)strlen(t))) {
                        prev = last;
                        last = i;
                    }
                }
            }

            if (diff_sut[0] == '\0' && gid[0]) {
                size_t lg = strlen(gid);
                if (lg >= 4 && gid[0]=='G' && gid[1]=='I' && gid[2]=='D' &&
                    gid[3] >= 'a' && gid[3] <= 'z') {
                    diff_sut[0] = gid[3];
                    diff_sut[1] = '\0';
                }
            }

            if (diff_hry[0] == '\0' && sid[0]) {
                size_t ls = strlen(sid);
                if (ls >= 4 && sid[0]=='S' && sid[1]=='I' && sid[2]=='D' &&
                    sid[3] >= 'A' && sid[3] <= 'Z') {
                    diff_hry[0] = sid[3];
                    diff_hry[1] = '\0';
                }
            }

            if (last >= 0) {
                strncpy(secs, sf[last], sizeof(secs));
                secs[sizeof(secs) - 1] = '\0';
            }

            if (prev >= 0) {
                strncpy(mins, sf[prev], sizeof(mins));
                mins[sizeof(mins) - 1] = '\0';
            }

            if (!pid_s[0] || strcmp(pid_s, pid_cur) != 0) continue;

            seconds = toIntDef(mins, 0) * 60 + toIntDef(secs, 0);

            printf("\t%s / %s / %s / %s / %s / %s / %d\n",
             gid[0] ? gid : "",
             pid_s[0] ? pid_s : "",
             sid[0] ? sid : "",
             date[0] ? date : "",
             diff_sut[0] ? diff_sut : "", 
             diff_hry[0] ? diff_hry : "", 
             seconds
            ); 
            
            printed++;
        }

        fseek(*fileSolutions, pos, SEEK_SET);

        printf("\n");
    }

    return 0;
}

void v(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol, int choice) {
    switch (choice) {
        case 1:
            v1(fSud, fPlr, fSol, fnSud, fnPlr, fnSol);
            break;

        case 2:
            v2(fSud, fPlr, fSol, fnSud, fnPlr, fnSol);
            break;

        case 3:
            printf("V3: Funkcia este nie je implementovana.\n");
            break;

        default:
            printf("V: Nespravna volba vypisu.\n");
            break;
    }
}

//h
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

    if (!isValidSIDFormat(sid) || !SIDExistsInSudoku(*fSudoku, sid)) {
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
        ns = splitHashInplace(tmp, sf, MAX_FIELDS);
        sid_s_norm[0] = '\0';

        for (i = 0; i < ns; i++) {
            if (sf[i] == NULL || sf[i][0] == '\0') continue;
            {
                char candidate[32];

                strncpy(candidate, sf[i], sizeof(candidate));

                candidate[sizeof(candidate) - 1] = '\0';

                trimSpaces(candidate);

                toUpperASCII(candidate);

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

    if (n > 1) qsort(arr, n, sizeof(HItem), cmpHitemGID);

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

int readNextNonemptyLine(char *buf, size_t bufsz) {
    while (fgets(buf, bufsz, stdin)) {
        chomp(buf);
        if (buf[0] != '\0') return 1;
    }
    return 0;
}

// ine
void handleCommandLoop(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol) {
    char **sudokuArr = NULL, **playersArr = NULL, **solutionsArr = NULL;
    int sudokuCnt = 0, playersCnt = 0, solutionsCnt = 0;
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
            if (!readNextNonemptyLine(sidline, sizeof(sidline))) {
                printf("H: Nespravny vstup.\n");
                continue;
            }
            cmd_h(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, sidline);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'n' || c == 'N')) {
            n(fSud, fPlr, fSol, &sudokuArr, &sudokuCnt, &playersArr, &playersCnt, &solutionsArr, &solutionsCnt);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'q' || c == 'Q')) {
            q(fSud, fPlr, fSol, &solutionsArr, &solutionsCnt);
            continue;
        }


        if (sudokuArr) {
            int i;
            for (i = 0; i < sudokuCnt; ++i) free(sudokuArr[i]);
            free(sudokuArr);
        }
        if (playersArr) {
            int i;
            for (i = 0; i < playersCnt; ++i) free(playersArr[i]);
            free(playersArr);
        }
        if (solutionsArr) {
            int i;
            for (i = 0; i < solutionsCnt; ++i) free(solutionsArr[i]);
            free(solutionsArr);
        }
    }
}



int main(void) {
    FILE *f1 = NULL, *f2 = NULL, *f3 = NULL;
    char *fname1 = "./Sudoku.txt";
    char *fname2 = "./RegisterHracov.txt";
    char *fname3 = "./RegisterRieseni.txt";

    handleCommandLoop(&f1, &f2, &f3, fname1, fname2, fname3);

    if (f1 != NULL) fclose(f1);
    if (f2 != NULL) fclose(f2);
    if (f3 != NULL) fclose(f3);

    return 0;
}
