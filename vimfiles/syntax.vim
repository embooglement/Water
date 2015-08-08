if exists("b:current_syntax")
	finish
endif

let b:current_syntax = "water"

syn keyword keywords and or not exists 
syn keyword keywords var let 
syn keyword keywords if else true false 
syn keyword keywords null func return 
syn keyword keywords while break continue

syn match operators '\v[-+*%^><=]'
syn match operators '\v[-+*%^><=]='
syn match operators '\v(\+\+)|(--)'

syn match stringLiterals '\v"[^"]*"'
syn match stringLiterals '\v\'[^\']*\''

syn match numberLiterals '\v[-+]?\d+(\.\d+)?'

syn match identifiers '\v[a-zA-Z_][a-zA-Z0-9_]*'

syn keyword todo contained TODO FIX FIXME HACK TEST NOTE
syn match comments '\v#.*$' contains=todo
syn region comments start='#-' end='-#' extend contains=todo

highlight def link keywords Keyword
highlight def link operators Operator
highlight def link stringLiterals String
highlight def link numberLiterals Number
highlight def link todo Todo
highlight def link comments Comment
highlight def link identifiers Identifier
