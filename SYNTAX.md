# expr

```bnf
expr ::= binary
       | unary_l
       | unary_r
       | literal
       | symbol
       | group
       | call

binary ::= expr
           op_b
           expr

unary_l ::= op_l
            expr

unary_r ::= expr
            op_r

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

call ::= symbol
         "("
         args?
         ")"
```

*left recursion fix: from the lowest to the highest priority*

```bnf
expr ::= expr_assign

######
# 01 # assignment
######

expr_assign ::= expr_nil ( assign_op expr_assign )?

assign_op ::= "="
            | "+="
            | "-="
            | "*="
            | "/="
            | "%="
            | "^="

######
# 02 # coalescing
######

expr_nil ::= expr_or ( nil_op expr_or )*

nil_op ::= "??"

######
# 03 # logical/bitwise or
######

expr_or ::= expr_xor ( or_op expr_xor )*

or_op ::= "or"
        | "||"

######
# 04 # logical/bitwise xor
######

expr_xor ::= expr_and ( xor_op expr_and )*

xor_op ::= "xor"
         | "~~"

######
# 05 # logical/bitwise and
######

expr_and ::= expr_eql ( and_op expr_eql )*

and_op ::= "and"
         | "&&"

######
# 06 # equality
######

expr_eql ::= expr_rel ( eql_op expr_rel )*

eql_op ::= "=="
         | "!="

######
# 07 # relation
######

expr_rel ::= expr_sft ( rel_op expr_sft )*

rel_op ::= "<"
         | ">"
         | "<="
         | ">="

######
# 08 # bit-shift
######

expr_sft ::= expr_add ( sft_op expr_add )*

sft_op ::= "shl"
         | "shr"

######
# 09 # 
######

expr_add ::= expr_mul ( add_op expr_mul )*

add_op ::= "+"
         | "-"

######
# 10 #
######

expr_mul ::= expr_pow ( mul_op expr_pow )*

mul_op ::= "*"
         | "/"
         | "%"

######
# 11 #
######

expr_pow ::= expr_unary ( pow_op expr_unary )?

pow_op ::= "^"

######
# 12 #
######

expr_unary ::= ( pre_op )* primary ( pst_op )*

pre_op ::= "@"
         | "&"
         | "!"
         | "not"

pst_op ::= "."
         | "?."
         | "!."
         | "::"

######
# 13 #
######

primary ::= literal
          | symbol
          | group
          | call

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
