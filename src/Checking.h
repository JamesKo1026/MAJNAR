#ifndef CHECKING_H
#define CHECKING_H
#include "include.h"
#include "struct.h"
#include "FindDom.h"

extern Xmg_Ntk_t *Ontk;
extern Xmg_Ntk_t *Gntk;
extern map<int, string> id_name;
extern int id_index;
extern node_info* nodeInfoTable;
extern int size;

extern bool checkImply(Xmg_Ntk_t *ntk, vector<SideInput_Pair_t> sideinputpairs);
extern bool check(Xmg_Ntk_t *ntk);
extern vector<int> Sim();

#endif
