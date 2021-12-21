/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int locate = 0;
static ScopeList globalScope = NULL;
static char * funcName;
static int inScope = FALSE;
char * name;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Error: Type error at line %d: %s\n",t->lineno, message);
  Error = TRUE;
}

static void symbolError(TreeNode * t, char * message)
{
  fprintf(listing, "Error: Symbol error at line %d: %s\n",t->lineno, message);
  Error = TRUE;
}

static void undeclaredError(TreeNode * t)
{
  if (t->kind.exp == CallK) {
    fprintf(listing, "Error: Undeclared Function \"%s\" at line %d\n", t->attr.name, t->lineno);
  }
  else if (t->kind.exp == IdK || t->kind.exp == IdxK) {
    fprintf(listing, "Error: Undeclared Variable \"%s\" at line %d\n",t->attr.name, t->lineno);
  }
  Error = TRUE;
}

static void redefinedError(TreeNode * t)
{
  if (t->kind.decl == FunK) {
    fprintf(listing, "Error: Redefined Function \"%s\" at line %d\n", t->attr.name, t->lineno);
  }
  else if (t->kind.decl == VarK) {
    fprintf(listing, "Error: Redefined Variable \"%s\" at line %d\n", t->attr.name, t->lineno);
  }
  else if (t->kind.decl == ArrVarK) {
    fprintf(listing, "Error: Redefined Variable \"%s\" at line %d\n", t->attr.name, t->lineno);
  }
  Error = TRUE;
}

static void funcDeclNotGlobal(TreeNode * t)
{
  fprintf(listing, "Error: Function Definition is not global at line %d (name : %s)\n", t->lineno, t->attr.name);
  Error = TRUE;
}

static void voidVarError(TreeNode * t, char * name)
{
  fprintf(listing, "Error: Variable Type cannot be Void at line %d (name : %s)\n", t->lineno, name);
  Error = TRUE;
}


