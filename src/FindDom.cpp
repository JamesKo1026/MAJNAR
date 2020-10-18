#include "FindDom.h"
#define CHECK_DOM 0
#define CHECK_FANOUTCONE 0

st_table *fanout_cone;
st_table *fanin_cone;

//***********************************************************
tval_t inv(tval_t t)
{
	if(t==_0_) return _1_;
	if(t==_1_) return _0_;
	if (t==_x_){
		cout << "inverse error for a _x_ in FindDom.cpp" << endl;
		//getchar();
	} 
	return _1_;
}

//***********************************************************
tval_t value_1(Xmg_Obj_t *t_node)
{
	return _1_;
}

//***********************************************************
tval_t value_0(Xmg_Obj_t *t_node)
{
	return _0_;
}

//***********************************************************
tval_t node_value(Xmg_Ntk_t *ntk, Xmg_Obj_t *n, Xmg_Obj_t *r, int index, tval_t tvalue) // n->r
{
//	unsigned i;
//	Xmg_Obj_t *fi;
/*	if(index>=0){		
		for(i=0; i<r->vFanins.size(); i++){
			fi = ntk->Obj_Ptr[r->vFanins[i]];
			if(fi==n) break;
		}
		if(i>=3) return _1_;
	}else{
		for(i=0; i<r->vFanins.size(); i++){
			fi = ntk->Obj_Ptr[r->vFanins[i]];
			if(fi==n){
				index = i;
				break;
			}
		}
	}	*/
/*	if(index<0){
		for(i=0; i<r->vFanins.size(); i++){
			fi = ntk->Obj_Ptr[r->vFanins[i]];
			if(fi==n){
				index = i;
				break;
			}
		}
	}*/
		
	switch(index){
		case 0:
			if(r->fCompl0 == 0) return tvalue;
			else return inv(tvalue);
			break;
		case 1:
			if(r->fCompl1 == 0) return tvalue;
			else return inv(tvalue);
			break;
		case 2:
			if(r->fCompl2 == 0) return tvalue;
			else return inv(tvalue);
			break;
		default:
			break;
	}
	return _1_;
}

//***********************************************************
void copy_value_infotable(vector<node_info> &nodeInfoTable_tmp)
{
	int i;
	for(i = 0; i < size; i++){
		nodeInfoTable_tmp[i].value = nodeInfoTable[i].value;
	}
}

//***********************************************************
void load_value_infotable(vector<node_info> &nodeInfoTable_tmp)
{
	int i;
	for(i = 0; i < size; i++){
		nodeInfoTable[i].value = nodeInfoTable_tmp[i].value;
	}
}

//***********************************************************
void refresh_value_infotable(vector<node_info> &nodeInfoTable_tmp)
{
	unsigned i;
	for(i = 0; i < nodeInfoTable_tmp.size(); i++){
		if(i == 0)
			nodeInfoTable_tmp[i].value = _0_;
		else if(i == 1)
			nodeInfoTable_tmp[i].value = _1_;
		else
			nodeInfoTable_tmp[i].value = _x_;
	}
}

//***********************************************************
void refresh_value_nodeInfotable()
{
	int i;
	for(i = 0; i < size; i++){
		if(i == 0)
			nodeInfoTable[i].value = _0_;
		else if(i == 1)
			nodeInfoTable[i].value = _1_;
		else
			nodeInfoTable[i].value = _x_;
	}
}

//***********************************************************
void RefreshNodeValue(st_table* st_set)
{
	st_generator *gen;
	Xmg_Obj_t *node;
	char *aValue;
	node_info *ir;
	// cout << "!!!" << endl;
	st_foreach_item(st_set, gen, (char **)&node, &aValue){
		if (node->Id == 0)
			nodeInfoTable[node->Id].value = _0_;
		else if (node->Id == 1)
			nodeInfoTable[node->Id].value = _1_;
		else
			nodeInfoTable[node->Id].value = _x_;
	}
	// cout << "!!!2" << endl;
}

//***********************************************************
void LoadNodeValue(st_table *st_set)
{
	st_generator *stgen;
	Xmg_Obj_t *node;
	char *aValue;
	
	st_foreach_item(st_set, stgen, (char **)&node, &aValue){
		nodeInfoTable[node->Id].value = (tval_t)(long)aValue;
	}
}

