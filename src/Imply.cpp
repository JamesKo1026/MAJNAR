#include "Imply.h"

#define RECUR 1

bool imply_forward(Xmg_Obj_t *, Xmg_Obj_t *);
bool imply_backward(Xmg_Obj_t *, Xmg_Obj_t *);


//***********************************************************
bool conflict_check_f(node_info *ir1, tval_t iv, Xmg_Obj_t *r)
{
	Xmg_Obj_t *fo;
	bool st;
	unsigned i;
	
	// conflict check
	if (ir1->value == _x_){
		ir1->value = iv;
		st_insert(ValuingNode, (char *)r, (char *)iv);
		for(i = 0; i < r->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[r->vFanouts[i]];
			st = imply_forward(r, fo);
			if(st == 0) {
				//printf("Unexpected error in conflict_check_f imply.cpp at line around 22\n");
				return 0;
			}
		}
		return 1;
	}else{
		if(ir1->value == iv) return 1;
		else {
			//printf("Unexpected error in conflict_check_f imply.cpp at line around 30\n");
			return 0;
		}
	}
}

//***********************************************************
// n->value has been assigned before imply_forward
//***********************************************************
bool imply_forward(Xmg_Obj_t *n, Xmg_Obj_t *r) // r is a fanout of n ( n->r )
{
	Xmg_Obj_t *fi;
	node_info *ir, *ir1, *ir2;
	bool st;
	vector<int> x_index_ary;
	int zero_count, one_count, x_count;
	unsigned i;
	int xIndex;
	tval_t iv;
	
	ir = &nodeInfoTable[n->Id];
	ir1 = &nodeInfoTable[r->Id];
	
	//cout << "imply_forward: " << id_name[n->Id] << " " << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
	
	if(n->Type == CONST) return 1;
	if(r->Type == PO) {
		fi = Gntk->Obj_Ptr[r->vFanins[0]];

		if (fi->fCompl0 == 0)	iv = ir->value;
		else iv = inv(ir->value);	

		if (ir1->value == _x_){
			ir1->value = iv;
			st_insert(ValuingNode, (char *)r, (char *)iv);
			return 1;
		}else{
			if (ir1->value != iv){
				//printf("F0\n");
				return 0;
			}else{
				return 1;
			}
		}
	}
	if(r->Type != NODE) {}
	
	int n_index;
	zero_count = 0;
	one_count = 0;
	x_count = 0;
	x_index_ary.clear();
	for(i = 0; i < r->vFanins.size(); i++){ // count other fanins (not including n)
		fi = Gntk->Obj_Ptr[r->vFanins[i]];
		ir2 = &nodeInfoTable[fi->Id];
		if(fi == n){ 
			n_index = i; 
			continue; 
		}
		if(ir2->value == _x_){
			x_count++;
			x_index_ary.push_back(i);
		}else if(ir2->value == node_value(Gntk, fi, r, i, _0_)){
			zero_count++;
		}else if(ir2->value == node_value(Gntk, fi, r, i, _1_)){
			one_count++;
		}
	}

	// cout << "imply_forward: " << id_name[n->Id] << "->" << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
	// cout << zero_count << "," << one_count << "," << x_count << endl;

	if (r->op_type == 1) {
		//check XOR
		if((zero_count + one_count + x_count) != 1){
			cout << "imply_forward: " << id_name[n->Id] << "->" << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
			cout << "[Unexpected error][XOR] exception counts occurs in imply_forward. total count: " << (zero_count + one_count + x_count) << endl;
			cout << zero_count << "," << one_count << "," << x_count << endl;
			return 0;
			getchar();
		}
		// cout << "imply_forward: " << id_name[n->Id] << " ---> " << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
		// cout << "zero_count: " << zero_count << ", one_count: " << one_count << ", x_count: " << x_count << endl;
		if(ir1->value != _x_){
			if(zero_count == 1 && node_value(Gntk, n, r, n_index, ir->value) == _0_ && ir1->value == _1_) return 0;
			if(zero_count == 1 && node_value(Gntk, n, r, n_index, ir->value) == _1_ && ir1->value == _0_) return 0;
			if(one_count == 1 && node_value(Gntk, n, r, n_index, ir->value) == _0_ && ir1->value == _0_) return 0;
			if(one_count == 1 && node_value(Gntk, n, r, n_index, ir->value) == _1_ && ir1->value == _1_) return 0;
		}
		
		if(x_count == 0){ // directly imply fanout r
			if(one_count == 1){
				iv = inv(node_value(Gntk, n, r, n_index, ir->value));
				return conflict_check_f(ir1, iv, r);
			}else{
				iv = node_value(Gntk, n, r, n_index, ir->value);
				return conflict_check_f(ir1, iv, r);
			}
		}else if(x_count == 1){
			xIndex = x_index_ary[0];
			if (xIndex > 1) {
				cout << "[ERROR] Unvalid xIndex!!!!!!" << endl;
				return 0;
				getchar();
			}

			if(ir1->value != _x_){
				fi = Gntk->Obj_Ptr[r->vFanins[xIndex]];
				ir2 = &nodeInfoTable[fi->Id];
				if(ir2->value == _x_){
					if (node_value(Gntk, n, r, n_index, ir->value) == ir1->value) {
						ir2->value = node_value(Gntk, n, r, xIndex, _0_);
					} else {
						ir2->value = node_value(Gntk, n, r, xIndex, _1_);
					}	
				//	ValuingNode.insert(valType(fi->Id, (int)ir2->value));
					st_insert(ValuingNode, (char *)fi, (char *)ir2->value);
					return imply_backward(fi, r);
				} 
				
				// else { 
				// 	// QA : this situation will happen???
				// 	if(!st_is_member(JTable, (char *)n)){
				// 	//	JTable.insert(valType(r->Id, (int)ir1->value));
				// 		st_insert(JTable, (char *)r, (char *)ir1->value);
				// 	}
				// 	return 1;
				// }
			}
			return 1;
		}
	} else {
		//check MAJ
		if((zero_count + one_count + x_count) != 2){
			cout << "imply_forward: " << id_name[n->Id] << "->" << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
			cout << "[Unexpected error][MAJ] exception counts occurs in imply_forward. total count: " << (zero_count + one_count + x_count) << endl;
			cout << zero_count << "," << one_count << "," << x_count << endl;
			return 0;
			getchar();
		}
		if(ir1->value != _x_){
			if(zero_count >= 2 && ir1->value == _1_) return 0;
			if(one_count >= 2 && ir1->value == _0_) return 0;
			if(zero_count == 1 && ir->value == node_value(Gntk, n, r, n_index, _0_) && ir1->value == _1_) return 0;
			if(one_count == 1 && ir->value == node_value(Gntk, n, r, n_index, _1_) && ir1->value == _0_) return 0;
		}
		
		if(x_count == 0){ // directly imply fanout r
			if(one_count == 2){
				iv = value_1(r);
				return conflict_check_f(ir1, iv, r);
			}else if(zero_count == 2){
				iv = value_0(r);
				return conflict_check_f(ir1, iv, r);
			}else{
				iv = node_value(Gntk, n, r, n_index, ir->value);
				return conflict_check_f(ir1, iv, r);
			}
		}else if(x_count == 1){
			xIndex = x_index_ary[0];
			if(one_count == 1 && ir->value == node_value(Gntk, n, r, n_index, _1_)){ // directly imply fanout r
				iv = value_1(r);
				return conflict_check_f(ir1, iv, r);
			}else if(zero_count == 1 && ir->value == node_value(Gntk, n, r, n_index, _0_)){ // directly imply fanout r
				iv = value_0(r);
				return conflict_check_f(ir1, iv, r);
			}else{ // backward imply case. r 's valuing fanin have different value
				if(ir1->value != _x_){
					fi = Gntk->Obj_Ptr[r->vFanins[xIndex]];
					ir2 = &nodeInfoTable[fi->Id];
					if(ir2->value == _x_){
						ir2->value = node_value(Gntk, fi, r, xIndex, ir1->value);
					//	ValuingNode.insert(valType(fi->Id, (int)ir2->value));
						st_insert(ValuingNode, (char *)fi, (char *)ir2->value);
						return imply_backward(fi, r);
					}
				}
				return 1;
			}
		}else if(x_count == 2){
			if(ir1->value != _x_){
				if(ir1->value != node_value(Gntk, n, r, n_index, ir->value)){ // imply other unknown fanins
					for(i=0; i<x_index_ary.size(); i++){
						xIndex = x_index_ary[i];
						fi = Gntk->Obj_Ptr[r->vFanins[xIndex]];
						ir2 = &nodeInfoTable[fi->Id];
						if(ir2->value == _x_){
							ir2->value = node_value(Gntk, fi, r, xIndex, ir1->value);
						//	ValuingNode.insert(valType(fi->Id, (int)ir2->value));
							st_insert(ValuingNode, (char *)fi, (char *)ir2->value);
							st = imply_backward(fi, r);
							if(st==0){
								return 0;
							}
						}
					}
					return 1;
				}else{ // QA : because r 's value may be unknown
					if(!st_is_member(JTable, (char *)n)){
					//	JTable.insert(valType(r->Id, (int)ir1->value));
						st_insert(JTable, (char *)r, (char *)ir1->value);
					}
					return 1;
				}
			}
			return 1;
		}
		return 1;
	} 
	return 1;
}

