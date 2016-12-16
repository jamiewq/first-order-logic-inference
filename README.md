# first-order-logic-inference
## Overview
This is a **subset** of [First Order Logic (FOL)](https://en.wikipedia.org/wiki/First-order_logic) inference engine. With a bunch of knowledge base sentences, it will tell if a query is True or False according to the knowledge base.

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
flex fol.l
bison -d fol.y
~~~
