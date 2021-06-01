# Dimpl

## Building

```
git clone --recurse-submodules git@github.com:leissa/dimpl.git
cd dimpl
mkdir build
cd build
cmake ..
make
```

## Syntax

```ebnf
(* patterns *)
p = ID [":" e]                                          (* id pattern *)
  | T                                                   (* tuple pattern *)
  ;

T = "(" p "," ... "," p ")";                            (* tuple pattern *)
A = "[" p "," ... "," p "]" | (* nothing *);            (* optional inline abstraction *)
b = [ID ":"] e;                                         (* binder *)
B = "{" s ... s [ e ] "}";                              (* block expression *)

(* nominals *)
n = "nom" ID ":" e "=" e                                (* nom *)
  | "struct" ID A "=" e                                 (* struct *)
  | "trait"  ID A "=" e                                 (* trait *)
  | "\"  ID A T ["->" e] e                              (* fn nom *)
  | "fn" ID A T ["->" e] e                              (* fn nom *)
  | "cn" ID A T          e                              (* cn nom *)
  ;

(* expressions *)
e = ID
  | PRE_OP e                                            (* prefix expression *)
  | e IN_OP e                                           (*  infix expression *)
  | e POST_OP                                           (* postix expression *)
  | "[" b "," ... "," b "]"                             (* sigma *)
  | "(" [ID "="] e "," ... "," [ID "="] e")" [":" e]    (* tuple *)
  | e "." ID                                            (* field  *)
  | "ar" "[" pt "," ... "," pt ";" e "]"                (* array *)
  |      "«" pt "," ... "," pt ";" e "»"                (* array (quote-style) *)
  | "pk" "(" pt "," ... "," pt ";" e ")"                (* pack *)
  |      "‹" pt "," ... "," pt ";" e "›"                (* pack (angle-style) *)
  | "\/" b "->" e                                       (* forall *)
  | "Fn" b "->" e                                       (* Fn *)
  | "Cn" b                                              (* Cn *)
  | "\"  A T ["->" e] e                                 (* ds abstraction *)
  | "fn" A T ["->" e] e                                 (* fn abstraction *)
  | "cn" A T          e                                 (* cn abstraction *)
  | e  "[" e "]"                                        (* ds application *)
  | e "!(" e ")"                                        (* cn application *)
  | e  "(" e ")"                                        (* fn application *)
  | "if" e B ["else" B]                                 (* if *)
  | "match" e "{" p "=>" e "," ... "," p "=>" e "}"     (* match *)
  | "while" e B                                         (* while *)
  | "for" p "in" B                                      (* for *)
  | B                                                   (* block *)
  ;

(* statements *)
s = n                                                   (* nominal statement *)
  | "let" p "=" e ";"                                   (* let statement *)
  | e A_OP e ";"                                        (* assignment statement *)
  | e ";"                                               (* expression statement *)
  ;
```

## Expressions

Precedence is dissolved as follows from strongest to lowest:
* Postfix operators (left-to-right)
* Prefix operators
* Infix operators (see below for complete table)

### Prefix Expressions

| Expression  | Desugared           |
| ----------  | ------------------- |
| `+e`       | todo    |
| `-e`       | todo    |
| `*e`       | todo    |
| `&e`       | todo    |
| `++e`       | todo    |
| `--e`       | todo    |

### Infix Expressions

| Assignment | Desugared           |
| ---------- | ------------------- |
| `e1  + e2` | `   add(e1, e2)`    |
| `e1  - e2` | `   sub(e1, e2)`    |
| `e1  * e2` | `   mul(e1, e2)`    |
| `e1  / e2` | `   div(e1, e2)`    |
| `e1  % e2` | `   mod(e1, e2)`    |
| `e1 >> e2` | `   shr(e1, e2)`    |
| `e1 << e2` | `   shl(e1, e2)`    |
| `e1 \| e2` | `bitor (e1, e2)`    |
| `e1  & e2` | `bitand(e1, e2)`    |
| `e1  ^ e2` | `bitxor(e1, e2)`    |
| `e1 == e2` | `    eq(e1, e2)`    |
| `e1 != e2` | `    ne(e1, e2)`    |
| `e1 <  e2` | `    lt(e1, e2)`    |
| `e1 <= e2` | `    le(e1, e2)`    |
| `e1 >  e2` | `    gt(e1, e2)`    |
| `e1 >= e2` | `    ge(e1, e2)`    |

### Postfix Expressions

## Statements

### Nominal Statement

### Let Statement

### Assignment Statement

Unlike many other languages, assignments are not expressions but statements.

The assignment `e_l = e_r` assigns the r-value of `e_r` to the l-value of `e_l`.
All other assignments are syntactic sugar.
The following tables sums up all possibilities:

| Assignment     | Desugared           |
| -------------- | ------------------- |
| `e_l  += e_r;` | `e_l = e_l  + e_r;` |
| `e_l  -= e_r;` | `e_l = e_l  - e_r;` |
| `e_l  *= e_r;` | `e_l = e_l  * e_r;` |
| `e_l  /= e_r;` | `e_l = e_l  / e_r;` |
| `e_l  %= e_r;` | `e_l = e_l  % e_r;` |
| `e_l >>= e_r;` | `e_l = e_l >> e_r;` |
| `e_l <<= e_r;` | `e_l = e_l << e_r;` |
| `e_l \|= e_r;` | `e_l = e_l \| e_r;` |
| `e_l  &= e_r;` | `e_l = e_l  & e_r;` |
| `e_l  ^= e_r;` | `e_l = e_l  ^ e_r;` |

### Expression Statement
