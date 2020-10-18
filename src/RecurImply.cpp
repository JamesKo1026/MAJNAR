#include "RecurImply.h"

bool recurimply_forward(Xmg_Obj_t *, Xmg_Obj_t *, st_table*);
bool recurimply_backward(Xmg_Obj_t *, Xmg_Obj_t *, st_table*);

//***********************************************************
bool recur_conflict_check_f(node_info *ir1, tval_t iv, Xmg_Obj_t *r, st_table* learn_st)
{
	Xmg_Obj_t *fo;
	bool st;
	unsigned i;
	
	// conflict check
	if (ir1->value==_x_){
		ir1->value = iv;
	//	learn_st.insert(valType(r->Id,(int)iv));
		st_insert(learn_st, (char *)r, (char *)iv);
		for(i=0; i<r->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[r->vFanouts[i]];
			st = recurimply_forward(r, fo, learn_st);
			if(st==0) return 0;
		}
		return 1;
	}else{
		if(ir1->value==iv) return 1;
		else return 0;
	}
}

//***********************************************************
// n->value has been assigned before recurimply_forward
//***********************************************************
bool recurimply_forward(Xmg_Obj_t *n, Xmg_Obj_t *r, st_table* learn_st) // r is a fanout of n ( n->r )
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
	
//	cout << "recurimply_forward: " << ID_Name[n->Id] << " " << ID_Name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
	
	if(n->Type == CONST) return 1;
	// if(r->Type == PO) {
	// 	fi = Gntk->Obj_Ptr[r->vFanins[0]];

	// 	if (fi->fCompl0 == 0)	iv = ir->value;
	// 	else iv = inv(ir->value);	

	// 	if (ir1->value == _x_){
	// 		ir1->value = iv;
	// 		st_insert(learn_st, (char *)r, (char *)iv);
	// 		return 1;
	// 	}else{
	// 		if (ir1->value != iv){
	// 			printf("F0\n");
	// 			return 0;
	// 		}else{
	// 			return 1;
	// 		}
	// 	}
	// }
	if(r->Type != NODE) {}
	
	int n_index;
	zero_count = 0;
	one_count = 0;
	x_count = 0;
	x_index_ary.clear();
	for(i = 0; i < r->vFanins.size(); i++){ // count other fanins (not including n)
		fi = Gntk->Obj_Ptr[r->vFanins[i]];
		ir2 = &nodeInfoTable[fi->Id];
		if(fi == n){ n_index = i; continue; }
		if(ir2->value == _x_){
			x_count++;
			x_index_ary.push_back(i);
		}else if(ir2->value == node_value(Gntk, fi, r, i, _0_)){
			zero_count++;
		}else if(ir2->value == node_value(Gntk, fi, r, i, _1_)){
			one_count++;
		}
	}

	if (r->op_type == 1) {
		//check XOR
		if((zero_count + one_count + x_count) != 1){
			cout << "recur_imply_forward: " << id_name[n->Id] << "->" << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
			cout << "[Unexpected error] exception counts occurs in recur_imply_forward. total count: " << (zero_count + one_count + x_count) << endl;
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
				return recur_conflict_check_f(ir1, iv, r, learn_st);
			}else{
				iv = node_value(Gntk, n, r, n_index, ir->value);
				return recur_conflict_check_f(ir1, iv, r, learn_st);
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
					st_insert(learn_st, (char *)fi, (char *)ir2->value);
					return recurimply_backward(fi, r, learn_st);
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
	} 
	else {
		//check MAJ
		if((zero_count + one_count + x_count) != 2){
			cout << "recur_imply_forward: " << id_name[n->Id] << "->" << id_name[r->Id] << ", " << ir->value << " " << ir1->value << endl;
			cout << "exception counts occurs in recur_imply_forward. total count: " << (zero_count + one_count + x_count) << endl;
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
				return recur_conflict_check_f(ir1, iv, r, learn_st);
			}else if(zero_count == 2){
				iv = value_0(r);
				return recur_conflict_check_f(ir1, iv, r, learn_st);
			}else{
				iv = node_value(Gntk, n, r, n_index, ir->value);
				return recur_conflict_check_f(ir1, iv, r, learn_st);
			}
		}else if(x_count == 1){
			xIndex = x_index_ary[0];
			if(one_count == 1 && ir->value == node_value(Gntk, n, r, n_index, _1_)){ // directly imply fanout r
				iv = value_1(r);
				return recur_conflict_check_f(ir1, iv, r, learn_st);
			}else if(zero_count == 1 && ir->value == node_value(Gntk, n, r, n_index, _0_)){ // directly imply fanout r
				iv = value_0(r);
				return recur_conflict_check_f(ir1, iv, r, learn_st);
			}else{ // backward imply case. r 's valuing fanin have different value
				if(ir1->value != _x_){
					fi = Gntk->Obj_Ptr[r->vFanins[xIndex]];
					ir2 = &nodeInfoTable[fi->Id];
					if(ir2->value == _x_){
						ir2->value = node_value(Gntk, fi, r, xIndex, ir1->value);
					//	learn_st.insert(valType(fi->Id, (int)ir2->value));
						st_insert(learn_st, (char *)fi, (char *)ir2->value);
						return recurimply_backward(fi, r, learn_st);
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
						//	learn_st.insert(valType(fi->Id, (int)ir2->value));
							st_insert(learn_st, (char *)fi, (char *)ir2->value);
							st = recurimply_backward(fi, r, learn_st);
							if(st==0){
								return 0;
							}
						}
					}
					return 1;
				}else{ // QA : because r 's value may be unknown
				//	if(JTable.count(r->Id)==0){
				//		JTable.insert(valType(r->Id, (int)ir1->value));
				//	}
				//	return 1;
				}
			}
			return 1;
		}
	}
	return 1;
}

