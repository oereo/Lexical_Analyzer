# Lexical Analyzer

## TEAM
- CAU University 
- Healim Hong, Sehoon In


## Goal
 - The goal of the this term-project is to implement a lexical analyzer (a.k.a., scanner) 
 - Implement the lexical analyzer with the following lexical specifications.

## Lexical specifications
 - Variable type
 - Signed integer
 - Literal string
 - Boolean string
 - Floating-point number
 - An identifier of variables and functions
 - Keywords for special statements
 - Arithmetic operators
 - Bitwisse operators
 - Assignment operators
 - Comparison operators
 - A terminating symbol of statements
 - A pair of symbols for defining area/scope of variables and functions
 - A pair of symbols for indicating a function/statement
 - A symbol for separating input arguments in functions
 - Whitespaces



## STEP

1. Define tokens 
2. Make regular expressions which describe the patterns of the tokens
3. Construct a NFA for the regular expressions
4. Translate the NFA into a DFA
5. Implement a program which does a lexical analysis(recognizing tokens)

## EXECUTION

- Input : A program written in a simplified C programming language
- Output : A symbol table which stores the information of all tokens including their names and optional values

An error report which explains why and where the error occurred(ex. line number)

