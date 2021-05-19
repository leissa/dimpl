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
p = _p [":" e];                                         (* pattern with optional type *)
pt = _p ":" e | e;                                      (* pattern with mandatory type *)
_p = ID | tp;                                           (* pattern base *)
tp = "(" p "," ... "," p ")";                           (* tuple pattern *)

(* nominals *)
n = "nom" ID ":" e "=" e                                (* nom *)
  | "struct" ID A "=" e                                 (* struct *)
  | "trait"  ID A "=" e                                 (* trait *)
  | "\"  ID A tp ["->" e] e                             (* fn nom *)
  | "fn" ID A tp ["->" e] e                             (* fn nom *)
  | "cn" ID A tp          e                             (* cn nom *)
  ;

(* expressions *)
e = ID
  | OP e                                                (* prefix expression *)
  | e OP e                                              (*  infix expression *)
  | e OP                                                (* postix expression *)
  | "[" pt "," ... "," pt "]"                           (* sigma *)
  | "(" [ID "="] e "," ... "," [ID "="] e")" [":" e]    (* tuple *)
  | e "." ID                                            (* field  *)
  | "ar" "[" pt "," ... "," pt ";" e "]"                (* array *)
  | "pk" "(" pt "," ... "," pt ";" e ")"                (* pack *)
  | "\/" pt "->" e                                      (* forall *)
  | "Fn" pt "->" e                                      (* Fn *)
  | "Cn" pt                                             (* Cn *)
  | "\"  A tp ["->" e] e                                (* ds abstraction *)
  | "fn" A tp ["->" e] e                                (* fn abstraction *)
  | "cn" A tp          e                                (* cn abstraction *)
  | e  "[" e "]"                                        (* ds application *)
  | e "!(" e ")"                                        (* cn application *)
  | e  "(" e ")"                                        (* fn application *)
  | "if" e B ["else" B]                                 (* if *)
  | "match" e "{" p "=>" e "," ... "," p "=>" e "}"     (* match *)
  | "while" e B                                         (* while *)
  | "for" p "in" e                                      (* for *)
  | B                                                   (* block *)
  ;

A = "[" p "," ... "," p "]" | (*nothing*)               (* optional inline abstraction *)
  ;
B = "{" s ... s [ e ] "}"                               (* block expression *)
  ;

(* statements *)
s = n                                                   (* nominal statement *)
  | "let" p "=" e ";"                                   (* let statement *)
  | e OP e ";"                                          (* assignment statement *)
  | e ";"                                               (* expression statement *)
  ;
```

## Expressions

### Prefix Expressions

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
