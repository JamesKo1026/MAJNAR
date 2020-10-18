#ifndef NODEAR_H
#define NODEAR_H
#include "include.h"
#include "struct.h"
#include "FindDom.h"
#include "Imply.h"
#include "Basic.h"
#include "Simplify.h"
#include "NodeMGR.h"

extern Xmg_Ntk_t *Gntk;
extern node_info* nodeInfoTable;
extern st_table *ValuingNode; // List of node who has been assigned value;
extern st_table *JTable; // List of "possible" j-froniter;
extern map<int, string> id_name;
//extern int id_index;
extern int size;
extern int substitute_count;
extern int replaced_count;

#endif