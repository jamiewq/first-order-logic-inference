fol.tab.c fol.tab.h: fol.y
	bison -d fol.y

lex.yy.c: fol.l fol.tab.h
	flex fol.l

fol: lex.yy.c fol.tab.c fol.tab.h
	g++ fol.tab.c lex.yy.c -ll -o fol

