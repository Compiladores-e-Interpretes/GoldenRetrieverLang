# GoldenRetrieverLang

Mini lenguaje de programacion academico con tematica de Golden Retriever.
La implementacion esta hecha con archivos `.c`, `.h`, `.l` y parser en `.y`.

## Estructura solicitada
- `lexer.l`
- `Makefile`
- `parser_helper.c`
- `parser_helper.h`
- `parser.y`
- `symbols.c`
- `symbols.h`
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
- Comentarios: `//` y `/* ... */`
- Operaciones: `+`, `-`, `*`, `/` para enteros y `+` para concatenar strings

## Subir a GitHub
1. Crea un repositorio nuevo en GitHub, por ejemplo `golden-retriever-lang`.
2. En terminal, dentro de esta carpeta:
```bash
git init
git add .
git commit -m "Entrega: GoldenRetrieverLang en C/Flex"
git branch -M main
git remote add origin https://github.com/TU_USUARIO/golden-retriever-lang.git
git push -u origin main
```
3. Entrega el enlace del repositorio.
# GoldenRetrieverLang