//***********************************************************
// useless????
bool conflict_check_b(node_info *ir1, tval_t iv, Xmg_Obj_t *fi, Xmg_Obj_t *n)
{
	if (ir1->value == _x_){
		ir1->value = iv;
	//	ValuingNode.insert(valType(fi->Id,(int)iv));
		st_insert(ValuingNode, (char *)fi, (char *)iv);
		return imply_backward(fi, n);
	}else{
		if(ir1->value==iv){
			return 1;
		}else{
			return 0;
		}
	}
}		

//***********************************************************
// n->value has been assigned before imply_backward
//***********************************************************
bool imply_backward(Xmg_Obj_t *n, Xmg_Obj_t *r) // r is the source node which causes n
{
	Xmg_Obj_t *fo, *fi;
	node_info *ir, *ir1;
	unsigned i;
	bool st;
	vector<int> x_index_ary;
	int zero_count, one_count, x_count;
	int xIndex;
	tval_t iv;
	
	ir = &nodeInfoTable[n->Id];
	
	//cout << "imply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
	
	if(n->Type == CONST) return 1;
	
	if(n->vFanouts.size() > 1){
		for(i = 0; i < n->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[n->vFanouts[i]];
			// cout << "node: " << id_name[n->Id] << ", fo: " << id_name[fo->Id] << endl;
			if(fo == r) continue;
			st = imply_forward(n, fo);
			if(st == 0) {
				//printf("Unexpected error in imply_backward imply.cpp at line around 258\n");
				return 0;
			}
		}
	}
	
	if(n->Type == PO){
		fi = Gntk->Obj_Ptr[n->vFanins[0]];
		ir1 = &nodeInfoTable[fi->Id];
		if (fi->fCompl0 == 0)	iv = ir->value;
		else iv = inv(ir->value);
		return conflict_check_b(ir1, iv, fi, n);	
	}	
	
	if(n->Type == PI) return 1;
	
	if(n->Type != NODE){}
	
	zero_count = 0;
	one_count = 0;
	x_count = 0;
	x_index_ary.clear();
	for(i = 0; i < n->vFanins.size(); i++){ // count all n 's fanins
		fi = Gntk->Obj_Ptr[n->vFanins[i]];
		ir1 = &nodeInfoTable[fi->Id];
		if(ir1->value == _x_){
			x_count++;
			x_index_ary.push_back(i);
		}else if(ir1->value == node_value(Gntk, fi, n, i, _0_)){
			zero_count++;
		}else if(ir1->value == node_value(Gntk, fi, n, i, _1_)){
			one_count++;
		}
	}

	// cout << "imply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
	// cout << zero_count << "," << one_count << "," << x_count << endl;

	if (n->op_type == 1) {
		//check XOR
		if((zero_count + one_count + x_count) != 2){
			cout << "imply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
			cout << "[Unexpected error][XOR] exception counts occurs in imply_backward. total count: " << (zero_count + one_count + x_count) << endl;
			cout << zero_count << "," << one_count << "," << x_count << endl;
			return 0;
			getchar();
		}

		if(ir->value != _x_){
			if(zero_count == 2 && ir->value == _1_) return 0;
			if(one_count == 2 && ir->value == _1_) return 0;
			if(zero_count == 1 && one_count == 1 && ir->value == _0_) return 0;
		}
		if(x_count == 0){
			return 1;
		}else if(x_count == 1){
			xIndex = x_index_ary[0];

			if (xIndex > 1) {
				cout << "[ERROR] Unvalid xIndex!!!!!!" << endl;
				return 0;
				getchar();
			}

			if(zero_count == 1){ // directly imply the unknown fanin
				fi = Gntk->Obj_Ptr[n->vFanins[xIndex]];
				ir1 = &nodeInfoTable[fi->Id];
				if(ir1->value == _x_){
					ir1->value = node_value(Gntk, fi, n, xIndex, ir->value);
					st_insert(ValuingNode, (char *)fi, (char *)ir1->value);
					return imply_backward(fi, n);
				}
			} else {
				fi = Gntk->Obj_Ptr[n->vFanins[xIndex]];
				ir1 = &nodeInfoTable[fi->Id];
				if(ir1->value == _x_){
					ir1->value = inv(node_value(Gntk, fi, n, xIndex, ir->value));
					st_insert(ValuingNode, (char *)fi, (char *)ir1->value);
					return imply_backward(fi, n);
				}
			}

			return 1;
		}else if(x_count == 2){
			if(!st_is_member(JTable, (char *)n)){
			//	JTable.insert(valType(n->Id, (int)ir->value));
				st_insert(JTable, (char *)n , (char *)ir->value);
			}
			return 1;
		}
	} else {
		//check MAJ
		if((zero_count + one_count + x_count) != 3){
			cout << "imply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
			cout << "[Unexpected error][MAJ] exception counts occurs in imply_backward. total count: " << (zero_count + one_count + x_count) << endl;
			cout << zero_count << "," << one_count << "," << x_count << endl;
			cout << "Type is: " << n->Type << endl;
			return 0;
			getchar();
		}
		if(ir->value != _x_){
			if(zero_count >= 2 && ir->value == _1_) return 0;
			if(one_count >= 2 && ir->value == _0_) return 0;
		}
		if(x_count == 0){
			return 1;
		}else if(x_count == 1){
			xIndex = x_index_ary[0];
			if(zero_count == 1 && one_count == 1){ // directly imply the unknown fanin
				fi = Gntk->Obj_Ptr[n->vFanins[xIndex]];
				ir1 = &nodeInfoTable[fi->Id];
				if(ir1->value == _x_){
					ir1->value = node_value(Gntk, fi, n, xIndex, ir->value);
				//	ValuingNode.insert(valType(fi->Id, (int)ir1->value));
					st_insert(ValuingNode, (char *)fi, (char *)ir1->value);
					return imply_backward(fi, n);
				}
			}
			return 1;
		}else if(x_count == 2){
			if((one_count == 1 && ir->value == _0_) || (zero_count == 1 && ir->value == _1_)){ // directly imply other unknown fanins
				for(i=0; i<x_index_ary.size(); i++){
					xIndex = x_index_ary[i];
					fi = Gntk->Obj_Ptr[n->vFanins[xIndex]];
					ir1 = &nodeInfoTable[fi->Id];
					if(ir1->value == _x_){
						ir1->value = node_value(Gntk, fi, n, xIndex, ir->value);
					//	ValuingNode.insert(valType(fi->Id, (int)ir1->value));
						st_insert(ValuingNode, (char *)fi, (char *)ir1->value);
						st = imply_backward(fi, n);
						if(st==0){
							return 0;
						}
					}
				}
				return 1;
			}else{
				if(!st_is_member(JTable, (char *)n)){
				//	JTable.insert(valType(n->Id, (int)ir->value));
					st_insert(JTable, (char *)n , (char *)ir->value);
				}
				return 1;
			}
		}else if(x_count == 3){
			if(!st_is_member(JTable, (char *)n)){
			//	JTable.insert(valType(n->Id, (int)ir->value));
				st_insert(JTable, (char *)n , (char *)ir->value);
			}
			return 1;
		}
	}
	return 1;
}