//***********************************************************
bool recur_conflict_check_b(node_info *ir1, tval_t iv, Xmg_Obj_t *fi, Xmg_Obj_t *n, st_table* learn_st)
{
	if (ir1->value==_x_){
		ir1->value = iv;
	//	learn_st.insert(valType(fi->Id,(int)iv));
		st_insert(learn_st, (char *)fi, (char *)iv);
		return recurimply_backward(fi, n, learn_st);
	}else{
		if(ir1->value==iv){
			return 1;
		}else{
			return 0;
		}
	}
}	

//***********************************************************
// n->value has been assigned before recurimply_backward
//***********************************************************
bool recurimply_backward(Xmg_Obj_t *n, Xmg_Obj_t *r, st_table* learn_st) // r is the source node which causes n
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
	
	// cout << "recurimply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
	
	if(n->Type == CONST) return 1;
	
	if(n->vFanouts.size()>1){
		for(i = 0; i < n->vFanouts.size(); i++){
			fo = Gntk->Obj_Ptr[n->vFanouts[i]];
			if(fo == r) continue;
			st = recurimply_forward(n, fo, learn_st);
			if(st == 0) return 0;
		}
	}
	
	// if(n->Type == PO){
	// 	fi = Gntk->Obj_Ptr[n->vFanins[0]];
	// 	ir1 = &nodeInfoTable[fi->Id];
	// 	if (fi->fCompl0 == 0)	iv = ir->value;
	// 	else iv = inv(ir->value);
	// 	return recur_conflict_check_b(ir1, iv, fi, n, learn_st);
	// }	
	
	if(n->Type == PI) return 1;
	
	if(n->Type != NODE){}
	
	zero_count = 0;
	one_count = 0;
	x_count = 0;
	x_index_ary.clear();
	for(i = 0; i < n->vFanins.size(); i++){ // count all n 's fanins
		fi = Gntk->Obj_Ptr[n->vFanins[i]];
		ir1 = &nodeInfoTable[fi->Id];
		// cout << "fi node: " << id_name[fi->Id] << ", and the value is: " << ir1->value << endl;
		if(ir1->value == _x_){
			x_count++;
			x_index_ary.push_back(i);
		}else if(ir1->value == node_value(Gntk, fi, n, i, _0_)){
			zero_count++;
		}else if(ir1->value == node_value(Gntk, fi, n, i, _1_)){
			one_count++;
		}
	}
	if (n->op_type == 1) {
		//check XOR
		if((zero_count + one_count + x_count) != 2){
			cout << "recur_imply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
			cout << "[Unexpected error] exception counts occurs in recur_imply_backward. total count: " << (zero_count + one_count + x_count) << endl;
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
					st_insert(learn_st, (char *)fi, (char *)ir1->value);
					return recurimply_backward(fi, n, learn_st);
				}
			} else {
				fi = Gntk->Obj_Ptr[n->vFanins[xIndex]];
				ir1 = &nodeInfoTable[fi->Id];
				if(ir1->value == _x_){
					ir1->value = inv(node_value(Gntk, fi, n, xIndex, ir->value));
					st_insert(learn_st, (char *)fi, (char *)ir1->value);
					return recurimply_backward(fi, n, learn_st);
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
	} 
	else {
		//check MAJ
		if((zero_count + one_count + x_count) != 3){
			cout << "recur_imply_backward: " << id_name[n->Id] << ", " << ir->value << endl;
			cout << "[Unexpected error] exception counts occurs in recur_imply_backward. total count: " << (zero_count + one_count + x_count) << endl;
			cout << zero_count << "," << one_count << "," << x_count << endl;
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
				//	learn_st.insert(valType(fi->Id, (int)ir1->value));
					st_insert(learn_st, (char *)fi, (char *)ir1->value);
					return recurimply_backward(fi, n, learn_st);
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
					//	learn_st.insert(valType(fi->Id, (int)ir1->value));
						st_insert(learn_st, (char *)fi, (char *)ir1->value);
						st = recurimply_backward(fi, n, learn_st);
						if(st==0){
							return 0;
						}
					}
				}
				return 1;
			}else{
			//	if(JTable.count(n->Id)==0){
			//		JTable.insert(valType(n->Id, (int)ir->value));
			//	}
				return 1;
			}
		}else if(x_count == 3){
		//	if(JTable.count(n->Id)==0){
		//		JTable.insert(valType(n->Id, (int)ir->value));
		//	}
			return 1;
		}
	}
	return 1;
}

