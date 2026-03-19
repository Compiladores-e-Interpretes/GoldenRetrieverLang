GoldenRetrieverLang - Reglas del Lenguaje

Implementacion solicitada en:
- `interprete_golden.c`
- `interprete_golden.h`
- `golden_lexer.l`

1) Objetivo
Lenguaje academico muy simple para practicar compiladores/interpretes.
Tematica: Golden Retriever.

2) Tipos de datos soportados
- HUESO: entero (int)
- CORREA: cadena de texto (string)

3) Identificadores
- Patron: letra o guion bajo inicial, seguido de letras, numeros o guion bajo.
- Forma regex: [A-Za-z_][A-Za-z0-9_]*
- Sensible a mayusculas y minusculas.
- No se recomienda usar palabras reservadas como identificadores.

4) Palabras reservadas
- HUESO   (declaracion de entero)
- CORREA  (declaracion de string)
- LADRA   (impresion en pantalla)

5) Operadores
- Asignacion: =
- Aritmeticos enteros: + - * /
- Agrupacion: ( )
- Fin de sentencia: ;

6) Comentarios
- Comentario de linea: // texto
- Comentario de bloque: /* texto */

7) Reglas semanticas
- HUESO solo guarda enteros.
- CORREA solo guarda strings.
- Solo se puede reasignar una variable ya declarada.
- + entre enteros suma enteros.
- + entre strings concatena strings.
- -, *, / solo para enteros.
- Division es entera (a // b).
- Division por cero: error.

8) Impresion en pantalla
Sintaxis:
LADRA(expresion);

9) Gramatica simplificada (EBNF)
programa      := { sentencia } EOF ;
sentencia     := decl_entero | decl_string | asignacion | print ;
decl_entero   := "HUESO" ID "=" expr ";" ;
decl_string   := "CORREA" ID "=" expr ";" ;
asignacion    := ID "=" expr ";" ;
print         := "LADRA" "(" expr ")" ";" ;
expr          := term { ("+" | "-") term } ;
term          := factor { ("*" | "/") factor } ;
factor        := NUMBER | STRING | ID | "(" expr ")" ;

10) Ejemplo rapido
HUESO pelotas = 10;
HUESO paseos = 5;
HUESO total = pelotas + paseos * 2;
CORREA perro = "Max";
LADRA(perro);
LADRA(total);
