#ifndef IMPLY_H
#define IMPLY_H
#include "include.h"
#include "struct.h"
#include "FindDom.h"
#include "RecurImply.h"
#include "Checking.h"
#include "NodeAR.h"

extern Xmg_Ntk_t *Gntk;
extern node_info* nodeInfoTable;
extern st_table *ValuingNode; // List of node who has been assigned value;
extern st_table *JTable; // List of "possible" j-froniter;
extern map<int, string> id_name;
//extern int id_index;
extern int size;

extern bool ActivateImply(Xmg_Obj_t *node, tval_t tvalue, vector<SideInput_Pair_t> sip_set);
extern bool AcitvateImplyFanoutCone(Xmg_Obj_t *node, tval_t tvalue);
extern bool imply_forward(Xmg_Obj_t *n, Xmg_Obj_t *r);
extern bool imply_backward(Xmg_Obj_t *n, Xmg_Obj_t *r);
extern bool SideInputImply(st_table *si_set);
extern bool ActivateImplyNAR(Xmg_Obj_t *node, bool value);

#endif
