#include "NodeMGR.h"
#define MAREUSE 1
#define CHECK_REUSE 0
#define SIMPLIFY 1
#define NODEAR 1
#define REPLACABLE 0
#define MAX_FANOUTS 6

void UpdateJTable(st_table *JTableTmp)
{
	st_generator *stgen;
	char *aValue;
	Xmg_Obj_t *node;

	st_foreach_item(JTable, stgen, (char **)&node, &aValue){
		if (!st_is_member(JTableTmp, (char *)node)){
			st_delete(JTable, (char **)&node, NIL(char *));
		}
	}
}
//***********************************************************
bool SubstituteSearch(Xmg_Obj_t *n, st_table *one_set, st_table *zero_set, vector<SideInput_Pair_t> st_sideinputpair)
{
	st_generator *stgen;
	Xmg_Obj_t *node;
	Xmg_Obj_t *pnode1, *pnode2;
	Xmg_Obj_t *best_node;
	int found = 0;
	char *v1, *v2;
	bool phase;
	bool best_phase;
	unsigned ncount = 0;
	st_table *fanout_cone = NULL;
	st_table *fanin_cone = NULL;
	st_table *st_set = NULL;
	
	Xmg_Obj_t *fo;
	Xmg_Obj_t *fi;
	
	fanout_cone = FindFanoutConeDfs(n);

	

	st_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);

	
	for(unsigned i = 0; i < n->vFanouts.size(); i++){
		fo = Gntk->Obj_Ptr[n->vFanouts[i]];
		for(unsigned j = 0; j < fo->vFanins.size(); j++){
			fi = Gntk->Obj_Ptr[fo->vFanins[j]];
			if(fi != n) st_insert(st_set, (char *)fi, (char *)1);
		}
	}


	for(vector<SideInput_Pair_t>::iterator iter = st_sideinputpair.begin(); iter != st_sideinputpair.end(); iter++){
		if (iter->index2 != -1) {	// MAJ
			pnode1 = Gntk->Obj_Ptr[iter->pnode1];
			pnode2 = Gntk->Obj_Ptr[iter->pnode2];
			st_insert(st_set, (char *)pnode1, (char *)1);
			st_insert(st_set, (char *)pnode2, (char *)1);
		} else if (iter->index2 == -1) {	// XOR
			pnode1 = Gntk->Obj_Ptr[iter->pnode1];
			st_insert(st_set, (char *)pnode1, (char *)1);
		}
		
	}
	
	st_foreach_item(one_set, stgen, (char **)&node, &v1){
		if(node == n) continue;
		if(node->Type == PO) continue;
		if(st_is_member(st_set, (char *)node)) continue;
		if(st_is_member(fanout_cone, (char *)node)) continue;
		//if(st_is_member(fanin_cone, (char *)node)) continue;
		if (st_lookup(zero_set, (char *)node, &v2)){
			if(((int)(long)v1 != (int)(long)v2) && (node->Level <= n->Level)){
				// && (node->Level <= n->Level)

				
				found = 1;
				substitute_count++;

				// GREEDYSIMPLIFY
				phase = 0;
				if((tval_t)(long)v1 != _1_) phase = 1;
				if(ncount == 0){
					best_node = node;
					best_phase = phase;
					ncount = node->Level;
				}else{
					if(node->Level < ncount){
						best_node = node;
						best_phase = phase;
						ncount = node->Level;
					}
				}
				
			}
			//cout << "node Id: " << node->Id << ", node->Level: " << node->Level << ", ncount: " << ncount << endl; 
		}
	}
	// cout << endl;
	if(found == 1){
		replaced_count++;
		node_pair tmp;
		tmp.node1 = n->Id;
		tmp.node2 = best_node->Id;
		replaced_table.push_back(tmp);
	//	cout << id_name[tmp.node1] << " " << id_name[tmp.node2] << endl;
		n->Simp = 1;
		best_node->Simp = 1;
#if SIMPLIFY
		merge_simplify(n, best_node, best_phase);
#endif
	}
	st_free_table(&fanout_cone);
	st_free_table(&st_set);	

	return found;
}