//***********************************************************
bool SideInputPairImply(SideInput_Pair_t sip, vector<SideInput_Pair_t> sip_set) // output new nodeInfoTable and ValuingNode
{
	Xmg_Obj_t *node1, *node2, *parent;
	Xmg_Obj_t *fo;
	node_info *ir1, *ir2;
	st_table *ValuingNode_tmp = NULL;
	st_table *ValuingNode01 = NULL;
	st_table *ValuingNode10 = NULL;
	vector<node_info> nodeInfoTable01;
	vector<node_info> nodeInfoTable10;
	vector<node_info> nodeInfoTable_tmp;
	unsigned i;
	bool st01;
	bool st10;
	unsigned index1;
	unsigned index2;
	int size = id_index;

	st_table *ValuingNode0 = NULL;
	vector<node_info> nodeInfoTable0;
	bool st0;

	/** MAJ **/
	if (sip.index2 != -1) {
		node1 = Gntk->Obj_Ptr[sip.pnode1];
		node2 = Gntk->Obj_Ptr[sip.pnode2];
		index1 = sip.index1;
		index2 = sip.index2;
		parent = Gntk->Obj_Ptr[sip.parent];

		ir1 = &nodeInfoTable[node1->Id];
		ir2 = &nodeInfoTable[node2->Id];
		
		node_info tmp;
		for(int k = 0; k < size; k++){
			if(k == 0) tmp.value = _0_;
			else if(k == 1) tmp.value = _1_;
			else tmp.value = _x_;
			
			tmp.OBS1 = NULL;
			tmp.OBS0 = NULL;
			tmp.OneFanoutFanin = 0;
			nodeInfoTable01.push_back(tmp);
			nodeInfoTable10.push_back(tmp);
			nodeInfoTable_tmp.push_back(tmp);
		}
		
		// Copy the valuing states.
		ValuingNode_tmp = st_copy(ValuingNode);
		copy_value_infotable(nodeInfoTable_tmp);
		
		//(0,1)
		st01 = 1;
		if(ir1->value != _x_){
			if(ir1->value != node_value(Gntk, node1, parent, index1, _0_))
				st01 = 0;
		}else{
			ir1->value = node_value(Gntk, node1, parent, index1, _0_);
		}
		if(ir2->value != _x_){
			if(ir2->value != node_value(Gntk, node2, parent, index2, _1_))
				st01 = 0;
		}else{
			ir2->value = node_value(Gntk, node2, parent, index2, _1_);
		}
		// cout << "[1.] st01: " << st01 << endl;
		// cout << "pnode1: " << id_name[node1->Id] << ", and index1: " << index1;
		// if (node2->Id == -1){
		// 	cout << ", pnode2: X, and index2: X" << endl;
		// } else {
		// 	cout << ", pnode2: " << id_name[node2->Id] << ", and index2: " << index2 << endl;
		// }
		// cout << "Parent node is: " << id_name[parent->Id] << endl;
		// valid pair (0,1) and start to imply
		if(st01 == 1){
			// node1
			if(node1->Type != CONST){
				st_insert(ValuingNode, (char *)node1, (char *)ir1->value);
				// cout << "node1: " << id_name[node1->Id] << endl;
				// cout << "[1.25] st01: " << st01 << endl;
				st01 = imply_backward(node1, node1);
				// cout << "[1.375] st01: " << st01 << endl;
				if(st01 == 1){ // node1 imply_backward is valid
					for(i = 0; i < node1->vFanouts.size(); i++){
						fo = Gntk->Obj_Ptr[node1->vFanouts[i]];
						// cout << "node1: " << id_name[node1->Id] << " ---> fo: " << id_name[fo->Id] << endl;
						st01 = imply_forward(node1, fo);
						// cout << "[1.5] st01: " << st01 << endl;
						if(st01 == 0){
							break;
						}
					}
					// cout << "[2.] st01: " << st01 << endl;
				}
			}
			// node2
			if(node2->Type != CONST){			
				st_insert(ValuingNode, (char *)node2, (char *)ir2->value);
				if(st01 == 1){ // node1 both of implications are valid
					st01 = imply_backward(node2, node2);
				}
				if(st01 == 1){ // node2 imply_backward is valid
					for(i = 0; i < node2->vFanouts.size(); i++){
						fo = Gntk->Obj_Ptr[node2->vFanouts[i]];
						st01 = imply_forward(node2, fo);
						if(st01 == 0){
							break;
						}
					}
					// cout << "[3.] st01: " << st01 << endl;
				}
			}
			// RecurImply to be continue...
			if(st01 == 1){
#if RECUR				
				st01 = RecurImply();
#endif
			}
			if(st01 == 1){
				// st01 = checkImply(Gntk, sip_set);
			}
		}
		if(st01 == 1){
			ValuingNode01 = st_copy(ValuingNode);
			copy_value_infotable(nodeInfoTable01);
		}else{
			ValuingNode01 = NULL;
			refresh_value_infotable(nodeInfoTable01);
		}
		// end (0,1)
		
		// reset information before (0,1) implication
		st_free_table(&ValuingNode);
		ValuingNode = st_copy(ValuingNode_tmp);
		load_value_infotable(nodeInfoTable_tmp);
		
		//(1,0)
		st10 = 1;
		if(ir1->value != _x_){
			if(ir1->value != node_value(Gntk, node1, parent, index1, _1_))
				st10 = 0;
		}else{
			ir1->value = node_value(Gntk, node1, parent, index1, _1_);
		}
		if(ir2->value != _x_){
			if(ir2->value != node_value(Gntk, node2, parent, index2, _0_))
				st10 = 0;
		}else{
			ir2->value = node_value(Gntk, node2, parent, index2, _0_);
		}
		// valid pair (1,0) and start to imply
		if(st10==1){
			// node1
			if(node1->Type != CONST){
				st_insert(ValuingNode, (char *)node1, (char *)ir1->value);
				st10 = imply_backward(node1, node1);
				
				if(st10 == 1){ // node1 imply_backward is valid
					for(i = 0; i < node1->vFanouts.size(); i++){
						fo = Gntk->Obj_Ptr[node1->vFanouts[i]];
						st10 = imply_forward(node1, fo);
						if(st10 == 0){
							break;
						}
					}
				}
			}	
			// node2
			if(node2->Type != CONST){
				st_insert(ValuingNode, (char *)node2, (char *)ir2->value);
				if(st10 == 1){ // node1 both of implications are valid
					st10 = imply_backward(node2, node2);
				}
				if(st10 == 1){ // node2 imply_backward is valid
					for(i = 0; i<node2->vFanouts.size(); i++){
						fo = Gntk->Obj_Ptr[node2->vFanouts[i]];
						st10 = imply_forward(node2, fo);
						if(st10 == 0){
							break;
						}
					}
				}
			}
		// RecurImply to be continue...
			if(st10 == 1){
#if RECUR
				st10 = RecurImply();
#endif
			}
			if(st10 == 1){
				// st10 = checkImply(Gntk, sip_set);
			}
		}
		if(st10 == 1){
			ValuingNode10 = st_copy(ValuingNode);
			copy_value_infotable(nodeInfoTable10);
		}else{
			ValuingNode10 = NULL;
			refresh_value_infotable(nodeInfoTable10);
		}
		// end (1,0)
		// cout << "st01: " << st01 << ", st10: " << st10 << endl;
		if(st01 == 0 && st10 == 0){
			st_free_table(&ValuingNode);
			ValuingNode = st_copy(ValuingNode_tmp);
			load_value_infotable(nodeInfoTable_tmp);
		
			nodeInfoTable_tmp.clear();
			nodeInfoTable01.clear();
			nodeInfoTable10.clear();
			st_free_table(&ValuingNode_tmp);
			st_free_table(&ValuingNode01);
			st_free_table(&ValuingNode10);
			return 0;
		}else if(st01 == 0){
			st_free_table(&ValuingNode);
			ValuingNode = st_copy(ValuingNode10);
			load_value_infotable(nodeInfoTable10);
		}else if(st10 == 0){
			st_free_table(&ValuingNode);
			ValuingNode = st_copy(ValuingNode01);
			load_value_infotable(nodeInfoTable01);
		}else{
			st_free_table(&ValuingNode);
			st_intersection(ValuingNode01,ValuingNode10);
			ValuingNode = st_copy(ValuingNode01);
			load_map_to_nodeInfotable(ValuingNode);
		}
		
		nodeInfoTable_tmp.clear();
		nodeInfoTable01.clear();
		nodeInfoTable10.clear();
		st_free_table(&ValuingNode_tmp);
		st_free_table(&ValuingNode01);
		st_free_table(&ValuingNode10);
	}

	/** XOR **/
	// else {
	// 	node1 = Gntk->Obj_Ptr[sip.pnode1];
	// 	index1 = sip.index1;
	// 	parent = Gntk->Obj_Ptr[sip.parent];

	// 	ir1 = &nodeInfoTable[node1->Id];

	// 	node_info tmp;
	// 	for(int k = 0; k < size; k++){
	// 		if(k == 0) tmp.value = _0_;
	// 		else if(k == 1) tmp.value = _1_;
	// 		else tmp.value = _x_;
			
	// 		tmp.OBS1 = NULL;
	// 		tmp.OBS0 = NULL;
	// 		tmp.OneFanoutFanin = 0;
	// 		nodeInfoTable0.push_back(tmp);
	// 		nodeInfoTable_tmp.push_back(tmp);
	// 	}
	// 	// Copy the valuing states.
	// 	ValuingNode_tmp = st_copy(ValuingNode);
	// 	copy_value_infotable(nodeInfoTable_tmp);


	// 	// sideinput = 0, make sure that ir1->value must be set to "0" 
	// 	st0 = 1;
	// 	if (ir1->value != _x_) {
	// 		if(ir1->value != node_value(Gntk, node1, parent, index1, _0_))
	// 			st0 = 0;
	// 	}else{
	// 		ir1->value = node_value(Gntk, node1, parent, index1, _0_);
	// 	}

	// 	// valid sideinput 0, and start to imply
	// 	if (st0 == 1) {
	// 		// node1
	// 		if(node1->Type != CONST){
	// 			st_insert(ValuingNode, (char *)node1, (char *)ir1->value);
	// 			st0 = imply_backward(node1, node1);
	// 			if(st0 == 1){ // node1 imply_backward is valid
	// 				for(i = 0; i < node1->vFanouts.size(); i++){
	// 					fo = Gntk->Obj_Ptr[node1->vFanouts[i]];
	// 					st0 = imply_forward(node1, fo);
	// 					if(st0 == 0){
	// 						break;
	// 					}
	// 				}
	// 			}
	// 		}
	// 		// RecurImply to be continue...
	// 		// if(st0 == 1){
	// 		// 	st0 = RecurImply();
	// 		// }
	// 		if(st0 == 1){
	// 			st0 = checkImply(Gntk, sip_set);
	// 		}
	// 	}
	// 	if(st0 == 1){
	// 		ValuingNode0 = st_copy(ValuingNode);
	// 		copy_value_infotable(nodeInfoTable0);
	// 	}else{
	// 		ValuingNode0 = NULL;
	// 		refresh_value_infotable(nodeInfoTable0);
	// 	}
	// 	// end sideinput 0

	// 	if(st0 == 0){
	// 		st_free_table(&ValuingNode);
	// 		ValuingNode = st_copy(ValuingNode_tmp);
	// 		load_value_infotable(nodeInfoTable_tmp);
		
	// 		nodeInfoTable_tmp.clear();
	// 		nodeInfoTable0.clear();
	// 		st_free_table(&ValuingNode_tmp);
	// 		st_free_table(&ValuingNode0);
	// 		return 0;
	// 	}else{
	// 		st_free_table(&ValuingNode);
	// 		ValuingNode = st_copy(ValuingNode0);
	// 		load_map_to_nodeInfotable(ValuingNode);	// update the nodeInfotable with ValuingNode table 
	// 	}
	// 	nodeInfoTable_tmp.clear();
	// 	nodeInfoTable0.clear();
	// 	st_free_table(&ValuingNode_tmp);
	// 	st_free_table(&ValuingNode0);
	// }
	return 1;
}

