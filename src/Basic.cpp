#include "Basic.h"

int *color;

//***********************************************************
double getTime()
{
	clock_t now = clock();
	return (double)(now - start)/CLOCKS_PER_SEC;
}

//***********************************************************
void DFS_Visit(Xmg_Ntk_t *ntk, int node_id)
{
	color[node_id] = GRAY;
	unsigned u;
	int fi_id;
	for(u = 0; u < ntk->Obj_Ptr[node_id]->vFanins.size(); u++){
		Xmg_Obj_t* fi = ntk->Obj_Ptr[ntk->Obj_Ptr[node_id]->vFanins[u]];
		fi_id = fi->Id;
		if(color[fi_id] == WHITE)
			DFS_Visit(ntk, fi_id);
	}
	color[node_id] = BLACK;
	if(ntk->Obj_Ptr[node_id]->Type == NODE)
		nList.push_back(node_id);
}

//***********************************************************
void DFS(Xmg_Ntk_t *ntk) // QA : Order is expected ?
{
//	int size = ntk->vPis_num + ntk->vPos_num + ntk->vNode_num+2+10;
	color = new int[size];
	
	int node_id;
	for(node_id = 0; node_id < size; node_id++)
		color[node_id] = WHITE;
		
	unsigned v;
	for(v = 0; v < ntk->vPos.size(); v++){
		node_id = ntk->vPos[v].Id;
		if(color[node_id] == WHITE)
			DFS_Visit(ntk, node_id);
	}
	delete [] color;
}

//***********************************************************
int get_level(Xmg_Ntk_t *ntk, Xmg_Obj_t *node)
{
	if(node->vFanins.size() == 0)
		return 0;
	else{
		vector<int> const_index_ary;
		unsigned i;
		Xmg_Obj_t *fi;
		node_info *ir;
		int zero_count = 0;
		int one_count = 0;
		unsigned max = 0;
		//cout << "Node op_type is: " << node->op_type << ", fanin size is: " << node->vFanins.size() << endl;

		for(i = 0; i < node->vFanins.size(); i++){
			fi = ntk->Obj_Ptr[node->vFanins[i]];
			ir = &nodeInfoTable[fi->Id];
			if(fi->Id < 2){
				if(ir->value == node_value(ntk, fi, node, i, _0_)) zero_count++;
				else if(ir->value == node_value(ntk, fi, node, i, _1_)) one_count++;
			}
			//cout << node->vFanins[i] << ", fi->Level: " << fi->Level << ", max: " << max << endl;
			if(fi->Level > max) max = fi->Level;
		}
		if (node->Type == Zombie)	return 0;
		
		if (node->op_type == 0) {
			if (zero_count == 1 && one_count == 1) return max;
			else if (zero_count >= 2 || one_count >= 2) return 0;
			else return max+1;
		} else {
			if (zero_count == 1 || one_count == 1) return max;
			else if (zero_count == 2 || one_count == 2) return 0;
			else return max+1;
		}
		
	}	
}

//***********************************************************
unsigned compute_level(Xmg_Ntk_t *ntk)
{
	unsigned max_level = 0;
	queue<int> Q;
	for(vector<Xmg_Obj_t>::iterator iter = ntk->vPis.begin(); iter != ntk->vPis.end(); iter++)
		Q.push(iter->Id);
	int *deg;
	deg = new int[size];
	for(int i = 0; i < size; i++)
		deg[i] = 0;
	
	int node_id;
	Xmg_Obj_t *node;
	Xmg_Obj_t *fo;
	while(!Q.empty()){
		node_id = Q.front();
		Q.pop();
		node = ntk->Obj_Ptr[node_id];
		for(vector<int>::iterator iter = node->vFanouts.begin(); iter != node->vFanouts.end(); iter++){
			fo = ntk->Obj_Ptr[*iter];
			
			/* For MAJ */
			if (fo->op_type == 0) {
				if(deg[fo->Id] < 3)
					++deg[*iter];
				if(deg[fo->Id] == 3){
					//	get node level
					fo->Level = get_level(ntk,fo);
					//cout << "compute level: " << fo->Level << ", node Id: " << fo->Id << "\n\n";
					if(fo->Level > max_level) max_level = fo->Level;
					Q.push(fo->Id);
				}
			} else if (fo->op_type == 1 && fo->Simp == 2) {
				if(deg[fo->Id] < 3)
					++deg[*iter];
				if(deg[fo->Id] == 3){
					//	get node level
					fo->Level = get_level(ntk,fo);
					//cout << "compute level: " << fo->Level << ", node Id: " << fo->Id << "\n\n";
					if(fo->Level > max_level) max_level = fo->Level;
					Q.push(fo->Id);
				}
			} else {	/* For XOR */
				if(deg[fo->Id] < 2)
					++deg[*iter];
				if(deg[fo->Id] == 2){
					//	get node level
					fo->Level = get_level(ntk,fo);
					//cout << "compute level: " << fo->Level << ", node Id: " << fo->Id << "\n\n";
					if(fo->Level > max_level) max_level = fo->Level;
					Q.push(fo->Id);
				}
			}
			
		}
	}	
	delete [] deg;	
	return max_level;
}

//***********************************************************
int compute_gate(Xmg_Ntk_t *ntk)
{
	int G_count = 0;
	unsigned i;
	int var_count = 0;
	for(vector<Xmg_Obj_t>::iterator iter = ntk->vObjs.begin(); iter != ntk->vObjs.end(); iter++){
		var_count = 0;
		// for(i = 0; i < iter->vFanins.size(); i++){
		// 	if(iter->vFanins[i] > 1) var_count++;
		// }
		// if(var_count > 1) G_count++;
		
		if (iter->op_type == 0 && iter->Type != Zombie) {
			for(i = 0; i < iter->vFanins.size(); i++){
				if(iter->vFanins[i] > 1) var_count++;
			}
			if(var_count > 1) G_count++;
		} else if (iter->op_type == 1 && iter->Type != Zombie) {
			for(i = 0; i < iter->vFanins.size(); i++){
				if(iter->vFanins[i] > 1) var_count++;
			}
			if(var_count > 0) G_count++;
		}
	}

	// for(vector<Xmg_Obj_t>::iterator iter = ntk->vPos.begin(); iter != ntk->vPos.end(); iter++){
	// 	var_count = 0;
	// 	for(i = 0; i < iter->vFanins.size(); i++){
	// 		if(iter->vFanins[i] > 1) var_count++;
	// 	}
	// 	if(var_count > 1) G_count++;
	// }
	return G_count;
}