//***********************************************************
int ComputeOBSTest(Xmg_Obj_t *node)
{
	Xmg_Obj_t *fo, *fi;
	unsigned i;
	int index = 0;
	node_info *ir;
	int zero_count = 0;
	int one_count = 0;
	unsigned fCompl = 0;
	
	if (node->vFanouts.size() > 1) return 0;	// only focus on the node which is single-fanout fanin node

	fo = Gntk->Obj_Ptr[node->vFanouts[0]];
	ir = &nodeInfoTable[fo->Id];
	
	for(i = 0; i < fo->vFanins.size(); i++){
		fi = Gntk->Obj_Ptr[fo->vFanins[i]];
		if(fi->Type == CONST){
			if (i == 0) fCompl = fo->fCompl0;
			else if (i == 1) fCompl = fo->fCompl1;
			else if (i == 2) fCompl = fo->fCompl2;
			
			if (fi->Id == 0 && fCompl == 0) zero_count++;
			else if (fi->Id == 0 && fCompl == 1) one_count++;
			else if (fi->Id == 1 && fCompl == 0) one_count++;
			else if (fi->Id == 1 && fCompl == 1) zero_count++;
		}
		if(fi == node) index = i;
	}
	
	if (node->op_type == 0) {
		if (zero_count == 1 && one_count == 1) return 0;
		if (zero_count == 1){
			if (ir->OBS1 == NULL) return 0;
			switch(index){
				case 0:
					if (fo->fCompl0 == 0) return 1;
					else return 2;
					break;
				case 1:
					if (fo->fCompl1 == 0) return 1;
					else return 2;
					break;
				case 2:
					if (fo->fCompl2 == 0) return 1;
					else return 2;
					break;
				default:
					break;
			}
		}
		if (one_count == 1){
			if (ir->OBS0 == NULL) return 0;
			switch(index){
				case 0:
					if (fo->fCompl0 == 0) return 3;
					else return 4;
					break;
				case 1:
					if (fo->fCompl1 == 0) return 3;
					else return 4;
					break;
				case 2:
					if (fo->fCompl2 == 0) return 3;
					else return 4;
					break;
				default:
					break;
			}
		}
	} 
	
	return 0;		
}

