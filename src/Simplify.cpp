#include "Simplify.h"
#define DEBUG 0

//***********************************************************
void Mig_ObjDeleteFanin(Xmg_Obj_t *pObj, Xmg_Obj_t *pFanin)
{
	vector<int> tmp;
	for(vector<int>::iterator iter = pObj->vFanins.begin(); iter != pObj->vFanins.end(); iter++){
		if(*iter != pFanin->Id)
			tmp.push_back(*iter);
	}
	pObj->vFanins.clear();
	pObj->vFanins = tmp;
	tmp.clear();
	
	for(vector<int>::iterator iter = pFanin->vFanouts.begin(); iter != pFanin->vFanouts.end(); iter++){
		if(*iter != pObj->Id)
			tmp.push_back(*iter);
	}
	pFanin->vFanouts.clear();
	pFanin->vFanouts = tmp;
	tmp.clear();	
}

//***********************************************************
void Mig_NtkDeleteObj(Xmg_Obj_t *pObj)
{
	unsigned i;
	Xmg_Obj_t *node;
	vector<int> vNodes;

	// delete fanins
	// [this block] it seems useless, because all nodes in here don't have any fanouts
	for(vector<int>::iterator iter = pObj->vFanouts.begin(); iter != pObj->vFanouts.end(); iter++){
		vNodes.push_back(*iter);
	}
	for(i = 0; i < vNodes.size(); i++){
		node = Gntk->Obj_Ptr[vNodes[i]];
		Mig_ObjDeleteFanin(node, pObj);
	}
	vNodes.clear();
	// [this block]

	// delete fanouts
	for(vector<int>::iterator iter = pObj->vFanins.begin(); iter != pObj->vFanins.end(); iter++){
		vNodes.push_back(*iter);
	}
	for(i = 0; i < vNodes.size(); i++){
		node = Gntk->Obj_Ptr[vNodes[i]];
		Mig_ObjDeleteFanin(pObj, node);
	}
	vNodes.clear();

	pObj->Type = Zombie;
	Gntk->vNode_num--;	
}


//***********************************************************
void Mig_ObjDeleteObj_rec(Xmg_Obj_t *pObj, int fOnlyNodes)
{
	vector<int> vNodes;
	if(pObj->Type == PI || pObj->Type == CONST) return;
	if(pObj->vFanouts.size() > 0) return;
	if(pObj->vFanins.size() == 0 && pObj->vFanouts.size() == 0) {
		cout << "This strange node \"" << id_name[pObj->Id] <<"\" has no fanins and fanouts" << endl;
		return;
	}

	// delete fanins and fanouts
	// first, record the fanins of this node
	for(vector<int>::iterator iter = pObj->vFanins.begin(); iter != pObj->vFanins.end(); iter++){
		vNodes.push_back(*iter);
	}

	Mig_NtkDeleteObj(pObj);

	if(fOnlyNodes){
		for(vector<int>::iterator iter = vNodes.begin(); iter != vNodes.end(); iter++){
			pObj = Gntk->Obj_Ptr[*iter];
			if(pObj->Type == NODE && pObj->vFanouts.size() == 0)
				Mig_ObjDeleteObj_rec(pObj,fOnlyNodes);
		}
	}
	vNodes.clear();
}

//***********************************************************
int Vec_IntFind(vector<int> p, int Entry)
{
	unsigned i;
	for(i = 0; i < p.size(); i++){
		if(p[i] == Entry)
			return i;
	}
	return -1;
}

//***********************************************************
void Mig_ObjPatchFanin(Xmg_Obj_t *pObj, Xmg_Obj_t *pFaninOld, Xmg_Obj_t *pFaninNew)
{
	int iFanin;
//	cout << "Mig_ObjPatchFanin " << ID_Name[pObj->Id] << " " << ID_Name[pFaninOld->Id] << " " << ID_Name[pFaninNew->Id] << endl;
	if(pFaninOld == pFaninNew) return;
	iFanin = Vec_IntFind(pObj->vFanins, pFaninOld->Id);
	if(iFanin == -1) return;

	// replace the old fanin entry by the new fanin entry (removes attributes)
	pObj->vFanins[iFanin] = pFaninNew->Id;

	// update the fanout of the fanin
	vector<int> vFanouts;
	vFanouts.clear();
	for(vector<int>::iterator iter = pFaninOld->vFanouts.begin(); iter != pFaninOld->vFanouts.end(); iter++){
		if(*iter != pObj->Id)
			vFanouts.push_back(*iter);
	}
	pFaninOld->vFanouts.clear();
	pFaninOld->vFanouts = vFanouts;
	
	pFaninNew->vFanouts.push_back(pObj->Id);
}