//***********************************************************
void load_map_to_nodeInfotable(st_table *st)
{
	st_generator *stgen;
	Xmg_Obj_t *node;
	char *aValue;
	
	for(int i = 0; i < size; i++){
		if(i == 0)
			nodeInfoTable[i].value = _0_;
		else if(i == 1)
			nodeInfoTable[i].value = _1_;
		else
			nodeInfoTable[i].value = _x_;
	}
	
	st_foreach_item(st, stgen, (char **)&node, &aValue){
		nodeInfoTable[node->Id].value = (tval_t)(long)aValue;
	}
}

//***********************************************************
void BackupImplied(st_table *implied_set)
{
	st_generator *stgen;
	node_info *ir;
	char *aValue;
	Xmg_Obj_t *node;
	
	st_foreach_item(ValuingNode, stgen, (char **)&node, &aValue){
		ir = &nodeInfoTable[node->Id];
	/*	if (ir->value==_x_){
			cout << "Exception in BackupImplied in NodeMGR.c " << ID_Name[node->Id] << " should not has _x_" << endl;
			continue;
		}*/
		st_insert(implied_set, (char *)node, aValue);
	}
}

//***********************************************************
void st_intersection(st_table *f_st, st_table *s_st)
{
	st_generator *stgen;
	Xmg_Obj_t *node;
	char *v1, *v2;
	
	st_foreach_item(f_st, stgen, (char **)&node, &v1){
		if(st_lookup(s_st, (char *)node, &v2)){
			if(v1 != v2){
				st_delete(f_st, (char **)&node, NIL(char *));
			}
		}else{
			st_delete(f_st, (char **)&node, NIL(char *));
		}
	}
}

//***********************************************************
vector<int> vector_intersection(vector<int> vec1, vector<int> vec2)
{
	vector<int> vec_intersection;
	sort(vec2.begin(), vec2.end());	// bacause we use "push_back", so now need to sort (by node Id)
	for(vector<int>::iterator iter1 = vec1.begin(); iter1 != vec1.end(); iter1++){
		for(vector<int>::iterator iter2 = vec2.begin(); iter2 != vec2.end(); iter2++){
			if(*iter1 == *iter2){
				vec_intersection.push_back(*iter1);
				break;
			}
		}
	}
	return vec_intersection;
}

//***********************************************************
int is_assign(Xmg_Obj_t *t_node)
{
	// if this node has already been assigned, return 1
	if(!EachNodeDom[t_node->Id].empty())
		return 1;
	
	return 0;
}

//***********************************************************
void recur_search_dom(Xmg_Obj_t *t_node)
{
	vector<int> dom_old;
	vector<int> dom_new;
	Xmg_Obj_t *fanout;
	int first;
	unsigned i;

	if(t_node->Type == PO) return;	// if this node is PO, return
	
	if(t_node->vFanouts.size() == 0) return;	// if there isn't any fanouts in this node, return
	
	if(is_assign(t_node)) return;	// if this node has been assigned, return
	
	first = 0;
	for(i = 0; i < t_node->vFanouts.size(); i++){
		fanout = Gntk->Obj_Ptr[t_node->vFanouts[i]];
		
		recur_search_dom(fanout);

		// "first" is used to handle the situation that the node have multiple fanouts, 
		// if the first one fanout of this node has been assigned, first will be 1,
		// and consider the node intersection of this fanout and the other fanouts (common path for all fannouts)  
		if(first == 0){
			first = 1;
			if(is_assign(fanout)){
				dom_old = EachNodeDom[fanout->Id];
			}else{
				dom_old.clear();
			}
		}else{
			dom_new = vector_intersection(EachNodeDom[fanout->Id], dom_old);
			dom_old.clear();
			dom_old = dom_new;
		}
	}
	
	dom_old.push_back(t_node->Id);
	EachNodeDom[t_node->Id] = dom_old;
}

//***********************************************************
vector<int> FindDom(Xmg_Obj_t *t_node)
{
	// if this node has already been assigned, return the dominators corresponding to its Id
	if(!EachNodeDom[t_node->Id].empty()){
		return EachNodeDom[t_node->Id];
	}else{
		recur_search_dom(t_node);
		return EachNodeDom[t_node->Id];
	}
}

//***********************************************************
void FanoutConeDfs(Xmg_Obj_t *node)
{
	Xmg_Obj_t *fo;
	unsigned j;
	
	st_insert(fanout_cone, (char *)node, (char *)1);
	
	for(j = 0; j < node->vFanouts.size(); j++){
		fo = Gntk->Obj_Ptr[node->vFanouts[j]];
		
		if(!st_is_member(fanout_cone, (char *)fo))
			FanoutConeDfs(fo);		
	}
}