//***********************************************************
void NodeMGR()
{
	Xmg_Obj_t *node;
	Xmg_Obj_t *fo;
	node_info *ir;
	node_info *ir_fo;
	int i;
	unsigned bound;
	vector<SideInput_Pair_t> st_sideinputpair;
	st_table *st_sideinput = NULL;
	st_table *one_set = NULL;
	st_table *zero_set = NULL;
	st_table *implied_set = NULL;
	st_table *JTableTmp = NULL;
	bool st;
	bool multiMG = 0;
	bool found = false;
	int computeOBS1;
	int computeOBS0;
	int computeOBSFlag;
	double temp_totalcount = 0, totalcount = 0;
//	double DOM = 0;
//	double IMP0 = 0;
//	double IMP1 = 0;
//	double buf = 0;
//	double SUB = 0;

	totalcount = (double)compute_gate(Gntk);
	// fannout constraint for the number of n = 3
	cout << "Fanout restriction: " << MAX_FANOUTS << endl;

	for(int round = 0; round < 2; round++){
		for(i = 0; i < size; i++) validTable[i] = 1;
		DFS(Gntk);

		reverse(nList.begin(), nList.end());

		cout << "DFS done" << endl;

		
		for(global_index = 0; global_index < nList.size(); global_index++){
			node = Gntk->Obj_Ptr[nList[global_index]];
			ir = &nodeInfoTable[node->Id];

			// fannout constraint for the number of n = 3
			if (node->vFanouts.size() > MAX_FANOUTS)	continue; 

			int constant_cnt = 0;
#if REPLACABLE
#else
			if (round == 1) {
#endif
				for (unsigned ii = 0; ii < node->vFanins.size(); ii++) {
					if (node->vFanins[ii] <= 1)
						constant_cnt++;
				}
				// cout << "constant_cnt: " << constant_cnt << ", node->op_type: " << node->op_type << endl;
				if (node->op_type == 1 && constant_cnt > 0)	continue;
				else if (node->op_type == 0 && constant_cnt > 1)	continue;
#if REPLACABLE
#else
			}
#endif
			
			if (node->Type != NODE)	continue;
			if (node->vFanouts.size() == 0)	continue;
			if (validTable[node->Id] == 0)	continue;
			else validTable[node->Id] = 0;
			
			// initialize
			st_sideinputpair.clear();
			st_sideinput = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			multiMG = 0;
#if MAREUSE			
			if (node->op_type == 1) {
				computeOBSFlag = 0;
				computeOBS1 = 1; computeOBS0 = 1;
			} else {
				computeOBSFlag = ComputeOBSTest(node);
				switch(computeOBSFlag){
					case 0: computeOBS1=1; computeOBS0=1; break;
					case 1: computeOBS1=0; computeOBS0=1; break;
					case 2: computeOBS1=1; computeOBS0=0; break;
					case 3: computeOBS1=1; computeOBS0=0; break;
					case 4: computeOBS1=0; computeOBS0=1; break;
					default: computeOBS1=1; computeOBS0=1; break;
				}
			}
#else
			computeOBSFlag = 0;
			computeOBS1 = 1; computeOBS0 = 1;
#endif	
			

#if CHECK_REUSE			
			Check which reuse strategy this mode can use
			cout << "node Id: " << node->Id << ", and the size of it's fanins are: " << node->vFanins.size();
			if (node->op_type == 0)
				cout << ", op_type is: MAJ" << endl;
			else
				cout << ", op_type is: XOR" << endl;
			cout << "Reuse strategy is: " << computeOBSFlag << endl;
#endif

			bound = 1;
			if(node->vFanouts.size() > bound){
			//	continue; // test
				multiMG = 1;
			//	cout << "-----------------------------" << endl;
			//	cout << "multiple fanout, nt: " << id_name[node->Id] << endl;
			//	continue;
			}else{
				multiMG = 0;
			//	buf = getTime();
				// cout << "THIS node is: " << id_name[node->Id] << endl;
				GetSideInputPair(node, st_sideinput, st_sideinputpair);
			//	DOM += (getTime()-buf);
				
			}
			
			ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			refresh_value_nodeInfotable();
			
			st = SideInputImply(st_sideinput);
			st_free_table(&st_sideinput);
			
			if(st==0){
				st_free_table(&ValuingNode);
				st_free_table(&JTable);
				st_free_table(&implied_set);
				st_free_table(&JTableTmp);
				st_free_table(&one_set);
				st_free_table(&zero_set);
				refresh_value_nodeInfotable();
				if(round == 1){
					// cout << "node is: " << id_name[node->Id] << ", is removed!!!" << endl;
					Removal(node, 0);
					remove_table.push_back(node->Id);
					remove_count++;
				}
				continue;	
			}
				
			// ValuingNode and JTable have something after sideinput implication
			implied_set = st_copy(ValuingNode);
			JTableTmp = st_copy(JTable);
			load_map_to_nodeInfotable(ValuingNode);
			
			// activate one and implication
			// cout << "-------------ActivateImply 1 at " << id_name[node->Id] << ", and op_type is:" << node->op_type << endl;
			//buf = getTime();
			if(computeOBS1 == 1){
				if(multiMG == 0) st = ActivateImply(node, _1_, st_sideinputpair);
				else st = AcitvateImplyFanoutCone(node, _1_);
				if(st == 0){
					st_free_table(&ValuingNode);
					st_free_table(&JTable);
					st_free_table(&implied_set);
					st_free_table(&JTableTmp);
					st_free_table(&one_set);
					st_free_table(&zero_set);
					refresh_value_nodeInfotable();
					if(round == 1){
						// cout << "node is: " << id_name[node->Id] << ", is removed!!! ... 1" << endl;
						Removal(node, 0);
						remove_table.push_back(node->Id);
						remove_count++;
					}
					continue;
				}
				one_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
				BackupImplied(one_set);
				ir->OBS1 = st_copy(one_set);
			}else{
				fo = Gntk->Obj_Ptr[node->vFanouts[0]];
				ir_fo = &nodeInfoTable[fo->Id];

				if(computeOBSFlag == 1) one_set = st_copy(ir_fo->OBS1);
				else one_set = st_copy(ir_fo->OBS0);

				if(ir->OneFanoutFanin == 1 && computeOBSFlag == 1) ir->OBS1 = st_copy(ir_fo->OBS1);
				MAreuse++;
			}
		// //	IMP1 += (getTime()-buf);	
				
			//reset
			st_free_table(&ValuingNode);
			st_free_table(&JTable);
			ValuingNode = st_copy(implied_set);
			JTable = st_copy(JTableTmp);
			load_map_to_nodeInfotable(ValuingNode);
			
			// activate zero and implication
			// cout << "-------------ActivateImply 0 at " << id_name[node->Id] << endl;
		//	buf = getTime();
			if(computeOBS0 == 1){
				if(multiMG == 0) st = ActivateImply(node, _0_, st_sideinputpair);
				else st = AcitvateImplyFanoutCone(node, _0_);
				if(st == 0){
					st_free_table(&ValuingNode);
					st_free_table(&JTable);
					st_free_table(&implied_set);
					st_free_table(&JTableTmp);
					st_free_table(&one_set);
					st_free_table(&zero_set);
					refresh_value_nodeInfotable();
					if(round == 1){
						// cout << "node is: " << id_name[node->Id] << ", is removed!!! ... 2" << endl;
						Removal(node, 1);
						remove_table.push_back(node->Id);
						remove_count++;
					}
					continue;
				}
				zero_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
				BackupImplied(zero_set);
				ir->OBS0 = st_copy(zero_set);
			}else{
				fo = Gntk->Obj_Ptr[node->vFanouts[0]];
				ir_fo = &nodeInfoTable[fo->Id];
				if(computeOBSFlag == 2) zero_set = st_copy(ir_fo->OBS1);
				else zero_set = st_copy(ir_fo->OBS0);
				if(ir->OneFanoutFanin == 1 && computeOBSFlag == 3) ir->OBS0 = st_copy(ir_fo->OBS0);
				MAreuse++;
			}
		//	IMP0 += (getTime()-buf);	
			//reset
			st_free_table(&ValuingNode);
			st_free_table(&JTable);
			st_free_table(&implied_set);
			st_free_table(&JTableTmp);
			refresh_value_nodeInfotable();

			
		//	buf = getTime();
			found = SubstituteSearch(node, one_set, zero_set, st_sideinputpair);
		//	SUB += (getTime()-buf);
#if NODEAR
			int operation = 2;	// 0: AND, 1: OR, 2: PURE MAJ
			bool HasConstantValue = 0;
			int ConstantValue = 0;
#if REPLACABLE
#else
			if (round == 1) {
#endif
				for (unsigned m = 0; m < node->vFanins.size(); m++){
					if (node->vFanins[m] <= 1){
						HasConstantValue = 1;
						if (m == 0) {
							if (node->fCompl0 == 0 && node->vFanins[m] == 0)
								ConstantValue = 0;
							else if (node->fCompl0 == 0 && node->vFanins[m] == 1)
								ConstantValue = 1;
							else if (node->fCompl0 == 1 && node->vFanins[m] == 0)
								ConstantValue = 1;
							else if (node->fCompl0 == 1 && node->vFanins[m] == 1)
								ConstantValue = 0;
						} else if (m == 1) {
							if (node->fCompl1 == 0 && node->vFanins[m] == 0)
								ConstantValue = 0;
							else if (node->fCompl1 == 0 && node->vFanins[m] == 1)
								ConstantValue = 1;
							else if (node->fCompl1 == 1 && node->vFanins[m] == 0)
								ConstantValue = 1;
							else if (node->fCompl1 == 1 && node->vFanins[m] == 1)
								ConstantValue = 0;
						} else if (m == 2) {
							if (node->fCompl2 == 0 && node->vFanins[m] == 0)
								ConstantValue = 0;
							else if (node->fCompl2 == 0 && node->vFanins[m] == 1)
								ConstantValue = 1;
							else if (node->fCompl2 == 1 && node->vFanins[m] == 0)
								ConstantValue = 1;
							else if (node->fCompl2 == 1 && node->vFanins[m] == 1)
								ConstantValue = 0;
						}

						if (ConstantValue == 0)
							operation = 0;
						else
							operation = 1;
						// cout << "This node has constant value!" << endl;
						break;
					}	
				}
#if REPLACABLE
#else
			}
#endif

#if REPLACABLE
#else
			if (round == 1) {
#endif
				if (found == 0 && node->op_type != 1 && HasConstantValue == 0){	// only handle MAJ node in NodeAR
					// cout << "-------------Before NAR Direct0" << endl;
					NodeAR(node, one_set, zero_set, 0, st_sideinputpair, operation);
					// cout << "-------------Before NAR Direct1" << endl;
					NodeAR(node, zero_set, one_set, 1, st_sideinputpair, operation);
				}

			 
				else if (found == 0 && node->op_type != 1 && HasConstantValue == 1){	// only handle MAJ node in NodeAR (AND, OR gate)
					// ConstantValue == 0, AND gate
					if (ConstantValue == 0){
						// cout << "-------------Before NAR_AND Direct0" << endl;
						NodeAR(node, one_set, zero_set, 0, st_sideinputpair, operation);
						// cout << "-------------Before NAR_AND Direct1" << endl;
						NodeAR(node, zero_set, one_set, 1, st_sideinputpair, operation);
					} 
					// ConstantValue == 1, OR gate
					else {
						// cout << "-------------Before NAR_OR Direct0" << endl;
						NodeAR(node, zero_set, one_set, 0, st_sideinputpair, operation);
						// cout << "-------------Before NAR_OR Direct1" << endl;
						NodeAR(node, one_set, zero_set, 1, st_sideinputpair, operation);
					}
				}
#if REPLACABLE
#else
			}
#endif
#endif			
			// free
			st_free_table(&one_set);
			st_free_table(&zero_set);
			duration = getTime();
			if(duration > 3600){
				nList.clear();
				return;
			}
		}
		nList.clear();

		temp_totalcount = (double)compute_gate(Gntk);
		if (temp_totalcount >= totalcount)
			break; 
	}

}