//***********************************************************
void Mig_ObjTransferFanout(Xmg_Obj_t *pNodeFrom, Xmg_Obj_t *pNodeTo)
{
//	cout << "Mig_ObjTransferFanout " << ID_Name[pNodeFrom->Id] << " " << ID_Name[pNodeTo->Id] << endl;
	unsigned i;
	vector<int> vFanouts;
	Xmg_Obj_t *fo;
	if(pNodeFrom->Type == PO || pNodeTo->Type == PO) return;
	if(pNodeFrom == pNodeTo) return;
	if(pNodeFrom->vFanouts.size() == 0) return;

	// get the fanouts of the old node
	for(vector<int>::iterator iter = pNodeFrom->vFanouts.begin(); iter != pNodeFrom->vFanouts.end(); iter++){
		vFanouts.push_back(*iter);
	}

	// patch the fanin of each of them
	for(i = 0; i < vFanouts.size(); i++){
		fo = Gntk->Obj_Ptr[vFanouts[i]];
		Mig_ObjPatchFanin(fo, pNodeFrom, pNodeTo);
	}
	vFanouts.clear();
}

//***********************************************************
void Mig_ObjReplace(Xmg_Obj_t *pNodeOld, Xmg_Obj_t *pNodeNew)
{
	// transfer the fanouts to the old node.
	if(pNodeOld == pNodeNew) return;
	if(pNodeOld->vFanouts.size() == 0) return;
	Xmg_Obj_t *temp;
	for(vector<int>::iterator iter = pNodeOld->vFanouts.begin(); iter != pNodeOld->vFanouts.end(); iter++){
		temp = Gntk->Obj_Ptr[*iter];
		if (temp == pNodeNew)	return;
	}
	Mig_ObjTransferFanout(pNodeOld, pNodeNew);
	// remove the old node
	Mig_ObjDeleteObj_rec(pNodeOld, 1);
}

//***********************************************************
int maj_xor_judge(Xmg_Obj_t *node)
{
	for(vector<int>::iterator iter = node->vFanins.begin(); iter != node->vFanins.end(); iter++){
		if(*iter <= 1) return 0;	// if the input is constant, this node isn't a MAJ or XOR node
	}
	if (node->op_type == 0)
		return 1;
	else
		return 2;
}

