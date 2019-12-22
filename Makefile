#The makefile for interpreter based on Flex and Bison
SRC_DIR = ./src
OBJ_DIR = ./src/obj
LEX_DIR = ./lex
YACC_DIR = ./yacc
INC_DIR = ./src/include

CC = gcc
FLAG = -w
interpreter: main.o util.o cm.tab.o cm.lex.o symtab.o analyze.o
	$(CC) $(FLAG) -o ./test/interpreter $(OBJ_DIR)/main.o $(OBJ_DIR)/util.o $(YACC_DIR)/cm.tab.o $(LEX_DIR)/cm.lex.o $(OBJ_DIR)/symtab.o $(OBJ_DIR)/analyze.o

main.o: $(SRC_DIR)/main.c $(INC_DIR)/parse.h $(INC_DIR)/analyze.h
	$(CC) $(FLAG) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o

cm.tab.o: cm.tab.c $(INC_DIR)/parse.h
	$(CC) $(FLAG) -c $(YACC_DIR)/cm.tab.c -o $(YACC_DIR)/cm.tab.o

cm.tab.c: $(YACC_DIR)/cm.y
	bison -d $(YACC_DIR)/cm.y -o $(YACC_DIR)/cm.tab.c

cm.lex.o: cm.lex.c $(YACC_DIR)/cm.tab.h
	$(CC) $(FLAG) -c $(LEX_DIR)/cm.lex.c -o $(LEX_DIR)/cm.lex.o

cm.lex.c: $(LEX_DIR)/cm.l
	flex -o $(LEX_DIR)/cm.lex.c $(LEX_DIR)/cm.l

symtab.o: $(SRC_DIR)/symtab.c $(INC_DIR)/symtab.h $(INC_DIR)/globals.h
	$(CC) $(FLAG) -c $(SRC_DIR)/symtab.c -o $(OBJ_DIR)/symtab.o

analyze.o: $(SRC_DIR)/analyze.c $(INC_DIR)/globals.h $(INC_DIR)/symtab.h $(INC_DIR)/analyze.h
	$(CC) $(FLAG) -c $(SRC_DIR)/analyze.c -o $(OBJ_DIR)/analyze.o

util.o: $(SRC_DIR)/util.c
	$(CC) $(FLAG) -c $(SRC_DIR)/util.c -o $(OBJ_DIR)/util.o

clean:
	-rm $(OBJ_DIR)/*.o
	-rm $(LEX_DIR)/*.lex.*
	-rm $(YACC_DIR)/*.tab.*
	-rm ./test/*.exe

.PHONY: test
ifeq ($(O), )
	O = 
endif
test:
	./test/interpreter.exe ./test/$(I) $(O)