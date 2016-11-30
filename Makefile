run: lex.yy.c fol.tab.c fol.tab.h header.h homework.cpp
	g++ -std=c++11 -o homework fol.tab.c homework.cpp lex.yy.c -ll; ./homework

fol.tab.c fol.tab.h: fol.y
	bison -d fol.y

lex.yy.c: fol.l fol.tab.h
	flex fol.l
