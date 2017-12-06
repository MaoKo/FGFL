/////////////////////////////////////////////////
// Lexical Specification of both (FGFL & FGFP) //
/////////////////////////////////////////////////

@LETTER		=	[a-zA-Z_] ;
@DIGIT		=	[0-9] ;
@IDENT		=	{LETTER}({LETTER}|{DIGIT})* ;

/* FGFL */
LOCAL_TOK	=	@{IDENT} ;
EQUAL		=	= ;
IGCASE		=	%igcase ;
SKIP		=	%skip ;

/* FGFP */
NON_TER		=	<{IDENT}'*> ;
ARROW		=	-> ;
UNION		=	\| ;

/* TODO: start symbol */
// START	=	%start ;

/* Both */
TOKEN		=	{IDENT} ;
SEMI		=	\; ;
COMMA		=	, ;

SPACE		=	[ \t\n]+ ;
COM		=	(//.*)|(/\*(\*+[^*/]|[^*])*\*+/) ;

%skip		SPACE, COM ;