//***********************************************************
bool ActivateImply(Xmg_Obj_t *node, tval_t tvalue, vector<SideInput_Pair_t> sip_set)
{
	node_info *ir;
	unsigned i;
	bool st;
	
	ir = &nodeInfoTable[node->Id];
	ir->value = tvalue;
	st_insert(ValuingNode, (char *)node, (char *)tvalue);
	st = imply_backward(node, node);
	if(st == 0){
		return 0;
	}
	// cout << "sip_set.size(): " << sip_set.size() << endl;
	for(i = 0; i < sip_set.size(); i++){
		// cout << "sip_set: " << i << endl;
		// cout << "pnode1: " << id_name[sip_set[i].pnode1] << ", and index1: " << sip_set[i].index1;
		// if (sip_set[i].pnode2 == -1){
		// 	cout << ", pnode2: X, and index2: X" << endl;
		// } else {
		// 	cout << ", pnode2: " << id_name[sip_set[i].pnode2] << ", and index2: " << sip_set[i].index2 << endl;
		// }
		// cout << "Parent node is: " << id_name[sip_set[i].parent] << endl;
		st = SideInputPairImply(sip_set[i], sip_set);
		if(st == 0){ 
			// cout << "Oops!!!!!!!!!!!" << endl;
			return 0;
		}
	}
	if(sip_set.empty()){
		// cout << "This node Id: " << id_name[node->Id] << ", and op_type is: " << node->op_type << endl;
		// cout << "There isn't any sideinput in this node!!!!!!!!!" << endl;
#if RECUR
		st = RecurImply();
#endif
		if(st == 0) return 0;
	}
	return 1;
}

