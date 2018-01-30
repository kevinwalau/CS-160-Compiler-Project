%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <iostream>

    #include "ast.hpp"
    #define YYMAXDEPTH      500
    #define YYDEBUG 1
     #define YYINITDEPTH 500
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

%error-verbose


%token T_ARROW
%token T_PLUS   
%token T_MINUS
%token T_MULT
%token T_DIVIDE
%token T_AND 
%token T_OR      
%token T_NOT               
%token T_OPENPAREN  
%token T_CLOSEPAREN  
%token T_OPENBRACKET 
%token T_CLOSEBRACKET 
%token T_SEMICOLON
%token T_COMMA  
%token T_DOT     
%token T_ASSIGNMENT             
%token T_LESSEQ  
%token T_LESS   
%token T_EQUALS             
%token T_IF 
%token T_ELSE        
%token T_WHILE    
%token T_REPEAT
%token T_UNTIL           
%token T_PRINT          
%token<integer_ptr> T_LITERAL
%token<integer_ptr> T_TRUE
%token<integer_ptr> T_FALSE      
%token T_BOOLEAN 
%token T_INTEGER 
%token T_NONE
%token T_NEW
%token T_EXTENDS
%token T_EOF
%token T_COLON 
%token T_RETURN
%token T_CLASS
%token<identifier_ptr> T_IDENTIFIER

%left T_OR 
%left T_AND
%left T_LESSEQ T_LESS T_EQUALS
%left T_PLUS T_MINUS
%left T_MULT T_DIVIDE
%right NOT UNARYMINUS



%type <expression_ptr>       Expr
%type <memberaccess_ptr>     MemberBody
%type <new_ptr>              ClassID
%type <expression_list_ptr>  Args
%type <methodcall_ptr>       MethodBody

%type <returnstatement_ptr>  ReturnBody;

%type<declaration_list_ptr>  Declrs;
%type<identifier_list_ptr>   Declr;
%type <type_ptr>             MemType;
%type <type_ptr>             ReturnType;
%type <declaration_ptr> X;

%type <methodbody_ptr>       Body;
%type <statement_list_ptr>   Stmnts;

%type <ifelse_ptr>           IfStmnt;
%type <statement_list_ptr> IfElseBody;
%type <while_ptr>            WhileBody;
%type <repeat_ptr>           RepeatUntil;
%type <print_ptr>            PrintBody;
%type <statement_ptr>        Stmnt;
%type <assignment_ptr>       Assignment;

%type <parameter_ptr>        Param;
%type <parameter_list_ptr>   Params;

%type<declaration_list_ptr> Mems;
%type<declaration_ptr> Mem;

%type <method_list_ptr>      Methods;
%type <method_ptr>           Method;

%type<class_ptr> ClassBody;
%type<identifier_ptr> ExtendsBody;
%type <class_list_ptr>       Classes;
%type <class_ptr>            Class;

%type<program_ptr>          Start;


%%


Start : Classes           {$$= new ProgramNode($1); astRoot=$$;}     
Classes : Classes Class   {$$= $1; $$->push_back($2);}
      | Class             {$$= new std::list<ClassNode*>(); $$->push_back($1);}
      ;            

Class : T_IDENTIFIER ExtendsBody T_OPENBRACKET ClassBody T_CLOSEBRACKET {$4->identifier_1= $1;
                                                               $4->identifier_2= $2;
                                                               $$=$4; }
      ;

ExtendsBody : T_EXTENDS T_IDENTIFIER {$$= $2;}
         | {$$= NULL;}
         ;
ClassBody : Mems Methods {$$= new ClassNode(NULL, NULL, $1, $2    ); }
            | Mems         {$$= new ClassNode(NULL, NULL, $1, NULL  ); }
            | Methods         {$$= new ClassNode(NULL, NULL, NULL, $1  ); }
            |                 {$$= new ClassNode(NULL, NULL, NULL, NULL); }               
            ;
Mems : Mems Mem T_SEMICOLON   {$$= $1; $$->push_back($2);}
        | Mem T_SEMICOLON           {$$= new std::list<DeclarationNode*>(); $$->push_back($1);}
Mem :  MemType T_IDENTIFIER   {std::list<IdentifierNode*> *s = new std::list<IdentifierNode*>();
                                       s->push_back($2);
                                       $$= new DeclarationNode($1,s); $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;  }
       ;
