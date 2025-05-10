# expr

```bnf
expr ::= unary
       | binary
       | literal
       | symbol
       | group
       | call

unary ::= op_u
          expr

binary ::= expr
           op_b
           expr

literal ::= NUMBER
          | STRING
          | "null"
          | "true"
          | "false"

symbol ::= XID_S
           XID_C*

group ::= "("
          expr
          ")"

call ::= expr
         duty
         symbol
         "("
         args?
         ")"

duty ::= "."
         |
         "?."
         |
         "!."
         |
         "::"

args ::= expr ("," expr)*
```

*left recursion fix: from the lowest to the highest priority*

```bnf
expr ::= expr_12

######
# 12 # assignment
######

expr_12 ::= expr_11 ( set_op expr_12 )?

set_op ::= "="
         | "+="
         | "-="
         | "*="
         | "/="
         | "%="
         | "^="

######
# 11 # nullptr coalescing
######

expr_11 ::= expr_10 ( nil_op expr_10 )*

nil_op ::= "??"

######
# 10 # logical/bitwise or
######

expr_10 ::= expr_09 ( or_op expr_09 )*

or_op ::= "or"
        | "||"

######
# 09 # logical/bitwise and
######

expr_09 ::= expr_08 ( and_op expr_08 )*

and_op ::= "and"
         | "&&"

######
# 08 # equality
######

expr_08 ::= expr_07 ( eql_op expr_07 )*

eql_op ::= "=="
         | "!="

######
# 07 # relation
######

expr_07 ::= expr_06 ( rel_op expr_06 )*

rel_op ::= "<"
         | ">"
         | "<="
         | ">="

######
# 06 # bit-shift
######

expr_06 ::= expr_05 ( sft_op expr_05 )*

sft_op ::= "shl"
         | "shr"

######
# 05 # arithmetic 3
######

expr_05 ::= expr_04 ( add_op expr_04 )*

add_op ::= "+"
         | "-"

######
# 04 # arithmetic 2
######

expr_04 ::= expr_03 ( mul_op expr_03 )*

mul_op ::= "*"
         | "/"
         | "%"

######
# 03 # arithmetic 1
######

expr_03 ::= expr_02 ( pow_op expr_03 )?

pow_op ::= "^"

######
# 02 # unary prefix
######

expr_02 ::= ( pre_op )* expr_01

pre_op ::= "@"
         | "&"
         | "!"
         | "not"

######
# 01 # primary
######

expr_01 ::= literal
          | symbol
          | group
          | call
```

# decl

```bnf
decl := fun
      | pure_fun
      | var
      | const_var

fun := "fun"
       symbol
       block

pure_fun := "fun!"
            symbol
            block

var := "let"
       symbol
       ":"
       type
       ("=" expr)?
       ";"

const_var := "let!"
             symbol
             ":"
             type
             "="
             expr
             ";"
```

# stmt

```bnf
stmt := for
      | while
      | if
      | match

for ::= "for"
        "("
        decl
        ";"
        expr
        ";"
        expr
        ")"
        block

while ::= "while"
          "("
          expr
          ")"
          block

if ::= "if"
       "("
       expr
       ")"
       block

match ::= "match"
          "("
          expr
          ")"
          block

block ::= "{"
          (expr | stmt)*
          "}"
```
