#ifndef RECURIMPLY_H
#define RECURIMPLY_H
#include "include.h"
#include "struct.h"
#include "FindDom.h"

extern Xmg_Ntk_t *Gntk;
extern node_info *nodeInfoTable;
extern st_table* ValuingNode;
extern st_table* JTable;
extern map<int, string> id_name;

extern bool RecurImply();

#endif