MemType : T_BOOLEAN    {$$= new BooleanTypeNode(); $$->objectClassName= "Boolean"; $$->basetype= bt_boolean;}
             | T_INTEGER    {$$= new IntegerTypeNode(); $$->objectClassName= "Integer"; $$->basetype= bt_integer;}
             | T_IDENTIFIER {$$= new ObjectTypeNode($1); $$->objectClassName= $1->name; $$->basetype= bt_object; }
             ;
Methods : Methods Method  {$$= $1; $$->push_back($2);}
        | Method          {$$= new std::list<MethodNode*>(); $$->push_back($1);}
        ;
ReturnType : T_BOOLEAN {$$= new BooleanTypeNode(); $$->objectClassName= "Boolean"; $$->basetype= bt_boolean;}
                    | T_INTEGER {$$= new IntegerTypeNode(); $$->objectClassName= "Integer"; $$->basetype= bt_integer;}
                    | T_IDENTIFIER {$$= new ObjectTypeNode($1); $$->objectClassName= $1->name; $$->basetype= bt_object;}
                    | T_NONE       {$$= new NoneNode(); $$->objectClassName="None"; $$->basetype= bt_none;}
                    ;
Method : T_IDENTIFIER T_OPENPAREN Params T_CLOSEPAREN T_ARROW ReturnType T_OPENBRACKET Body T_CLOSEBRACKET {$$= new MethodNode( $1, $3,$6, $8 ); $$->objectClassName= $6->objectClassName; $$->basetype= $6->basetype;}
        | T_IDENTIFIER T_OPENPAREN T_CLOSEPAREN T_ARROW ReturnType T_OPENBRACKET Body T_CLOSEBRACKET {$$= new MethodNode( $1, NULL,$5, $7 );}
        ;
Params :  Params T_COMMA Param                  {$$= $1; $$->push_back($3);}
           |  Param                                     {$$= new std::list<ParameterNode*>(); $$->push_back($1);}
           ;

Stmnts : Stmnts Stmnt   {$$=$1; $$->push_back($2);}
           | Stmnt              {$$= new std::list<StatementNode*>(); $$->push_back($1);    }
           ;
Stmnt  : MethodBody T_SEMICOLON {$$= new CallNode($1);}
           | Assignment T_SEMICOLON {$$=$1;}
           | WhileBody {$$=$1;}
           | RepeatUntil T_SEMICOLON {$$=$1;}
           | PrintBody T_SEMICOLON {$$=$1;}
           | IfStmnt {$$=$1;}
           ;
Assignment : T_IDENTIFIER T_DOT T_IDENTIFIER T_ASSIGNMENT Expr   {$$= new AssignmentNode($1,$3,$5);}                                                        
          | T_IDENTIFIER  T_ASSIGNMENT Expr                     {$$= new AssignmentNode($1,NULL,$3);}
          ;
WhileBody : T_WHILE Expr T_OPENBRACKET Stmnts T_CLOSEBRACKET {$$= new WhileNode($2,$4);}
          ;
MemberBody : T_IDENTIFIER T_DOT T_IDENTIFIER                                                           {$$= new MemberAccessNode($1,$3);}
              ;

Param : T_IDENTIFIER T_COLON MemType              {$$= new ParameterNode($3, $1); $$->objectClassName= $3->objectClassName; $$->basetype= $3->basetype;}              
     ;
Body :Declrs Stmnts ReturnBody {$$= new MethodBodyNode($1,$2,$3);}
     | Declrs ReturnBody           {$$= new MethodBodyNode($1,NULL,$2);}
     | Stmnts ReturnBody                      {$$= new MethodBodyNode(NULL,$1,$2);}
     | ReturnBody                                 {$$= new MethodBodyNode(NULL, NULL, $1);}
     ;

Declrs : Declrs X  T_SEMICOLON   {$$= $1; $$->push_back($2);}
                      | X T_SEMICOLON                         {$$= new std::list<DeclarationNode*>(); $$->push_back($1);}
                      ;
X : MemType Declr  {$$= new DeclarationNode($1,$2); $$->basetype= $1->basetype; $$->objectClassName= $1->objectClassName;}

Declr :  Declr T_COMMA T_IDENTIFIER {$$=$1; $$->push_back($3);    }
            |  T_IDENTIFIER                     {$$= new std::list<IdentifierNode*>(); $$->push_back($1);  }
            ;

ReturnBody : T_RETURN Expr T_SEMICOLON  {$$= new ReturnStatementNode($2); $$->basetype=$2->basetype; $$->objectClassName= $2->objectClassName; }
               |                                  {$$= NULL;}
               ;


                                                      
Args :  Args T_COMMA Expr                                                                 {$$=$1; $$->push_back($3);  }
          |  Expr                                                                                   {$$= new std::list<ExpressionNode*>(); $$->push_back($1);}
          ;
