/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

#define BUCKET_SIZE 211

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
{
    int lineno;
    struct LineListRec * next;
} * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
{
    char * name;
    TreeNode * treeNode;
    ExpType type;
    LineList lines;
    int memloc; /* memory location for variable */
    struct BucketListRec * next;
} * BucketList;

typedef struct ScopeListRec
{
    char * name;
    BucketList bucket[BUCKET_SIZE];
    struct ScopeListRec * parent;
    struct ScopeListRec * next;
    int level;
} * ScopeList;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * node );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int addLocation(void);

BucketList get_bucket(char * name);
int st_lookup(char * name);
int st_lookup_current(char * name);
void st_line_add(char * name, int lineno);

ScopeList createScope(char * name);
ScopeList scTop(void);
void scPush(ScopeList scope);
void scPop(void);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);
void printSymbolTab(FILE * listing);
void printFunTab(FILE * listing);
void printFunGlob(FILE * listing);
void printFunParam(FILE * listing);

#endif