//***********************************************************
bool ActivateImplyNAR(Xmg_Obj_t *node, bool value)
{
	node_info *ir;
	unsigned i;
	bool st;
	Xmg_Obj_t* fo;
	tval_t tvalue;
	
	ir = &nodeInfoTable[node->Id];

	if (value == 0)
		tvalue = _0_;
	else if (value == 1)
		tvalue = _1_;
	
	if (ir->value == _x_){
		ir->value = tvalue;
	} else {
		if (ir->value != tvalue)	return 0;
	}
		
	st_insert(ValuingNode, (char *)node, (char *)tvalue);
	// cout << "This node Id: " << id_name[node->Id] << ", and op_type is: " << node->op_type << ", value is: " << value << endl;
	// cout << "---------Before backward" << endl;
	// PrintItems(ValuingNode);
	st = imply_backward(node, node);
	// cout << "---------Before forward" << endl;
	// PrintItems(ValuingNode);
	if(st == 0){
		return 0;
	}

	for(i = 0; i < node->vFanouts.size(); i++){
		fo = Gntk->Obj_Ptr[node->vFanouts[i]];
		st = imply_forward(node, fo);
		if(st == 0){
			// printf("[Unexpected error] in imply_forward in SideinputImply of imply.cpp 800\n");
			return 0;
		}
	}
	// cout << "=======Print in Activate=======" << endl;
	// st_generator* temp_stgen;
	// char* value_t;
	// Xmg_Obj_t* node_t;
	// st_foreach_item(ValuingNode, temp_stgen, (char **)&node_t, &value_t){
	// 	cout << "Node is: " << id_name[node_t->Id] << ", op_type is: " << node_t->op_type << ", value is: ";
	// 	if ((tval_t)(long)value_t == _0_)
	// 		cout << "_0_"; 
	// 	else if ((tval_t)(long)value_t == _1_)
	// 		cout << "_1_"; 
	// 	else if ((tval_t)(long)value_t == _x_)
	// 		cout << "_x_"; 
	// 	cout << endl;
	// }
	// cout << "===============================" << endl;
	return 1;
}

