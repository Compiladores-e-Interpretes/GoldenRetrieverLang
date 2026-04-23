# Proyecto Final — Construcción de un Compilador
## GoldenRetrieverLang (GRL)

**Curso:** Compiladores
**Valor total del proyecto:** 30%
**Lenguaje de implementación:** C (con Flex + Bison)
**Archivo fuente de pruebas:** `prueba`, `calculadora.grl`, `calculadora2.grl`

---

## 1. Resumen Ejecutivo

GoldenRetrieverLang (GRL) es un lenguaje de programación académico de tipado estático,
con sintaxis temática en español basada en la figura del perro Golden Retriever.
El compilador/intérprete se implementó en C siguiendo las fases clásicas descritas
por Aho, Lam, Sethi y Ullman en *Compiladores: principios, técnicas y herramientas*:

1. **Análisis léxico** (`lexer.l`, vía Flex).
2. **Análisis sintáctico** (`parser.y`, vía Bison) con construcción de un **AST**.
3. **Análisis semántico** (`src/symbols.c`, `src/parser_helper.c`) con tabla de símbolos
   basada en **pila de ámbitos** y **shadowing** lexical por bloque.
4. **Generación/Ejecución** (`src/interpreter.c`) mediante un intérprete de árbol
   (*tree-walking interpreter*) que recorre el AST y produce la salida final.

El entregable es el binario `golden`, que recibe un archivo fuente `.grl` y ejecuta
el programa validando sintaxis, tipos y ámbitos en el proceso.

---

## 2. Propuesta del Lenguaje

### 2.1 Tipos de datos

| Palabra clave | Tipo interno | Descripción                |
|---------------|--------------|----------------------------|
| `HUESO`       | `int`        | Entero con signo.          |
| `CORREA`      | `char*`      | Cadena de texto (string).  |

### 2.2 Palabras reservadas

`HUESO`, `CORREA`, `LADRA`, `RETORNA`, `IF`, `ELIF`, `ELSE`, `SI`, `SINO_SI`,
`SINO`, `OLFATEA_HUESO`, `OLFATEA_CORREA`.

### 2.3 Operadores

- Aritméticos enteros: `+`, `-`, `*`, `/` (división entera).
- Concatenación de strings: `+`.
- Comparación (solo enteros, retornan `HUESO` 0/1): `==`, `!=`, `<`, `<=`, `>`, `>=`.
- Asignación: `=`.
- Agrupación: `(` `)`, bloques `{` `}`.

### 2.4 Gramática simplificada (EBNF)

```ebnf
programa      := { sentencia } EOF ;
sentencia     := decl_var | asignacion | print | condicional
               | bloque | decl_func | retorno ;
decl_var      := ("HUESO" | "CORREA") ID "=" expr ";" ;
asignacion    := ID "=" expr ";" ;
print         := "LADRA" "(" expr ")" ";" ;
condicional   := (IF|SI) "(" expr ")" bloque
                 { (ELIF|SINO_SI) "(" expr ")" bloque }
                 [ (ELSE|SINO) bloque ] ;
bloque        := "{" { sentencia } "}" ;
decl_func     := ("HUESO"|"CORREA") ID "(" [ parametros ] ")" bloque ;
parametros    := param_decl { "," param_decl } ;
param_decl    := ("HUESO"|"CORREA") ID ;
retorno       := "RETORNA" expr ";" ;
expr          := sum { comparador sum } ;
sum           := term { ("+" | "-") term } ;
term          := factor { ("*" | "/") factor } ;
factor        := NUMBER | STRING | ID | llamada | entrada | "(" expr ")" ;
llamada       := ID "(" [ expr { "," expr } ] ")" ;
entrada       := "OLFATEA_HUESO" "(" ")" | "OLFATEA_CORREA" "(" ")" ;
```

---

## 3. Implementación de las Fases (Rúbrica I — 10%)

### 3.1 Análisis Léxico — `lexer.l`

Se utilizó **Flex** para generar el analizador léxico. El archivo `lexer.l`
define las clases de caracteres, patrones y acciones que convierten el flujo
de caracteres de entrada en **tokens** reconocidos por el parser.

**Aspectos destacables:**

- Tokens para palabras reservadas (`HUESO`, `CORREA`, `LADRA`, `RETORNA`,
  `IF/ELIF/ELSE`, alias españoles `SI/SINO_SI/SINO`, funciones de entrada).
