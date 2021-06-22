" Vim syntax file 
" Language:   dimpl
" Maintainer: Roland Leißa
" Version: 0.1
"
if exists("b:current_syntax")
  finish
endif

syn keyword     dimplConditional   if else match
syn keyword     dimplException     return break continue
syn keyword     dimplRepeat        for in while 
syn keyword     dimplStorageClass  let mut
syn keyword     dimplStructure     nom struct trait 
syn keyword     dimplTypedef       type
syn keyword     dimplType          Bool Idx Sint Uint Float Nat Type Kind Cn Fn Mem
syn keyword     dimplType          S8 S16 S32 S64
syn keyword     dimplType          U8 U16 U32 U64
"syn keyword     dimplOperator      as
syn keyword     dimplFunction      cn fn
syn keyword     dimplTodo          TODO FIXME containedin=dimplBlockComment,dimplLineComment
syn match       dimplNumber        '\v<\d(\d|_)*(i8|u8|i16|u16|i32|u32|i64|u64){0,1}>'
syn match       dimplLineComment   '\v\s*//.*$'
syn region      dimplBlockComment  start='/\*'  end='\*/'
syn region      dimplString        start=+"+    end=+"+
syn match       dimplFunction      '\v[\/]'
syn match       dimplPartialEval   '\v[@$]'
syn match       dimplAddressOf     '\v[&]'
syn match       dimplIdentifier    '\v\w*\s*\:'
syn match       dimplType          '→'
syn match       dimplType          '->'
syn match       dimplType          '«'
syn match       dimplType          '»'

let b:current_syntax = "dimpl"

hi def link dimplConditional   Conditional
hi def link dimplException     Exception
hi def link dimplRepeat        Repeat
hi def link dimplStorageClass  StorageClass
hi def link dimplStructure     Structure
hi def link dimplTypedef       Keyword
hi def link dimplType          Type
hi def link dimplNumber        Number
hi def link dimplOperator      Keyword
hi def link dimplAddressOf     Operator
hi def link dimplPartialEval   PreProc
hi def link dimplFunction      Keyword
hi def link dimplLineComment   Comment
hi def link dimplBlockComment  Comment
hi def link dimplString        String
hi def link dimplTodo          Todo
hi def link dimplIdentifier    Identifier

iabbr TO  →
iabbr ->  →
iabbr LAM λ
iabbr ALL ∀
iabbr AL  «
iabbr AR  »
iabbr PL  ‹
iabbr PR  ›
