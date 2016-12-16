# first-order-logic-inference
## Overview

This is a **subset** of [First Order Logic (FOL)](https://en.wikipedia.org/wiki/First-order_logic) inference engine. With a bunch of knowledge base sentences, it will tell if a query is True or False according to the knowledge base. This project utilized Universal **Resolution** Law of FOL and **Refutation Proof**.

Interesting Example:

~~~
( Phd(x) => HighQualified(x) )
( Phd(x) | EarlyEarnings(x) )
( HighQualified(x) => Rich(x) )
( EarlyEarnings(x) => Rich(x) )

It can prove : Rich(You) is True
~~~

This project is currently limited:

1. Input format is kinda restricted. 
2. Query must be single literal sentence. But You can still query with multiple sentences to simulate conjunction
3. No existential quantifier ( E )

## Features
* Input parsing with LEX/YACC
* Sentence Indexing ( Table )
* [Set of Support](http://www.doc.ic.ac.uk/~sgc/teaching/pre2012/v231/lecture9.html) Resolution strategy ( It is there but not used currently )
* Converting arbitrary FOL into CNF sentences.
* Literal Factoring ( simple example: collapse A(x) | A(y) to A(x) )
* FOL Sentence stringify
* Unification
* Several tesing sets

## Compile
Make sure you have LEX/YACC or Flex/Bison ready. Useful links: 

1. [What are lex and yacc?](http://aquamentus.com/flex_bison.html)
2. [Youtube tutorial](https://www.youtube.com/watch?v=54bo1qaHAfk)

-
~~~
$ flex fol.l
$ bison -d fol.y
$ g++ -std=c++11 -o fol fol.tab.c main.cpp lex.yy.c -ll

or $ make
~~~

### Input Restriction
Any operator and its operands will always be surrounded by a parenthesis. Parenthesis will only appear in test cases when there is an operator with an operand or operands. So we don't worry about operator priority and parentheses confusions for now.

For example:

~~~
(A(x) & B(x)) Valid
A(x) & B(x) InValid
((A(x) & B(x)) => C(x)) Valid
(A(x) | B(x)) => C(x) InValid
(~A(x)) Valid
~A(x) InValid
(A(x)) InValid
A(x) Valid
~~~~A(x) InValid
(~(~(~(~A(x))))) Valid
No nested Predicates.
For example:
A(x,y) Valid 
A(B(x), y) InValid
~~~

No TRUE, FALSE,True,False literals in the input

'=', '<=>' operators are not valid in the input


### Getting Started
Format for input.txt:

~~~<NQ = NUMBER OF QUERIES><QUERY 1>…<QUERY NQ><NS = NUMBER OF GIVEN SENTENCES IN THE KNOWLEDGE BASE><SENTENCE 1>…<SENTENCE NS>
~~~
* Each **Query** will be a single literal of the form Predicate(Constant) or ~Predicate(Constant).* **Variables** are all **single lowercase** letters.* All predicates (such as Sibling) and constants (such as John) are case-sensitive alphabetical strings that**begin with an uppercase letter**.* Each predicate takes at least one argument. Predicates will take at most 100 arguments. A givenpredicate name will not appear with different number of arguments.* There will be at most 100 queries and 1000 sentences in the knowledge base.* See the sample input below for spacing patterns.###Format for output.txt:
For each query, determine if that query can be inferred from the knowledge base or not, one query per line:
~~~<ANSWER 1>…<ANSWER NQ>
~~~
Each answer is either TRUE if the program can prove that the corresponding query sentence is true given theknowledge base, or FALSE if it cannot.

###Examples:
input.txt

~~~
2
Sibling(Alice,Bob)
Sibling(Bob,Alice)
2
((Kid(a) | Sibling(a,b)) => Sibling(b,a))
Kid(x)
~~~

output.txt

~~~
TRUE
TRUE
~~~

-

input.txt

~~~
2
Ancestor(Liz,Billy)
Ancestor(Liz,Bob)
6
Mother(Liz,Charley)
Father(Charley,Billy)
((~Mother(x,y)) | Parent(x,y))
((~Father(x,y)) | Parent(x,y))
((~Parent(x,y)) | Ancestor(x,y))
((~(Parent(x,y) & Ancestor(y,z))) | Ancestor(x,z))
~~~

output.txt

~~~
TRUE
FALSE
~~~

-

input.txt

~~~
6
F(Bob)
H(John)
~H(Alice)
~H(John)
G(Bob)
G(Tom)
14
(A(x) => H(x))
(D(x,y) => (~H(y)))
((B(x,y) & C(x,y)) => A(x))
B(John,Alice)
B(John,Bob)
((D(x,y) & Q(y)) => C(x,y))
D(John,Alice)
Q(Bob)
D(John,Bob)
(F(x) => G(x))
(G(x) => H(x))
(H(x) => F(x))
(R(x) => H(x))
R(Tom)
~~~

output.txt

~~~
FALSE
TRUE
TRUE
FALSE
FALSE
TRUE
~~~
