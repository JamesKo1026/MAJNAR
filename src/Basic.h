#ifndef BASIC_H
#define BASIC_H
#include "include.h"
#include "struct.h"
#include "FindDom.h"

extern clock_t start;
extern vector<int> nList;
//extern int id_index;
extern int size;

extern double getTime();
extern unsigned compute_level(Xmg_Ntk_t_ *ntk);
extern int compute_gate(Xmg_Ntk_t_ *ntk);
extern void DFS(Xmg_Ntk_t_ *ntk);

#endif
