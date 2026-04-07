AST y funciones en GoldenRetrieverLang

- El compilador ahora construye un AST antes de ejecutar.
- Soporta declaraciones de funciones con retorno `HUESO` o `CORREA`.
- Las funciones usan `RETORNA expr;` para devolver un valor.
- El scope usa shadowing por pila de ambitos: una variable local oculta la externa.
- La sintaxis base sigue siendo la del proyecto, con `HUESO`, `CORREA` y `LADRA`.