- Identificadores con expresión regular `[A-Za-z_][A-Za-z0-9_]*`.
- Números enteros (`{DIGITO}+`) con conversión vía `atoi`.
- Strings con soporte de secuencias de escape `\n`, `\t`, `\"` (función
  interna `unescape_string`) y gestión manual de memoria (`xstrdup`).
- Comentarios de línea `// ...` y de bloque `/* ... */`.
- Operadores relacionales multicarácter (`==`, `!=`, `<=`, `>=`) antes que
  los de un solo carácter para evitar conflictos de máximo-munch.
- Activación de `%option yylineno` para reportes de error con número de línea.

**Diagrama simplificado de transición de tokens:**

```
entrada ──► [Flex DFA] ──► (token, yylval) ──► parser
```

### 3.2 Análisis Sintáctico — `parser.y`

El parser se implementa con **Bison** (LALR(1)). Se declaran precedencias
para resolver ambigüedad de la gramática de expresiones:

```
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
```

Durante el análisis, **cada regla construye nodos del AST** usando los
constructores definidos en `src/ast.c` (`ast_make_*`). El AST resultante
se almacena en la variable estática `g_root` dentro de `parser.y`, y es
recorrido por el intérprete una vez que el parseo termina con éxito.

**Tipos de nodo del AST** (`src/headers/ast.h`):

```
AST_BLOCK, AST_SCOPE, AST_VAR_DECL, AST_ASSIGN, AST_PRINT, AST_IF,
AST_INPUT, AST_FUNC_DECL, AST_RETURN, AST_INT, AST_STRING, AST_VAR,
AST_BINOP, AST_FUNC_CALL
```

Esto cubre declaraciones, asignaciones, impresiones, condicionales con
cadenas `IF/ELIF/ELSE`, literales, variables, operaciones binarias y
funciones definidas por el usuario con parámetros.

### 3.3 Análisis Semántico — `src/symbols.c`, `src/parser_helper.c`

La tabla de símbolos se modela como una **pila de ámbitos** (`scope_stack`
con `MAX_SCOPES = 64` y `MAX_SYMBOLS_PER_SCOPE = 256`). Cada entrada de
símbolo almacena nombre, valor (`Value`), tipo y un `offset` incremental
que emula la asignación de memoria local en un marco de activación.

**Reglas semánticas verificadas:**

| Regla                                               | Mecanismo                                 |
|-----------------------------------------------------|-------------------------------------------|
| No re-declarar variable en el mismo ámbito          | `declare_symbol` devuelve 0 si ya existe  |
| Una variable debe estar declarada antes de usarse   | `variable_value_checked` → `helper_fail`  |
| `HUESO` solo guarda enteros, `CORREA` solo strings  | Cotejo de `expected_type` en declaración  |
| Reasignación preserva el tipo original              | `assign_symbol` compara tipos             |
| `+` permitido entre enteros y entre strings         | `add_values_checked`                      |
| `-`, `*`, `/` exclusivos de enteros                 | `sub/mul/div_values_checked`              |
| División por cero                                   | `div_values_checked` → error en línea     |
| Comparación solo entre enteros                      | `eval_compare`                            |
| Condición de `IF` debe ser `HUESO`                  | `exec_if` valida tipo de la condición     |
| Argumentos de función deben coincidir en número/tipo| `eval_call` en `interpreter.c`            |
| `RETORNA` solo dentro de funciones                  | Flag `in_function` en `exec_statement`    |
| Shadowing de variables por bloque                   | `push_scope` / `pop_scope`                |

**Gestión de ámbitos:** al entrar a un `{...}` se llama `push_scope`;
al salir, `pop_scope` libera los valores del nivel cerrado y restaura
el `next_offset` al `offset_base` del ámbito, reciclando la memoria
lógica. La búsqueda (`find_in_stack`) recorre del tope hacia abajo,
garantizando que la declaración más interna gane (*shadowing*).

### 3.4 Generación / Ejecución — `src/interpreter.c`

Como entregable final se optó por un **intérprete de AST**
(*tree-walking interpreter*) en lugar de generación a código máquina
o ensamblador, decisión que se justifica por el alcance académico y
la posibilidad de demostrar todas las fases semánticas en tiempo real.

**Flujo de ejecución (`execute_ast`):**

1. Registrar todas las funciones declaradas (`register_all_functions`)
   para permitir llamadas hacia adelante.
2. Recorrer las sentencias de nivel superior del AST, omitiendo
   declaraciones de función (ya registradas).
3. Para cada sentencia, invocar `exec_statement`, que despacha por
   `node->type` a la rutina correspondiente (declaración, asignación,
   `LADRA`, `IF`, `SCOPE`, `RETURN`).
