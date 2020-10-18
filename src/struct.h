#ifndef STRUCT_H
#define STRUCT_H
#include "include.h"
#include "st.h"

typedef enum{
	_0_, _1_, _x_
} tval_t;

typedef struct SideInput_Pair_t_ SideInput_Pair_t;	// MAJ: side-input pair, XOR: side input
struct SideInput_Pair_t_{ // information of input pair
	int pnode1; // pair node1
	int pnode2; // pair node2 
	int index1; // pair node1's index based on parent
	int index2; // pair node2's index based on parent
	int parent; // their parent
};

typedef struct node_info_ node_info;
struct node_info_{
	tval_t value; // value
	int valid;
	st_table *OBS1;
	st_table *OBS0;
	bool OneFanoutFanin;
};

typedef struct Xmg_Obj_t_ Xmg_Obj_t;
struct Xmg_Obj_t_{
	int Id; // node id
//	node_info info;
	unsigned Type; // 1:Const, 3:PI, 4:PO, 9:Node
	unsigned op_type; // 0:MAJ, 1:XOR, 2:I/O
	unsigned fCompl0; // 1: have an inverter.
	unsigned fCompl1; 
	unsigned fCompl2; // MAJ: 3 fanins, XOR: 2 fanins
	unsigned Level;
	unsigned Simp; // 0: not be simplified(two 2-XOR->one 3-XOR), 1: has been NodeMGR or NAR, 2: be simplified(two 2-XOR->one 3-XOR)
	Vec_Int_t vFanins;
	Vec_Int_t vFanouts;
};

typedef struct Xmg_Ntk_t_ Xmg_Ntk_t;
struct Xmg_Ntk_t_{
	string pName; // network name
	Vec_Obj_t vObjs;
	Vec_Obj_t vPis;
	Vec_Obj_t vPos;
	Xmg_Obj_t ** Obj_Ptr;
	int vPis_num;
	int vPos_num;
	int vNode_num;
};

typedef struct node_pair_ node_pair;
struct node_pair_{
	int node1;
	int node2;
};

typedef map<int, int>::value_type valType;
typedef map<pair<int, int>, int>::value_type pairType;
typedef map<SideInput_Pair_t, int>::value_type sipType;

#endif
