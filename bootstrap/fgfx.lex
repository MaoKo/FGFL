/////////////////////////////////////////////////
// Lexical Specification of both (FGFL & FGFP) //
/////////////////////////////////////////////////

(* High Precedence *)

SPACE		=	/  [ \t\n]+  / ;
COM		=	/  (\/\/.*)|(\(\*(\*+[^*)]|[^*])*\*+\))  / ;

%skip
	SPACE	,
	COM	,
;

@LETTER		=	/  [a-zA-Z_]  / ;
@DIGIT		=	/  [0-9]  / ;
@IDENT		=	/  {LETTER}({LETTER}|{DIGIT})*  / ;

(* FGFL *)

LOCAL_TOK	=	/  @{IDENT}  / ;
EQUAL		=	/  =  / ;
REGEX		=	/  \/([^\/\\\n]|\\(.|\n))*\/  / ;

(* FGFP *)

NTER		=	/  <{IDENT}'?>		/ ;
ARROW		=	/  ->			/ ;
UNION		=	/  \|			/ ;
LITERAL		=	/  '([^\\\n']|\\['\\])'	/ ;
STR		=	/  \"([^\\\n"]|\\[\"])+\" / ;

(* Both FGFL & FGFP *)

DIRECTIVE	=	/  %{IDENT}	/ ;
GLOBAL_TOK	=	/  {IDENT}	/ ;
SEMI		=	/  ;		/ ;
COMMA		=	/  ,		/ ;
LBRACK		=	/  \[		/ ;
RBRACK		=	/  ]		/ ;

%special DIRECTIVE -> [
		START, INCLUDE, MATCH,
		SKIP, SPECIAL, IGCASE,
	];