//***********************************************************
st_table *FindFanoutConeDfs(Xmg_Obj_t *node)
{
	Xmg_Obj_t *fo;
	unsigned j;

	fanout_cone = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	st_insert(fanout_cone, (char *)node ,(char *)1);	
	
	for(j = 0; j < node->vFanouts.size(); j++){
		fo = Gntk->Obj_Ptr[node->vFanouts[j]];
		if(!st_is_member(fanout_cone, (char *)fo))
			FanoutConeDfs(fo);
	}
	
	return fanout_cone;	
}

//***********************************************************
void FaninConeDfs(Xmg_Obj_t *node)
{
	Xmg_Obj_t *fi;
	unsigned j;
	
	st_insert(fanin_cone, (char *)node, (char *)1);
	
	for(j = 0; j < node->vFanins.size(); j++){
		fi = Gntk->Obj_Ptr[node->vFanins[j]];
		
		if(!st_is_member(fanin_cone, (char *)fi))
			FaninConeDfs(fi);		
	}
}

//***********************************************************
st_table *FindFaninConeDfs(Xmg_Obj_t *node)
{
	Xmg_Obj_t *fi;
	unsigned j;

	fanin_cone = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	st_insert(fanin_cone, (char *)node ,(char *)1);	
	
	for(j = 0; j < node->vFanins.size(); j++){
		fi = Gntk->Obj_Ptr[node->vFanins[j]];
		if(!st_is_member(fanin_cone, (char *)fi))
			FaninConeDfs(fi);
	}
	
	return fanin_cone;	
}

