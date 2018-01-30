%option yylineno
%pointer

%{
    #include <stdlib.h>
    #include <errno.h>
    #include <limits.h>
    #include "ast.hpp"
    #include "parser.hpp"
    void yyerror(const char *);
%}
%x       CSYLE_COMMENT
ID       [A-Za-z][A-Za-z0-9]*
num      0|[1-9][0-9]*


%%

<INITIAL>"/*"                      BEGIN(CSYLE_COMMENT);

<CSYLE_COMMENT>[\n\t\r ]				{}
<CSYLE_COMMENT><<EOF>>             { yyerror("dangling comment"); }  
<CSYLE_COMMENT>"*/"                BEGIN(INITIAL);  
<CSYLE_COMMENT>.				{}


[\n\t\r ]+          ;

"->"                 { return  T_ARROW;   }


"new"                { return  T_NEW; }
"class"              { return  T_CLASS;}
"extends"            { return  T_EXTENDS;}

"print"              { return  T_PRINT;      }
"+"                  { return  T_PLUS;       }
"-"                  { return  T_MINUS;      }
"*"                  { return  T_MULT;      }
"/"                  { return  T_DIVIDE;      }
"and"                { return  T_AND;      }
"or"                 { return  T_OR;      }
"not"                { return  T_NOT;     }

"("                  { return  T_OPENPAREN;     }
")"                  { return  T_CLOSEPAREN;     }
"{"                  { return  T_OPENBRACKET;     }
"}"                  { return  T_CLOSEBRACKET;     }
";"                  { return  T_SEMICOLON;  }
","                  { return  T_COMMA;      }
"."                  { return  T_DOT;        }
"="                  { return  T_ASSIGNMENT ;        }

"while"              { return  T_WHILE;      }

"repeat"             { return  T_REPEAT;      }
"until"              { return  T_UNTIL;      }

"<="                 { return  T_LESSEQ;   }
"<"                  { return  T_LESS;        }
"equals"             { return  T_EQUALS;   }
"if"                 { return  T_IF;      }
"else"               { return  T_ELSE;     }

"boolean"            { return  T_BOOLEAN;    }
"integer"            { return  T_INTEGER;    }

"none"               { return  T_NONE;}




<<EOF>>              { yyterminate();}



":"                  { return  T_COLON;  }  
"return"             { return  T_RETURN; }
"true"               { yylval.integer_ptr = new IntegerNode(1); return  T_TRUE;}
"false"              { yylval.integer_ptr = new IntegerNode(0); return  T_FALSE;}

{num}                { yylval.integer_ptr= new IntegerNode(atoi(yytext)); return  T_LITERAL;}
{ID}                 { yylval.identifier_ptr  = new IdentifierNode(yytext) ; return T_IDENTIFIER;}
 /* value detector */

.                 { yyerror("invalid character"); }

%%

int yywrap(void) {
  return 1;
}
