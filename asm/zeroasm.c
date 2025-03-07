#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define DEBUG 0

#define byte unsigned char
#define uint unsigned int

#define true 1
#define false 0

#define ABORT(msg) {printf("## %s\07\n", msg); exit(10);}
#define ERROR(f)   if(flag == ' '){ flag = f; clm = ptr - stm + 1; }
#define ERRTN(flg) {ERROR(flg) return ERR;}

/* 行の種類の定義 */
#define CMNT 0 /* コメント行   */
#define HOME 1 /* 改ページ     */
#define ERR  2 /* エラー行     */
#define MOVE 3 /* 演算転送命令 */
#define JUMP 4 /* 条件分岐命令 */
#define EQU  5 /* equ 命令     */
#define ORG  6 /* org 命令     */

/* データ型の定義 */
#define CNS 1 /* 定数    : n */
#define ADR 2 /* アドレス: a */
#define REG 3 /* レジスタ: r */
#define CND 4 /* 条件    : c */
#define FNC 5 /* 関数名  : f */
#define RSV 6 /* 先約語  : ? */

/* シンボル・テーブルの定義 */
#define TABLE struct table_struct
TABLE {
    char *name; /* 英数字名 */
    int  value; /* 値       */
    char type;  /* データ型 */
    char def;   /* 定義回数 */
};

/* シンボル・テーブル */
#define LSIZE 200
#define RNMB 53
TABLE symbol [LSIZE+1]= {
    {"acc", 0x00, REG, 1}, {"psw", 0x01, REG, 1}, {"adH", 0x02, REG, 1}, {"adL", 0x03, REG, 1},
    {"mem", 0x04, REG, 1}, {"tmr", 0x05, REG, 1}, {"pio", 0x06, REG, 1}, {"sio", 0x07, REG, 1},

    {"reg0", 0x10, REG, 1}, {"reg1", 0x11, REG, 1}, {"reg2", 0x12, REG, 1}, {"reg3", 0x13, REG, 1},
    {"reg4", 0x14, REG, 1}, {"reg5", 0x15, REG, 1}, {"reg6", 0x16, REG, 1}, {"reg7", 0x17, REG, 1},
    {"reg8", 0x18, REG, 1}, {"reg9", 0x19, REG, 1}, {"regA", 0x1A, REG, 1}, {"regB", 0x1B, REG, 1},
    {"regC", 0x1C, REG, 1}, {"regD", 0x1D, REG, 1}, {"regE", 0x1E, REG, 1}, {"regF", 0x1F, REG, 1},

    {"move", 0x0, FNC, 1}, {"inc",  0x1, FNC, 1}, {"incc", 0x2, FNC, 1}, {"dec",  0x3, FNC, 1},
    {"decc", 0x4, FNC, 1}, {"add",  0x5, FNC, 1}, {"adc",  0x6, FNC, 1}, {"sub",  0x7, FNC, 1},
    {"sbc",  0x8, FNC, 1}, {"and",  0x9, FNC, 1}, {"or",   0xA, FNC, 1}, {"xor",  0xB, FNC, 1},
    {"rr",   0xC, FNC, 1}, {"rl",   0xD, FNC, 1}, {"rrc",  0xE, FNC, 1}, {"rlc",  0xF, FNC, 1},

    {"c", 0, CND, 1}, {"n", 1, CND, 1}, {"v", 2, CND, 1}, {"o", 3, CND, 1},
    {"h", 4, CND, 1}, {"i", 5, CND, 1}, {"z", 6, CND, 1}, {"s", 7, CND, 1},

    {"cns", CNS, RSV, 1}, {"adr", ADR, RSV, 1},
    {"reg", REG, RSV, 1}, {"cnd", CND, RSV, 1},
    {"fnc", FNC, RSV, 1},

    {"", 0, 0, 0}
};
int lnmb=RNMB;

/* ファイル・ポインタ */
FILE *src, *obj1, *obj2, *list;
/* オブジェクト・ファイル名 */
char name[81], *end;

/* 解析パラメータ */
char flag, clm, rom1, rom2, stm[81], *ptr;
int page, adrs;
int pass, line, size, val;
TABLE *label, *sl;

void pass_1();
void pass_2();
int put_object();
void print_list();
void print_label(int errcnt);