//***********************************************************
void GetSideInputPair(Xmg_Obj_t *ns, st_table *si_set, vector<SideInput_Pair_t> &sip_set)
{
	vector<int> ary_d;
	
	sip_set.clear();
	// find dominators of node
	ary_d = FindDom(ns); // ary_d includes ns

#if CHECK_DOM
	// Check domators
	cout << "Node is: " << id_name[ns->Id] << ", and the dominators are: " << endl;
	for (unsigned k = 0; k < ary_d.size(); k++) {
		cout << id_name[ary_d[k]] << " ";
	}
	cout << endl;
#endif

	st_table *focone_st;
	// find fanout cone of node
	focone_st = FindFanoutConeDfs(ns);
	
#if CHECK_FANOUTCONE
	st_generator* stgen;
	char* value;
	Xmg_Obj_t* node;
	// Check fanout cone
	cout << "Node is: " << id_name[ns->Id] << ", and the fanout cone are: " << endl;
	st_foreach_item(focone_st, stgen, (char **)&node, &value) {
		cout << id_name[node->Id] << " ";
	}
	cout << endl;
#endif

	unsigned const_fCompl = 0;
	int const_id = 0;
	int const_num = 0;
	Xmg_Obj_t *fi;
	Xmg_Obj_t *nd;
	int si_count = 0;
	SideInput_Pair_t sip;

	// cout << "Node is: " << ns->Id << ", and the op_type is: ";
	// if (ns->op_type == 0)
	// 	cout << "MAJ" << endl;
	// else if  (ns->op_type == 1)
	// 	cout << "XOR" << endl;
	//cout << "Starting to go though the dominators ..." << endl;

	for(int i = ary_d.size()-2; i >=0; i--){
		nd = Gntk->Obj_Ptr[ary_d[i]];
		si_count = 0;
		const_num = 0;


		// if (nd->op_type == 1) {
		// 	for(unsigned j = 0; j < nd->vFanins.size(); j++){
		// 		fi = Gntk->Obj_Ptr[nd->vFanins[j]];
		// 		if(!st_is_member(focone_st, (char *)fi)){
		// 			if(si_count == 0){
		// 				sip.pnode1 = fi->Id;
		// 				sip.parent = nd->Id;
		// 				sip.index1 = j;
		// 				sip.pnode2 = -1;
		// 				sip.index2 = -1;
		// 				sip_set.push_back(sip);
		// 			}
		// 			si_count++;
		// 		}
		// 		if(si_count >= 1) break;
		// 	}
		// 	if(si_count == 1){
		// 		Xmg_Obj_t *si_node = Gntk->Obj_Ptr[sip.pnode1];
		// 		unsigned si_fCompl;
		// 		tval_t noncontrolling_value;

		// 		if(sip.index1 == 0) si_fCompl = nd->fCompl0;
		// 		else if(sip.index1 == 1) si_fCompl = nd->fCompl1;

		// 		if(si_fCompl == 0) noncontrolling_value = _0_;
		// 		else if(si_fCompl == 1) noncontrolling_value = _1_;

		// 		st_insert(si_set, (char *)si_node, (char *)noncontrolling_value);

		// 		// cout << "sip.index1: " << sip.index1 << ", nd->fCompl0: " << nd->fCompl0 << ", nd->fCompl1: " << nd->fCompl1 
		// 		// 	 << ", si_fcoml: " << si_fCompl << endl;

		// 		// if (nd->op_type != 1)
		// 		// 	cout << "[Unexpected Error] The op_type of this node is wrong!!!" << endl; 

		// 		// cout << "[XOR] nd: " << nd->Id << ", and the sideinput of it are: " << si_node->Id << endl;
		// 		// cout << "Noncontrolling_value is: ";
		// 		// if (noncontrolling_value == _0_)
		// 		// 	cout << "0" << endl;
		// 		// else if (noncontrolling_value == _1_)
		// 		// 	cout << "1" << endl;
		// 	}
		// } else {
			for(unsigned j = 0; j < nd->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[nd->vFanins[j]];
				if(!st_is_member(focone_st, (char *)fi)){
					if(fi->Type == CONST){
						if(j == 0) const_fCompl = nd->fCompl0;
						else if(j == 1) const_fCompl = nd->fCompl1;
						else if(j == 2) const_fCompl = nd->fCompl2;
						const_id = fi->Id;
						const_num++;
						continue;
					}
					if(si_count == 0){
						sip.pnode1 = fi->Id;
						sip.index1 = j;
					}else if(si_count == 1){
						sip.pnode2 = fi->Id;
						sip.parent = nd->Id;
						sip.index2 = j;
						sip_set.push_back(sip);
					}
					si_count++;
				}
				if(si_count >= 2) break;
			}
			if(const_num == 1 && si_count == 1){
				Xmg_Obj_t *si_node = Gntk->Obj_Ptr[sip.pnode1];
				unsigned si_fCompl = 0;
				tval_t noncontrolling_value = _x_;

				if(sip.index1 == 0) si_fCompl = nd->fCompl0;
				else if(sip.index1 == 1) si_fCompl = nd->fCompl1;
				else if(sip.index1 == 2) si_fCompl = nd->fCompl2;

				if((const_id == 0 && const_fCompl == 0) || (const_id == 1 && const_fCompl == 1)){ // pure 0 -> noncontrolling value = 1
					if(si_fCompl == 0) noncontrolling_value = _1_;
					else if(si_fCompl == 1) noncontrolling_value = _0_;			
				}
				else if((const_id == 1 && const_fCompl == 0) || (const_id == 0 && const_fCompl == 1)){ // pure 1 -> noncontrolling value = 0
					if(si_fCompl == 0) noncontrolling_value = _0_;
					else if(si_fCompl == 1) noncontrolling_value = _1_;			
				}
				st_insert(si_set, (char *)si_node, (char *)noncontrolling_value);
				
				// if (nd->op_type != 0)
				// 	cout << "[Unexpected Error] The op_type of this node is wrong!!!" << endl; 

				// cout << "[MAJ] nd: " << id_name[nd->Id] << ", and the sideinput of it are: " << id_name[si_node->Id] << endl;
				// cout << "Noncontrolling_value is: ";
				// if (noncontrolling_value == _0_)
				// 	cout << "0" << endl;
				// else if (noncontrolling_value == _1_)
				// 	cout << "1" << endl;
			}
		// }
		
	}
	st_free_table(&focone_st);
	ary_d.clear();
	// cout << "\n\n";
}

