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
- RETORNA (retorno de funciones)
- IF / ELIF / ELSE (condicionales)
- SI / SINO_SI / SINO (alias en espanol)
- OLFATEA_HUESO / OLFATEA_CORREA (entrada por teclado)

5) Operadores
- Asignacion: =
- Aritmeticos enteros: + - * /
- Agrupacion: ( )
- Comparacion: == != < <= > >=
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
- Comparaciones solo para enteros y retornan HUESO (0 falso, 1 verdadero).

8) Impresion en pantalla
Sintaxis:
LADRA(expresion);

9) Condicionales (if / elif / else)
Sintaxis recomendada:
IF (expr) { ... }
ELIF (expr) { ... }
ELSE { ... }

Alias en espanol:
SI (expr) { ... }
SINO_SI (expr) { ... }
SINO { ... }

10) Entrada de datos
Sintaxis:
HUESO x = OLFATEA_HUESO();
CORREA nombre = OLFATEA_CORREA();

Reglas:
- Cada llamada lee una linea completa de stdin.
- HUESO debe ser un entero valido en esa linea.
- CORREA conserva la linea tal cual (sin el salto de linea).

Nota: para usar entrada interactiva, ejecute el compilador con archivo de programa:
./golden tu_programa.grl

11) Gramatica simplificada (EBNF)
programa      := { sentencia } EOF ;
sentencia     := decl_entero | decl_string | asignacion | print ;
decl_entero   := "HUESO" ID "=" expr ";" ;
decl_string   := "CORREA" ID "=" expr ";" ;
asignacion    := ID "=" expr ";" ;
print         := "LADRA" "(" expr ")" ";" ;
condicional   := (IF|SI) "(" expr ")" bloque { (ELIF|SINO_SI) "(" expr ")" bloque } [ (ELSE|SINO) bloque ] ;
bloque        := "{" { sentencia } "}" ;
comparador    := "==" | "!=" | "<" | "<=" | ">" | ">=" ;
expr          := sum { comparador sum } ;
sum           := term { ("+" | "-") term } ;
term          := factor { ("*" | "/") factor } ;
factor        := NUMBER | STRING | ID | entrada | "(" expr ")" ;
entrada       := "OLFATEA_HUESO" "(" ")" | "OLFATEA_CORREA" "(" ")" ;

10) Ejemplo rapido
HUESO pelotas = 10;
HUESO paseos = 5;
HUESO total = pelotas + paseos * 2;
CORREA perro = "Max";
LADRA(perro);
LADRA(total);
