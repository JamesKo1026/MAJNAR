#ifndef SIMPLIFY_H
#define SIMPLIFY_H
#include "include.h"
#include "struct.h"
#include "Basic.h"

extern Xmg_Ntk_t *Gntk;
extern vector<int> *EachNodeDom;
extern map<int,string> id_name;
extern map<string,int> name_id;
extern int size;
extern int maj_count;
extern int xor_count;
extern node_info *nodeInfoTable;
extern unsigned global_index;
extern vector<int> nList;

extern void merge_simplify(Xmg_Obj_t *n, Xmg_Obj_t *node, bool phase);
extern void NodeARSimplify(Xmg_Obj_t* node, Xmg_Obj_t* A, bool Afault, Xmg_Obj_t* B, bool Bfault, Xmg_Obj_t* C, bool Cfault, bool direct);
extern void NodeARSimplify_TWOINPUTS(Xmg_Obj_t* node, Xmg_Obj_t* A, bool Afault, Xmg_Obj_t* B, bool Bfault, bool direct, bool operation);
extern void Removal(Xmg_Obj_t *node, bool FAULT);
extern void SimplifyXOR(Xmg_Ntk_t *ntk);

#endif