void statement();
int move_statement();
int if_statement();
int goto_statement();
int org_statement();
int equ_statement();

void skip_space();
int check_chr(char c);
int check_wrd(char *s);

TABLE *catalog_symbol();
void define_label(int v, int t);

int get_expression(int *t);
int get_term(int *t);
int get_ior(int *t);
int get_and(int *t);
int get_not(int *t);
int get_shift(int *t);
int get_value(int *t);
void t_check(int s, int t);
int x_digit(int c);


/*********************/
/* アセンブラ メイン */
/*********************/
int main(int argn, char *argv[]) {
    if (argn != 2)
        ABORT("usage: ASM src");
    strcpy(name, argv[1]);
    if (!(end=strchr(name, '.')))
        end = name + strlen(name);
    pass_1();
    pass_2();
}

/* 文の読み込み */
char *get_statement() {
    char *r;
    int n;
    r = fgets(stm, 80, src);
    if (r == 0)
        return NULL;
    n = strlen(stm);
    if (stm[n-1] == '\n')
        stm[n-1] = '\0';
    return r;
}

/* パス-1 */
void pass_1() {
    pass = 1;
    page = adrs = 0;
    src = fopen(name, "r");
    if (!src) ABORT("src: not found");
    while (get_statement()) {
        statement();
        adrs += size;
    }
    fclose(src);
    printf("** PASS-1 END **\n");
}

/* パス-2 */
void pass_2() {
    uint n, errcnt = 0;

    pass = 2;
    page = adrs = 0;
    src = fopen(name, "r");
    strcpy(end, ".r1");  obj1 = fopen(name, "wb+");
    strcpy(end, ".r2");  obj2 = fopen(name, "wb+");
    strcpy(end, ".lst"); list = fopen(name, "w");
    if (!obj1 || !obj2)
        ABORT("can't create obj files");
    if (!list)
        ABORT("cant't create a list file");
    fseek(obj1, 0L, 0);
    fseek(obj2, 0L, 0);
    for (n = 0; n <= 0x7FFF; n++) {
        fputc(0x81, obj1);
        fputc(0x00, obj2);
    }
    while (get_statement()) {
        statement();
        if (line == MOVE || line == JUMP)
            put_object();
        print_list();
        adrs += size;
        if (flag != ' ')
            ++errcnt;
    }
    print_label(errcnt);
    fclose(list);
    fclose(obj2);
    fclose(obj1);
    fclose(src);
}

/* オブジェクト生成 */
int put_object() {
    long loc;

    if (adrs > 127)
        ERRTN('V');
    loc = page * 128 + adrs;
    fseek(obj1, loc, 0);
    if (fgetc(obj1) != 0x81)
        ERRTN('J')
    fseek(obj1, loc, 0);
    fseek(obj2, loc, 0);
    fputc(rom1, obj1);
    fputc(rom2, obj2);
    return 0;
}

/* ソースリスト印刷 */
void print_list() {
    static int pag = 0, lin = 0;

    if (line == HOME || lin == 0) {
        fprintf(list, "\f\nF(no) PGAD R1R2 --- SOURCE LIST --- \
                               P.%d\n\n", ++pag);
        lin = 1;
    } else if (lin++ == 65)
        lin = 0;
    if (line == HOME)
        return;
    if (flag != ' ')  fprintf(list, "%c(%02d) ", flag, clm);
    else              fprintf(list, "      ");
    if (line >= MOVE) fprintf(list, "%02X%02X ", page, adrs);
    else              fprintf(list, "     ");
    if (line == MOVE || line == JUMP)
        fprintf(list, "%02X%02X ", ((byte)rom1), ((byte)rom2));
    else if (line == EQU) fprintf(list, "%04X ", val);
    else                  fprintf(list, "     ");
    fprintf(list, "\t%s\n", stm);
}

/* シンボルテーブルの比較 */
int symcmp(const void *ss, const void *tt) {
    TABLE *s = (TABLE *)ss;
    TABLE *t = (TABLE *)tt;
    return strcmp(s->name, t->name);
}

