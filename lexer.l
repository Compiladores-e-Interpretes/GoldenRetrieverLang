%option noyywrap yylineno noinput nounput

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbols.h"
#include "parser.tab.h"

static char* xstrdup(const char* s) {
    size_t n = strlen(s) + 1;
    char* out = (char*)malloc(n);
    if (out == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }
    memcpy(out, s, n);
    return out;
}

static char* unescape_string(const char* literal) {
    size_t len = strlen(literal);
    char* out = (char*)malloc(len);
    size_t i = 1;
    size_t j = 0;

    if (out == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }

    while (i + 1 < len) {
        if (literal[i] == '\\' && i + 2 < len) {
            i++;
            if (literal[i] == 'n') {
                out[j++] = '\n';
            } else if (literal[i] == 't') {
                out[j++] = '\t';
            } else {
                out[j++] = literal[i];
            }
        } else {
            out[j++] = literal[i];
        }
        i++;
    }

    out[j] = '\0';
    return out;
}
%}

LETRA      [A-Za-z_]
DIGITO     [0-9]
IDF        {LETRA}({LETRA}|{DIGITO})*
ENTERO     {DIGITO}+
CADENA     \"([^\"\\]|\\.)*\"

%%
"HUESO"                      { return HUESO; }
"CORREA"                     { return CORREA; }
"LADRA"                      { return LADRA; }

{ENTERO}                      { yylval.ival = atoi(yytext); return NUMBER; }
{CADENA}                      { yylval.sval = unescape_string(yytext); return STRING; }
{IDF}                         { yylval.sval = xstrdup(yytext); return ID; }

"+"                           { return '+'; }
"-"                           { return '-'; }
"*"                           { return '*'; }
"/"                           { return '/'; }
"="                           { return '='; }
"("                           { return '('; }
")"                           { return ')'; }
"{"                           { return '{'; }
"}"                           { return '}'; }
";"                           { return ';'; }

"//"[^\n]*                   { /* comentario de linea */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* comentario de bloque */ }
[ \t\r\n]+                   { /* ignorar espacios */ }

.                             { return yytext[0]; }
%%
