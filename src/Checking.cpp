#include "Checking.h"

//***********************************************************
bool checkImply(Xmg_Ntk_t *ntk, vector<SideInput_Pair_t> sideinputpairs)
{
//	cout << "checkImply()" << endl;
	Xmg_Obj_t *node1, *node2;
	Xmg_Obj_t *parent;
	int index1, index2;
	node_info *ir1, *ir2;
	for(vector<SideInput_Pair_t>::iterator iter = sideinputpairs.begin(); iter != sideinputpairs.end(); iter++){
		index1 = iter->index1;
		index2 = iter->index2;
		//cout << "index1: " << index1 << ", index2: " << index2 << endl;
		/** XOR **/
		if (index2 == -1) {
			node1 = ntk->Obj_Ptr[iter->pnode1];
			parent = ntk->Obj_Ptr[iter->parent];
			// cout << "Parent node op_type is: " << parent->op_type << endl;
			// cout << "index1: " << index1 << ", index2: " << index2 << endl;
			ir1 = &nodeInfoTable[node1->Id];
			if(ir1->value != _x_){
				if(node_value(ntk, node1, parent, index1, ir1->value) != _0_) {
					// cout << "[Unexpected error] in checkimply in Checking.cpp! ...XOR" << endl;
					return 0;
				}
			}
		}
		/** MAJ **/
		else {
			node1 = ntk->Obj_Ptr[iter->pnode1];
			node2 = ntk->Obj_Ptr[iter->pnode2];
			parent = ntk->Obj_Ptr[iter->parent];
			// cout << "Parent node op_type is: " << parent->op_type << endl;
			// cout << "index1: " << index1 << ", index2: " << index2 << endl;
			ir1 = &nodeInfoTable[node1->Id];
			ir2 = &nodeInfoTable[node2->Id];
			if(ir1->value != _x_ && ir2->value != _x_){
				if(node_value(ntk, node1, parent, index1, ir1->value) == node_value(ntk, node2, parent, index2, ir2->value)) {
					// cout << "[Unexpected error] in checkimply in Checking.cpp! ...MAJ" << endl;
					return 0;
				}
			}
		} 
	}
	return 1;
}

//***********************************************************
bool check(Xmg_Ntk_t *ntk)
{
	cout << "=== Check output ===" << endl;
	for(vector<Xmg_Obj_t>::iterator po = ntk->vPos.begin(); po != ntk->vPos.end(); po++){
		if(po->Type != PO){
			cout << id_name[po->Id] << " is not PO" << endl;
			return 0;
		}
		if(po->op_type == 0 && po->vFanins.size() != 3){
			cout << id_name[po->Id] << " MAJ's fanin count is not 3" << endl;
			return 0;
		}

		if(po->op_type == 1 && po->vFanins.size() != 2){
			cout << id_name[po->Id] << " XOR's fanin count is not 2" << endl;
			return 0;
		}
	}

	cout << "=== Check node ===" << endl;
	for(vector<Xmg_Obj_t>::iterator n = ntk->vObjs.begin(); n != ntk->vObjs.end(); n++){
		if(n->Type != NODE){
			cout << id_name[n->Id] << " is not NODE" << endl;
			return 0;
		}
		if(n->op_type == 0 && n->vFanins.size() != 3){
			cout << id_name[n->Id] << " MAJ's fanin count is not 3" << endl;
			return 0;
		}

		if(n->op_type == 1 && n->vFanins.size() != 2){
			cout << id_name[n->Id] << " XOR's fanin count is not 2" << endl;
			return 0;
		}

	/*	if(n->vFanouts.size()==0){
			cout << id_name[n->Id] << " has no fanout" << endl;
		//	return 0;
		}*/
	}
	return 1;
}

//***********************************************************
unsigned int MajoriyValue(unsigned int a, unsigned int b, unsigned int c)
{
	unsigned int result;
	unsigned int g1;
	unsigned int g2;
	unsigned int g3;
	g1 = (a & b);
	g2 = (b & c);
	g3 = (a & c);
	result = g1;
	result = (result | g2);
	result = (result | g3);
	return result;
}

