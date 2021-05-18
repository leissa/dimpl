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

A = "[" p "," ... "," p "]" | (*nothing*);              (* optional inline abstraction *)

B = "{" s ... s [ e ] "}";                              (* block expression *)

(* statement *)
s = e ";"                                               (* expression statement *)
  | "let" p "=" e ";"                                   (* let statement *)
  | i                                                   (* item statement *)

```