//***********************************************************
bool SideInputImply(st_table *si_set)
{
	st_generator *stgen;
	char *value;
	node_info *ir;
	Xmg_Obj_t *fo, *node;
	bool st;
	unsigned i;

	//cout << "Start SideInputImply ... " << endl;
	st_foreach_item(si_set, stgen, (char **)&node, &value){
		//if (node->op_type == 1)	continue;
		//cout << "Node is : " << id_name[node->Id] << " " << endl;
		ir = &nodeInfoTable[node->Id];
		ir->value = (tval_t)(long)value;
		st_insert(ValuingNode, (char *)node , value);
		st = imply_backward(node, node);
		if (st == 0){
			// printf("[Unexpected error] in imply_backward in SideinputImply of imply.cpp 1\n");
			return 0;
		}
		
		for(i = 0; i < node->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[node->vFanouts[i]];
			st = imply_forward(node, fo);
			if(st == 0){
				// printf("[Unexpected error] in imply_forward in SideinputImply of imply.cpp 2\n");
				return 0;
			}
		}
	}
	//cout << "END SideInputImply ... " << endl;
	//cout << endl;
	return 1;
}

//***********************************************************
bool AcitvateImplyFanoutCone(Xmg_Obj_t *node, tval_t tvalue)
{
	Xmg_Obj_t *fo;
	vector<int> fanouts;
	bool st;
	vector<SideInput_Pair_t> sideinputpair;
	st_table *st_sideinput = NULL;
	unsigned i;
	st_table* fvaluingNode;
	st_table* svaluingNode;

	// cout << "This node is: " << id_name[node->Id] << ", op_type is: ";
	// if (node->op_type == 0)
	// 	cout << "[MAJ]";
	// else if (node->op_type == 1)
	// 	cout << "[XOR]";
	// cout << "\n  Multiple fanouts!!!!!!!!!!" << endl;
	
	fanouts = node->vFanouts;
	// flearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	for(i = 0; i < fanouts.size(); i++){
		fo = Gntk->Obj_Ptr[fanouts[i]];
		sideinputpair.clear();
		st_sideinput = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
		GetSideInputCouples(node, fo, st_sideinput, sideinputpair);
			
		ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
		JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
		
		st = SideInputImply(st_sideinput);
		st_free_table(&st_sideinput);
		
		if(st == 1) st = ActivateImply(node, tvalue, sideinputpair);
		
		if(st == 1){
		//	fvaluingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
		//	BackupImplied(fvaluingNode);
			//ValuingNode.clear();
			//JTable.clear();
			fvaluingNode = st_copy(ValuingNode);
			st_free_table(&ValuingNode);
			st_free_table(&JTable);
			refresh_value_nodeInfotable();
			break;
		}
		st_free_table(&ValuingNode);
		st_free_table(&JTable);
		refresh_value_nodeInfotable();
	}

	if(i < fanouts.size()){
		i = i + 1;
		for(;i < fanouts.size(); i++){
			fo = Gntk->Obj_Ptr[fanouts[i]];
			sideinputpair.clear();
			st_sideinput = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			GetSideInputCouples(node, fo, st_sideinput, sideinputpair);
			
			ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			
			st = SideInputImply(st_sideinput);
			st_free_table(&st_sideinput);
		
			if(st == 1) st = ActivateImply(node, tvalue, sideinputpair);
			if(st == 1){
			//	svaluingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			//	BackupImplied(svaluingNode);
			//	svaluingNode = ValuingNode;
			//	tmp.clear();
			//	tmp = map_intersection(fvaluingNode, svaluingNode);
			//	fvaluingNode.clear();
			//	fvaluingNode = tmp; // intersection result
				svaluingNode = st_copy(ValuingNode);
				st_intersection(fvaluingNode, svaluingNode);
				st_free_table(&svaluingNode);
				
				if(st_count(fvaluingNode) == 0){
					st_free_table(&ValuingNode);
					st_free_table(&JTable);
					refresh_value_nodeInfotable();
					break;
				}
			}
			st_free_table(&ValuingNode);
			st_free_table(&JTable);
			refresh_value_nodeInfotable();
		}
	}else{
		return 0;
	}

	ValuingNode = st_copy(fvaluingNode);

	st_free_table(&fvaluingNode);

	// cout << "ValuingNode: " << endl;
	// st_generator *temp_stgen;
	// Xmg_Obj_t *n;
	// char *v;
	// st_foreach_item(ValuingNode, temp_stgen, (char **)&n, &v){
	// 	cout << "Node is: " << id_name[n->Id] << ", op_type is: ";

	// 	if (n->op_type == 0)
	// 		cout << "[MAJ]";
	// 	else if (n->op_type == 1)
	// 		cout << "[XOR]";

	// 	cout << ", and value is: ";

	// 	if ((tval_t)(long)v == _0_)
	// 		cout << "_0_";
	// 	else if ((tval_t)(long)v == _1_)
	// 		cout << "_1_";

	// 	cout << endl;
	// }

	return 1;
}