4. Las expresiones se evalúan en `eval_expr`, que produce un `Value`
   (entero o string) correctamente liberado tras su uso.
5. La llamada a función (`eval_call`) abre un scope nuevo, enlaza
   parámetros, ejecuta el cuerpo y recupera el valor vía `ExecResult`
   (`has_return`, `value`).

**Estado efímero que se representa:** pila de ámbitos, tabla de
funciones enlazada (`g_functions`), y offsets simulados que permiten
discutir cómo se vería la traducción a un marco de activación real.

---

## 4. Funcionamiento y Pruebas (Rúbrica II — 10%)

### 4.1 Compilación

```bash
make
```

Genera el binario `golden` a partir de `parser.y`, `lexer.l` y los
archivos de `src/`. Los flags `-Wall -Wextra -std=c11` aseguran
compilación limpia.

### 4.2 Ejecución

```bash
./golden <archivo.grl>
```

Si no se proporciona archivo, se lee de `stdin`. Los programas que
usan `OLFATEA_HUESO()` u `OLFATEA_CORREA()` leen una línea de la
entrada estándar por cada llamada.

### 4.3 Casos de prueba incluidos

#### 4.3.1 `prueba` — Shadowing de 3 niveles

Valida la corrección de la pila de ámbitos. Un mismo identificador
(`energia`, `estado`) se declara en 4 niveles diferentes, y al salir
de cada bloque el valor visible vuelve al nivel externo.

**Salida esperada** (con entrada `Luna\n7\n`):

```
100
Max
70
Patio
40
70
10
Parque
40
Patio
70
100
Luna
7
```

Ejecutable mediante `make run` (entrada stdin integrada) o
`make test` que compara bit a bit contra la salida esperada.

#### 4.3.2 `calculadora.grl` — Aritmética básica con control de flujo

Pide dos enteros por teclado, calcula suma/resta/multiplicación,
y usa un `IF/ELSE` para proteger la división por cero.

#### 4.3.3 `calculadora2.grl` — Menú con `ELIF` anidados

Ilustra cadenas `IF/ELIF/ELIF/ELSE` y un `IF` interno para la
validación de división por cero. Demuestra el alcance por bloque
de variables locales (`sum`, `res`, `div`, `mul`).

### 4.4 Manejo de errores (Robustez)

Todos los errores se canalizan por `helper_fail(const char* msg)`
definido en `src/parser_helper.c`:

```c
void helper_fail(const char* msg) {
    fprintf(stderr, "Error en linea %d: %s\n", yylineno, msg);
    cleanup_symbols();
    exit(1);
}
```

- **Léxico:** caracteres no reconocidos producen tokens incorrectos,
  rechazados por el parser.
- **Sintáctico:** `yyerror` dispara "Error de sintaxis" con
  `yylineno`.
- **Semántico:** mensajes específicos — *Variable no declarada*,
  *Declaracion invalida o variable repetida*, *Suma entre tipos
  incompatibles*, *Division entre cero*, *Comparacion solo para
  enteros*, *Funcion no declarada*, *Cantidad de argumentos
  invalida*, *RETORNA fuera de una funcion*, etc.
- Antes de salir, `cleanup_symbols` libera la pila de ámbitos,
  evitando fugas aún en caso de error.

### 4.5 Demostración sugerida (script)

1. `make clean && make` — compilación limpia.
2. `make test` — verifica shadowing de 3 niveles.
3. `./golden calculadora2.grl` con entradas `1 8 7` → `15`.
4. `./golden calculadora2.grl` con entradas `3 10 0` → mensaje de
   división por cero (sin crash).
5. Prueba de error: ejecutar un programa que declara `HUESO x = "texto";`
   y mostrar el mensaje *Declaracion invalida*.
6. Prueba de alcance: usar una variable fuera de su bloque y mostrar
   *Variable no declarada*.

---

## 5. Defensa y Sustentación (Rúbrica III — 10%)

### 5.1 Dominio técnico — puntos clave

- **¿Por qué Flex + Bison?** Son herramientas estándar derivadas de
  Lex/Yacc descritas en el libro de Aho. Generan DFA y parser LALR(1)
  automáticamente y permiten concentrar el esfuerzo en la semántica.
- **¿Por qué intérprete de AST y no codegen?** El alcance del curso
  privilegia claridad de las fases sobre emisión a backend. El AST
  hace visible el análisis sintáctico y semántico; la ejecución
  valida todas las reglas en tiempo real.
