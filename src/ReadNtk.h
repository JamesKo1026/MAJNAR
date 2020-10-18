#ifndef READNTK_H
#define READNTK_H
#include "include.h"
#include "struct.h"
#include "Basic.h"

extern Xmg_Ntk_t *Ontk;
extern Xmg_Ntk_t *Gntk;

extern int id_index;
extern int substitute_count;
extern int replaced_count;

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
//extern vector<node_info> nodeInfoTable;
//extern vector<int> validTable;
//extern vector<vector<int> >EachNodeDom;
extern vector<int> nList;
extern node_info *nodeInfoTable;
extern int *validTable;
extern vector<int> *EachNodeDom;
extern int size;

extern int Ontk_level;
extern int Gntk_level;

extern bool ReadNtk(char *filename);

#endif
