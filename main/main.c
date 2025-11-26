#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define LINE_MAX 1024
#define MAX_FIELDS 16
#define SAMPLE_LIMIT 10

// Sturctures

typedef struct {
    char PID[32];
    char Identita[128];
    char Krajina[64];
    int  RokNar;
} MPlayer;

typedef struct {
    char SID[32];
    char NarHry;
    char GID[32];
    char NarSut;
    char DatHry[9];
    int  Trvanie;
} MResult;

typedef struct MNode {
    MPlayer player;
    MResult result;
    struct MNode *next;
} MNode;

// Support functions

char *dupstr(const char *s) {
    size_t n; 
    char *p;

    if (!s) {
        return NULL;
    }

    n = strlen(s) + 1;
    p = (char*)malloc(n);

    if (p) {
        memcpy(p, s, n);
    }

    return p;
}

void chomp(char *s) {
    size_t n = strlen(s);

    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[--n] = '\0';
    }
}

void trimSpaces(char *s) {
    char *p = s;
    size_t n;

    while (*p == ' ' || *p == '\t') p++;
    if (p != s) {
        memmove(s, p, strlen(p) + 1);
    }

    n = strlen(s);

    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
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

void toUpperASCII(char *s) {
    for (; *s; ++s) {
        if (*s >= 'a' && *s <= 'z') *s = (char)(*s - 'a' + 'A');
    }
}

int isGIDOk(const char *s) {
    return s && strlen(s)==7 && s[0]=='G' && s[1]=='I' && s[2]=='D' && (s[3]>='a' && s[3]<='z') && isdigit((unsigned char)s[4]) && isdigit((unsigned char)s[5]) && isdigit((unsigned char)s[6]);
}

int isPIDOk(const char *s) {
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

int readNextNonemptyLine(char *buf, size_t bufsz) {
    while (fgets(buf, bufsz, stdin)) {
        chomp(buf);
        if (buf[0] != '\0') return 1;
    }
    return 0;
}

int nextRand(int *seed) {
    long s = (long)(*seed);

    s = s * 1103515245L + 12345L;
    *seed = (int)(s & 0x7FFFFFFF);

    return (*seed >> 16) & 0x7FFF;
}

int cmp_line_by_gid(const void *pa, const void *pb) {
    const char *sa = *(const char * const *)pa;
    const char *sb = *(const char * const *)pb;
    char ga[64], gb[64];
    int i;

    i = 0; 
    
    while (sa[i] && sa[i] != '#' && i < (int)sizeof(ga)-1) {
        ga[i] = sa[i];
        i++;
    }

    ga[i] = '\0';
    i = 0;

    while (sb[i] && sb[i] != '#' && i < (int)sizeof(gb)-1) {
        gb[i] = sb[i];
        i++;
    }

    gb[i] = '\0';

    return strcmp(ga, gb);
}

int loadSudoku81(FILE *fSudoku, const char *sid, char out81[82]) {
    char line[LINE_MAX];
    int count = 0;
    int inside = 0;

    if (!fSudoku || !sid || !*sid) return 0;

    rewind(fSudoku);

    while (fgets(line, sizeof(line), fSudoku)) {
        char *p = line;
        chomp(p);

        if (!inside) {
            char *sidpos = strstr(p, sid);
            if (!sidpos) continue;
            sidpos = strchr(sidpos, '#');
            if (!sidpos) continue;
            p = sidpos + 1;
            inside = 1;
        }

        while (*p && count < 81) {
            if (*p == '#') {
                inside = 0; 
                break; 
            }

            if (*p >= '1' && *p <= '9') {
                out81[count++] = *p;
            }

            p++;
        }

        if (count >= 81) break;
    }

    if (count != 81) {
        out81[0] = '\0'; return 0;
    }

    out81[81] = '\0';

    return 1;
}

int isValidSIDFormat_E(const char *sid) {
    int i;

    if (!sid) return 0;
    if (strlen(sid) != 8) return 0;
    if (sid[0] != 'S' || sid[1] != 'I' || sid[2] != 'D') return 0;
    if (sid[3] < 'A' || sid[3] > 'Z') return 0;

    for (i = 4; i < 8; i++) if (sid[i] < '0' || sid[i] > '9') return 0;

    return 1;
}

int seedFromSID(const char *sid) {
    long s = 2166136261L;
    int i;

    for (i = 0; sid[i]; ++i) {
        s ^= (long)(unsigned char)sid[i];
        s *= 16777619L;
    }

    if (s < 0) {
        s = -s;
    }

    return (int)(s & 0x7FFFFFFF);
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


void blankRow(int *seed, int X, char row[9]) {
    int used[9];
    int k = 0;
    int col;
    int i;

    for (i = 0; i < 9; ++i) {
        used[i] = 0;
    }
    while (k < X) {
        col = nextRand(seed) % 9;
        if (!used[col]) {
            used[col] = 1; row[col] = ' '; k++;
        }
    }
}

int writeBoardPipes(const char grid[9][10], const char *outname) {
    FILE *fo = fopen(outname, "w");

    int r, c;

    if (!fo) return 0;

    for (r = 0; r < 9; ++r) {
        for (c = 0; c < 9; ++c) {
            fputc('|', fo);
            fputc(grid[r][c], fo); 
        }

        fputs("|\n", fo);
    }

    fclose(fo);
    
    return 1;
}

//Main functions

//d
void cmd_d(MNode **pHead) {
    MNode *i, *j;
    MResult tmpRes;
    int swapped;

    if (*pHead == NULL) {
        return;
    }

    i = *pHead;

    while (i != NULL) {
        do {
            swapped = 0;
            j = *pHead;

            while (j != NULL && j->next != NULL) {

                if (strcmp(j->player.PID, i->player.PID) == 0 &&
                    strcmp(j->next->player.PID, i->player.PID) == 0) 
                {
                    if (j->result.Trvanie > j->next->result.Trvanie) {

                        tmpRes = j->result;
                        j->result = j->next->result;
                        j->next->result = tmpRes;

                        swapped = 1;
                    }
                }

                j = j->next;
            }
        } while (swapped);

        i = i->next;
    }
}

//s
void cmd_s(MNode **pHead) {
    char gid[32];
    int removed = 0;

    MNode *cur;
    MNode *prev;
    MNode *tmp;

    if (*pHead == NULL) {
        printf("S: Spajany zoznam nie je vytvoreny.\n");

        return;
    }

    if (!fgets(gid, sizeof(gid), stdin)) {
        printf("S: Nespravny vstup.\n");

        return;
    }

    chomp(gid);

    while (*pHead != NULL && strcmp((*pHead)->result.GID, gid) == 0) {
        tmp = *pHead;
        *pHead = (*pHead)->next;

        free(tmp);

        removed++;
    }

    prev = *pHead;

    if (prev != NULL) {
        cur = prev->next;
    } else {
        cur = NULL;
    }

    while (cur != NULL) {
        if (strcmp(cur->result.GID, gid) == 0) {
            tmp = cur;
            prev->next = cur->next;
            cur = cur->next;

            free(tmp);

            removed++;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }

    printf("S: Vymazalo sa : %d zaznamov !\n", removed);
}

//a
int cmd_a(MNode **pHead) {
    char line[256];
    int Y;
    char pidNew[16];
    char meno[64];
    char krajina[64];
    char rokStr[32];
    int rok;
    int maxA = 0;
    int num;
    int pos;

    MNode *cur;
    MNode *node;
    MNode *prev;

    if (fgets(line, sizeof(line), stdin) == NULL) {
        printf("A: Nespravny vstup.\n");

        return 0;
    }

    chomp(line);

    if (sscanf(line, "%d", &Y) != 1) {
        printf("A: Nespravny vstup.\n");

        return 0;
    }

    if (fgets(line, sizeof(line), stdin) == NULL) {
        printf("A: Nespravny vstup.\n");

        return 0;
    }

    chomp(line);

    if (fgets(meno, sizeof(meno), stdin) == NULL) {
        printf("A: Nespravny vstup.\n");
        
        return 0;
    }

    chomp(meno);

    if (fgets(krajina, sizeof(krajina), stdin) == NULL) {
        printf("A: Nespravny vstup.\n");

        return 0;
    }

    chomp(krajina);

    if (fgets(rokStr, sizeof(rokStr), stdin) == NULL) {
        printf("A: Nespravny vstup.\n");

        return 0;
    }

    chomp(rokStr);
    rok = atoi(rokStr);
    cur = *pHead;

    while (cur != NULL) {
        if (strcmp(cur->player.Identita, meno) == 0 && cur->player.RokNar == rok) {
            printf("A: Duplicita zaznamu.\n");

            return 0;
        }
        cur = cur->next;
    }

    cur = *pHead;

    while (cur != NULL) {
        if (strncmp(cur->player.PID, "PIDa", 4) == 0) {
            num = atoi(cur->player.PID + 4);

            if (num > maxA) {
                maxA = num;
            }
        }
        cur = cur->next;
    }

    num = maxA + 1;
    sprintf(pidNew, "PIDa%05d", num);

    node = (MNode*)malloc(sizeof(MNode));

    if (node == NULL) {
        printf("A: Nepodarilo sa alokovat pamat.\n");
        return 0;
    }

    strcpy(node->player.PID, pidNew);
    strcpy(node->player.Identita, meno);
    strcpy(node->player.Krajina, krajina);

    node->player.RokNar = rok;

    node->result.SID[0] = '\0';
    node->result.NarHry = 0;
    node->result.GID[0] = '\0';
    node->result.NarSut = 0;
    node->result.DatHry[0] = '\0';
    node->result.Trvanie = 0;

    node->next = NULL;

    if (Y <= 1 || *pHead == NULL) {
        node->next = *pHead;
        *pHead = node;
        pos = 1;
    } else {
        prev = NULL;
        cur  = *pHead;
        pos  = 1;

        while (cur != NULL && pos < Y) {
            prev = cur;
            cur  = cur->next;
            pos++;
        }

        prev->next = node;
        node->next = cur;
    }

    printf("A: Uspesne pridany zaznam na pozicii %d.\n", pos);

    return 0;
}


//m
int cmd_m(FILE **fSudoku, FILE **fPlayers, FILE **fSolutions,  const char *fnSudoku, const char *fnPlayers, const char *fnSolutions, MNode **pHeadM) {
    char lineP[LINE_MAX];
    char bufP[LINE_MAX];
    char *pf[MAX_FIELDS];
    int nf;
    int i;
    char lineS[LINE_MAX];
    char bufS[LINE_MAX];
    char *sf[MAX_FIELDS];
    int ns;
    int playersCnt = 0;
    int playersCap = 0;
    int found;
    int nRecords = 0;

    MPlayer *players = NULL;
    MNode *head = NULL;
    MNode *tail = NULL;
    MNode *cur;
    MNode *next;
    MNode *node;

    FILE *fp;

    if (*fSudoku == NULL || *fPlayers == NULL || *fSolutions == NULL) {
        printf("M: Neotvoreny subor.\n");

        return 0;
    }

    cur = *pHeadM;

    while (cur != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }

    *pHeadM = NULL;
    fp = *fPlayers;
    rewind(fp);

    while (fgets(lineP, sizeof(lineP), fp)) {
        MPlayer pl;

        chomp(lineP);

        if (lineP[0] == '\0') {
            continue;
        }

        strncpy(bufP, lineP, sizeof(bufP));
        bufP[sizeof(bufP) - 1] = '\0';

        for (i = 0; i < MAX_FIELDS; ++i) {
            pf[i] = NULL;
        }

        nf = splitHashInplace(bufP, pf, MAX_FIELDS);

        memset(&pl, 0, sizeof(MPlayer));

        if (nf >= 1 && pf[0]) {
            strncpy(pl.PID, pf[0], sizeof(pl.PID));
            pl.PID[sizeof(pl.PID) - 1] = '\0';
        }
        if (nf >= 2 && pf[1]) {
            strncpy(pl.Identita, pf[1], sizeof(pl.Identita));
            pl.Identita[sizeof(pl.Identita) - 1] = '\0';
        }
        if (nf >= 3 && pf[2]) {
            strncpy(pl.Krajina, pf[2], sizeof(pl.Krajina));
            pl.Krajina[sizeof(pl.Krajina) - 1] = '\0';
        }
        if (nf >= 4 && pf[3]) {
            pl.RokNar = atoi(pf[3]);
        } else {
            pl.RokNar = 0;
        }

        if (playersCnt == playersCap) {
            int newCap = (playersCap == 0) ? 8 : playersCap * 2;

            MPlayer *tmpPlayers = (MPlayer*)realloc(players, newCap * sizeof(MPlayer));

            if (tmpPlayers == NULL) {
                free(players);
                printf("M: Neotvoreny subor.\n");

                return 0;
            }
            players = tmpPlayers;
            playersCap = newCap;
        }
        players[playersCnt++] = pl;
    }


    fp = *fSolutions;
    rewind(fp);

    while (fgets(lineS, sizeof(lineS), fp)) {
        char gid[64] = "";
        char pid[64] = "";
        char sid[32] = "";
        char date[16] = "";
        char mins[16] = "";
        char secs[16] = "";
        int seconds = 0;
        int last = -1;
        int prev = -1;

        MPlayer plFound;
        MResult res;

        chomp(lineS);

        if (lineS[0] == '\0') {
            continue;
        }

        strncpy(bufS, lineS, sizeof(bufS));
        bufS[sizeof(bufS) - 1] = '\0';

        for (i = 0; i < MAX_FIELDS; ++i) {
            sf[i] = NULL;
        }

        ns = splitHashInplace(bufS, sf, MAX_FIELDS);

        for (i = 0; i < ns; ++i) {
            char tmp[64];

            if (sf[i] == NULL) {
                continue;
            }

            strncpy(tmp, sf[i], sizeof(tmp));
            tmp[sizeof(tmp) - 1] = '\0';
            trimSpaces(tmp);

            if (tmp[0] == '\0') {
                continue;
            }

            if (gid[0] == '\0' && strncmp(tmp, "GID", 3) == 0) {
                strncpy(gid, tmp, sizeof(gid));
                gid[sizeof(gid) - 1] = '\0';

                continue;
            }

            if (pid[0] == '\0' && strncmp(tmp, "PID", 3) == 0) {
                strncpy(pid, tmp, sizeof(pid));
                pid[sizeof(pid) - 1] = '\0';

                continue;
            }

            if (sid[0] == '\0' && strncmp(tmp, "SID", 3) == 0) {
                strncpy(sid, tmp, sizeof(sid));
                sid[sizeof(sid) - 1] = '\0';

                continue;
            }

            if (date[0] == '\0' &&
                strlen(tmp) == 8 && isNdigits(tmp, 8)) {
                strncpy(date, tmp, sizeof(date));
                date[sizeof(date) - 1] = '\0';

                continue;
            }
        }

        for (i = 0; i < ns; ++i) {
            char t[32];

            if (sf[i] == NULL) {
                continue;
            }

            strncpy(t, sf[i], sizeof(t));
            t[sizeof(t) - 1] = '\0';
            trimSpaces(t);

            if (!t[0]) {
                continue;
            }

            if (isNdigits(t, (int)strlen(t))) {
                prev = last;
                last = i;
            }
        }

        if (last >= 0 && sf[last]) {
            strncpy(secs, sf[last], sizeof(secs));
            secs[sizeof(secs) - 1] = '\0';
        }

        if (prev >= 0 && sf[prev]) {
            strncpy(mins, sf[prev], sizeof(mins));
            mins[sizeof(mins) - 1] = '\0';
        }

        seconds = toIntDef(mins, 0) * 60 + toIntDef(secs, 0);
        found = 0;

        for (i = 0; i < playersCnt; ++i) {
            if (strcmp(players[i].PID, pid) == 0) {
                plFound = players[i];
                found = 1;

                break;
            }
        }

        if (!found) {
            continue;
        }

        memset(&res, 0, sizeof(MResult));

        if (sid[0]) {
            strncpy(res.SID, sid, sizeof(res.SID));
            res.SID[sizeof(res.SID) - 1] = '\0';
        }

        if (gid[0]) {
            strncpy(res.GID, gid, sizeof(res.GID));
            res.GID[sizeof(res.GID) - 1] = '\0';
            res.NarSut = gid[3];
        } else {
            res.NarSut = ' ';
        }

        if (date[0]) {
            strncpy(res.DatHry, date, sizeof(res.DatHry));
            res.DatHry[sizeof(res.DatHry) - 1] = '\0';
        }

        res.Trvanie = seconds;
        res.NarHry = 'A';

        node = (MNode*)malloc(sizeof(MNode));

        if (node == NULL) {
            cur = head;

            while (cur != NULL) {
                next = cur->next;
                free(cur);
                cur = next;
            }
            free(players);

            printf("M: Neotvoreny subor.\n");

            return 0;
        }

        node->player = plFound;
        node->result = res;
        node->next = NULL;

        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        nRecords++;
    }

    free(players);

    *pHeadM = head;

    printf("M: Nacitalo sa %d zaznamov.\n", nRecords);

    return 0;
}

// n
int cmd_n(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol, char ***outPlayers,   int *outPlayersCnt, char ***outSolutions, int *outSolutionsCnt) {
    FILE *fp;
    char line[LINE_MAX];
    char **pArr = NULL, **sArr = NULL;
    int pCnt = 0, sCnt = 0, pCap = 0, sCap = 0;

    if (*fPlr == NULL) {
        *fPlr = fopen(fnPlr, "r");
    }

    if (*fSol == NULL) {
        *fSol = fopen(fnSol, "r");
    }

    if (*fPlr == NULL || *fSol == NULL) {
        printf("N: Neotvoreny subor.\n"); 
        return 0;
    }

    rewind(*fPlr);

    fp = *fPlr;
    
    while (fgets(line, sizeof(line), fp)) {
        chomp(line);

        if (!line[0]) {
            continue;
        }

        if (pCnt == pCap) {
            int newCap = (pCap == 0) ? 16 : (pCap * 2);
            char **tmp = (char**)realloc(pArr, newCap * sizeof(char*));

            if (!tmp) {
                printf("N: Neotvoreny subor.\n"); 

                goto fail;
            }

            pArr = tmp; 
            pCap = newCap;
        }
        pArr[pCnt++] = dupstr(line);

        if (!pArr[pCnt-1]) {
            printf("N: Neotvoreny subor.\n"); 
            goto fail; 
        }
    }

    rewind(*fSol);

    fp = *fSol;

    while (fgets(line, sizeof(line), fp)) {
        chomp(line);

        if (!line[0]) {
            continue; 
        }

        if (sCnt == sCap) {
            int newCap = (sCap == 0) ? 32 : (sCap * 2);
            char **tmp = (char**)realloc(sArr, newCap * sizeof(char*));
            if (!tmp) {
                printf("N: Neotvoreny subor.\n"); 
                goto fail;
            }

            sArr = tmp;
            sCap = newCap;
        }

        sArr[sCnt++] = dupstr(line);

        if (!sArr[sCnt - 1]) {
            printf("N: Neotvoreny subor.\n");
            goto fail;
        }
    }

    *outPlayers = pArr;
    *outPlayersCnt = pCnt;
    *outSolutions = sArr;
    *outSolutionsCnt = sCnt;

    return 1;

    fail:
        if (pArr) {
            int i;
            for (i=0;i<pCnt;i++) {
                free(pArr[i]); 
                free(pArr);
            }
        }

        if (sArr) {
            int i; 
            for (i=0;i<sCnt;i++) {
                free(sArr[i]); 
                free(sArr);
            }
        }

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

// w 
int w(char ***solutionsArr, int *solutionsCnt) {
    char pid_target[64];
    int i, write_idx, removed = 0;

    if (!solutionsArr || !solutionsCnt || !*solutionsArr) {
        printf("W: Polia nie su vytvorene.\n");
        return 0;
    }

    if (!readNextNonemptyLine(pid_target, sizeof(pid_target))) {
        return 0;
    }

    trimSpaces(pid_target);

    write_idx = 0;

    for (i = 0; i < *solutionsCnt; ++i) {
        char *line = (*solutionsArr)[i];
        char tmp[LINE_MAX];
        char *fld[MAX_FIELDS];
        char pid_in_line[64];
        int nf, j;

        if (!line) continue;

        strncpy(tmp, line, sizeof(tmp));
        tmp[sizeof(tmp) - 1] = '\0';

        for (j = 0; j < MAX_FIELDS; ++j) fld[j] = NULL;

        nf = splitHashInplace(tmp, fld, MAX_FIELDS);

        pid_in_line[0] = '\0';
        if (nf >= 2 && fld[1]) {
            strncpy(pid_in_line, fld[1], sizeof(pid_in_line));
            pid_in_line[sizeof(pid_in_line) - 1] = '\0';
            trimSpaces(pid_in_line);
        }

        if (pid_in_line[0] != '\0' && strcmp(pid_in_line, pid_target) == 0) {
            free(line);
            
            removed++;
        } else {
            (*solutionsArr)[write_idx++] = line;
        }
    }

    *solutionsCnt = write_idx;

    if (removed > 0) {
        char **tmpArr = (char**)realloc(*solutionsArr, (*solutionsCnt) * sizeof(char*));
        if (tmpArr) *solutionsArr = tmpArr;
    }

    printf("W: Vymazalo sa : %d zaznamov !\n", removed);

    return 0;
}

// e
int cmd_e(FILE **fSudoku, const char *fnSudoku, const char *sid_in, int X) {
    char sid[32];
    char digits81[82];
    char grid[9][10];
    int r, c, seed;

    if (!sid_in || strlen(sid_in) != 8 || strncmp(sid_in, "SID", 3) != 0 || sid_in[3] < 'A' || sid_in[3] > 'Z' || !isdigit((unsigned char)sid_in[4]) || !isdigit((unsigned char)sid_in[5]) || !isdigit((unsigned char)sid_in[6]) || !isdigit((unsigned char)sid_in[7]) || X < 1 || X > 5) {
        printf("E: Nespravny vstup.\n");

        return 0;
    }

    strncpy(sid, sid_in, sizeof(sid));

    sid[sizeof(sid) - 1] = '\0';

    chomp(sid);

    if (!fSudoku) {
        printf("E: Polia nie su vytvorene.\n");
        return 0;
    }

    if (*fSudoku == NULL) {
        *fSudoku = fopen(fnSudoku, "r");
    }

    if (*fSudoku == NULL) {
        printf("E: Polia nie su vytvorene.\n");
        return 0;
    }

    {
        char line[LINE_MAX];
        int count = 0;
        int inside = 0;

        rewind(*fSudoku);

        while (fgets(line, sizeof(line), *fSudoku)) {
            char *p = line;

            chomp(p);

            if (!inside) {
                char *sidpos = strstr(p, sid);

                if (!sidpos) {
                    continue;
                }

                sidpos = strchr(sidpos, '#');

                if (!sidpos) {
                    continue;
                }

                p = sidpos + 1;
                inside = 1;
            }

            while (*p && count < 81) {
                if (*p == '#') {
                    inside = 0; break;
                }

                if (*p >= '1' && *p <= '9') {
                    digits81[count++] = *p;
                }

                p++;
            }

            if (count >= 81) {
                break;
            }
        }

        if (count != 81) {
            printf("E: Nespravny vstup.\n");

            return 0;
        }

        digits81[81] = '\0';
    }

    for (r = 0; r < 9; ++r) {
        for (c = 0; c < 9; ++c) {
            grid[r][c] = digits81[r * 9 + c];
        }

        grid[r][9] = '\0';
    }

    {
        long s = 2166136261L;

        for (c = 0; sid[c]; ++c) {
            s ^= (long)(unsigned char)sid[c];
            s *= 16777619L;
        }

        if (s < 0) {
            s = -s;
        }

        seed = (int)(s & 0x7FFFFFFF);

        for (r = 0; r < 9; ++r) {
            int used[9];
            int k = 0;

            for (c = 0; c < 9; ++c) {
                used[c] = 0;
            }

            while (k < X) {
                long ls = (long)seed;
                ls = ls * 1103515245L + 12345L;
                seed = (int)(ls & 0x7FFFFFFF);
                c = (seed >> 16) % 9;

                if (!used[c]) {
                    used[c] = 1;
                    grid[r][c] = ' ';
                    k++;
                }
            }
        }
    }

    {
        FILE *fo = fopen("Vystup_E.txt", "w");
        if (!fo) {
            printf("E: Polia nie su vytvorene.\n");
            return 0;
        }
        for (r = 0; r < 9; ++r) {
            for (c = 0; c < 9; ++c) {
                fputc('|', fo);
                fputc(grid[r][c], fo);
            }
            fputs("|\n", fo);
        }
        fclose(fo);
    }

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

// v2
int v2(char **playersArr, int playersCnt, char **solutionsArr, int solutionsCnt) {
    int i, printed;
    char lineP[LINE_MAX], bufP[LINE_MAX], *pf[MAX_FIELDS];
    char lineS[LINE_MAX], bufS[LINE_MAX], *sf[MAX_FIELDS];

    if (!playersArr || playersCnt <= 0 || !solutionsArr || solutionsCnt < 0) {
        printf("V2: Nenaplnene polia.\n");

        return 0;
    }

    for (i = 0; i < playersCnt; ++i) {
        int nf, ns, j;
        const char *pid, *meno, *krajina, *rok;

        if (!playersArr[i]) {
            continue;
        }

        strncpy(lineP, playersArr[i], sizeof(lineP));

        lineP[sizeof(lineP)-1] = '\0';

        chomp(lineP);

        strncpy(bufP, lineP, sizeof(bufP));

        bufP[sizeof(bufP)-1] = '\0';

        for (j = 0; j < MAX_FIELDS; ++j) {
            pf[j] = NULL;
        }

        nf = splitHashInplace(bufP, pf, MAX_FIELDS);

        pid = (nf >= 1 && pf[0]) ? pf[0] : "";
        meno = (nf >= 2 && pf[1]) ? pf[1] : "";
        krajina = (nf >= 3 && pf[2]) ? pf[2] : "";
        rok = (nf >= 4 && pf[3]) ? pf[3] : "";

        printf("PID: %s / %s / %s\n", pid, rok, krajina);
        printf("Identita: %s\n", meno);
        printf("Vysledok:\n");

        printed = 0;
        {
            int k;
            for (k = 0; k < solutionsCnt && printed < SAMPLE_LIMIT; ++k) {
                char gid[64] = "";
                char pid_s[64] = "";
                char sid[32] = "";
                char date[16] = "";
                char diff_sut[8] = "";
                char diff_hry[8] = "";
                char mins[16] = "";
                char secs[16] = "";
                int seconds = -1;
                int last = -1, prev = -1;

                if (!solutionsArr[k]) continue;

                strncpy(lineS, solutionsArr[k], sizeof(lineS));
                lineS[sizeof(lineS)-1] = '\0';
                chomp(lineS);

                strncpy(bufS, lineS, sizeof(bufS));
                bufS[sizeof(bufS)-1] = '\0';

                for (j = 0; j < MAX_FIELDS; ++j) sf[j] = NULL;
                ns = splitHashInplace(bufS, sf, MAX_FIELDS);

                for (j = 0; j < ns; ++j) {
                    char tmp[64];
                    if (!sf[j]) continue;

                    strncpy(tmp, sf[j], sizeof(tmp));
                    tmp[sizeof(tmp)-1] = '\0';
                    trimSpaces(tmp);

                    if (gid[0] == '\0' && strncmp(tmp, "GID", 3) == 0) {
                        strncpy(gid, tmp, sizeof(gid)); gid[sizeof(gid)-1] = '\0';

                        continue;
                    }

                    if (pid_s[0] == '\0' && strncmp(tmp, "PID", 3) == 0) {
                        strncpy(pid_s, tmp, sizeof(pid_s)); pid_s[sizeof(pid_s)-1] = '\0';

                        continue;
                    }

                    if (sid[0] == '\0' && strncmp(tmp, "SID", 3) == 0) {
                        strncpy(sid, tmp, sizeof(sid)); sid[sizeof(sid)-1] = '\0';

                        continue;
                    }

                    if (date[0] == '\0' && strlen(tmp) == 8 && isNdigits(tmp, 8)) {
                        strncpy(date, tmp, sizeof(date)); date[sizeof(date)-1] = '\0';

                        continue;
                    }
                }

                if (gid[0] && strlen(gid) >= 4) {
                    diff_sut[0] = gid[3];
                    diff_sut[1] = '\0';
                }

                if (sid[0] && strlen(sid) >= 4) {
                    diff_hry[0] = sid[3];
                    diff_hry[1] = '\0';
                }

                last = -1; prev = -1;

                for (j = 0; j < ns; ++j) {
                    char tt[32];
                    int ok;

                    if (!sf[j]) continue;

                    strncpy(tt, sf[j], sizeof(tt)); tt[sizeof(tt)-1] = '\0';
                    trimSpaces(tt);

                    if (!tt[0]) continue;

                    ok = isNdigits(tt, (int)strlen(tt));

                    if (ok) {
                        prev = last; 
                        last = j;
                    }
                }

                if (last >= 0 && sf[last]) {
                    strncpy(secs, sf[last], sizeof(secs));
                    secs[sizeof(secs)-1] = '\0';
                }
                if (prev >= 0 && sf[prev]) {
                    strncpy(mins, sf[prev], sizeof(mins));
                    mins[sizeof(mins)-1] = '\0';
                }

                seconds = toIntDef(mins, 0) * 60 + toIntDef(secs, 0);

                if (pid_s[0] == '\0' || strcmp(pid_s, pid) != 0) {
                    continue;
                }

                printf("\t%s / %s / %s / %s / %s / %s / %d\n",
                    gid[0] ? gid : "",
                    pid_s[0] ? pid_s : "",
                    sid[0] ? sid : "",
                    date[0] ? date : "",
                    diff_sut[0] ? diff_sut : "",
                    diff_hry[0] ? diff_hry : "",
                    seconds);

                printed++;
            }
        }

        printf("\n");
    }

    return 0;
}

// v3
void v3(MNode *head) {
    MNode *p, *q;
    int firstPrinted;

    if (head == NULL) {
        printf("V3: Nenaplneny spajany zoznam.\n");

        return;
    }

    p = head;

    while (p != NULL) {
        int alreadySeen = 0;

        q = head;

        while (q != p) {
            if (strcmp(q->player.PID, p->player.PID) == 0) {
                alreadySeen = 1;

                break;
            }
            q = q->next;
        }

        if (alreadySeen) {
            p = p->next;

            continue;
        }

        printf("PID: %s\n", p->player.PID);
        printf("Identita: %s\n", p->player.Identita);
        printf("Krajina: %s\n", p->player.Krajina);
        printf("RokNar: %d\n", p->player.RokNar);
        printf("Vysledky:\n");

        firstPrinted = 0;
        q = head;

        while (q != NULL) {
            if (strcmp(q->player.PID, p->player.PID) == 0) {
                if (q->result.SID[0] != '\0') {
                    printf("\t%s / %c / %s / %c / %s / %d\n",
                           q->result.SID,
                           q->result.NarHry ? q->result.NarHry : ' ',
                           q->result.GID,
                           q->result.NarSut ? q->result.NarSut : ' ',
                           q->result.DatHry,
                           q->result.Trvanie);
                    firstPrinted = 1;
                }
            }
            q = q->next;
        }

        printf("\n");

        p = p->next;
    }
}

// v
void v(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol, int choice, char **playersArr, int playersCnt, char **solutionsArr, int solutionsCnt, MNode *head) {
    switch (choice) {
        case 1:
            v1(fSud, fPlr, fSol, fnSud, fnPlr, fnSol);
            break;

        case 2:
            v2(playersArr, playersCnt, solutionsArr, solutionsCnt);
            break;

        case 3:
            v3(head);
            break;

        default:
            printf("V: Nespravna volba vypisu.\n");
            break;
    }
}

//h
int cmd_h(FILE **fSudoku, FILE **fPlayers, FILE **fSolutions, const char *fnSudoku, const char *fnPlayers, const char *fnSolutions, const char *sid_input_line) {
    char sid[32];
    char lineS[LINE_MAX], tmp[LINE_MAX], *sf[MAX_FIELDS];
    char sid_s_norm[32];
    char **rows = NULL;
    int n = 0, cap = 0; 
    int ns, i;

    FILE *fo;

    (void)fPlayers;
    (void)fnPlayers;

    {
        if (*fSudoku == NULL) {
            *fSudoku = fopen(fnSudoku, "r");
        }
        
        if (*fSolutions == NULL) {
            *fSolutions = fopen(fnSolutions, "r");
        }
    }

    if (*fSudoku == NULL || *fSolutions == NULL) {
        printf("H: Neotvoreny txt subor.\n");

        return 0;
    }

    strncpy(sid, sid_input_line, sizeof(sid));

    sid[sizeof(sid)-1] = '\0';

    chomp(sid);

    if (!isValidSIDFormat(sid) || !SIDExistsInSudoku(*fSudoku, sid)) {
        printf("H: Nespravny vstup.\n");

        return 0;
    }

    rewind(*fSolutions);

    while (fgets(lineS, sizeof(lineS), *fSolutions)) {
        chomp(lineS);

        if (lineS[0] == '\0') {
            continue;
        }

        strncpy(tmp, lineS, sizeof(tmp));
        tmp[sizeof(tmp)-1] = '\0';

        for (i = 0; i < MAX_FIELDS; i++) {
            sf[i] = NULL;
        }

        ns = splitHashInplace(tmp, sf, MAX_FIELDS);

        sid_s_norm[0] = '\0';

        for (i = 0; i < ns; i++) {
            if (sf[i] == NULL || sf[i][0] == '\0') {
                continue;
            }

            {
                char candidate[32];

                strncpy(candidate, sf[i], sizeof(candidate));
                candidate[sizeof(candidate)-1] = '\0';
                trimSpaces(candidate);
                toUpperASCII(candidate);

                if (strlen(candidate) == 8 && strncmp(candidate, "SID", 3) == 0) {
                    strcpy(sid_s_norm, candidate);

                    break;
                }
            }
        }

        if (sid_s_norm[0] == '\0') {
            continue; 
        }

        if (strcmp(sid_s_norm, sid) != 0) {
            continue;
        }

        if (n == cap) {
            int newCap = (cap == 0) ? 64 : (cap * 2);
            char **tmpArr = (char**)realloc(rows, newCap * sizeof(char*));

            if (!tmpArr) {
                int k;

                for (k = 0; k < n; ++k) {
                    free(rows[k]);
                }

                free(rows);

                printf("H: Neotvoreny txt subor.\n");

                return 0;
            }
            rows = tmpArr;
            cap = newCap;
        }

        rows[n] = dupstr(lineS);

        if (!rows[n]) {
            int k;

            for (k = 0; k < n; ++k) {
                free(rows[k]);
            }

            free(rows);

            printf("H: Neotvoreny txt subor.\n");

            return 0;
        }

        n++;
    }

    if (n > 1) {
        qsort(rows, (size_t)n, sizeof(char*), cmp_line_by_gid);
    }

    fo = fopen("Vystup_H.txt", "w");

    if (!fo) {
        int k;

        for (k = 0; k < n; ++k) {
            free(rows[k]);
        }

        free(rows);

        printf("H: Neotvoreny txt subor.\n");

        return 0;
    }
    for (i = 0; i < n; ++i) {
        fputs(rows[i], fo);
        fputc('\n', fo);
    }

    fclose(fo);

    for (i = 0; i < n; ++i) {
        free(rows[i]);
    }

    free(rows);

    printf("H: Uspešne vytvoreny sumar.\n");

    return 0;
}

// ine
void handleCommandLoop(FILE **fSud, FILE **fPlr, FILE **fSol, const char *fnSud, const char *fnPlr, const char *fnSol, MNode **pHead) {
    char **sudokuArr = NULL;
    char **playersArr = NULL;
    char **solutionsArr= NULL;

    int sudokuCnt = 0;
    int playersCnt = 0;
    int solutionsCnt = 0;
    char cmdline[256];
    char c;
    int choice;
    char sidline[128];

    MNode *mHead = NULL;

    while (fgets(cmdline, sizeof(cmdline), stdin)) {
        chomp(cmdline);

        if (cmdline[0] == '\0') continue;

        c = 0;
        choice = -1;
        memset(sidline, 0, sizeof(sidline));

        if (sscanf(cmdline, " %c %d", &c, &choice) == 2 && (c=='v' || c=='V')) {
            switch (choice) {
                case 1:
                    v1(fSud, fPlr, fSol, fnSud, fnPlr, fnSol);
                    break;

                case 2:
                    v2(playersArr, playersCnt, solutionsArr, solutionsCnt);
                    break;

                case 3:
                    v3(*pHead);
                    break;

                default:
                    printf("V: Nespravna volba vypisu.\n");
                    break;
            }
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
            if (cmd_n(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, &playersArr, &playersCnt, &solutionsArr, &solutionsCnt)) {
                printf("");
            }

            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'q' || c == 'Q')) {
            q(fSud, fPlr, fSol, &solutionsArr, &solutionsCnt);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'w' || c == 'W')) {
            w(&solutionsArr, &solutionsCnt);
            continue;
        }

        if (sscanf(cmdline, " %c %127s %d", &c, sidline, &choice) == 3 && (c == 'e' || c == 'E')) {
            cmd_e(fSud, fnSud, sidline, choice);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'e' || c == 'E')) {
            char line2[256];
            char sidbuf[128];
            char xbuf[64];

            printf("Zadajte SID a X (1-5): ");

            if (!readNextNonemptyLine(line2, sizeof(line2))) {
                printf("E: Nespravny vstup.\n");
                continue;
            }

            sidbuf[0] = '\0'; xbuf[0] = '\0';

            if (sscanf(line2, " %127s %63s", sidbuf, xbuf) != 2) {
                printf("E: Nespravny vstup.\n");
                continue;
            }

            choice = atoi(xbuf);
            cmd_e(fSud, fnSud, sidbuf, choice);

            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'm' || c == 'M')) {
            cmd_m(fSud, fPlr, fSol, fnSud, fnPlr, fnSol, pHead);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'a' || c == 'A')) {
            cmd_a(pHead);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 's' || c == 'S')) {
            cmd_s(pHead);
            continue;
        }

        if (sscanf(cmdline, " %c", &c) == 1 && (c == 'd' || c == 'D')) {
            cmd_d(pHead);
            continue;
        }

    }

    if (sudokuArr) {
        for (int i = 0; i < sudokuCnt; i++) free(sudokuArr[i]);
        free(sudokuArr);
    }
    if (playersArr) {
        for (int i = 0; i < playersCnt; i++) free(playersArr[i]);
        free(playersArr);
    }
    if (solutionsArr) {
        for (int i = 0; i < solutionsCnt; i++) free(solutionsArr[i]);
        free(solutionsArr);
    }
}



int main(void) {
    FILE *f1 = NULL, *f2 = NULL, *f3 = NULL;

    char *fname1 = "./Sudoku.txt";
    char *fname2 = "./RegisterHracov.txt";
    char *fname3 = "./RegisterRieseni.txt";

    MNode *head = NULL;

    handleCommandLoop(&f1, &f2, &f3, fname1, fname2, fname3, &head);

    if (f1 != NULL) fclose(f1);
    if (f2 != NULL) fclose(f2);
    if (f3 != NULL) fclose(f3);

    
    MNode *tmp;
    while (head != NULL) {
        tmp = head->next;
        free(head);
        head = tmp;
    }

    return 0;
}