//***********************************************************
void merge_simplify(Xmg_Obj_t *n, Xmg_Obj_t *node, bool phase)
{
	Xmg_Obj_t *fo, *fi;
	unsigned i,j;
	node_info *ir;
	
//	cout << "merge_simplify: " << ID_Name[n->Id] << " " << ID_Name[node->Id] << endl;
	// if(maj_xor_judge(n) == 1) maj_count++;
	// else if (maj_xor_judge(n) == 2) xor_count++;

	// update n's fo's OneFanoutFanin
	if(n->vFanouts.size() == 1){
		fo = Gntk->Obj_Ptr[n->vFanouts[0]];
		ir = &nodeInfoTable[fo->Id];
		ir->OneFanoutFanin = 0;
	}
	
	if(phase==0){
		Mig_ObjReplace(n, node);
	}else{
		for(i = 0; i < n->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[n->vFanouts[i]];
			for(j = 0; j < fo->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[fo->vFanins[j]];
				if(n == fi){
					// j is 0 or 1 or 2
					if(j == 0){
						if(fo->fCompl0 == 0) fo->fCompl0 = 1;
						else fo->fCompl0 = 0;
					}else if(j == 1){
						if(fo->fCompl1 == 0) fo->fCompl1 = 1;
						else fo->fCompl1 = 0;
					}else{
						if (fo->op_type == 1) {
							cout << "[Unexpected ERROR] XOR node shouldn't have the third inverter!!!!" << endl;
							return;
							getchar();
						}
						if(fo->fCompl2 == 0) fo->fCompl2 = 1;
						else fo->fCompl2 = 0;
					}
					break;
				}
			}
		}
		Mig_ObjReplace(n, node);
	}
	// refresh nList
	nList.clear();
	DFS(Gntk);
	reverse(nList.begin(), nList.end());
	// global_index=0;
	
	// refresh EachNodeDom
	for(int k = 0; k < size; k++){
		EachNodeDom[k].clear();
	}
	
	// Free each node's OBS1, OBS0
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		ir = &nodeInfoTable[po->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){
		ir = &nodeInfoTable[pn->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}
}

//***********************************************************
void Removal(Xmg_Obj_t *node, bool FAULT)
{
	Xmg_Obj_t *fo;
	Xmg_Obj_t *fi;
	unsigned i,j;
	node_info *ir;
	Xmg_Obj_t *ConstOne;
//	Xmg_Obj_t *ConstZero;
	ConstOne = Gntk->Obj_Ptr[name_id["1"]];
//	ConstZero = Gntk->Obj_Ptr[name_id["0"]];
	
	// if(maj_xor_judge(node) == 1) maj_count++;
	// else if (maj_xor_judge(node) == 2) xor_count++;
	
	// update node's fo's OneFanoutFanin
	if(node->vFanouts.size() == 1){
		fo = Gntk->Obj_Ptr[node->vFanouts[0]];
		ir = &nodeInfoTable[fo->Id];
		ir->OneFanoutFanin = 0;
	}
	
	if(FAULT == 1){
		Mig_ObjReplace(node, ConstOne);
	}else{
		for(i = 0; i < node->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[node->vFanouts[i]];
			for(j = 0; j < fo->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[fo->vFanins[j]];
				if(node == fi){
					// j is 0 or 1 or 2
					if(j == 0){
						if(fo->fCompl0 == 0) fo->fCompl0 = 1;
						else fo->fCompl0 = 0;
					}else if(j == 1){
						if(fo->fCompl1 == 0) fo->fCompl1 = 1;
						else fo->fCompl1 = 0;
					}else{
						if(fo->fCompl2 == 0) fo->fCompl2 = 1;
						else fo->fCompl2 = 0;
					}
					break;
				}
			}
		}
		Mig_ObjReplace(node, ConstOne);
	}
	// refresh nList
	nList.clear();
	DFS(Gntk);
	reverse(nList.begin(), nList.end());
	// global_index = 0;
	
	// refresh EachNodeDom
	for(int k = 0; k < size; k++){
		EachNodeDom[k].clear();
	}
	// Free each node's OBS1, OBS0
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		ir = &nodeInfoTable[po->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){
		ir = &nodeInfoTable[pn->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}
}
void SimplifyXOR(Xmg_Ntk_t *ntk)
{
	cout << "Before SimplifyXOR, size: " << ntk->vNode_num << endl;
	for(unsigned i = 0; i < ntk->vObjs.size(); i++){
		Xmg_Obj_t* n = ntk->Obj_Ptr[ntk->vObjs[i].Id];
		if (n->Type == Zombie || n->op_type == 0 || n->vFanouts.size() != 1 || n->Simp == 1 || n->Simp == 2)	continue;

		for (unsigned j = 0; j < n->vFanouts.size(); j++){
			Xmg_Obj_t* fanout_n = ntk->Obj_Ptr[n->vFanouts[j]];
			if (fanout_n->Type == Zombie || fanout_n->op_type == 0 || fanout_n->Simp == 1 || fanout_n->Simp == 2)	break;

			// cout << "n Node is: " << id_name[n->Id] << ", fanout_n Node is: " << id_name[fanout_n->Id] << endl;

			// record original fanins in node n
			vector<int> temp_fanins;
			for (unsigned k = 0; k < n->vFanins.size(); k++){
				temp_fanins.push_back(n->vFanins[k]);
			}

			if (temp_fanins.size() > 2){
				cout << "n Node is: " << id_name[n->Id] << ", fanout_n Node is: " << id_name[fanout_n->Id] << endl;
				for (unsigned ii = 0; ii < temp_fanins.size(); ii++){
					cout << id_name[temp_fanins[ii]] << " ";
				}
				cout << endl;
				for (unsigned ii = 0; ii < fanout_n->vFanins.size(); ii++){
					cout << id_name[fanout_n->vFanins[ii]] << " ";
				}
				cout << endl;
				cout << "[Unexpected error] Wrong fanin size!!!!!!" << endl;
				break;
				getchar();
				return;
			}

			// record the inverters info. in node n
			// vector<int> temp_invs;
			// for (unsigned k = 0; k < n->vFanins.size(); k++){
			// 	if (ntk->Obj_Ptr[n->vFanins[k]]->fCompl0)
			// 	temp_invs.push_back(n->vFanins[k]);
			// }
			
			// erase the index between node n and node fanout_n
			int index = 0;
			for (unsigned k = 0; k < fanout_n->vFanins.size(); k++){
				if (ntk->Obj_Ptr[fanout_n->vFanins[k]] == n) {
					index = k;
					fanout_n->vFanins.erase(fanout_n->vFanins.begin()+index);
					if (index >= 2) {
						cout << "n Node is: " << id_name[n->Id] << ", fanout_n Node is: " << id_name[fanout_n->Id] << endl;
						cout << "index: " << index << endl;
						cout << "[Unexpected error] Wrong Index!!!!!!" << endl;
						getchar();
						return;
					}
					break;
				}
			}
			//cout << "index: " << index << endl;
			vector<int> temp_fanouts;
			for (unsigned k = 0; k < temp_fanins.size(); k++){
				if (index == 0)
					fanout_n->vFanins.insert(fanout_n->vFanins.begin()+k, temp_fanins[k]);
				else
					fanout_n->vFanins.push_back(temp_fanins[k]);


				// record the other which are not be influenced fanouts of the fanins of node n
				for (unsigned kk = 0; kk < ntk->Obj_Ptr[temp_fanins[k]]->vFanouts.size(); kk++){
					Xmg_Obj_t* temp_fo = ntk->Obj_Ptr[ntk->Obj_Ptr[temp_fanins[k]]->vFanouts[kk]];
					if (temp_fo != n)
						temp_fanouts.push_back(temp_fo->Id);
				}

				// update each fanouts of the fanins of node n
				ntk->Obj_Ptr[temp_fanins[k]]->vFanouts = temp_fanouts;
				ntk->Obj_Ptr[temp_fanins[k]]->vFanouts.push_back(fanout_n->Id);
				temp_fanouts.clear();
			}
			

			if (index == 0) {
				if (fanout_n->fCompl1 == 1) {
					fanout_n->fCompl2 = 1;
				}
				fanout_n->fCompl0 = n->fCompl0;
				fanout_n->fCompl1 = n->fCompl1;
			} else {
				if (fanout_n->fCompl0 == 1) {
					fanout_n->fCompl0 = 1;
				}
				fanout_n->fCompl1 = n->fCompl0;
				fanout_n->fCompl2 = n->fCompl1;
			}
			// for (unsigned k = 0; k < fanout_n->vFanins.size(); k++) {
			// 	if (k == 0) {
			// 		cout << "1st Fanin is: " << id_name[fanout_n->vFanins[k]] << endl;
			// 		cout << "fCompl0: " << fanout_n->fCompl0 << endl;
			// 	} else if (k == 1) {
			// 		cout << "2nd Fanin is: " << id_name[fanout_n->vFanins[k]] << endl;
			// 		cout << "fCompl1: " << fanout_n->fCompl1 << endl;
			// 	} else {
			// 		cout << "3th Fanin is: " << id_name[fanout_n->vFanins[k]] << endl;
			// 		cout << "fCompl2: " << fanout_n->fCompl2 << endl;
			// 	}
			// }

			n->vFanins.clear();
			n->vFanouts.clear();
			n->Type = Zombie;
			ntk->vNode_num--;
			fanout_n->Simp = 2;
		}
	}
	cout << "After SimplifyXOR, size: " << ntk->vNode_num << endl;
}

//***********************************************************
void NodeARSimplify(Xmg_Obj_t* node, Xmg_Obj_t* A, bool Afault, Xmg_Obj_t* B, bool Bfault, Xmg_Obj_t* C, bool Cfault, bool direct)
{
	Xmg_Obj_t *fi0, *fi1, *fi2, *fo, *fi;
	unsigned i, j;
	int count;
	node_info *ir;

	int constant_index = 3;
	// ***** << Detect the index of the constant value >> *****
	for (i = 0; i < node->vFanins.size(); i++){
		if (node->vFanins[i] <= 1){
			constant_index = i;
			cout << "[ERROR] Constant value doesn't allowed!!" << endl;
			getchar();
			return;
		}
	}

#if DEBUG
	// ***** << List the basic infomations of target node, node A, B, C, and their fault respectively >> *****
	cout << "======Basic Info. for Nt, Nf1, Nf2, Nf3=======" << endl;
	cout << "Target node is: " << id_name[node->Id] << endl;
	cout << "Node Nf1 is: " << id_name[A->Id] << ", and fault is: " << Afault << endl;
	cout << "Node Nf2 is: " << id_name[B->Id] << ", and fault is: " << Bfault << endl;
	cout << "Node Nf3 is: " << id_name[C->Id] << ", and fault is: " << Cfault << endl;
	cout << "Direct is: " << direct << endl;
	cout << "==============================================" << endl;
#endif
	count = compute_gate(Gntk);

	// cout << "======Original Target Node=======" << endl;
	// cout << "Original Target Node is: " << id_name[node->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < node->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[node->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < node->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[node->vFanouts[i]]->Id] << " ";
	// 	}
	// 	cout << endl;
	// cout << "=================================" << endl;
	
	if ((Afault == 1) && (Bfault == 1) && (Cfault == 1)){
		node->fCompl0 = 0;
		node->fCompl1 = 0;
		node->fCompl2 = 0;
	} else if ((Afault == 0) && (Bfault == 1) && (Cfault == 1)){
		node->fCompl0 = 1;
		node->fCompl1 = 0;
		node->fCompl2 = 0;
	} else if ((Afault == 1) && (Bfault == 0) && (Cfault == 1)){
		node->fCompl0 = 0;
		node->fCompl1 = 1;
		node->fCompl2 = 0;
	} else if ((Afault == 0) && (Bfault == 0) && (Cfault == 1)){
		node->fCompl0 = 1;
		node->fCompl1 = 1;
		node->fCompl2 = 0;
	} else if ((Afault == 1) && (Bfault == 1) && (Cfault == 0)){
		node->fCompl0 = 0;
		node->fCompl1 = 0;
		node->fCompl2 = 1;
	} else if ((Afault == 0) && (Bfault == 1) && (Cfault == 0)){
		node->fCompl0 = 1;
		node->fCompl1 = 0;
		node->fCompl2 = 1;
	} else if ((Afault == 1) && (Bfault == 0) && (Cfault == 0)){
		node->fCompl0 = 0;
		node->fCompl1 = 1;
		node->fCompl2 = 1;
	} else if ((Afault == 0) && (Bfault == 0) && (Cfault == 0)){
		node->fCompl0 = 1;
		node->fCompl1 = 1;
		node->fCompl2 = 1;
	} 
	fi0 = Gntk->Obj_Ptr[node->vFanins[0]];

	// cout << "======Original Node fi0, A=======" << endl;
	// cout << "Original Node fi0 is: " << id_name[fi0->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi0->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi0->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi0->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi0->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Original Node A is: " << id_name[A->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < A->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[A->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < A->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[A->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "=================================" << endl;

	Mig_ObjPatchFanin(node, fi0, A);
	if (fi0->vFanouts.size() == 0 && fi0->vFanins.size() != 0 && fi0->Type != Zombie) Mig_ObjDeleteObj_rec(fi0, 1);
#if DEBUG
	cout << "PROCESSING SIMPLIFY ... fi0 -> A" << endl;
#endif
	

	fi1 = Gntk->Obj_Ptr[node->vFanins[1]];

	// cout << "======Original Node fi1, B=======" << endl;
	// cout << "Original Node fi1 is: " << id_name[fi1->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi1->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi1->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi1->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi1->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Original Node B is: " << id_name[B->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < B->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[B->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < B->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[B->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "=================================" << endl;

	Mig_ObjPatchFanin(node, fi1, B);
	if (fi1->vFanouts.size() == 0 && fi1->vFanins.size() != 0 && fi1->Type != Zombie) Mig_ObjDeleteObj_rec(fi1, 1);
#if DEBUG
	cout << "PROCESSING SIMPLIFY ... fi1 -> B" << endl;
#endif
	

	fi2 = Gntk->Obj_Ptr[node->vFanins[2]];

	// cout << "======Original Node fi2, C=======" << endl;
	// cout << "Original Node fi2 is: " << id_name[fi2->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi2->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi2->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi2->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi2->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Original Node C is: " << id_name[C->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < C->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[C->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < C->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[C->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "=================================" << endl;

	Mig_ObjPatchFanin(node, fi2, C);
	if (fi2->vFanouts.size() == 0 && fi2->vFanins.size() != 0 && fi2->Type != Zombie) Mig_ObjDeleteObj_rec(fi2, 1);
#if DEBUG
	cout << "PROCESSING SIMPLIFY ... fi2 -> C" << endl;
#endif
	// cout << "======Node fi0, A=======" << endl;
	// cout << "Node fi0 is: " << id_name[fi0->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi0->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi0->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi0->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi0->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Node A is: " << id_name[A->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < A->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[A->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < A->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[A->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "========================" << endl;

	// cout << "======Node fi1, B=======" << endl;
	// cout << "Node fi1 is: " << id_name[fi1->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi1->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi1->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi1->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi1->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Node B is: " << id_name[B->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < B->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[B->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < B->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[B->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "========================" << endl;

	// cout << "======Node fi2, C=======" << endl;
	// cout << "Node fi2 is: " << id_name[fi2->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi2->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi2->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi2->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi2->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Node C is: " << id_name[C->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < C->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[C->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < C->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[C->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "========================" << endl;
#if DEBUG
	cout << "FINAL PROCESSING SIMPLIFY ... node -> node_update" << endl;
	cout << "======Updated Target Node=======" << endl;
	cout << "Updated Target Node is: " << id_name[node->Id] << endl;
	cout << "Fanin nodes are: ";
	for (i = 0; i < node->vFanins.size(); i++){
		cout << id_name[Ontk->Obj_Ptr[node->vFanins[i]]->Id] << " ";
	}
	cout << endl;
	cout << "Fanout nodes are: ";
	for (i = 0; i < node->vFanouts.size(); i++){
		cout << id_name[Ontk->Obj_Ptr[node->vFanouts[i]]->Id] << " ";
	}
	cout << endl;
	cout << "================================" << endl;
#endif	
	if (direct == 1){	// change node's phase
		for(i = 0; i < node->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[node->vFanouts[i]];
			for(j = 0; j < fo->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[fo->vFanins[j]];
				if(node == fi){
					// j is 0 or 1 or 2
					if(j == 0){
						if(fo->fCompl0 == 0) fo->fCompl0 = 1;
						else fo->fCompl0 = 0;
					}else if(j == 1){
						if(fo->fCompl1 == 0) fo->fCompl1 = 1;
						else fo->fCompl1 = 0;
					}else{
						if(fo->fCompl2 == 0) fo->fCompl2 = 1;
						else fo->fCompl2 = 0;
					}
					break;
				}
			}
		}
	}
	
	// refresh nList
	nList.clear();
	DFS(Gntk);
	reverse(nList.begin(), nList.end());
	// global_index = 0;
	
	// refresh EachNodeDom
	for(int k = 0; k < size; k++){
		EachNodeDom[k].clear();
	}
	// Free each node's OBS1, OBS0
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		ir = &nodeInfoTable[po->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){

		/* fix some bugs */
		if (pn->Type == Zombie) {
			pn->vFanins.clear();
			pn->vFanouts.clear();
		}
		/* fix some bugs */
		
		ir = &nodeInfoTable[pn->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}

	if (compute_gate(Gntk) >= count) printf("Unexpected\n");
}

//***********************************************************
void NodeARSimplify_TWOINPUTS(Xmg_Obj_t* node, Xmg_Obj_t* A, bool Afault, Xmg_Obj_t* B, bool Bfault, bool direct, bool operation)
{
	Xmg_Obj_t *fi0, *fi1, *fi2, *fo, *fi;
	unsigned i, j;
	int count;
	node_info *ir;
	int constant_index = 0;

	// ***** << Detect the index of the constant value >> *****
	for (i = 0; i < node->vFanins.size(); i++){
		if (node->vFanins[i] <= 1){
			constant_index = i;
			break;
		}
	}
#if DEBUG
	// ***** << List the basic infomations of target node, node A, B, C, and their fault respectively >> *****
	cout << "======Basic Info. for Nt, Nf1, Nf2 ... TWOINPUTS======" << endl;
	cout << "Target node is: " << id_name[node->Id] << endl;
	cout << "Node Nf1 is: " << id_name[A->Id] << ", and fault is: " << Afault << endl;
	cout << "Node Nf2 is: " << id_name[B->Id] << ", and fault is: " << Bfault << endl;
	cout << "Direct is: " << direct << endl;
	cout << "======================================================" << endl;
#endif
	count = compute_gate(Gntk);

	// cout << "======Original Target Node=======" << endl;
	// cout << "Original Target Node is: " << id_name[node->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < node->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[node->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < node->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[node->vFanouts[i]]->Id] << " ";
	// 	}
	// 	cout << endl;
	// cout << "=================================" << endl;

	
	
	/* AND operation */
	if (operation == 0) {
		if ((Afault == 1) && (Bfault == 1)){
			if (constant_index == 0) {
				node->fCompl1 = 0;
				node->fCompl2 = 0;
			} else if (constant_index == 1) {
				node->fCompl0 = 0;
				node->fCompl2 = 0;
			} else {
				node->fCompl0 = 0;
				node->fCompl1 = 0;
			}
			
		} else if ((Afault == 0) && (Bfault == 1)){
			if (constant_index == 0) {
				node->fCompl1 = 1;
				node->fCompl2 = 0;
			} else if (constant_index == 1) {
				node->fCompl0 = 1;
				node->fCompl2 = 0;
			} else {
				node->fCompl0 = 1;
				node->fCompl1 = 0;
			}
		} else if ((Afault == 1) && (Bfault == 0)){
			if (constant_index == 0) {
				node->fCompl1 = 0;
				node->fCompl2 = 1;
			} else if (constant_index == 1) {
				node->fCompl0 = 0;
				node->fCompl2 = 1;
			} else {
				node->fCompl0 = 0;
				node->fCompl1 = 1;
			}
		} else if ((Afault == 0) && (Bfault == 0)){
			if (constant_index == 0) {
				node->fCompl1 = 1;
				node->fCompl2 = 1;
			} else if (constant_index == 1) {
				node->fCompl0 = 1;
				node->fCompl2 = 1;
			} else {
				node->fCompl0 = 1;
				node->fCompl1 = 1;
			}
		}
	} 
	/* OR operation */
	else {
		if ((Afault == 0) && (Bfault == 0)){
			if (constant_index == 0) {
				node->fCompl1 = 0;
				node->fCompl2 = 0;
			} else if (constant_index == 1) {
				node->fCompl0 = 0;
				node->fCompl2 = 0;
			} else {
				node->fCompl0 = 0;
				node->fCompl1 = 0;
			}
		} else if ((Afault == 1) && (Bfault == 0)){
			if (constant_index == 0) {
				node->fCompl1 = 1;
				node->fCompl2 = 0;
			} else if (constant_index == 1) {
				node->fCompl0 = 1;
				node->fCompl2 = 0;
			} else {
				node->fCompl0 = 1;
				node->fCompl1 = 0;
			}
		} else if ((Afault == 0) && (Bfault == 1)){
			if (constant_index == 0) {
				node->fCompl1 = 0;
				node->fCompl2 = 1;
			} else if (constant_index == 1) {
				node->fCompl0 = 0;
				node->fCompl2 = 1;
			} else {
				node->fCompl0 = 0;
				node->fCompl1 = 1;
			}
		} else if ((Afault == 1) && (Bfault == 1)){
			if (constant_index == 0) {
				node->fCompl1 = 1;
				node->fCompl2 = 1;
			} else if (constant_index == 1) {
				node->fCompl0 = 1;
				node->fCompl2 = 1;
			} else {
				node->fCompl0 = 1;
				node->fCompl1 = 1;
			}
		}
	}
	
	int first_nonconstant_index = 0, second_nonconstant_index = 0;
	if (constant_index == 0) {
		first_nonconstant_index = 1;
		second_nonconstant_index = 2;
	} else if (constant_index == 1) {
		first_nonconstant_index = 0;
		second_nonconstant_index = 2;
	} else {
		first_nonconstant_index = 0;
		second_nonconstant_index = 1;
	}
	
	fi0 = Gntk->Obj_Ptr[node->vFanins[first_nonconstant_index]];

	// cout << "======Original Node fi0, A=======" << endl;
	// cout << "Original Node fi0 is: " << id_name[fi0->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi0->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi0->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi0->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi0->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Original Node A is: " << id_name[A->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < A->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[A->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < A->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[A->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "=================================" << endl;

	Mig_ObjPatchFanin(node, fi0, A);
	if (fi0->vFanouts.size() == 0 && fi0->vFanins.size() != 0) Mig_ObjDeleteObj_rec(fi0, 1);
#if DEBUG
	cout << "PROCESSING SIMPLIFY ... fi0 -> A" << endl;
#endif	
	

	fi1 = Gntk->Obj_Ptr[node->vFanins[second_nonconstant_index]];

	// cout << "======Original Node fi1, B=======" << endl;
	// cout << "Original Node fi1 is: " << id_name[fi1->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi1->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi1->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi1->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi1->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Original Node B is: " << id_name[B->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < B->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[B->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < B->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[B->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "=================================" << endl;

	Mig_ObjPatchFanin(node, fi1, B);
	if (fi1->vFanouts.size() == 0 && fi1->vFanins.size() != 0 && fi1->Type != Zombie) Mig_ObjDeleteObj_rec(fi1, 1);
#if DEBUG
	cout << "PROCESSING SIMPLIFY ... fi1 -> B" << endl;
#endif
	

	// cout << "======Node fi0, A=======" << endl;
	// cout << "Node fi0 is: " << id_name[fi0->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi0->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi0->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi0->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi0->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Node A is: " << id_name[A->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < A->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[A->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < A->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[A->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "========================" << endl;

	// cout << "======Node fi1, B=======" << endl;
	// cout << "Node fi1 is: " << id_name[fi1->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < fi1->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[fi1->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < fi1->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[fi1->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "----------------------" << endl;
	// cout << "Node B is: " << id_name[B->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < B->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[B->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// 	for (i = 0; i < B->vFanouts.size(); i++){
	// 		cout << id_name[Ontk->Obj_Ptr[B->vFanouts[i]]->Id] << " ";
	// 	}
	// cout << endl;
	// cout << "========================" << endl;

	// cout << "FINAL PROCESSING SIMPLIFY ... node -> node_update" << endl;
	// cout << "======Updated Target Node=======" << endl;
	// cout << "Updated Target Node is: " << id_name[node->Id] << endl;
	// cout << "Fanin nodes are: ";
	// for (i = 0; i < node->vFanins.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[node->vFanins[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "Fanout nodes are: ";
	// for (i = 0; i < node->vFanouts.size(); i++){
	// 	cout << id_name[Ontk->Obj_Ptr[node->vFanouts[i]]->Id] << " ";
	// }
	// cout << endl;
	// cout << "================================" << endl;
	
	if (direct == 1){	// change node's phase
		for(i = 0; i < node->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[node->vFanouts[i]];
			for(j = 0; j < fo->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[fo->vFanins[j]];
				if(node == fi){
					// j is 0 or 1 or 2
					if(j == 0){
						if(fo->fCompl0 == 0) fo->fCompl0 = 1;
						else fo->fCompl0 = 0;
					}else if(j == 1){
						if(fo->fCompl1 == 0) fo->fCompl1 = 1;
						else fo->fCompl1 = 0;
					}else{
						if(fo->fCompl2 == 0) fo->fCompl2 = 1;
						else fo->fCompl2 = 0;
					}
					break;
				}
			}
		}
	}
	
	// refresh nList
	nList.clear();
	DFS(Gntk);
	reverse(nList.begin(), nList.end());
	// global_index = 0;
	
	// refresh EachNodeDom
	for(int k = 0; k < size; k++){
		EachNodeDom[k].clear();
	}
	// Free each node's OBS1, OBS0
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		ir = &nodeInfoTable[po->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){
		ir = &nodeInfoTable[pn->Id];
		if(ir->OBS1 != NULL) st_free_table(&(ir->OBS1));
		if(ir->OBS0 != NULL) st_free_table(&(ir->OBS0));
	}

	if (compute_gate(Gntk) >= count) printf("Unexpected\n");
}