//***********************************************************
void GetSideInputCouples(Xmg_Obj_t *ns, Xmg_Obj_t *nt, st_table *si_set, vector<SideInput_Pair_t> &sip_set)
{
	vector<int> ary_d;

	sip_set.clear();
	
	// find dominators of node
	ary_d = FindDom(nt); // ary_d includes nt
	
	st_table *focone_st;
	// find fanout cone of node
	focone_st = FindFanoutConeDfs(ns);
	
	unsigned const_fCompl = 0;
	int const_id = 0;
	int const_num = 0;
	Xmg_Obj_t *fi;
	Xmg_Obj_t *nd;
	int si_count = 0;
	SideInput_Pair_t sip;
	
	for(int i = ary_d.size()-1; i >= 0; i--){
		nd = Gntk->Obj_Ptr[ary_d[i]];
		si_count = 0;
		const_num = 0;

		// if (nd->op_type == 1) {
		// 	for(unsigned j = 0; j < nd->vFanins.size(); j++){
		// 		fi = Gntk->Obj_Ptr[nd->vFanins[j]];
		// 		if(!st_is_member(focone_st, (char *)fi)){
		// 			if(si_count == 0){
		// 				sip.pnode1 = fi->Id;
		// 				sip.parent = nd->Id;
		// 				sip.index1 = j;
		// 				sip.pnode2 = -1;
		// 				sip.index2 = -1;
		// 				sip_set.push_back(sip);
		// 			}
		// 			si_count++;
		// 		}
		// 		if(si_count >= 1) break;
		// 	}
		// 	if(si_count == 1){
		// 		Xmg_Obj_t *si_node = Gntk->Obj_Ptr[sip.pnode1];
		// 		unsigned si_fCompl;
		// 		tval_t noncontrolling_value;

		// 		if(sip.index1 == 0) si_fCompl = nd->fCompl0;
		// 		else if(sip.index1 == 1) si_fCompl = nd->fCompl1;

		// 		if(si_fCompl == 0) noncontrolling_value = _0_;
		// 		else if(si_fCompl == 1) noncontrolling_value = _1_;

		// 		st_insert(si_set, (char *)si_node, (char *)noncontrolling_value);

		// 		// cout << "sip.index1: " << sip.index1 << ", nd->fCompl0: " << nd->fCompl0 << ", nd->fCompl1: " << nd->fCompl1 
		// 		// 	 << ", si_fcoml: " << si_fCompl << endl;

		// 		// if (nd->op_type != 1)
		// 		// 	cout << "The op_type of this node is wrong!!!" << endl; 

		// 		// cout << "[XOR] nd's Id: " << nd->Id << ", and the sideinput of it are: " << si_node->Id << endl;
		// 		// cout << "Noncontrolling_value is: ";
		// 		// if (noncontrolling_value == _0_)
		// 		// 	cout << "0" << endl;
		// 		// else if (noncontrolling_value == _1_)
		// 		// 	cout << "1" << endl;
		// 	}
		// } else {
			for(unsigned j = 0; j < nd->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[nd->vFanins[j]];
				if(!st_is_member(focone_st, (char *)fi)){
					if(fi->Type == CONST){
						if(j == 0) const_fCompl = nd->fCompl0;
						else if(j == 1) const_fCompl = nd->fCompl1;
						else if(j == 2) const_fCompl = nd->fCompl2;
						const_id = fi->Id;
						const_num++;
						continue;
					}
					if(si_count == 0){
						sip.pnode1 = fi->Id;
						sip.index1 = j;
					}else if(si_count == 1){
						sip.pnode2 = fi->Id;
						sip.parent = nd->Id;
						sip.index2 = j;
						sip_set.push_back(sip);
					}
					si_count++;
				}
				if(si_count >= 2) break;
			}
			if(const_num == 1 && si_count == 1){
				Xmg_Obj_t *si_node = Gntk->Obj_Ptr[sip.pnode1];
				unsigned si_fCompl = 0;
				tval_t noncontrolling_value = _x_;

				if(sip.index1 == 0) si_fCompl = nd->fCompl0;
				else if(sip.index1 == 1) si_fCompl = nd->fCompl1;
				else if(sip.index1 == 2) si_fCompl = nd->fCompl2;

				if((const_id == 0 && const_fCompl == 0) || (const_id == 1 && const_fCompl == 1)){ // pure 0 -> noncontrolling value = 1
					if(si_fCompl == 0) noncontrolling_value = _1_;
					else if(si_fCompl == 1) noncontrolling_value = _0_;			
				}
				else if((const_id == 1 && const_fCompl == 0) || (const_id == 0 && const_fCompl == 1)){ // pure 1 -> noncontrolling value = 0
					if(si_fCompl == 0) noncontrolling_value = _0_;
					else if(si_fCompl == 1) noncontrolling_value = _1_;			
				}
				st_insert(si_set, (char *)si_node, (char *)noncontrolling_value);
				
				// if (nd->op_type != 0)
				// 	cout << "The op_type of this node is wrong!!!" << endl; 

				// cout << "[MAJ] nd's Id: " << nd->Id << ", and the sideinput of it are: " << si_node->Id << endl;
				// cout << "Noncontrolling_value is: ";
				// if (noncontrolling_value == _0_)
				// 	cout << "0" << endl;
				// else if (noncontrolling_value == _1_)
				// 	cout << "1" << endl;
			}
		// } 
		
	}
	st_free_table(&focone_st);
	ary_d.clear();	
}