//***********************************************************
vector<int> Sim()
{
	Xmg_Obj_t *node;
	Xmg_Obj_t *fo, *fi;
	queue<int> Q1;
	queue<int> Q2;
	srand(time(NULL));
	
	int size = id_index;
	int *Ontk_visit = new int[size];
	int *Gntk_visit = new int[size];
	int *Ontk_deg = new int[size];
	int *Gntk_deg = new int[size];
	unsigned int *Ontk_value = new unsigned int[size];
	unsigned int *Gntk_value = new unsigned int[size];
//	int *Ontk_level = new int[size];
//	int *Gntk_level = new int[size];
	vector<int> result;
	
	for(int count=0; count < 10000; count++){
	
		for(int k=0; k<size; k++){
			Ontk_visit[k] = 0;
			Gntk_visit[k] = 0;
			Ontk_deg[k] = 0;
			Gntk_deg[k] = 0;
		}
		int i;
		Ontk_value[0] = 0;
		Ontk_value[1] = 2147483647;
		Ontk_visit[0] = 1;
		Ontk_visit[1] = 1;
		Gntk_value[0] = 0;
		Gntk_value[1] = 2147483647;
		Gntk_visit[0] = 1;
		Gntk_visit[1] = 1;
		
		
		Q1.push(0);
		Q1.push(1);
		Q2.push(0);
		Q2.push(1);
		
		for(Vec_Obj_t::iterator iter = Ontk->vPis.begin(); iter != Ontk->vPis.end(); iter++){
		//	pi = Ontk->Obj_Ptr[*iter];
			if(iter->Id <= 1) continue;
			Q1.push(iter->Id);
			Q2.push(iter->Id);
			Ontk_value[iter->Id] = rand()%2147483648;
			Gntk_value[iter->Id] = Ontk_value[iter->Id];
			Ontk_visit[iter->Id] = 1;
			Gntk_visit[iter->Id] = 1;
		}
		
		unsigned int value;
		int pnode_id;
	//	cout << "sss" << endl;
		while(!Q1.empty()){
			pnode_id = Q1.front();
			Q1.pop();
			node = Ontk->Obj_Ptr[pnode_id]; 
		//	cout << "pop: " << O_id_name[node->Id] << endl;
			for(vector<int>::iterator iter = node->vFanouts.begin(); iter != node->vFanouts.end(); iter++){
				fo = Ontk->Obj_Ptr[*iter];
			//	cout << "fo: " << O_id_name[fo->Id] << endl;
				if(Ontk_deg[fo->Id] < 3){
					++Ontk_deg[fo->Id];
				}// cout << "ok1" << endl;
				if(Ontk_deg[fo->Id] == 3){
					Ontk_visit[fo->Id] = 1;
				//	cout << "ok2" << endl;
					// Do function
					unsigned int fi0, fi1, fi2;
				//	cout << "size: " << fo->vFanins.size() << endl;
					for(unsigned j=0; j< fo->vFanins.size(); j++){
				//	cout <<"j: " << j << " " << fo->vFanins[j] << endl;
						fi = Ontk->Obj_Ptr[fo->vFanins[j]];
					//	cout << "ok2.5 " << id_name[fi->Id]  << endl;
						if(j==0){
							if(fo->fCompl0==0) fi0 = Ontk_value[fi->Id];
							else fi0 = ((~Ontk_value[fi->Id])-2147483648);
						}else if(j==1){
							if(fo->fCompl1==0) fi1 = Ontk_value[fi->Id];
							else fi1 = ((~Ontk_value[fi->Id])-2147483648); 
						}else if(j==2){
							if(fo->fCompl2==0) fi2 = Ontk_value[fi->Id];
							else fi2 = ((~Ontk_value[fi->Id])-2147483648); 
						}//cout << "ok2.6 " << id_name[fi->Id]  << endl;
					}
				//	cout << "ok3" << endl;
					value = MajoriyValue(fi0, fi1, fi2);
				//	cout << "ok4" << endl;
					Ontk_value[fo->Id] = value;
					Q1.push(fo->Id);
			//		cout << "push: " << O_id_name[fo->Id] << endl;
				}
			}
		}
	//	cout << "Q2---------------" << endl;
		while(!Q2.empty()){
			pnode_id = Q2.front();
			Q2.pop();
			node = Gntk->Obj_Ptr[pnode_id]; 
		//	cout << "pop: " << id_name[node->Id] << ", " << node->vFanouts.size() <<  endl;
			for(vector<int>::iterator iter = node->vFanouts.begin(); iter != node->vFanouts.end(); iter++){
				fo = Gntk->Obj_Ptr[*iter];
			//	cout << "fo: " << O_id_name[fo->Id] << endl;
				if(Gntk_deg[fo->Id] < 3){
					++Gntk_deg[fo->Id];
				}// cout << "ok1" << endl;
				if(Gntk_deg[fo->Id] == 3){
					Gntk_visit[fo->Id] = 1;
				//	cout << "ok2" << endl;
					// Do function
					unsigned int fi0, fi1, fi2;
				//	cout << "size: " << fo->vFanins.size() << endl;
					for(unsigned j=0; j< fo->vFanins.size(); j++){
				//	cout <<"j: " << j << " " << fo->vFanins[j] << endl;
						fi = Gntk->Obj_Ptr[fo->vFanins[j]];
					//	cout << "ok2.5 " << id_name[fi->Id]  << endl;
						if(j==0){
							if(fo->fCompl0==0) fi0 = Gntk_value[fi->Id];
							else fi0 = ((~Gntk_value[fi->Id])-2147483648);
						}else if(j==1){
							if(fo->fCompl1==0) fi1 = Gntk_value[fi->Id];
							else fi1 = ((~Gntk_value[fi->Id])-2147483648); 
						}else if(j==2){
							if(fo->fCompl2==0) fi2 = Gntk_value[fi->Id];
							else fi2 = ((~Gntk_value[fi->Id])-2147483648); 
						}//cout << "ok2.6 " << id_name[fi->Id]  << endl;
					}
				//	cout << "ok3" << endl;
					value = MajoriyValue(fi0, fi1, fi2);
				//	cout << "ok4" << endl;
					Gntk_value[fo->Id] = value;
					Q2.push(fo->Id);
				//	cout << "push: " << id_name[fo->Id] << endl;
				}
			}
		}
		
		
		//	Gntk
		// checking
		for(i=0;i<size;i++){
			node = Ontk->Obj_Ptr[i];
			if(node!=NULL){
			/*	if(node->Type == NODE && node->vFanouts.size()>0 && Ontk_visit[node->Id]==0){
					cout << id_name[node->Id] << "is not visited" << endl;
					exit(1);
				}*/
				if(node->Type == PO && Ontk_visit[node->Id]==0){
					cout << id_name[node->Id] << "is not visited" << endl;
					exit(1);
				}
			}
		}
		for(i=0;i<size;i++){
			node = Gntk->Obj_Ptr[i];
			if(node!=NULL){
			/*	if(node->Type == NODE && node->vFanouts.size()>0 && Gntk_visit[node->Id]==0){
					cout << id_name[node->Id] << "is not visited(Ontk)" << endl;
					exit(1);
				}*/
				if(node->Type == PO && Gntk_visit[node->Id]==0){
					cout << id_name[node->Id] << "is not visited(Gntk)" << endl;
					exit(1);
				}
			}
		}
		//cout << "*********                     neq list: " << endl;
		for(Vec_Obj_t::iterator iter = Ontk->vPos.begin(); iter != Ontk->vPos.end(); iter++){
			if(Ontk_value[iter->Id] != Gntk_value[iter->Id]){
				result.push_back(iter->Id);
				//cout << id_name[iter->Id] << " " << endl;
			}
		}
	//	cout << "end simulation" << endl;
		if(!result.empty())
			return result;
	}
	return result;
}

