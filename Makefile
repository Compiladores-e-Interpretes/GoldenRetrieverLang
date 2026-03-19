CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -Wextra -std=c11
TARGET = golden

all: $(TARGET)

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

$(TARGET): parser.tab.c lex.yy.c parser_helper.c symbols.c
	$(CC) $(CFLAGS) -o $(TARGET) parser.tab.c lex.yy.c parser_helper.c symbols.c

run: $(TARGET)
	./$(TARGET) < prueba

clean:
	rm -f $(TARGET) parser.tab.c parser.tab.h lex.yy.c
