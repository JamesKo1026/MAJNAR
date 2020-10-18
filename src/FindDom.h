#ifndef FINDDOM_H
#define FINDDOM_H
#include "include.h"
#include "struct.h"

extern Xmg_Ntk_t *Ontk;
extern Xmg_Ntk_t *Gntk;

//extern int id_index;
extern int size;

extern map<int,string> O_id_name;
extern map<string,int> O_name_id;
extern map<int,string> id_name;
extern map<string,int> name_id;

extern map<int,int> PI_judge;
extern map<int,int> PO_judge;

extern st_table *ValuingNode;
extern node_info* nodeInfoTable;
extern vector<int> *EachNodeDom;

extern tval_t inv(tval_t t);
extern tval_t value_1(Xmg_Obj_t *t_node);
extern tval_t value_0(Xmg_Obj_t *t_node);
extern tval_t node_value(Xmg_Ntk_t *ntk, Xmg_Obj_t *n, Xmg_Obj_t *r, int index, tval_t tvalue);
extern void copy_value_infotable(vector<node_info> &nodeInfoTable_tmp);
extern void load_value_infotable(vector<node_info> &nodeInfoTable_tmp);
extern void refresh_value_infotable(vector<node_info> &nodeInfoTable_tmp);
extern void refresh_value_nodeInfotable();
extern void RefreshNodeValue(st_table* st_set);
extern void LoadNodeValue(st_table* st_set);
extern void load_map_to_nodeInfotable(st_table *st);
extern void BackupImplied(st_table *implied_set);
extern void st_intersection(st_table *f_st, st_table *s_st);
extern st_table * FindFanoutConeDfs(Xmg_Obj_t *node);
extern st_table * FindFaninConeDfs(Xmg_Obj_t *node);
extern void GetSideInputPair(Xmg_Obj_t *ns, st_table *si_set, vector<SideInput_Pair_t> &sip_set);
extern void GetSideInputCouples(Xmg_Obj_t *ns, Xmg_Obj_t *nt, st_table *si_set, vector<SideInput_Pair_t> &sip_set);

#endif