/* ラベル一覧表の印刷 */
void print_label(int errcnt /* エラーカウント */) {
    TABLE *p;
    int n;

    if (lnmb > RNMB) {
        fprintf(list, "\f\n\n ** symbol list **\n\n");
        for (n = 0; n < 4; n++)
            fprintf(list, "NAME____ cE HEXA    ");
        fprintf(list, "\n");
        qsort(symbol+RNMB, lnmb-RNMB, sizeof(TABLE), symcmp);
        for (n = 0, p = symbol+RNMB; n < lnmb-RNMB; n++, p++) {
            if (n && (n%4 == 0)) fputc('\n', list);
            fprintf(list, "%-8.8s%c%c%c %04x    ",
                p->name,
                (strlen(p->name) > 8) ? '*' : ' ',
                "?narcf?"[(int)p->type],
                (p->def == 0) ? 'U' : (p->def == 1) ? ' ' : 'M',
                p->value);
        }
        fprintf(list, "\n\n symbol no = %d\n", lnmb-RNMB);
    }
    if (errcnt) {
        fprintf(list, "\n\n ## ERROR COUNT = %d\n", errcnt);
        printf("\n ## ERROR COUNT = %d\07\n", errcnt);
    } else {
        fprintf(list, "\n\n ** NO ERROR **\n");
        printf("\n ** NO ERROR **\n");
    }
}


/**************/
/* 文法の解析 */
/**************/

/* 行形式 */
void statement() {

#if DEBUG
    static int n=1;
    printf("s%3d: %s\n", n, stm);
#endif

    ptr = stm; flag = ' '; rom1 = rom2 = size = 0;
    if (*ptr == '\f') { line = HOME; return; }
#if DEBUG
    printf("s%3d: label: %p --- %s\n", n, label, ptr);
#endif
    label = isupper(*ptr) ? catalog_symbol() : NULL;
    if (label) check_chr(':');
#if DEBUG
    printf("s%3d: label: %p === %s\n", n, label, ptr);
#endif
    line = check_wrd("org")  ? org_statement()
         : check_wrd("equ")  ? equ_statement()
         : check_wrd("if")   ? if_statement()
         : check_wrd("goto") ? goto_statement()
         : isalpha(*ptr)     ? move_statement()
         : CMNT;
    skip_space();
    if (*ptr && *ptr != ';') ERROR('E');
#if DEBUG
    printf("s%3d: %s\n", n++, ptr);
#endif
    define_label((page<<8)|adrs, ADR);
}


/* 演算·転送命令 */
int move_statement () {
    int fnc, g1, g2, t;

    g1 = get_expression(&t);
    if (t != REG) ERRTN('Z');
    rom1 |= (g1 << 1);
    if (check_chr(',')) {
        if (!check_wrd("psw")) ERRTN('W');
        rom1 |= 0x40;
    }
    if (!check_chr('=')) ERRTN('=');
    fnc = get_expression(&t);
    switch(t) {
    case FNC:
        rom1 |= (fnc >> 3);
        rom2 |= (fnc << 5);
        if (!check_chr('(')) ERRTN('(');
        g2= get_expression(&t);
        if (t != REG) ERRTN('R');
        rom2 |= g2;
        if (!check_chr(')')) ERRTN(')');
        break;
    case REG:
        rom2 |= fnc;
        break;
    case CNS:
        if (fnc > 255 || fnc < -128) ERRTN('N');
        rom1 |= 0x80;
        rom2 |= fnc;
        break;
    default:
        ERRTN('S');
        break;
    }
    size = 1;
    return MOVE;
}

/* 条件命令 */
int if_statement() {
    int n, t;

    rom1 |= 0xE0;
    if (check_chr('!')) rom1 |= 0x10;
    n = get_expression(&t);
    if ((t != CND && t != CNS) || n < 0 || n > 7) ERRTN('1');
    rom1 |= n << 1;
    if (!check_wrd("goto")) ERRTN('T')
    return goto_statement();
}

/* 分歧命令 */
int goto_statement() {
    uint n;
    int t, f=0;

    rom1 |= 0xC0;
    if (check_wrd("acc")) {
        rom2 |= 0x80; rom1 |= 0x01;
        if (!check_chr(',')) ERRTN(',');
        f = 1;
    } else if (check_wrd("zero")) {
        rom2 |= 0x80;
        if (!check_chr(',')) ERRTN(',');
        f = 2;
    }
    n = get_expression(&t);
    if ((t != ADR && t != CNS) || (n & 0x80)) ERROR('A');
    if (f != 1 && (n>>8) != (f ? 0 : page)) ERROR('P');
    rom2 |= (n & 0x7F);
    size = 1;
    return JUMP;
}