//***********************************************************
vector<int> FindJary()
{
	vector<int> j_ary;
//	int j, k, h;
	Xmg_Obj_t *node, *fi;
	node_info *ir, *ir_fi;
//	tval_t aValue;
	unsigned i;
	int zero_count, one_count, x_count;
//	int index;
	st_generator *stgen;
	char *aValue;
	
	st_foreach_item(JTable, stgen, (char **)&node, &aValue){
		ir = &nodeInfoTable[node->Id];
		if(node->Type != NODE) continue;
		if(ir->value == _x_) continue;

		zero_count = 0;
		one_count = 0;
		x_count = 0;
		for(i = 0; i < node->vFanins.size(); i++){
			fi = Gntk->Obj_Ptr[node->vFanins[i]];
			ir_fi = &nodeInfoTable[fi->Id];
			if(ir_fi->value == _x_){
				x_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, node, i, _1_)){
				one_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, node, i, _0_)){
				zero_count++;
			}

			if (node->op_type == 0) {
				if(zero_count >= 2 && ir->value == _0_) continue;
				if(one_count >= 2 && ir->value == _1_) continue;
			} else if (node->op_type == 1) {
				if(zero_count == 2 && ir->value == _0_) continue;
				if(one_count == 2 && ir->value == _0_) continue;
				if(zero_count == 1 && one_count == 1 && ir->value == _1_) continue;
			}
			
			j_ary.push_back(node->Id);
		}
	}
	return j_ary;
}

//***********************************************************
void reset_unknown(st_table *st)
{
	st_generator *stgen;
	Xmg_Obj_t *node;
	char *aValue;
	node_info *ir;

	st_foreach_item(st, stgen, (char **)&node, &aValue){
		ir = &nodeInfoTable[node->Id];
		ir->value = _x_;
	}
}