MethodBody : T_IDENTIFIER T_OPENPAREN Args T_CLOSEPAREN                                                     {$$= new MethodCallNode($1, NULL, $3); }
           | T_IDENTIFIER T_OPENPAREN  T_CLOSEPAREN                                                              {$$= new MethodCallNode($1, NULL, NULL);}
           | T_IDENTIFIER T_DOT T_IDENTIFIER T_OPENPAREN Args T_CLOSEPAREN                                  {$$= new MethodCallNode($1, $3, $5);}
           | T_IDENTIFIER T_DOT T_IDENTIFIER T_OPENPAREN T_CLOSEPAREN                                            {$$= new MethodCallNode($1, $3, NULL);}     
           ;
ClassID :  T_IDENTIFIER                                                                                {$$= new NewNode($1,NULL); $$->basetype= bt_object; $$->objectClassName= $1->name;}
           |  T_IDENTIFIER T_OPENPAREN Args T_CLOSEPAREN                                                    {$$= new NewNode($1,$3)  ; $$->basetype= bt_object; $$->objectClassName= $1->name;}
	   	     |  T_IDENTIFIER T_OPENPAREN T_CLOSEPAREN													                                      {$$= new NewNode($1,NULL); $$->basetype= bt_object; $$->objectClassName= $1->name;}          
           ;
 
RepeatUntil : T_REPEAT T_OPENBRACKET Stmnts T_CLOSEBRACKET T_UNTIL T_OPENPAREN  Expr T_CLOSEPAREN {$$= new RepeatNode($3,$7);}
             ;
IfStmnt : T_IF Expr T_OPENBRACKET Stmnts T_CLOSEBRACKET IfElseBody {$$= new IfElseNode($2,$4,$6);}
        ;
IfElseBody : T_ELSE T_OPENBRACKET Stmnts T_CLOSEBRACKET                  {$$= $3;}
           |                                                             {$$= NULL;}
           ; 
PrintBody : T_PRINT Expr                                          {$$= new PrintNode($2);}
      ;

Expr : T_OPENPAREN Expr T_CLOSEPAREN                                                                 {$$= $2;}
           | Expr T_AND    Expr                                                               {$$= new AndNode($1, $3);     $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;}
           | Expr T_OR     Expr                                                               {$$= new OrNode($1, $3);      $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;}
           | Expr T_LESSEQ Expr                                                               {$$= new LessEqualNode($1, $3); $$->objectClassName= "Boolean"; $$->basetype= bt_boolean;}
           | Expr T_LESS   Expr                                                               {$$= new LessNode($1, $3) ;   $$->objectClassName= "Boolean"; $$->basetype= bt_boolean;}
           | Expr T_PLUS   Expr                                                               {$$= new PlusNode($1, $3) ;   $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;}       
           | Expr T_EQUALS Expr                                                               {$$= new EqualNode($1, $3);   $$->objectClassName= "Boolean"; $$->basetype= bt_boolean;}
           | Expr T_MINUS  Expr                                                               {$$= new MinusNode($1, $3);   $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;}       
           | Expr T_MULT   Expr                                                               {$$= new TimesNode($1, $3);   $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;}       
           | Expr T_DIVIDE Expr                                                               {$$= new DivideNode($1,$3);   $$->objectClassName= $1->objectClassName; $$->basetype= $1->basetype;}       
           | T_MINUS Expr  %prec UNARYMINUS                                                         {$$= new NegationNode($2);    $$->objectClassName= $2->objectClassName; $$->basetype= $2->basetype;}
           | T_NOT   Expr  %prec NOT                                                                {$$= new NotNode($2);       $$->objectClassName= $2->objectClassName; $$->basetype= $2->basetype;}  
           | T_LITERAL                                                                                    {$$= new IntegerLiteralNode($1);$$->objectClassName= "Integer"; $$->basetype= bt_integer;}
           | MemberBody                                                                                {$$= $1;}
           | T_IDENTIFIER                                                                                 {$$= new VariableNode($1);}
           | MethodBody                                                                                   {$$=$1;}
           | T_TRUE                                                                                       {$$= new BooleanLiteralNode($1);$$->objectClassName= "Boolean"; $$->basetype=bt_boolean;}
           | T_FALSE                                                                                      {$$= new BooleanLiteralNode($1);$$->objectClassName= "Boolean"; $$->basetype=bt_boolean;}
           | T_NEW ClassID                                                                             {$$=$2;}
           ;  
 

%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(0);
}