/* ORG 疑似命令 */
int org_statement() {
    int n, t;

    n = get_expression(&t);
    if (check_chr(',')) {
        switch(t) {
        case ADR: page = n >> 8; break;
        case CNS: page = n; break;
        default:  ERRTN('P'); break;
        }
        n = get_expression(&t);
        switch(t) {
        case ADR: adrs = n & 0xFF; break;
        case CNS: adrs = n; break;
        default:  ERRTN('A'); break;
        }
    } else {
        switch(t) {
        case ADR: case CNS: page = n >> 8; adrs = n & 0xFF; break;
        default: ERRTN('A'); break;
        }
    }
    if (page < 0 || page > 255) ERRTN('P');
    if (adrs < 0 || adrs > 127) ERRTN('A');
    return ORG;
}

/* EQU 疑似命令 */
int equ_statement () {
    int v, t, v1, t1;
    if (!label) ERRTN('L');
    v = get_expression(&t);
    if (check_chr(',')) {
        v1 = get_expression(&t1);
        if (t1 != RSV) ERROR('Q');
        t = v1;
    }
    if(flag != ' ') label = 0;
    define_label(v, t);
    return EQU;
}


/**********************/
/* ソース文字列の処理 */
/**********************/

/* 空白文字の読み飛ばし */
void skip_space() {
    while (isspace(*ptr)) ++ptr;
}

/* 区切り文字の照合 */
int check_chr(char c /* 区切り文字*/) {
    skip_space();
    if (*ptr != c) return '\0';
    ++ptr;
    return c;
}

/* 文字列の照合 */
int check_str(char *s /* 文字列 */) {
    char *p;

    skip_space();
    for (p=ptr; *s && *s == *p; p++) ++s;
    if (*s) return false;
    ptr = p;
    return true;
}

/* 単語の照合 */
int check_wrd(char *s /* 単語 */) {
    char *p;

    skip_space();
    for (p=ptr; *s && *s == *p; p++) ++s;
    if (*s || isalnum(*p)) return false;
    ptr = p;
    return true;
}


/****************/
/* ラベルの処理 */
/****************/

/* ラベル作成 */
TABLE *create_symbol(char *s /* 名前 */, int z /* 文字数 */) {
    char *m;
    TABLE *r = symbol+lnmb;

#if DEBUG
    static int n=1;
    printf("cr%2d: %s(len=%d)\n", n, s, z);
#endif

    if (lnmb >= LSIZE) ABORT("ラベル数がオーバーしました");
    m = malloc(z+1);
    if (!m) ABORT("メモリが確保できません");
    if (lnmb + 1 < LSIZE) (r+1)->name = r->name;
    r->name = strncpy(m, s, z);
    r->name[z] = '\0';
    r->value = 0;
    r->def = 0;
    ++lnmb;
    return r;
}

/* ラベルの登録 */
TABLE *catalog_symbol() {
    int c;
    TABLE *p=NULL;
    char *t=ptr;

#if DEBUG
    static int n=1;
    printf("ct%2d: %d, %s\n", n, lnmb, t);
#endif

    while (isalnum(*ptr)) ++ptr;
    c = *ptr; *ptr = '\0';
#if DEBUG
    printf("ct%2d: %s, %s\n", n, t, ptr + 1);
    printf("ct  : p->name: %p, %s\n", symbol+lnmb, (symbol+lnmb)->name);
#endif
    for (p=symbol+lnmb; p >= symbol; p--)
        if (strcmp(p->name, t) == 0) break;
    if (p < symbol) p = create_symbol(t, ptr-t);
    *ptr = c;
#if DEBUG
    printf("ct%2d: %s\n", n++, ptr);
#endif
    return p;
}

/* ラベルの検索 */
int search_symbol(int *t /* 型 */) {

#if DEBUG
    printf("ss  : %p, %08x\n", t, *t);
#endif

    sl = catalog_symbol();
    if (!sl || sl->def == 0) { ERROR('U'); return(0); }
    *t = sl->type;
    return sl->value;
}

/* ラベルの定義 */
void define_label(int v, /* 値 */ int t /* 型 */) {
#if DEBUG
    printf("d   : %d, %d\n", v, t);
#endif
    if (!label) return;
    if (pass == 1) {
        label->value = v;
        if (label->def < 2) ++label->def;
        label->type = t;
    } else if (label->def > 1)
        ERROR('M');
    label = 0;
}


