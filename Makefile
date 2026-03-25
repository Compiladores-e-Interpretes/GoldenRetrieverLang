CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -Wextra -std=c11
LEX_CFLAGS = -Wno-sign-compare
TARGET = golden

.PHONY: all run test clean

all: $(TARGET)

parser.tab.c parser.tab.h: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l parser.tab.h
	$(FLEX) lexer.l

$(TARGET): parser.tab.c lex.yy.c parser_helper.c symbols.c
	$(CC) $(CFLAGS) $(LEX_CFLAGS) -o $(TARGET) parser.tab.c lex.yy.c parser_helper.c symbols.c

run: $(TARGET)
	./$(TARGET) < prueba

test: $(TARGET)
	@tmp_out=$$(mktemp); \
	tmp_exp=$$(mktemp); \
	./$(TARGET) < prueba > "$$tmp_out"; \
	printf "100\nMax\n70\nPatio\n40\n70\n10\nParque\n40\nPatio\n70\n100\n" > "$$tmp_exp"; \
	if diff -u "$$tmp_exp" "$$tmp_out"; then \
		echo "Test shadowing OK"; \
	else \
		echo "Test shadowing FAIL"; \
		rm -f "$$tmp_out" "$$tmp_exp"; \
		exit 1; \
	fi; \
	rm -f "$$tmp_out" "$$tmp_exp"

clean:
	rm -f $(TARGET) parser.tab.c parser.tab.h lex.yy.c