//***********************************************************
bool RecurImply()
{
	vector<int> j_ary, index_ary;
	unsigned i, j; 
	int h, aIndex;
	Xmg_Obj_t *node, *fi, *node_tmp;
	node_info *ir, *ir_fi, *ir_tmp;
	st_table* flearn_st;
	st_table* slearn_st;
//	st_table* tmp_st;
	bool st;
	unsigned k, index;
	st_generator *stgen;
	char *aValue;
	int zero_count, one_count, x_count;
	
//	flearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
//	slearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
//	tmp_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	// find j-frontiers
	j_ary = FindJary();
	
	// Start recursive learning
	for(i = 0; i < j_ary.size(); i++){
		node = Gntk->Obj_Ptr[j_ary[i]];
		ir = &nodeInfoTable[node->Id];
		index_ary.clear();
		h = 0;
		for(j = 0; j < node->vFanins.size(); j++){
			fi = Gntk->Obj_Ptr[node->vFanins[j]];
			ir_fi = &nodeInfoTable[fi->Id];
			if(ir_fi->value == _x_) index_ary.push_back(h);
			h++;
		}

		if (node->op_type == 0) {
			if(index_ary.size() == 0) continue;
			if(index_ary.size() == 3) continue;
			if(index_ary.size() == 1){ // continue; // can imply ??
				zero_count = 0;
				one_count = 0;
				x_count = 0;
				for(j = 0; j < node->vFanins.size(); j++){
					fi = Gntk->Obj_Ptr[node->vFanins[j]];
					ir_fi = &nodeInfoTable[fi->Id];
					if(ir_fi->value == _x_){
						x_count++;
					}else if(ir_fi->value == node_value(Gntk, fi, node, j, _1_)){
						one_count++;
					}else if(ir_fi->value == node_value(Gntk, fi, node, j, _0_)){
						zero_count++;
					}
				}
				// checking
				if(zero_count >= 2 && ir->value == value_0(node)) continue;
				if(one_count >= 2 && ir->value == value_1(node)) continue;
				if(one_count != 1 || zero_count != 1) continue;
				
				if(x_count > 1){
					cout << "x_count > 1 in one unknown fanin case in RecurImply()" << endl;
					getchar();
				}
				aIndex = index_ary[0];
				fi = Gntk->Obj_Ptr[node->vFanins[aIndex]];
				ir_fi = &nodeInfoTable[fi->Id];
				
			//	flearn_st.clear();
			//	st_reset_table(flearn_st);
				flearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
				if(ir_fi->value != _x_) continue;
				
				ir_fi->value = node_value(Gntk, fi, node, aIndex, ir->value);
			//	flearn_st.insert(valType(fi->Id, (int)ir_fi->value));
			//	st_insert_table(flearn_st, fi->Id, (int)ir_fi->value);
				st_insert(flearn_st, (char *)fi, (char *)ir_fi->value);
				st = recurimply_backward(fi, fi, flearn_st);
				reset_unknown(flearn_st);
				if(st == 1){
					break;
				}
				
			//	flearn_st.clear();
			//	st_reset_table(flearn_st);
				index_ary.clear();
				j_ary.clear();
				st_free_table(&flearn_st);
				return 0;
			}//else continue;
			
			// Firstly, find a fanin and its implication is valid (st=1)
			index = 0;
			for(k=0; k<index_ary.size(); k++){
				aIndex = index_ary[k];
				fi = Gntk->Obj_Ptr[node->vFanins[aIndex]];
				ir_fi = &nodeInfoTable[fi->Id];
				
			//	flearn_st.clear();
			//	st_reset_table(flearn_st);
				flearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
				ir_fi->value = node_value(Gntk, fi, node, aIndex, ir->value);
			//	flearn_st.insert(valType(fi->Id, (int)ir_fi->value));
			//	st_insert_table(flearn_st, fi->Id, (int)ir_fi->value);
				st_insert(flearn_st, (char *)fi, (char *)ir_fi->value);
				st = recurimply_backward(fi, fi, flearn_st);
				reset_unknown(flearn_st);
				if(st==1){
					break;
				}
			//	flearn_st.clear();
				st_free_table(&flearn_st);
				index++;
			}
			// secondly, justify other fanins
			if(index<index_ary.size()){
				for(k = index+1; k<index_ary.size(); k++){
					aIndex = index_ary[k];
					fi = Gntk->Obj_Ptr[node->vFanins[aIndex]];
					ir_fi = &nodeInfoTable[fi->Id];
					
				//	slearn_st.clear();
					slearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
					ir_fi->value = node_value(Gntk, fi, node, aIndex, ir->value);
				//	slearn_st.insert(valType(fi->Id, (int)ir_fi->value));
				//	st_insert_table(slearn_st, fi->Id, (int)ir_fi->value);
					st_insert(slearn_st, (char *)fi, (char *)ir_fi->value);
					st = recurimply_backward(fi, fi, slearn_st);
					reset_unknown(slearn_st);
					if(st==1){
						st_intersection(flearn_st, slearn_st);
					//	tmp_st.clear();
					//	tmp_st = map_intersection(flearn_st, slearn_st);
					//	flearn_st.clear();
					//	flearn_st = tmp_st;
				//	cout << "st1ss" << endl;
					//	st_reset_table(tmp_st);
					//	st_intersection(tmp_st, flearn_st, slearn_st);
					//	st_reset_table(flearn_st);
					//	st_copy(flearn_st, tmp_st);
					//	cout << "st1ee" << endl;
					}
				//	slearn_st.clear();
					st_free_table(&slearn_st);
				}
			}else{
				j_ary.clear();
				index_ary.clear();
			//	st_free_table(flearn_st);
			//	st_free_table(slearn_st);
			//	st_free_table(tmp_st);
				return 0;
			}
			
			// update node value
			st_foreach_item(flearn_st, stgen, (char **)&node_tmp, &aValue){
				ir_tmp = &nodeInfoTable[node_tmp->Id];
				if(ir_tmp->value != _x_){
					if(ir_tmp->value != (tval_t)(long)aValue){
						j_ary.clear();
						index_ary.clear();
						return 0;
					}
				}else{
					ir_tmp->value = (tval_t)(long)aValue;
					st_insert(ValuingNode, (char *)node_tmp , aValue);
				}
			}
		}
		
		// else if (node->op_type == 1) {
		// 	if(index_ary.size() == 0) continue;
		// 	if(index_ary.size() == 2) continue;
		// 	if(index_ary.size() == 1){
		// 		zero_count = 0;
		// 		one_count = 0;
		// 		x_count = 0;
		// 		for(j = 0; j < node->vFanins.size(); j++){
		// 			fi = Gntk->Obj_Ptr[node->vFanins[j]];
		// 			ir_fi = &nodeInfoTable[fi->Id];
		// 			if(ir_fi->value == _x_){
		// 				x_count++;
		// 			}else if(ir_fi->value == node_value(Gntk, fi, node, j, _1_)){
		// 				one_count++;
		// 			}else if(ir_fi->value == node_value(Gntk, fi, node, j, _0_)){
		// 				zero_count++;
		// 			}
		// 		}
		// 		// checking
		// 		if(zero_count == 2 && ir->value == value_0(node)) continue;
		// 		if(one_count == 2 && ir->value == value_0(node)) continue;
		// 		if(zero_count == 1 && one_count == 1 && ir->value == value_1(node)) continue;
		// 		if((one_count + zero_count) > 1) continue;
				
		// 		if(x_count > 1){
		// 			cout << "x_count > 1 in one unknown fanin case in RecurImply() ... XOR gate" << endl;
		// 			getchar();
		// 		}
		// 		aIndex = index_ary[0];
		// 		fi = Gntk->Obj_Ptr[node->vFanins[aIndex]];
		// 		ir_fi = &nodeInfoTable[fi->Id];
				
		// 		flearn_st = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
		// 		if(ir_fi->value != _x_) {
		// 			cout << "Unexpected error in RecurImply() ... XOR gate" << endl;
		// 			continue;
		// 		}

		// 		// only when the another fanin is "1", ir_fi value != ir value (need to inverse)
		// 		tval_t temp_value;
		// 		if (zero_count == 0 && one_count == 1) {
		// 			if (ir->value == _1_)
		// 				temp_value = _0_;
		// 			else if (ir->value == _0_)
		// 				temp_value = _1_; 
		// 		}
		// 		ir_fi->value = node_value(Gntk, fi, node, aIndex, temp_value);
			
		// 		st_insert(flearn_st, (char *)fi, (char *)ir_fi->value);
		// 		st = recurimply_backward(fi, fi, flearn_st);
		// 		reset_unknown(flearn_st);
		// 		if(st == 1){
		// 			break;
		// 		}
				
		// 		index_ary.clear();
		// 		j_ary.clear();
		// 		st_free_table(&flearn_st);
		// 		return 0;
		// 	}//else continue;

		// }
		index_ary.clear();
	}
	j_ary.clear();
	return 1;
}
