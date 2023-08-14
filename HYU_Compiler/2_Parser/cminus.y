/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedNum;     /* for use in array assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LCURLY RCURLY LBRACE RBRACE SEMI COMMA
%token ERROR 

%% /* Grammar for TINY */

program     : decl_list
                 { savedTree = $1; }
            ;
decl_list   : decl_list decl
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
                   }
                   else $$ = $2;
                 }
            | decl { $$ = $1; }
            ;
decl        : var_decl { $$ = $1; }
            | fun_decl { $$ = $1; }
            ;
identifier  : ID { savedName = copyString(tokenString);
                   savedLineNo = lineno; }
            ;
number      : NUM { savedNum = atoi(tokenString); 
                    savedLineNo = lineno; }
            ;
type_spec   : INT { 
                $$ = newDeclNode(TypeK);
                $$->attr.type = "int";
            }
            | VOID { 
                $$ = newDeclNode(TypeK);
                $$->attr.type = "void";
            }
            ;
var_decl    : type_spec identifier SEMI { 
                $$ = newDeclNode(VarK);
                $$->child[0] = $1;
                $$->attr.name = savedName;
                $$->lineno = lineno;
            }
            | type_spec identifier LBRACE number RBRACE SEMI
            {
                $$ = newDeclNode(ArrVarK);
                $$->attr.name = savedName;
                $$->child[0] = $1;
                $$->lineno = lineno;
            }
            ;
fun_decl    : type_spec identifier {
                $$ = newDeclNode(FunK);
                $$->attr.name = savedName;
                $$->lineno = lineno;
            }
            LPAREN params RPAREN comp_stmt {
                $$ = $3;
                $$->child[0] = $1;
                $$->child[1] = $5;
                $$->child[2] = $7;
            }
            ;
params      : param_list { $$ = $1; }
            | VOID {
                $$ = newDeclNode(ParamK);
                $$->attr.name = "none";
            }
            ;
param_list  : param_list COMMA param {
                YYSTYPE t = $1;
                if (t != NULL) {
                    while(t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $3;
                    $$ = $1;
                }
                else $$ = $3;
            }
            | param { $$ = $1; }
            ;
param       : type_spec identifier {
                $$ = newDeclNode(ParamK);
                $$->child[0] = $1;
                $$->attr.name = savedName;
            }
            | type_spec identifier LBRACE RBRACE {
                $$ = newDeclNode(ArrParamK);
                $$->child[0] = $1;
                $$->attr.name = savedName;
            }
            ;
comp_stmt   : LCURLY local_decl state_list RCURLY {
                $$ = newStmtNode(CompK);
                $$->child[0] = $2;
                $$->child[1] = $3;
            }
            ;
local_decl  : local_decl var_decl {
                YYSTYPE t = $1;
                if (t != NULL) {
                    while(t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
                }
                else $$ = $2;
            }
            | { $$ = NULL; }
            ;
state_list  : state_list statement {
                YYSTYPE t = $1;
                if (t != NULL) {
                    while(t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
                }
                else $$ = $2;
            }
            | { $$ = NULL; }
            ;
statement   : expr_stmt { $$ = $1; }
            | comp_stmt { $$ = $1; }
            | select_stmt { $$ = $1; }
            | iter_stmt { $$ = $1; }
            | return_stmt { $$ = $1; }
            ;
expr_stmt   : expression SEMI { $$ = $1; }
            | SEMI { $$ = NULL; }
            ;
select_stmt : IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE {
                $$ = newStmtNode(IfK);
                $$->child[0] = $3;
                $$->child[1] = $5;
                $$->child[2] = NULL;
            }
            | IF LPAREN expression RPAREN statement ELSE statement {
                $$ = newStmtNode(IfEK);
                $$->child[0] = $3;
                $$->child[1] = $5;
                $$->child[2] = $7;
            }
            ;
iter_stmt   : WHILE LPAREN expression RPAREN statement {
                $$ = newStmtNode(WhileK);
                $$->child[0] = $3;
                $$->child[1] = $5;
            }
            ;
return_stmt : RETURN SEMI {
                $$ = newStmtNode(ReturnK);
                $$->child[0] = NULL;
            }
            | RETURN expression SEMI {
                $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
            }
            ;
expression  : var ASSIGN expression {
                $$ = newExpNode(AssignK);
                $$->child[0] = $1;
                $$->child[1] = $3;
            }
            | simple_exp { $$ = $1; }
            ;
var         : identifier {
                $$ = newExpNode(IdK);
                $$->attr.name = savedName;
            }
            | identifier {
                $$ = newExpNode(IdK);
                $$->attr.name = savedName;
            }
            LBRACE expression RBRACE {
                $$ = $2;
                $$->child[0] = $4;
            }
            ;
simple_exp  : add_exp relop add_exp {
                $$ = $2;
                $$->child[0] = $1;
                $$->child[1] = $3;
            }
            | add_exp { $$ = $1; }
            ;
relop       : LE {
                $$ = newExpNode(OpK);
                $$->attr.op = LE;
            }
            | LT {
                $$ = newExpNode(OpK);
                $$->attr.op = LT;
            }
            | GT {
                $$ = newExpNode(OpK);
                $$->attr.op = GT;
            }
            | GE {
                $$ = newExpNode(OpK);
                $$->attr.op = GE;
            }
            | EQ {
                $$ = newExpNode(OpK);
                $$->attr.op = EQ;
            }
            | NE {
                $$ = newExpNode(OpK);
                $$->attr.op = NE;
            }
            ;
add_exp     : add_exp addop term {
                $$ = $2;
                $$->child[0] = $1;
                $$->child[1] = $3;
            }
            | term { $$ = $1; }
            ;
addop       : PLUS {
                $$ = newExpNode(OpK);
                $$->attr.op = PLUS;
            }
            | MINUS {
                $$ = newExpNode(OpK);
                $$->attr.op = MINUS;
            }
            ;
term        : term mulop factor {
                $$ = $2;
                $$->child[0] = $1;
                $$->child[1] = $3;
            }
            | factor { $$ = $1; }
            ;
mulop       : TIMES {
                $$ = newExpNode(OpK);
                $$->attr.op = TIMES;
            }
            | OVER {
                $$ = newExpNode(OpK);
                $$->attr.op = OVER;
            }
            ;
factor      : LPAREN expression RPAREN { $$ = $2; }
            | var { $$ = $1; }
            | call { $$ = $1; }
            | number {
                $$ = newExpNode(ConstK);
                $$->attr.val = savedNum;
            }
            ;
call        : identifier {
                $$ = newExpNode(CallK);
                $$->attr.name = savedName;
            }
            LPAREN args RPAREN {
                $$= $2;
                $$->child[0] = $4;
            }
            ;
args        : arg_list { $$= $1; }
            | { $$ = NULL; }
            ;
arg_list    : arg_list COMMA expression {
                YYSTYPE t = $1;
                if (t != NULL) {
                    while (t->sibling != NULL)
                        t = t->sibling;
                    t->sibling = $3;
                    $$ = $1;
                }
                else $$ = $3;
            }
            | expression { $$ = $1; }
            ;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