/************/
/* 式の解析 */
/************/

int get_expression(int *t /* 型 */) {
    int r, r1, t1, f, g;

    if ((f=check_chr('-')) || (g=check_chr('+')));
    r = get_term(t);
    if (f) r = -r;
    if ((f || g) && *t != CNS) t_check(*t, FNC);
    while ((f=check_chr('+')) || check_chr('-')) {
        r1 = get_term(&t1);
        r = (f ? r + r1 : r - r1);
        if (!f && *t == ADR && t1 == ADR) *t = t1 = CNS;
        t_check(*t, t1);
    }
    return r;
}

/* 積の解析 */
int get_term(int *t /* 型 */) {
    int r, r1, t1, f, g;

    r = get_ior(t);
    while ((f=check_chr('*')) || (g=check_chr('/')) || check_chr('%')) {
        r1 = get_ior(&t1);
        r = (f ? r * r1 : g ? r / r1 : r % r1);
        t_check(*t, t1);
    }
    return r;
}

/* 論理和の解析 */
int get_ior(int *t /* 型 */) {
    int r, t1;

    r = get_and(t);
    while (check_chr('|')) {
        r |= get_and(&t1);
        t_check(*t, t1);
    }
    return r;
}

/* 論理積の解析 */
int get_and(int *t /* 型 */) {
    int r, t1;

    r = get_not(t);
    while (check_chr('&')) {
        r &= get_not(&t1);
        t_check(*t, t1);
    }
    return r;
}

/* 論理否定の解析 */
int get_not(int *t /* 型 */) {
    int r;

    if (check_chr('~')) {
        r = (~get_shift(t)) & 0xFF;
        t_check(CNS, *t);
        return r;
    } else
        return get_shift(t);
}

/* シフトの解析 */
int get_shift(int *t /* 型 */) {
    int r, t1;

    r = get_value(t);
    if (check_str("<<")) {
        r = (r << get_value(&t1)) & 0xFF;
        t_check(*t, t1);
    } else if (check_str(">>")) {
        r = (r >> get_value(&t1)) & 0xFF;
        t_check(*t, t1);
    }
    return r;
}

/* 値の解析 */
int get_value(int *t /* 型 */) {
    int r=0;

    if (check_chr('(')) {
        r = get_expression(t);
        if (!check_chr(')')) ERROR(')');
        if (*t == FNC) ERROR('F');
    } else if (check_chr('$')) {
        r = (page << 8) | adrs;
        *t = ADR;
    } else if (*ptr == '0') {
        while (isxdigit(*ptr)) r = r * 16 + x_digit(*ptr++);
        *t = CNS;
    } else if (isdigit(*ptr)) {
        while (isdigit(*ptr)) r = r * 10 + (*ptr++ - '0');
        *t = CNS;
    } else if (check_wrd("page")) {
        if (!check_chr('(')) ERROR('(');
        *t = CNS;
        if (isupper(*ptr)) r = search_symbol(t) >> 8;
        if (*t != ADR) ERROR('L');
        if (!check_chr(')')) ERROR(')');
        *t = CNS;
    } else if (check_wrd("adrs")) {
        if (!check_chr('(')) ERROR('(');
        *t = CNS;
        if (isupper(*ptr)) r = search_symbol(t) & 0xFF;
        if (*t != ADR) ERROR('L');
        if (!check_chr(')')) ERROR(')');
        *t = CNS;
    } else if (isalpha(*ptr))
        r = search_symbol(t);
    else
        ERROR('X');
    return r;
}

/* 型チェック */
void t_check(int s, /* 第1項 */ int t /* 第2項 */) {
    switch (s) {
    case CNS: if (t != CNS) ERROR('N'); break;
    case ADR: if (t != CNS) ERROR('A'); break;
    case REG: ERROR('R'); break;
    case CND: ERROR('1'); break;
    case FNC: ERROR('F'); break;
    case RSV: ERROR('Y'); break;
    }
}

/* 16進数1桁の値 */
int x_digit(int c /* 文字コード */) {
    if (c >= 'a' && c <= 'f') return(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return(c - 'A' + 10);
    return c - '0';
}
