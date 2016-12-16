run: lex.yy.c fol.tab.c fol.tab.h header.h main.cpp
	g++ -std=c++11 -o fol fol.tab.c main.cpp lex.yy.c -ll; ./fol

fol.tab.c fol.tab.h: fol.y
	bison -d fol.y

lex.yy.c: fol.l fol.tab.h
	flex fol.l