static void insertBuiltIn(void)
{
  TreeNode * func;
  TreeNode * param;
  TreeNode * typeSpec;
  TreeNode * compStmt;

  func = newDeclNode(FunK);
  typeSpec = newDeclNode(TypeK);
  typeSpec->attr.type = INT;
  func->type = Integer;
  
  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  func->lineno = 0;
  func->attr.name = "input";
  func->child[0] = typeSpec;
  func->child[1] = NULL;
  func->child[2] = compStmt;

  st_insert("input", 0, addLocation(), func);

  func = newDeclNode(FunK);
  typeSpec = newDeclNode(TypeK);
  typeSpec->attr.type = VOID;
  func->type = Void;

  param = newDeclNode(ParamK);
  param->attr.name = "arg";
  param->type = Integer;
  param->child[0] = newDeclNode(FunK);
  param->child[0]->attr.type = INT;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  func->lineno = 0;
  func->attr.name = "output";
  func->child[0] = typeSpec;
  func->child[1] = param;
  func->child[2] = compStmt;

  st_insert("output", 0, addLocation(), func);

}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ 
  switch (t->nodekind)
  { case DeclK:
      switch (t->kind.decl)
      {
        case FunK:
          funcName = t->attr.name;
          if (st_lookup_current(t->attr.name) >= 0) {
            redefinedError(t);
            break;
          }
          if (scTop() != globalScope) {
            funcDeclNotGlobal(t);
            break;
          }
          st_insert(funcName, t->lineno, addLocation(), t);
          scPush(createScope(funcName));
          inScope = TRUE;

          switch (t->child[0]->attr.type)
          {
            case INT:
              t->type = Integer;
              break;
            case VOID:
            default:
              t->type = Void;
              break;
          }
          break;
        case VarK:
        case ArrVarK:
          { 
            char * name;
            if (t->kind.decl == VarK) {
              name = t->attr.name;
              t->type = Integer;
            }
            else {
              name = t->attr.name;
              t->type = IntegerArray;
            }
            if (st_lookup(name) < 0) st_insert(name, t->lineno, addLocation(), t);
            else redefinedError(t);
          }
          break;
        case ParamK:
          if (t->child[0]->attr.type == VOID) break;
          if (st_lookup(t->attr.name) == -1) {
            st_insert(t->attr.name, t->lineno, addLocation(), t);
            t->type = Integer;
          }
          break;
        case ArrParamK:
          if (t->child[0]->attr.type == VOID) break;
          if (st_lookup(t->attr.name) == -1) {
            st_insert(t->attr.name, t->lineno, addLocation(), t);
            t->type = IntegerArray;
          }
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if (inScope) inScope = FALSE;
          else {
            ScopeList scope = createScope(funcName);
            scPush(scope);
            locate++;
          }
          t->attr.scope = scTop();
          break;
        default:
          break;
      }
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
        case IdxK:
        case CallK:
          if (st_lookup(t->attr.name) == -1) {
          /* not yet in table, so treat as new definition */
            undeclaredError(t);
          }
          else {
          /* already in table, so ignore location, 
             add line number of use only */
            st_line_add(t->attr.name, t->lineno);
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void insertCheck(TreeNode * t)
{
  if (t->nodekind == StmtK && t->kind.stmt == CompK) {
    scPop();
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
  globalScope = createScope("global");
  scPush(globalScope);
  insertBuiltIn();
  traverse(syntaxTree, insertNode, insertCheck);
  scPop();
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void beforeCheck(TreeNode * t)
{
  switch (t->nodekind)
  {
    case DeclK:
      switch (t->kind.decl)
      {
        case FunK:
          funcName = t->attr.name;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      {
        case CompK:
          scPush(t->attr.scope);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/// TODO
/// Modify checkNode
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case AssignK:
          if (t->child[0]->type == Void || t->child[1]->type == Void)
            typeError(t,"invalid variable type");
          else if (t->child[0]->type==IntegerArray && t->child[0]->child[0]==NULL)
            typeError(t->child[0],"invalid variable type");
          else if (t->child[1]->type==IntegerArray && t->child[1]->child[0]==NULL)
            typeError(t->child[0],"invalid variable type");
          else t->type = t->child[0]->type;
          break;
        case OpK:
        {
          ExpType lt, rt;
          TokenType op;

          lt = t->child[0]->type;
          rt = t->child[1]->type;
          op = t->attr.op;

          if (lt==IntegerArray && t->child[0]->child[0]!=NULL) lt = Integer;
          if (rt==IntegerArray && t->child[1]->child[0]!=NULL) rt = Integer;

          if (lt == Void || rt == Void)
            typeError(t,"invalid void variable");
          else if (lt != rt)
            typeError(t,"invalid expression");
          else t->type = Integer;
          break;
        }
        case CallK:
        {
          BucketList bk = st_lookup(t->attr.name);
          TreeNode * funcNode = NULL;
          TreeNode * arg;
          TreeNode * param;

          if (bk == NULL) break;
          funcNode = bk->treeNode;
          arg = t->child[0];
          param = funcNode->child[1];

          if (funcNode->kind.decl != FunK) {
            typeError(t,"invalid expression");
            break;
          }
          while (arg != NULL) {
            if (param == NULL || arg->type == Void) {
              typeError(arg,"invalid function call");
              break;
            }
            ExpType pt = param->type;
            ExpType at = arg->type;
            if (pt != at) {
              typeError(arg,"invalid function call");
              break;
            }
            else {
              arg = arg->sibling;
              param = param->sibling;
            }
          }
          if (arg == NULL && param != NULL && param->child[0]->attr.type != VOID)
            typeError(t->child[0],"invalid function call");
          t->type = funcNode->type;
          break;
        }
        case ConstK:
          t->type = Integer;
          break;
        case IdK:
        case IdxK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          scPop();
          break;
        case IfK:
        case IfEK:
          if (t->child[0] == NULL)
            typeError(t,"expected expression is missing");
          else if (t->child[0]->type == Void)
            typeError(t->child[0],"invalid if condition");
          break;
        case WhileK:
          if (t->child[0] == NULL)
            typeError(t->child[0],"expected expression is missing");
          else if (t->child[0]->type == Void)
            typeError(t->child[0],"invalid while condition");
          break;
        case ReturnK:
        {
          TreeNode * ret = get_bucket(funcName)->treeNode;
          if (ret->type == Void && t->child[0] != NULL)
            typeError(t,"invalid return type");
          else if (ret->type == Integer && (t->child[0]==NULL || t->child[0]->type==Void || t->child[0]->type==IntegerArray))
            typeError(t,"invalid return type");
          else if (ret->type == IntegerArray && (t->child[0]==NULL || t->child[0]->type==Void || t->child[0]->type==IntegerArray))
            typeError(t,"invalid return type");
          break;
        }
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      {
        case VarK:
        case ArrVarK:
          if (t->child[0]->attr.type == VOID) {
            char * name;
            if (t->kind.decl == VarK) name = t->attr.name;
            else name = t->attr.name;
            voidVarError(t, name);
            break;
          }
          break;
        default:
          break;
      }
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ 
  scPush(globalScope);
  traverse(syntaxTree,beforeCheck,checkNode);
  scPop();
}