- **¿Cómo se maneja el shadowing?** Con pila de ámbitos; la búsqueda
  de un identificador se detiene en el primer nivel de arriba hacia
  abajo que lo contenga. Ver `find_in_stack` en `src/symbols.c`.
- **¿Por qué un `offset` por variable si no se emite ensamblador?**
  Se mantiene como documentación del modelo de activación y para
  extender fácilmente a una fase de generación en futuras iteraciones.
- **¿Cómo se resuelven funciones con llamadas hacia adelante?**
  En `execute_ast` se hace un pase previo (`register_all_functions`)
  que registra cada `AST_FUNC_DECL` en la lista enlazada `g_functions`
  antes de ejecutar el resto del programa.
- **¿Qué pasa si un `RETORNA` se encuentra fuera de una función?**
  Se propaga el flag `has_return` en `ExecResult`; `execute_ast`
  detecta que ocurrió a nivel global y dispara `helper_fail`.

### 5.2 Organización del código

```
.
├── lexer.l                 # Análisis léxico (Flex)
├── parser.y                # Análisis sintáctico + construcción AST (Bison)
├── src/
│   ├── ast.c               # Constructores y liberación del AST
│   ├── interpreter.c       # Intérprete tree-walking + funciones
│   ├── parser_helper.c     # Errores y aritmética con chequeo de tipos
│   ├── symbols.c           # Pila de ámbitos y tabla de símbolos
│   └── headers/
│       ├── ast.h
│       ├── interpreter.h
│       ├── parser_helper.h
│       └── symbols.h
├── Makefile                # Targets: all, run, test, clean
├── prueba                  # Prueba de shadowing (3 niveles)
├── calculadora.grl         # Calculadora básica
├── calculadora2.grl        # Calculadora con menú
├── ReglasLenguaje.L        # Especificación del lenguaje
└── README.md               # Guía de uso
```

### 5.3 Decisiones de diseño justificadas

| Decisión                           | Justificación                                         |
|------------------------------------|-------------------------------------------------------|
| C11 + Flex + Bison                 | Cobertura directa del contenido de Aho et al.         |
| Tipado estático con 2 tipos        | Fuerza al parser a razonar sobre tipos sin complicar. |
| AST como IR principal              | Permite separar limpiamente sintaxis y ejecución.     |
| Pila de ámbitos en arreglo fijo    | Rendimiento O(1) de push/pop y código sencillo.       |
| Intérprete vs codegen              | Prioriza demostrar semántica; extensible a codegen.   |
| Mensajes de error con `yylineno`   | Localización útil para el usuario final.              |

### 5.4 Limitaciones reconocidas

- No hay bucles (`WHILE`/`FOR`); el lenguaje cubre condicionales y
  funciones con recursión.
- La división es entera; no hay `float`.
- Concatenación solo entre strings; no hay conversión implícita
  entero ↔ string.
- Máximo 64 niveles de ámbito y 256 símbolos por nivel (constantes
  ajustables en `symbols.c`).

### 5.5 Posibles extensiones futuras

1. Generación a código intermedio de tres direcciones (TAC) y de ahí
   a ensamblador x86-64 o WebAssembly.
2. Tipos adicionales (booleano explícito, flotante).
3. Bucles `MIENTRAS`/`PARA` y `ROMPE`/`CONTINUA`.
4. Optimizaciones básicas sobre el AST: plegado de constantes,
   eliminación de código muerto.

---

## 6. Conclusiones

El proyecto cubre las cuatro fases clásicas de un compilador y produce
un sistema funcional capaz de analizar, validar y ejecutar programas
escritos en GoldenRetrieverLang. La separación clara entre
`lexer.l`, `parser.y`, la tabla de símbolos y el intérprete hace que
cada fase sea verificable y defendible de manera independiente.

La prueba de shadowing (`prueba`) y las calculadoras (`calculadora*.grl`)
evidencian que el analizador léxico, el parser, el chequeo de tipos
y la gestión de ámbitos funcionan en conjunto de forma estable y
predecible, cumpliendo con los criterios de *Funcionalidad Completa y
Estabilidad*, *Manejo de Errores* y *Ejecución y Resultados* de la
rúbrica.

---

## 7. Referencias

- Aho, A., Lam, M., Sethi, R. y Ullman, J. (2008). *Compiladores:
  principios, técnicas y herramientas* (2a ed.). Pearson Educación.
- Flex manual — https://westes.github.io/flex/manual/
- Bison manual — https://www.gnu.org/software/bison/manual/
- Compiler Tools catalog — http://catalog.compilertools.net/
