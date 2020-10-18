#ifndef NODEMGR_H
#define NODEMGR_H
#include "include.h"
#include "struct.h"
#include "FindDom.h"
#include "Imply.h"
#include "Basic.h"
#include "Simplify.h"
#include "NodeAR.h"

extern Xmg_Ntk_t *Ontk;
extern Xmg_Ntk_t *Gntk;

//extern int id_index;
extern int substitute_count;
extern int replaced_count;
extern int replaced_count_NAR;
extern int replaced_count_NAR_AND;
extern int replaced_count_NAR_OR;
extern int remove_count;
extern int MAreuse;
extern double duration;
extern int size;

extern vector<int> remove_table;
extern vector<node_pair> replaced_table;

extern map<int,string> O_id_name;
extern map<string,int> O_name_id;
extern map<int,string> id_name;
extern map<string,int> name_id;

extern map<int,int> PI_judge;
extern map<int,int> PO_judge;

extern st_table *ValuingNode;
extern st_table *JTable;
extern node_info* nodeInfoTable;
extern int *validTable;
extern vector<int> *EachNodeDom;
extern vector<int> nList;
extern unsigned global_index;

extern void PrintItems(st_table* st_set);
extern void UpdateJTable(st_table *JTableTmp);
extern void NodeMGR();
extern void NodeAR(Xmg_Obj_t* node, st_table* one_set, st_table* zero_set, bool direct, vector<SideInput_Pair_t> st_sideinputpair, int operation);

#endif
