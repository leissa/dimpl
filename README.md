# Dimpl

## Building

```
git clone --recurse-submodules git@github.com:leissa/dimpl.git
cd dimpl
mkdir build
cmake -S . -B build
cmake --build build -j $(nproc)
```

## Syntax

```ebnf
(* binders *)
b = [ID ":"] e                                          (* id binder *)
  | "[" b "," ... "," b "]"                             (* sigma binder *)
  ;

(* patterns *)
p = ["mut"] ID [":" e]                                  (* id pattern *)
  | T                                                   (* tuple pattern *)
  ;

T = "(" p "," ... "," p ")";                            (* tuple pattern *)

(* nominals *)
n = "nom" ID ":" e "=" e                                (* nom nominal *)
  | "struct" ID T* "=" e                                (* struct *)
  | "trait"  ID T* "=" e                                (* trait *)
  | "λ"  ID T+ ["->" e] ("=" e | B)                     (* λ  nominal *)
  | "fn" ID T+ ["->" e] ("=" e | B)                     (* fn nominal *)
  | "cn" ID T+          ("=" e | B)                     (* cn nominal *)
  ;

(* statements *)
s = n                                                   (* nominal statement *)
  | "let" p "=" e ";"                                   (* let statement *)
  | e A_OP e ";"                                        (* assignment statement *)
  | e ";"                                               (* expression statement *)
  ;

(* expressions *)
e = ID
  | PRE_OP e                                            (* prefix expression *)
  | e IN_OP e                                           (*  infix expression *)
  | e POST_OP                                           (* postix expression *)
  | "[" b "," ... "," b "]"                             (* sigma *)
  | "(" [ID "="] e "," ... "," [ID "="] e")" [":" e]    (* tuple *)
  | e "." ID                                            (* field *)
  | "«" p "," ... "," p ";" e "»"                       (* array *)
  | "‹" p "," ... "," p ";" e "›"                       (* pack *)
  | "∀"  b "->" e                                       (* ∀  *)
  | "Fn" b "->" e                                       (* Fn *)
  | "Cn" b                                              (* Cn *)
  | "λ"  p "," ... "," p ["->" e] ("=" e | B)           (* λ  *)
  | "fn" p "," ... "," p ["->" e] ("=" e | B)           (* fn *)
  | "cn" p "," ... "," p          ("=" e | B)           (* cn *)
  | e  "[" e "]"                                        (* direct-style application *)
  | e "!(" e ")"                                        (* cn application *)
  | e  "(" e ")"                                        (* fn application *)
  | "if" e B ["else" B]                                 (* if *)
  | "match" e "{" p "=>" e "," ... "," p "=>" e "}"     (* match *)
  | "while" e B                                         (* while *)
  | "for" p "," ... "," p "in" B                        (* for *)
  | B                                                   (* block *)
  ;

B = "{" s ... s [ e ] "}";                              (* block expression *)

```

## ASCII-Alternatives

## Ambiguities

Ambiguities in the grammar are resolved as follows:
* A statement beginning with `λ`, `fn`, `cn` is a _nominal statement_.
* TODO

## Expressions

Precedence is dissolved as follows from strongest to lowest:
* Postfix operators (left-to-right)
* Prefix operators
* Infix operators (see below for complete table)

### Prefix Expressions

| Expression  | Desugared           |
| ----------  | ------------------- |
| `+e`        | todo    |
| `-e`        | todo    |
| `*e`        | todo    |
| `&e`        | todo    |
| `++e`       | todo    |
| `--e`       | todo    |

### Infix Expressions

| Assignment | Desugared                            |
| ---------- | ------------------------------------ |
| `e1  + e2` | `   Add.typeof[e1]].add(e1, e2)`     |
| `e1  - e2` | `   Sub[typeof[e1]].sub(e1, e2)`     |
| `e1  * e2` | `   Mul[typeof[e1]].mul(e1, e2)`     |
| `e1  / e2` | `   Div[typeof[e1]].div(e1, e2)`     |
| `e1  % e2` | `   Mod[typeof[e1]].mod(e1, e2)`     |
| `e1 >> e2` | `   Shr[typeof[e1]].shr(e1, e2)`     |
| `e1 << e2` | `   Shl[typeof[e1]].shl(e1, e2)`     |
| `e1 \| e2` | `Bitor [typeof[e1]].bitor (e1, e2)`  |
| `e1  & e2` | `Bitand[typeof[e1]].bitand(e1, e2)`  |
| `e1  ^ e2` | `Bitxor[typeof[e1]].bitxor(e1, e2)`  |
| `e1 == e2` | `    Eq[typeof[e1]].eq(e1, e2)`      |
| `e1 != e2` | `    Ne[typeof[e1]].ne(e1, e2)`      |
| `e1 <  e2` | `    Lt[typeof[e1]].lt(e1, e2)`      |
| `e1 <= e2` | `    Le[typeof[e1]].le(e1, e2)`      |
| `e1 >  e2` | `    Gt[typeof[e1]].gt(e1, e2)`      |
| `e1 >= e2` | `    Ge[typeof[e1]].ge(e1, e2)`      |

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
