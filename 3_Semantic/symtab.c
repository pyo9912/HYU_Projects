/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

#define MAX_SCOPE 1000

static ScopeList scopes[MAX_SCOPE];
static ScopeList scopeStack[MAX_SCOPE];
static int location[MAX_SCOPE];
static int scopeIdx = 0;
static int stackIdx = 0;

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

int addLocation(void)
{
  return location[stackIdx - 1]++;
}

ScopeList createScope( char * name)
{
  ScopeList newScope;
  newScope = (ScopeList)malloc(sizeof(struct ScopeListRec));
  newScope->name = name;
  newScope->level = stackIdx;
  newScope->parent = scTop();
  scopes[scopeIdx++] = newScope;

  return newScope;
}

ScopeList scTop(void)
{
  if (stackIdx > 0) {
    return scopeStack[stackIdx - 1];
  }
  return NULL;
}

void scPush(ScopeList scope)
{
  scopeStack[stackIdx] = scope;
  location[stackIdx++] = 0;
}

void scPop(void)
{
  if (stackIdx > 0) {
    stackIdx--;
  }
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * node )
{ 
  int h = hash(name);
  ScopeList curScope = scTop();
  BucketList l = curScope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = node;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = curScope->bucket[h];
    curScope->bucket[h] = l; }
  else /* found in table, so just add line number */
  { //LineList t = l->lines;
    //while (t->next != NULL) t = t->next;
    //t->next = (LineList) malloc(sizeof(struct LineListRec));
    //t->next->lineno = lineno;
    //t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or NULL if not found
 */
BucketList get_bucket( char * name )
{ 
  int h = hash(name);
  ScopeList curScope = scTop();
  while (curScope != NULL) {
    BucketList l = curScope->bucket[h];
    while ((l != NULL)  && (strcmp(name,l->name)!=0))
      l = l->next;
    if (l != NULL) {
      return l;
    }
    curScope = curScope->parent;
  }
  return NULL;
}

int st_lookup ( char * name )
{ 
  BucketList l = get_bucket(name);
  if (l != NULL) {
    return l->memloc;
  }
  return -1;
}

int st_lookup_current ( char * name )
{ int h = hash(name);
  ScopeList curScope = scTop();
  BucketList l = curScope->bucket[h];
  while ((l != NULL) && strcmp(name,l->name)!=0)
    l = l->next;
  if (l != NULL) return l->memloc;
  return -1;
}

void st_line_add( char * name, int lineno )
{ 
  int h = hash(name);
  ScopeList curScope = scTop();
  while (curScope != NULL) {
    BucketList l = curScope->bucket[h];
    while ((l != NULL) && strcmp(name,l->name)!=0)
      l = l->next;
    if (l != NULL) {
      LineList ll = l->lines;
      while (ll->next != NULL) {
        ll = ll->next;
      }
      ll->next = (LineList)malloc(sizeof(struct LineListRec));
      ll->next->lineno = lineno;
      ll->next->next = NULL;
    }
    curScope = curScope->parent;
  }
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{
  printSymbolTab(listing);
  fprintf(listing,"\n");
  printFunTab(listing);
  fprintf(listing,"\n");
  printFunGlob(listing);
  fprintf(listing,"\n");
  printFunParam(listing);
}

void printSymbolTab(FILE * listing)
{
  int sc, bk;
  fprintf(listing,"< Symbol Table >\n");
  fprintf(listing,"Variable Name  Variable Type  Scope Name  Location   Line Numbers\n");
  fprintf(listing,"-------------  -------------  ----------  --------   ------------\n");
  for (sc = 0; sc < scopeIdx; sc++)
  { ScopeList curScope = scopes[sc];
    BucketList * hashTable = curScope->bucket;
    for (bk = 0; bk < SIZE; bk++){
      if (hashTable[bk] != NULL)
      { BucketList l = hashTable[bk];
        while (l != NULL)
        { LineList t = l->lines;
          fprintf(listing,"%-14s ",l->name);
          TreeNode * node = l->treeNode;
          switch (node->nodekind)
          {
            case DeclK:
              switch (node->kind.decl) {
                case VarK:
                  switch (node->type)
                  {
                    case Void:
                      fprintf(listing, "%-15s", "Void");
                      break;
                    case Integer:
                      fprintf(listing, "%-15s", "Integer");
                      break;
                    default:
                      break;
                  }
                  break;
                case ArrVarK:
                  fprintf(listing, "%-15s", "IntegerArray");
                  break;
                case FunK:
                  fprintf(listing, "%-15s", "Function");
                  break;
                case ParamK:
                  fprintf(listing, "%-15s", "Integer");
                  break;
                case ArrParamK:
                  fprintf(listing, "%-15s", "IntegerArray");
                  break;
                default:
                  break;
              }
              break;
            default:
              break;
          }
          fprintf(listing, "%-12s", curScope->name);
          fprintf(listing, "%-10d", l->memloc);
          while (t != NULL)
          { fprintf(listing,"%2d ",t->lineno);
            t = t->next;
          }
          fprintf(listing,"\n");
          l = l->next;
        }
      }
    }
  }
} /* printSymTab */

void printFunTab(FILE * listing)
{
  int sc, bk, ps, pb;
  int haveParam;
  fprintf(listing,"< Function Table >\n");
  fprintf(listing,"Function Name  Scope Name  Return Type  Parameter Name   Parameter Type\n");
  fprintf(listing,"-------------  ----------  -----------  --------------   --------------\n");
  for (sc = 0; sc < scopeIdx; sc++)
  { ScopeList curScope = scopes[sc];
    BucketList * hashTable = curScope->bucket;
    for (bk = 0; bk < SIZE; bk++)
    {
      if (hashTable[bk] != NULL)
      { BucketList l = hashTable[bk];
        TreeNode * node = l->treeNode;
        while (l != NULL)
        {
          switch (node->nodekind)
          {
            case DeclK:
              if (node->kind.decl == FunK) {
                fprintf(listing, "%-15s", l->name);
                fprintf(listing, "%-12s", curScope->name);
                switch (node->type)
                {
                  case Void:
                    fprintf(listing, "%-15s", "Void");
                    break;
                  case Integer:
                    fprintf(listing, "%-15s", "Integer");
                    break;
                  default:
                    break;
                }
              }
              haveParam = 0;
              for (ps = 0; ps < scopeIdx; ps++) {
                ScopeList paramScope = scopes[ps];
                if (strcmp(paramScope->name, l->name) != 0) continue;
                BucketList * pl = paramScope->bucket;
                for (pb = 0; pb < SIZE; pb++) {
                  if (pl[pb] != NULL) {
                    BucketList pbl = pl[pb];
                    TreeNode * pnode = pbl->treeNode;
                    while (pbl != NULL)
                    {
                      switch (pnode->nodekind)
                      {
                        case DeclK:
                          if (node->kind.decl == ParamK) {
                            if (strcmp(node->attr.name,"none")!=0) {
                              haveParam = 1;
                              fprintf(listing, "\n");
                              fprintf(listing, "%-40s", "");
                              fprintf(listing, "%-10s", pbl->name);
                              switch (pnode->type)
                              {
                                case Void:
                                  fprintf(listing, "%-15s", "Void");
                                  break;
                                case Integer:
                                  fprintf(listing, "%-15s", "Integer");
                                  break;
                                default:
                                  break;
                              }
                            }
                          }
                          else if (node->kind.decl == ArrParamK) {
                            if (strcmp(node->attr.name,"none")!=0) {
                              haveParam = 1;
                              fprintf(listing, "\n");
                              fprintf(listing, "%-45s", "");
                              fprintf(listing, "%-15s", pbl->name);
                              switch (pnode->type)
                              {
                                case Integer:
                                  fprintf(listing, "%-15s", "Integer");
                                  break;
                                case IntegerArray:
                                  fprintf(listing, "%-15s", "IntegerArray");
                                  break;
                                default:
                                  break;
                              }
                            }
                          }
                          break;
                        default:
                          break;
                      }
                      pbl = pbl->next;
                    }
                  }
                }
                break;
              }
              if (!haveParam) {
                fprintf(listing, "%-15s", "");
                if (strcmp(l->name, "output")!=0) {
                  fprintf(listing, "%-14s", "Void");
                }
                else {
                  fprintf(listing, "\n%-57s%-13s", "", "Integer");
                }
                fprintf(listing, "\n");
              }
              break;
            default:
              break;
          }
          l = l->next;
        }
      }
    }
  }
}

void printFunGlob(FILE * listing)
{
  int sc, bk;
  fprintf(listing, "< Function and Global Variables >\n");
  fprintf(listing,"   ID Name      ID Type    Data Type \n");
  fprintf(listing,"-------------  ---------  -----------\n");
  for (sc = 0; sc < scopeIdx; sc++)
  { ScopeList curScope = scopes[sc];
    BucketList * hashTable = curScope->bucket;
    if (strcmp(curScope->name, "global")!=0) continue;
    for (bk = 0; bk < SIZE; bk++) {
      if (hashTable[bk] != NULL) {
        BucketList l = hashTable[bk];
        while (l != NULL) {
          TreeNode * node = l->treeNode;
          switch (node->nodekind)
          {
            case DeclK:
              fprintf(listing, "%-15s", l->name);
              switch (node->kind.decl)
              {
                case FunK:
                  fprintf(listing, "%-12s", "Function");
                  switch (node->type)
                  {
                    case Void:
                      fprintf(listing, "%-12s", "Void");
                      break;
                    case Integer:
                      fprintf(listing, "%-12s", "Integer");
                      break;
                    default:
                      break;
                  }
                  break;
                case VarK:
                  fprintf(listing, "%-12s", "Variable");
                  switch (node->type)
                  {
                    case Void:
                      fprintf(listing, "%-12s", "Void");
                      break;
                    case Integer:
                      fprintf(listing, "%-12s", "Integer");
                      break;
                    default:
                      break;
                  }
                  break;
                case ArrVarK:
                  fprintf(listing, "%-12s", "Varaible");
                  fprintf(listing, "%-15s", "IntegerArray");
                  break;
                default:
                  break;
              }
              fprintf(listing, "\n");
              break;
            default:
              break;
          }
          l = l->next;
        }
      }
    }
  }
}

void printFunParam(FILE * listing)
{
  int sc, bk;
  int haveParam;
  fprintf(listing, "< Function Parameters and Local Variables >\n");
  fprintf(listing,"  Scope Name    Nested Level     ID Name      Data Type \n");
  fprintf(listing,"--------------  ------------  -------------  -----------\n");
  for (sc = 0; sc < scopeIdx; sc++)
  { ScopeList curScope = scopes[sc];
    BucketList * hashTable = curScope->bucket;
    if (strcmp(curScope->name, "global") != 0) continue;
    for (bk = 0; bk < SIZE; bk++) {
      if (hashTable[bk] != NULL)
      { BucketList l = hashTable[bk];
        while (l != NULL) {
          TreeNode * node = l->treeNode;
          switch (node->nodekind)
          {
            case DeclK:
              haveParam = 1;
              fprintf(listing, "%-16s", curScope->name);
              fprintf(listing, "%-14d", curScope->level);
              switch (node->kind.decl)
              {
                case VarK:
                  switch (node->type)
                  {
                    case Void:
                      fprintf(listing, "%-15s", node->attr.name);
                      fprintf(listing, "%-15s", "Void");
                      break;
                    case Integer:
                      fprintf(listing, "%-15s", node->attr.name);
                      fprintf(listing, "%-12s", "Integer");
                      break;
                  }
                  break;
                case ArrVarK:
                  fprintf(listing, "%-15s", node->attr.name);
                  fprintf(listing, "%-12s", "IntegerArray");
                  break;
                case ParamK:
                  fprintf(listing, "%-15s", node->attr.name);
                  fprintf(listing, "%-12s", "Integer");
                  break;
                case ArrParamK:
                  fprintf(listing, "%-15s", node->attr.name);
                  fprintf(listing, "%-12s", "IntegerArray");
                  break;
                default:
                  break;
              }
              fprintf(listing, "\n");
              break;
            default:
              break;
          }
          l = l->next;
        }
      }
    }
    if (!haveParam) fprintf(listing, "\n");
  } 
}