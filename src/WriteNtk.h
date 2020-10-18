#ifndef WRITENTK_H
#define WRITENTK_H
#include "include.h"
#include "struct.h"

extern Xmg_Ntk_t *Gntk;

extern map<int,string> O_id_name;
extern map<string,int> O_name_id;
extern map<int,string> id_name;
extern map<string,int> name_id;

extern bool WriteGntk(char *filename);

#endif
