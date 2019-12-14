#include <cstdio>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <iterator>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>

using namespace std ;

#ifdef GLPK
extern "C" {
#include "glpk.h"
}
#endif

#ifdef LPSOLVE
extern "C" {
#include "lpkit.h"
}
#endif 


#define MAXNODES  40000

typedef struct sitemquan {
   string itemname ;
   int    quantity ;
} SItemQuan ;

typedef struct itemquan {
   int    itemno   ;
   int    quantity ;
} ItemQuan ;

class ItemQuanLess {
   public:
      bool operator() (const ItemQuan & p, const ItemQuan & q) {
                      return(p.itemno < q.itemno) ;
      }
} ;

typedef set<ItemQuan,ItemQuanLess> ItemQuanSet  ;
typedef set<ItemQuan,ItemQuanLess> ::iterator ItemQuanIterator ;
typedef map<string,int,less<string> > HashTable ;
typedef set<int,less<int> > IntSet  ;
typedef set<int,less<int> > ::iterator IntSetIterator ;


typedef struct request {
   ItemQuanSet lhs ; 
   ItemQuanSet rhs ;
   int         priority ; 
} Request ; 


class Vertex { 
 public: 
    vector <int> F     ;
    vector <int> B     ; 
    vector <int> FQ    ; 
    vector <int> BQ    ; 
    int    indegree    ;
    int    outdegree   ;
    int    icount      ;
    int    ocount      ;
    int    label       ;
    int    glpk        ;
    Vertex() { 
      label = 0 ; 
    }
} ; 

// Function prototypes 
int  inssupply(string,int) ; 
void insrequest(ItemQuanSet,ItemQuanSet,int) ; 
void print_sys() ; 
void buildandor() ; 
int  bc_components() ; 
extern void davisu(vector<Vertex> &,int,int) ; 
void dfs_comps(int,int) ; 
int checksoln(IntSet & soln,int incflag) ; 

#ifdef GLPK
void glpksolve(int) ; 
#endif

#ifdef LPSOLVE
void lpsolve(int,int) ; 
#endif

#ifdef CPLEX
int solvebycplex(int compno,int incflag) ; 
#endif 


/* This simple routine frees up the pointer *ptr, and sets *ptr to NULL */

static void
free_and_null (char **ptr)
{
   if ( *ptr != NULL ) {
      free (*ptr);
      *ptr = NULL;
   }
} /* END free_and_null */
