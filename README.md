# GoldenRetrieverLang

Mini lenguaje de programacion academico con tematica de Golden Retriever.
La implementacion esta hecha con archivos `.c`, `.h`, `.l` y parser en `.y`.

## Estructura solicitada
- `lexer.l`
- `Makefile`
- `parser.y`
- `src/`
- `src/headers/`
- `prueba` (archivo de prueba sin extension `.nova`)
- `ReglasLenguaje.L`

## Requisitos
- `flex`
- `bison`
- `gcc`
- `make`

## Compilacion
```bash
make
```

## Ejecucion
```bash
make run
```

Para usar entradas por teclado:
```bash
./golden tu_programa.grl
```
Luego escribe las entradas en stdin (una linea por cada `OLFATEA_*()`).

Salida esperada:
```text
Golden: Max
20
10
```

## Elementos solicitados
- Tipos de datos: `HUESO` (entero), `CORREA` (string)
- Identificadores: regex `[A-Za-z_][A-Za-z0-9_]*`
- Impresion: `LADRA(expresion);`
- Condicionales: `IF/ELIF/ELSE` o `SI/SINO_SI/SINO`
- Entrada: `OLFATEA_HUESO()` y `OLFATEA_CORREA()`
- Comparaciones: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Comentarios: `//` y `/* ... */`
- Operaciones: `+`, `-`, `*`, `/` para enteros y `+` para concatenar strings

