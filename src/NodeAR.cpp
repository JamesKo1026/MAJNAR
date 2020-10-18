#include "NodeAR.h"
#define SIMPLIFY 1
#define DEPTHPRESERVED 1
#define RECUR 1

void PrintItems(st_table* st_set_t)
{
	st_generator* temp_stgen;
	Xmg_Obj_t* node;
	char* value;
	cout << "The size of st_table is: " << st_count(st_set_t) << endl;
	if (st_count(st_set_t) == 0) {
		cout << "This st_table is empty!!" << endl;
		return;
	}
		
	st_foreach_item(st_set_t, temp_stgen, (char **)&node, &value){
		cout << "Node is: " << id_name[node->Id] << ", op_type is: " << node->op_type << ", value is: ";
		if ((tval_t)(long)value == _0_)
			cout << "_0_";     
		else if ((tval_t)(long)value == _1_)
			cout << "_1_"; 
		else if ((tval_t)(long)value == _x_)
			cout << "_x_"; 
		cout << endl;
	}
	cout << "===========================" << endl;
}

int FindCandidateBC(st_table* nofanin, Xmg_Obj_t* node, st_table* CandA, st_table* zero_set, st_table* fanout_cone, bool direct)
{
	// cout << "Start to find nf2, nf3" << endl;
    st_generator *stgen, *stgenn, *stgennn;
	Xmg_Obj_t *A, *B, *C, *znode, *fi, *fo;
	char *v1, *v2, *v3, *v4, *v5;
	bool Afault, Bfault, Cfault, st;
	st_table *implied_set, *JTableTmp, *LearnN, *Ccantuse;
	node_info *ir, *ir_fi;
	bool phase, onlyoneSFoFi, todo, found;
	unsigned i, ncount;
	bool AisFanin = 0, BisFanin = 0;

	// **************************** << NOTE >> ****************************
	// we can't pick the node which is in the fanin-cone in any fanins of the Target node (Recursive delete will cause error)
	st_table *fanin_cone_fi0 = NULL;
	st_table *fanin_cone_fi1 = NULL;
	st_table *fanin_cone_fi2 = NULL;
	fanin_cone_fi0 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone_fi1 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone_fi2 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	for (i = 0; i < node->vFanins.size(); i++) {
		fi = Gntk->Obj_Ptr[node->vFanins[i]];
		if (fi->vFanouts.size() == 1) {
			if (i == 0)
				fanin_cone_fi0 = FindFaninConeDfs(fi);
			else if (i == 1)
				fanin_cone_fi1 = FindFaninConeDfs(fi);
			else
				fanin_cone_fi2 = FindFaninConeDfs(fi);
		}
	}

	// Update zero_set, ValuingNode, and JTable;
	ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
    //ValuingNodeC = st_init_table(st_ptrcmp, st_ptrhash);
	JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	st_foreach_item(zero_set, stgen, (char **)&znode, &v1){
		//if (st_is_member(nofanin, (char *)znode))  continue;
		if (znode->Type == PO)	continue;

        ir = &nodeInfoTable[znode->Id];
		ir->value = (tval_t)(long)v1;
		st_insert(ValuingNode, (char *)znode, (char *)v1);
	}

    st_foreach_item(ValuingNode, stgen, (char **)&znode, &v1){
        ir = &nodeInfoTable[znode->Id];
		if(znode->Type != NODE) continue;
		if(ir->value == _x_) continue;

		int zero_count = 0;
		int one_count = 0;
		int x_count = 0;
		for(i = 0; i < znode->vFanins.size(); i++){
			fi = Gntk->Obj_Ptr[znode->vFanins[i]];
			ir_fi = &nodeInfoTable[fi->Id];
			if(ir_fi->value == _x_){
				x_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _1_)){
				one_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _0_)){
				zero_count++;
			}
            if(zero_count >= 2 && ir->value == _0_) continue;
            if(one_count >= 2 && ir->value == _1_) continue;
		}
        if (x_count == 3 || (x_count >= 2 && ir->value == _0_ && zero_count == 1) || (x_count >= 2 && ir->value == _1_ && one_count == 1))
            st_insert(JTable, (char *)znode, (char *)v1);
	}
	
	// backup
	implied_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	BackupImplied(implied_set);
	JTableTmp = st_copy(JTable);

    int cnt = 0;
    for (unsigned i = 0; i < node->vFanins.size(); i++) {
        Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        if (fi->vFanouts.size() == 1)   cnt++;
    }
    if (cnt > 1)   onlyoneSFoFi = 0; // more than one fanin
    else    onlyoneSFoFi = 1;    // just one fanin

	
	LearnN = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
 
	// cout << "---------ValuingNode0" << endl;
	// PrintItems(ValuingNode);
	// cout << "---------CandA" << endl;
	// PrintItems(CandA);
	// cout << "---------implied_set" << endl;
	// PrintItems(implied_set);
	// cout << "---------JTable0" << endl;
	// PrintItems(JTable);
	
	// Foreach Candidate A
	st_foreach_item(CandA, stgen, (char **)&A, &v1){
		Ccantuse = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);

		if (AisFanin || BisFanin) {
			cout << "[Unexpected error]" << endl;
			cout << "AisFanin: " << AisFanin << ", BisFanin: " << BisFanin << endl;
			getchar();
			return 1;
		}
		// cout << "1.) A is: " << id_name[A->Id] << endl;
		// cout << "---------LearnN" << endl;
		// PrintItems(LearnN);

		// if (st_is_member(LearnN, (char *)A)) {
		// 	continue;
		// }
		if (st_is_member(fanin_cone_fi0, (char *)A))
			continue;

        // bool onlyfanouttonode = 0;
		// // eliminate the node which only fanouts to "node"
		// if (onlyoneSFoFi == 1){
        //     for (unsigned i = 0; i < node->vFanins.size(); i++) {
        //         Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        //         if (fi == A && fi->vFanouts.size() == 1)   {
        //             onlyfanouttonode = 1;
        //             break;
        //         }
        //     }
		// }
        // if (onlyfanouttonode)   continue;

        for (unsigned j = 0; j < node->vFanins.size(); j++) {
            Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[j]];
            if (fi == A) {
                AisFanin = 1;
                break;
            }
        }
				
		if ((int)(long)v1 == 1) Afault = 1;
		else Afault = 0;

		// cout << "---------ValuingNode0.5" << endl;
		// PrintItems(ValuingNode);
		
		// cout << "2.) A is: " << id_name[A->Id] << endl;
		st = ActivateImplyNAR(A, Afault);
		// cout << "---------ValuingNode1" << endl;
		// PrintItems(ValuingNode);
		// cout << "---------JTable2" << endl;
		// PrintItems(JTable);
		
		if (st == 1){
			// perform logic implication for node
			st_free_table(&JTable);
			JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			
			st = imply_backward(node, node);
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			
			if (st == 1){
				for(unsigned k = 0; k < node->vFanouts.size(); k++){
                    fo = Gntk->Obj_Ptr[node->vFanouts[k]];
                    st = imply_forward(node, fo);
                    if(st == 0){
                        break;
                    }
					// cout << "---------ValuingNode2.25" << endl;
					// PrintItems(ValuingNode);
                }
				if (st == 1){
#if RECUR
					st = RecurImply();
#endif
				}
			}	
		}
		// cout << "st: " << st << endl;
		// cout << "---------JTable3" << endl;
		// PrintItems(JTable);
		if (st == 0) {
			// refresh_value_nodeInfotable();
            // st_free_table(&ValuingNode);
			// LoadNodeValue(implied_set);
			
            // // st_free_table(&JTable);
            // // st_free_table(&implied_set);
			// // st_free_table(&JTableTmp);
			// // st_free_table(&LearnN);
			// //ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			// ValuingNode = st_copy(implied_set);
			// UpdateJTable(JTableTmp);
			// // cout << "Countinue1" << endl;
			// continue;

			if (direct == 0){
				if (Afault == 1) phase = 0;
				else phase = 1;
			}else{
				if (Afault == 1) phase = 1;
				else phase = 0;
			}		
			refresh_value_nodeInfotable();
            st_free_table(&ValuingNode);
            st_free_table(&JTable);
            st_free_table(&implied_set);
			st_free_table(&JTableTmp);
			st_free_table(&LearnN);

			replaced_count++;
			// cout << "This node can be merge in NodeAR.cpp!!(General)" << endl;
			// cout << "Node is: " << id_name[node->Id] << ", Node Nf1 is: " << id_name[A->Id] << endl;
#if SIMPLIFY 
			merge_simplify(node, A, phase);
#endif 
			return 2;
		}	
		// if (st == 0){
		// 	if (direct == 0){
		// 		if (Afault == 1) phase = 0;
		// 		else phase = 1;
		// 	}else{
		// 		if (Afault == 1) phase = 1;
		// 		else phase = 0;
		// 	}		
			// refresh_value_nodeInfotable();
            // st_free_table(&ValuingNode);
            // st_free_table(&JTable);
            // st_free_table(&implied_set);
			// st_free_table(&JTableTmp);
			// st_free_table(&LearnN);

		// 	replaced_count++;
		// 	cout << "This node can be merge in NodeAR.cpp!!" << endl;
		// 	cout << "Node is: " << id_name[node->Id] << ", Node Nf1 is: " << id_name[A->Id] << endl; 
		// 	merge_simplify(node, A, phase);
 
		// 	return;
		// }

		// cout << "---------ValuingNode2.5" << endl;
		// PrintItems(ValuingNode);

		// **************************** << We don't choose the fanin of the target node as nf2, nf3 >> ****************************
		st_generator* stgennnn;
		Xmg_Obj_t* temp_node;
		char* value;
		st_foreach_item(ValuingNode, stgennnn, (char **)&temp_node, &value){
			// if (temp_node->Type != Zombie && temp_node->vFanins.size() == 0 && temp_node->vFanouts.size() == 0) {
			// 	temp_node->Type = Zombie;
			// }
			if (st_is_member(nofanin, (char*)temp_node) || temp_node == node 
				|| st_is_member(fanout_cone, (char *)temp_node) || temp_node->Type == Zombie 
				|| (st_is_member(fanin_cone_fi0, (char *)temp_node) || st_is_member(fanin_cone_fi1, (char *)temp_node) || st_is_member(fanin_cone_fi2, (char *)temp_node))){
				ir = &nodeInfoTable[temp_node->Id];
				if (temp_node->Id == 0)
					ir->value = _0_;
				else if (temp_node->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&temp_node, NIL(char *));
			}
			// if (temp_node->Type == Zombie) {
			// 	cout << "This node \" " << id_name[temp_node->Id] << " \" is Zombie!!" << endl;
			// }
			if (st_is_member(implied_set, (char*)temp_node)){
				nodeInfoTable[temp_node->Id].value = (tval_t)(long)value;
			}
		}

		// cout << "---------ValuingNode3" << endl;
		// PrintItems(ValuingNode);
		// cout << "---------implied_set" << endl;
		// PrintItems(implied_set);
		// cout << "---------JTable4" << endl;
		// PrintItems(JTable);

		// refresh_value_nodeInfotable();
		// st_free_table(&ValuingNode);
		// LoadNodeValue(implied_set);
		// ValuingNode = st_copy(implied_set);

		// cout << "---------ValuingNode_after_copy" << endl;
		// PrintItems(ValuingNode);

		// found = 0; ncount = 0;
        // ValuingNodeC = st_copy(ValuingNodeB);

		// if (st_count(ValuingNode) < 3){}	continue;

		// Update & Find Candidate B and Candidate C
		st_foreach_item(ValuingNode, stgenn, (char **)&B, &v2){
			// cout << "---------Ccantuse0" << endl;
			// PrintItems(Ccantuse);
			if (B->Type == PO)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
			if (B == A)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
#if DEPTHPRESERVED
			// depth preservation
			if (B->Level >= node->Level)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
#endif
			// if (B->op_type == 1)	continue;
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			// cout << "---------Ccantuse2" << endl;
			// PrintItems(Ccantuse);
			// cout << "---------Ccantuse1" << endl;
			// PrintItems(Ccantuse);
			
			st_insert(Ccantuse, (char *)B, (char *)v2);

			// cout << "---------Ccantuse2" << endl;
			// PrintItems(Ccantuse);
			
			if (!st_is_member(implied_set, (char *)B)){
				
                for (unsigned i = 0; i < node->vFanins.size(); i++) {
                    Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                    if (fi == B) {
                        BisFanin = 1;
                        break;
                    }
                }

				todo = 1;

				// Find Candidate B
                // B cannot be the only one fanin which only fanouts to node
				// int cnt = 0;
                // if (BisFanin == 1){
                //     if (B->vFanouts.size() == 1) {
                //         for (unsigned i = 0; i < node->vFanins.size(); i++) {
                //             Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                //             if (fi->vFanouts.size() == 1)   cnt++;
                //         }
                //     }   
                // }
                // if (cnt == 1)   todo = 0;

				if (todo == 1){
					st_foreach_item(ValuingNode, stgennn, (char **)&C, &v4){
						// cout << "---------Ccantuse3" << endl;
						// PrintItems(Ccantuse);
						// cout << "Nf1: " << id_name[A->Id] << ", Nf2: " << id_name[B->Id] << ", Nf3: " << id_name[C->Id] << endl;
						
						if (st_is_member(Ccantuse, (char *)C))	continue;	// can't take same node on candB and candC, (nf2 ,nf3) should be different
#if DEPTHPRESERVED						
						// depth preservation
						if (C->Level >= node->Level)	{
							continue;
						}
#endif						
						if (st_lookup(CandA, (char *)B, &v3) && st_is_member(implied_set, (char *)C)){
							// cout << "Nf1: " << id_name[A->Id] << ", Nf2: " << id_name[B->Id] << ", Nf3: " << id_name[C->Id] << endl;
							// cout << "Nf2 in MAs(nt = sa0): " << (int)(long)v3 << ", Nf2 in imp((Nf1 = 1) U MAs(nt = sa1): " << (int)(long)v2 << endl;
							if ((int)(long)v2 != (int)(long)v3){

								if ((int)(long)v2 == 0) Bfault = 1;
								else Bfault = 0;
								if ((int)(long)v4 == 0) Cfault = 1;
								else Cfault = 0;

								refresh_value_nodeInfotable();
								st_free_table(&ValuingNode);
								st_free_table(&JTable);
								st_free_table(&implied_set);
								st_free_table(&JTableTmp);
								st_free_table(&LearnN);
								st_free_table(&Ccantuse);
								
								replaced_count_NAR++;
								node->Simp = 1;
#if SIMPLIFY								
								NodeARSimplify(node, A, Afault, B, Bfault, C, Cfault, direct);
#endif								
								return 2;
							}
						}
					}
					
				}
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				// cout << "---------ValuingNode_delete1" << endl;
				// PrintItems(ValuingNode);
				if (!st_is_member(LearnN, (char *)B)) st_insert(LearnN, (char *)B, (char *)v2);
			}
			// cout << "---------ValuingNode_delete2" << endl;
			// PrintItems(ValuingNode);
		}
		st_free_table(&Ccantuse);
		UpdateJTable(JTableTmp);
		// cout << "---------ValuingNode_delete" << endl;
		// PrintItems(ValuingNode);

	}
	refresh_value_nodeInfotable();
	st_free_table(&ValuingNode);
	st_free_table(&JTable);
	st_free_table(&implied_set);
	st_free_table(&JTableTmp);	
	st_free_table(&LearnN);
	st_free_table(&fanin_cone_fi0);
	st_free_table(&fanin_cone_fi1);
	st_free_table(&fanin_cone_fi2);
	return 1;
}

int FindCandidateB(st_table* nofanin, Xmg_Obj_t* node, st_table* CandA, st_table* zero_set, st_table* fanout_cone, bool direct)
{
	// cout << "Start to find nf2, nf3" << endl;
    st_generator *stgen, *stgenn, *stgennn;
	Xmg_Obj_t *A, *B, *C, *znode, *fi, *fo;
	char *v1, *v2, *v3, *v4, *v5;
	bool Afault, Bfault, Cfault, st;
	st_table *implied_set, *JTableTmp, *LearnN, *Ccantuse;
	node_info *ir, *ir_fi;
	bool phase, onlyoneSFoFi, todo, found;
	unsigned i, ncount;
	bool AisFanin = 0, BisFanin = 0;

	// **************************** << NOTE >> ****************************
	// we can't pick the node which is in the fanin-cone in any fanins of the Target node (Recursive delete will cause error)
	st_table *fanin_cone_fi0 = NULL;
	st_table *fanin_cone_fi1 = NULL;
	st_table *fanin_cone_fi2 = NULL;
	fanin_cone_fi0 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone_fi1 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone_fi2 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	for (i = 0; i < node->vFanins.size(); i++) {
		fi = Gntk->Obj_Ptr[node->vFanins[i]];
		if (fi->vFanouts.size() == 1) {
			if (i == 0)
				fanin_cone_fi0 = FindFaninConeDfs(fi);
			else if (i == 1)
				fanin_cone_fi1 = FindFaninConeDfs(fi);
			else
				fanin_cone_fi2 = FindFaninConeDfs(fi);
		}
	}

	// if (st_is_member(fanin_cone_fi2, (char *)Gntk->Obj_Ptr[0])) {
	// 	cout << "Check!!!" << endl;
	// 	return;
	// }
		

	// Update zero_set, ValuingNode, and JTable;
	ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
    //ValuingNodeC = st_init_table(st_ptrcmp, st_ptrhash);
	JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	st_foreach_item(zero_set, stgen, (char **)&znode, &v1){
		//if (st_is_member(nofanin, (char *)znode))  continue;
		if (znode->Type == PO)	continue;
        ir = &nodeInfoTable[znode->Id];
		ir->value = (tval_t)(long)v1;
		st_insert(ValuingNode, (char *)znode, (char *)v1);
	}

    st_foreach_item(ValuingNode, stgen, (char **)&znode, &v1){
        ir = &nodeInfoTable[znode->Id];
		if(znode->Type != NODE) continue;
		if(ir->value == _x_) continue;

		int zero_count = 0;
		int one_count = 0;
		int x_count = 0;
		for(i = 0; i < znode->vFanins.size(); i++){
			fi = Gntk->Obj_Ptr[znode->vFanins[i]];
			ir_fi = &nodeInfoTable[fi->Id];
			if(ir_fi->value == _x_){
				x_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _1_)){
				one_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _0_)){
				zero_count++;
			}
            if(zero_count >= 2 && ir->value == _0_) continue;
            if(one_count >= 2 && ir->value == _1_) continue;
		}
        if (x_count == 3 || (x_count >= 2 && ir->value == _0_ && zero_count == 1) || (x_count >= 2 && ir->value == _1_ && one_count == 1))
            st_insert(JTable, (char *)znode, (char *)v1);
	}
	
	// backup
	implied_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	BackupImplied(implied_set);
	JTableTmp = st_copy(JTable);

    int cnt = 0;
    for (unsigned i = 0; i < node->vFanins.size(); i++) {
        Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        if (fi->vFanouts.size() == 1)   cnt++;
    }
    if (cnt > 1)   onlyoneSFoFi = 0; // more than one fanin
    else    onlyoneSFoFi = 1;    // just one fanin

	
	LearnN = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
 
	// cout << "---------ValuingNode0" << endl;
	// PrintItems(ValuingNode);
	// cout << "---------CandA" << endl;
	// PrintItems(CandA);
	// cout << "---------implied_set" << endl;
	// PrintItems(implied_set);
	// cout << "---------JTable0" << endl;
	// PrintItems(JTable);
	
	// Foreach Candidate A
	st_foreach_item(CandA, stgen, (char **)&A, &v1){

		Ccantuse = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);

		if (AisFanin || BisFanin) {
			cout << "[Unexpected error]" << endl;
			cout << "AisFanin: " << AisFanin << ", BisFanin: " << BisFanin << endl;
			getchar();
			return 1;
		}

		// if (st_is_member(LearnN, (char *)A)) {
		// 	continue;
		// }

		if (st_is_member(fanin_cone_fi0, (char *)A))
			continue;

        for (unsigned j = 0; j < node->vFanins.size(); j++) {
            Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[j]];
            if (fi == A) {
                AisFanin = 1;
                break;
            }
        }
				
		if ((int)(long)v1 == 1) Afault = 1;
		else Afault = 0;

		// cout << "A is: " << id_name[A->Id] << endl;
		st = ActivateImplyNAR(A, Afault);
		
		if (st == 1){
			// perform logic implication for node
			st_free_table(&JTable);
			JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			
			st = imply_backward(node, node);
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			
			if (st == 1){
				for(unsigned k = 0; k < node->vFanouts.size(); k++){
                    fo = Gntk->Obj_Ptr[node->vFanouts[k]];
                    st = imply_forward(node, fo);
                    if(st == 0){
                        break;
                    }
					
                }
				if (st == 1){
#if RECUR
					st = RecurImply();
#endif
				}
			}	
		}
		
		if (st == 0) {

			if (direct == 0){
				if (Afault == 1) phase = 0;
				else phase = 1;
			}else{
				if (Afault == 1) phase = 1;
				else phase = 0;
			}		
			refresh_value_nodeInfotable();
            st_free_table(&ValuingNode);
            st_free_table(&JTable);
            st_free_table(&implied_set);
			st_free_table(&JTableTmp);
			st_free_table(&LearnN);

			replaced_count++;
#if SIMPLIFY 
			merge_simplify(node, A, phase);
#endif
			return 2;
		}	

		// **************************** << We don't choose the fanin of the target node as nf2, nf3 >> ****************************
		st_generator* stgennnn;
		Xmg_Obj_t* temp_node;
		char* value;
		st_foreach_item(ValuingNode, stgennnn, (char **)&temp_node, &value){
			if (st_is_member(nofanin, (char*)temp_node) || temp_node == node 
			|| st_is_member(fanout_cone, (char *)temp_node) || temp_node->Type == Zombie
			|| (st_is_member(fanin_cone_fi0, (char *)temp_node) || st_is_member(fanin_cone_fi1, (char *)temp_node) || st_is_member(fanin_cone_fi2, (char *)temp_node))){
				ir = &nodeInfoTable[temp_node->Id];
				if (temp_node->Id == 0)
					ir->value = _0_;
				else if (temp_node->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&temp_node, NIL(char *));
			}
			if (st_is_member(implied_set, (char*)temp_node)){
				nodeInfoTable[temp_node->Id].value = (tval_t)(long)value;
			}
		}

		// Update & Find Candidate B
		st_foreach_item(ValuingNode, stgenn, (char **)&B, &v2){
			if (B->Type == PO)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
			if (B == A)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}

			if (!st_is_member(implied_set, (char *)B)){
				
                for (unsigned i = 0; i < node->vFanins.size(); i++) {
                    Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                    if (fi == B) {
                        BisFanin = 1;
                        break;
                    }
                }

				todo = 1;

				if (todo == 1){
						
						if (st_lookup(CandA, (char *)B, &v3)){
							if ((int)(long)v2 != (int)(long)v3){

								if ((int)(long)v2 == 0) Bfault = 1;
								else Bfault = 0;

								/* After Oral */
								C = Gntk->Obj_Ptr[0];	// nf3 will be constant 0
								Cfault = 1;
								/* After Oral */

								refresh_value_nodeInfotable();
								st_free_table(&ValuingNode);
								st_free_table(&JTable);
								st_free_table(&implied_set);
								st_free_table(&JTableTmp);
								st_free_table(&LearnN);
								
								replaced_count_NAR++;
								node->Simp = 1;
#if SIMPLIFY								
								NodeARSimplify(node, A, Afault, B, Bfault, C, Cfault, direct);
#endif								
								return 2;
							}
						}
					
				}
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				// cout << "---------ValuingNode_delete1" << endl;
				// PrintItems(ValuingNode);
				if (!st_is_member(LearnN, (char *)B)) st_insert(LearnN, (char *)B, (char *)v2);
			}
			// cout << "---------ValuingNode_delete2" << endl;
			// PrintItems(ValuingNode);
		}
		UpdateJTable(JTableTmp);
		// cout << "---------ValuingNode_delete" << endl;
		// PrintItems(ValuingNode);
	}
	refresh_value_nodeInfotable();
	st_free_table(&ValuingNode);
	st_free_table(&JTable);
	st_free_table(&implied_set);
	st_free_table(&JTableTmp);	
	st_free_table(&LearnN);
	st_free_table(&fanin_cone_fi0);
	st_free_table(&fanin_cone_fi1);
	st_free_table(&fanin_cone_fi2);
	return 1;
}


int FindCandidateB_AND(st_table* nofanin, Xmg_Obj_t* node, st_table* CandA, st_table* zero_set, st_table* fanout_cone, bool direct)
{
	// cout << "Start to find nf2 ... AND" << endl;
    st_generator *stgen, *stgenn, *stgennn;
	Xmg_Obj_t *A, *B, *znode, *fi, *fo;
	char *v1, *v2, *v3, *v4, *v5;
	bool Afault, Bfault, Cfault, st;
	st_table *implied_set, *JTableTmp, *LearnN, *Ccantuse;
	node_info *ir, *ir_fi;
	bool phase, onlyoneSFoFi, todo, found;
	unsigned i, ncount;
	bool AisFanin = 0, BisFanin = 0;

	// **************************** << NOTE >> ****************************
	// we can't pick the node which is in the fanin-cone in any fanins of the Target node (Recursive delete will cause error)
	st_table *fanin_cone_fi0 = NULL;
	st_table *fanin_cone_fi1 = NULL;
	fanin_cone_fi0 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone_fi1 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	int temp_i = 0;
	for (i = 0; i < node->vFanins.size(); i++) {
		if (node->vFanins[i] <= 1)	continue;
		fi = Gntk->Obj_Ptr[node->vFanins[i]];
		if (fi->vFanouts.size() == 1) {
			if (temp_i == 0)
				fanin_cone_fi0 = FindFaninConeDfs(fi);
			else if (temp_i == 1)
				fanin_cone_fi1 = FindFaninConeDfs(fi);
			temp_i++;
		}
	}

	// Update zero_set, ValuingNode, and JTable;
	ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
    //ValuingNodeC = st_init_table(st_ptrcmp, st_ptrhash);
	JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	st_foreach_item(zero_set, stgen, (char **)&znode, &v1){
		if (st_is_member(nofanin, (char *)znode))  continue;
		if (znode->Type == PO)	continue;
        ir = &nodeInfoTable[znode->Id];
		ir->value = (tval_t)(long)v1;
		st_insert(ValuingNode, (char *)znode, (char *)v1);
	}

    st_foreach_item(ValuingNode, stgen, (char **)&znode, &v1){
        ir = &nodeInfoTable[znode->Id];
		if(znode->Type != NODE) continue;
		if(ir->value == _x_) continue;

		int zero_count = 0;
		int one_count = 0;
		int x_count = 0;
		for(i = 0; i < znode->vFanins.size(); i++){
			fi = Gntk->Obj_Ptr[znode->vFanins[i]];
			ir_fi = &nodeInfoTable[fi->Id];
			if(ir_fi->value == _x_){
				x_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _1_)){
				one_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _0_)){
				zero_count++;
			}
            if(zero_count >= 2 && ir->value == _0_) continue;
            if(one_count >= 2 && ir->value == _1_) continue;
		}
        if (x_count == 3 || (x_count >= 2 && ir->value == _0_ && zero_count == 1) || (x_count >= 2 && ir->value == _1_ && one_count == 1))
            st_insert(JTable, (char *)znode, (char *)v1);
	}
	
	// backup
	implied_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	BackupImplied(implied_set);
	JTableTmp = st_copy(JTable);

    int cnt = 0;
    for (unsigned i = 0; i < node->vFanins.size(); i++) {
        Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        if (fi->vFanouts.size() == 1)   cnt++;
    }
    if (cnt > 1)   onlyoneSFoFi = 0; // more than one fanin
    else    onlyoneSFoFi = 1;    // just one fanin

	
	LearnN = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
 
	// cout << "---------ValuingNode0" << endl;
	// PrintItems(ValuingNode);
	// cout << "---------CandA" << endl;
	// PrintItems(CandA);
	// cout << "---------JTable0" << endl;
	// PrintItems(JTable);
	
	// Foreach Candidate A
	st_foreach_item(CandA, stgen, (char **)&A, &v1){

		Ccantuse = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);

		if (AisFanin || BisFanin) {
			cout << "[Unexpected error]" << endl;
			cout << "AisFanin: " << AisFanin << ", BisFanin: " << BisFanin << endl;
			getchar();
			return 1;
		}

		if (st_is_member(LearnN, (char *)A)) {

			continue;
		}

		if (st_is_member(fanin_cone_fi0, (char *)A))
			continue;

        // bool onlyfanouttonode = 0;
		// // eliminate the node which only fanouts to "node"
		// if (onlyoneSFoFi == 1){
        //     for (unsigned i = 0; i < node->vFanins.size(); i++) {
        //         Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        //         if (fi == A && fi->vFanouts.size() == 1)   {
        //             onlyfanouttonode = 1;
        //             break;
        //         }
        //     }
		// }
        // if (onlyfanouttonode)   continue;

        for (unsigned j = 0; j < node->vFanins.size(); j++) {
            Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[j]];
            if (fi == A) {
                AisFanin = 1;
                break;
            }
        }
				
		if ((int)(long)v1 == 1) Afault = 1;
		else Afault = 0;

		// cout << "---------ValuingNode0.5" << endl;
		// PrintItems(ValuingNode);
		
		// cout << "A is: " << id_name[A->Id] << endl;
		st = ActivateImplyNAR(A, Afault);
		// cout << "---------ValuingNode1" << endl;
		// PrintItems(ValuingNode);
		// cout << "---------JTable2" << endl;
		// PrintItems(JTable);
		
		if (st == 1){
			// perform logic implication for node
			st_free_table(&JTable);
			JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			
			st = imply_backward(node, node);
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			
			if (st == 1){
				for(unsigned k = 0; k < node->vFanouts.size(); k++){
                    fo = Gntk->Obj_Ptr[node->vFanouts[k]];
                    st = imply_forward(node, fo);
                    if(st == 0){
                        break;
                    }
					// cout << "---------ValuingNode2.25" << endl;
					// PrintItems(ValuingNode);
                }
				if (st == 1){
#if RECUR
					st = RecurImply();
#endif
				}
			}	
		}
		// cout << "st: " << st << endl;
		// cout << "---------JTable3" << endl;
		// PrintItems(JTable);
		if (st == 0) {
			// refresh_value_nodeInfotable();
            // st_free_table(&ValuingNode);
			// LoadNodeValue(implied_set);
			
            // // st_free_table(&JTable);
            // // st_free_table(&implied_set);
			// // st_free_table(&JTableTmp);
			// // st_free_table(&LearnN);
			// //ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			// ValuingNode = st_copy(implied_set);
			// UpdateJTable(JTableTmp);
			// // cout << "Countinue1" << endl;
			// continue;

			if (direct == 0){
				if (Afault == 1) phase = 0;
				else phase = 1;
			}else{
				if (Afault == 1) phase = 1;
				else phase = 0;
			}		
			refresh_value_nodeInfotable();
            st_free_table(&ValuingNode);
            st_free_table(&JTable);
            st_free_table(&implied_set);
			st_free_table(&JTableTmp);
			st_free_table(&LearnN);

			replaced_count++;
			// cout << "This node can be merge in NodeAR.cpp!!(AND)" << endl;
			// cout << "Node is: " << id_name[node->Id] << ", Node Nf1 is: " << id_name[A->Id] << endl;
#if SIMPLIFY 
			merge_simplify(node, A, phase);
#endif
			return 2;
		}	
		// if (st == 0){
		// 	if (direct == 0){
		// 		if (Afault == 1) phase = 0;
		// 		else phase = 1;
		// 	}else{
		// 		if (Afault == 1) phase = 1;
		// 		else phase = 0;
		// 	}		
			// refresh_value_nodeInfotable();
            // st_free_table(&ValuingNode);
            // st_free_table(&JTable);
            // st_free_table(&implied_set);
			// st_free_table(&JTableTmp);
			// st_free_table(&LearnN);

		// 	replaced_count++;
		// 	cout << "This node can be merge in NodeAR.cpp!!" << endl;
		// 	cout << "Node is: " << id_name[node->Id] << ", Node Nf1 is: " << id_name[A->Id] << endl; 
		// 	merge_simplify(node, A, phase);
 
		// 	return;
		// }

		// cout << "---------ValuingNode2.5" << endl;
		// PrintItems(ValuingNode);

		// **************************** << We don't choose the fanin of the target node as nf2, nf3 >> ****************************
		st_generator* stgennnn;
		Xmg_Obj_t* temp_node;
		char* value;
		st_foreach_item(ValuingNode, stgennnn, (char **)&temp_node, &value){
			// if (temp_node->Type != Zombie && temp_node->vFanins.size() == 0 && temp_node->vFanouts.size() == 0) {
			// 	temp_node->Type = Zombie;
			// }
			// if (temp_node->Type == Zombie) {
			// 	cout << "This node \" " << id_name[temp_node->Id] << " \" is Zombie!! ... AND" << endl;
			// }
			// if (id_name[temp_node->Id] == "n299") {
			// 	cout << "This node is \"n299\", and the Type is: " << temp_node->Type << endl; 
			// }
			if (st_is_member(nofanin, (char*)temp_node) || temp_node == node 
			|| st_is_member(fanout_cone, (char *)temp_node) || temp_node->Type == Zombie
			|| (st_is_member(fanin_cone_fi0, (char *)temp_node) || st_is_member(fanin_cone_fi1, (char *)temp_node))){
				ir = &nodeInfoTable[temp_node->Id];
				if (temp_node->Id == 0)
					ir->value = _0_;
				else if (temp_node->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&temp_node, NIL(char *));
				// if (temp_node->Type == Zombie) {
				// 	cout << "This node \" " << id_name[temp_node->Id] << " \" is Zombie!! ... AND" << endl;
				// }
			}
			if (st_is_member(implied_set, (char*)temp_node)){
				nodeInfoTable[temp_node->Id].value = (tval_t)(long)value;
			}
		}

		// cout << "---------ValuingNode3" << endl;
		// PrintItems(ValuingNode);
		// cout << "---------implied_set" << endl;
		// PrintItems(implied_set);
		// cout << "---------JTable4" << endl;
		// PrintItems(JTable);

		// refresh_value_nodeInfotable();
		// st_free_table(&ValuingNode);
		// LoadNodeValue(implied_set);
		// ValuingNode = st_copy(implied_set);

		// cout << "---------ValuingNode_after_copy" << endl;
		// PrintItems(ValuingNode);

		// found = 0; ncount = 0;
        // ValuingNodeC = st_copy(ValuingNodeB);

		// if (st_count(ValuingNode) < 3){}	continue;

		// Update & Find Candidate B
		st_foreach_item(ValuingNode, stgenn, (char **)&B, &v2){
			// cout << "---------Ccantuse0" << endl;
			// PrintItems(Ccantuse);
			if (B->Type == PO)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
			if (B == A)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
#if DEPTHPRESERVED
			// depth preservation
			if (B->Level >= node->Level)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
#endif
			// if (B->op_type == 1)	continue;
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			// cout << "---------Ccantuse2" << endl;
			// PrintItems(Ccantuse);
			// cout << "---------Ccantuse1" << endl;
			// PrintItems(Ccantuse);
			

			// cout << "---------Ccantuse2" << endl;
			// PrintItems(Ccantuse);
			
			if (!st_is_member(implied_set, (char *)B)){
				
                for (unsigned i = 0; i < node->vFanins.size(); i++) {
                    Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                    if (fi == B) {
                        BisFanin = 1;
                        break;
                    }
                }

				todo = 1;

				// Find Candidate B
                // B cannot be the only one fanin which only fanouts to node
				// int cnt = 0;
                // if (BisFanin == 1){
                //     if (B->vFanouts.size() == 1) {
                //         for (unsigned i = 0; i < node->vFanins.size(); i++) {
                //             Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                //             if (fi->vFanouts.size() == 1)   cnt++;
                //         }
                //     }   
                // }
                // if (cnt == 1)   todo = 0;

				if (todo == 1){
						// cout << "---------Ccantuse3" << endl;
						// PrintItems(Ccantuse);
						// cout << "Nf1: " << id_name[A->Id] << ", Nf2: " << id_name[B->Id] << ", Nf3: " << id_name[C->Id] << endl;
						
						if (st_lookup(CandA, (char *)B, &v3)){
							if ((int)(long)v2 != (int)(long)v3){

								if ((int)(long)v2 == 0) Bfault = 1;
								else Bfault = 0;

								refresh_value_nodeInfotable();
								st_free_table(&ValuingNode);
								st_free_table(&JTable);
								st_free_table(&implied_set);
								st_free_table(&JTableTmp);
								st_free_table(&LearnN);
								
								replaced_count_NAR_AND++;
								node->Simp = 1;
#if SIMPLIFY								
								NodeARSimplify_TWOINPUTS(node, A, Afault, B, Bfault, direct, 0);
#endif								
								return 2;
							}
						}
					
				}
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				// cout << "---------ValuingNode_delete1" << endl;
				// PrintItems(ValuingNode);
				if (!st_is_member(LearnN, (char *)B)) st_insert(LearnN, (char *)B, (char *)v2);
			}
			// cout << "---------ValuingNode_delete2" << endl;
			// PrintItems(ValuingNode);
		}
		UpdateJTable(JTableTmp);
		// cout << "---------ValuingNode_delete" << endl;
		// PrintItems(ValuingNode);
	}	
	refresh_value_nodeInfotable();
	st_free_table(&ValuingNode);
	st_free_table(&JTable);
	st_free_table(&implied_set);
	st_free_table(&JTableTmp);	
	st_free_table(&LearnN);
	st_free_table(&fanin_cone_fi0);
	st_free_table(&fanin_cone_fi1);
	return 1;
}

int FindCandidateB_OR(st_table* nofanin, Xmg_Obj_t* node, st_table* CandA, st_table* zero_set, st_table* fanout_cone, bool direct)
{
	// cout << "Start to find nf2 ... OR" << endl;
    st_generator *stgen, *stgenn, *stgennn;
	Xmg_Obj_t *A, *B, *znode, *fi, *fo;
	char *v1, *v2, *v3, *v4, *v5;
	bool Afault, Bfault, Cfault, st;
	st_table *implied_set, *JTableTmp, *LearnN, *Ccantuse;
	node_info *ir, *ir_fi;
	bool phase, onlyoneSFoFi, todo, found;
	unsigned i, ncount;
	bool AisFanin = 0, BisFanin = 0;

	// **************************** << NOTE >> ****************************
	// we can't pick the node which is in the fanin-cone in any fanins of the Target node (Recursive delete will cause error)
	st_table *fanin_cone_fi0 = NULL;
	st_table *fanin_cone_fi1 = NULL;
	fanin_cone_fi0 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone_fi1 = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	int temp_i = 0;
	for (i = 0; i < node->vFanins.size(); i++) {
		if (node->vFanins[i] <= 1)	continue;
		fi = Gntk->Obj_Ptr[node->vFanins[i]];
		if (fi->vFanouts.size() == 1) {
			if (temp_i == 0)
				fanin_cone_fi0 = FindFaninConeDfs(fi);
			else if (temp_i == 1)
				fanin_cone_fi1 = FindFaninConeDfs(fi);
			temp_i++;
		}
	}

	// Update zero_set, ValuingNode, and JTable;
	ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
    //ValuingNodeC = st_init_table(st_ptrcmp, st_ptrhash);
	JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	st_foreach_item(zero_set, stgen, (char **)&znode, &v1){
		if (st_is_member(nofanin, (char *)znode))  continue;
		if (znode->Type == PO)	continue;
        ir = &nodeInfoTable[znode->Id];
		ir->value = (tval_t)(long)v1;
		st_insert(ValuingNode, (char *)znode, (char *)v1);
	}

    st_foreach_item(ValuingNode, stgen, (char **)&znode, &v1){
        ir = &nodeInfoTable[znode->Id];
		if(znode->Type != NODE) continue;
		if(ir->value == _x_) continue;

		int zero_count = 0;
		int one_count = 0;
		int x_count = 0;
		for(i = 0; i < znode->vFanins.size(); i++){
			fi = Gntk->Obj_Ptr[znode->vFanins[i]];
			ir_fi = &nodeInfoTable[fi->Id];
			if(ir_fi->value == _x_){
				x_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _1_)){
				one_count++;
			}else if(ir_fi->value == node_value(Gntk, fi, znode, i, _0_)){
				zero_count++;
			}
            if(zero_count >= 2 && ir->value == _0_) continue;
            if(one_count >= 2 && ir->value == _1_) continue;
		}
        if (x_count == 3 || (x_count >= 2 && ir->value == _0_ && zero_count == 1) || (x_count >= 2 && ir->value == _1_ && one_count == 1))
            st_insert(JTable, (char *)znode, (char *)v1);
	}
	
	// backup
	implied_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	BackupImplied(implied_set);
	JTableTmp = st_copy(JTable);

    int cnt = 0;
    for (unsigned i = 0; i < node->vFanins.size(); i++) {
        Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        if (fi->vFanouts.size() == 1)   cnt++;
    }
    if (cnt > 1)   onlyoneSFoFi = 0; // more than one fanin
    else    onlyoneSFoFi = 1;    // just one fanin

	
	LearnN = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
 
	// cout << "---------ValuingNode0" << endl;
	// PrintItems(ValuingNode);
	// cout << "---------CandA" << endl;
	// PrintItems(CandA);
	// cout << "---------JTable0" << endl;
	// PrintItems(JTable);
	
	// Foreach Candidate A
	st_foreach_item(CandA, stgen, (char **)&A, &v1){

		Ccantuse = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);

		if (AisFanin || BisFanin) {
			cout << "[Unexpected error]" << endl;
			cout << "AisFanin: " << AisFanin << ", BisFanin: " << BisFanin << endl;
			getchar();
			return 1;
		}

		if (st_is_member(LearnN, (char *)A)) {
			continue;
		}

		if (st_is_member(fanin_cone_fi0, (char *)A))
			continue;

        // bool onlyfanouttonode = 0;
		// // eliminate the node which only fanouts to "node"
		// if (onlyoneSFoFi == 1){
        //     for (unsigned i = 0; i < node->vFanins.size(); i++) {
        //         Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
        //         if (fi == A && fi->vFanouts.size() == 1)   {
        //             onlyfanouttonode = 1;
        //             break;
        //         }
        //     }
		// }
        // if (onlyfanouttonode)   continue;

        for (unsigned j = 0; j < node->vFanins.size(); j++) {
            Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[j]];
            if (fi == A) {
                AisFanin = 1;
                break;
            }
        }
				
		if ((int)(long)v1 == 0) Afault = 0;
		else Afault = 1;
		// bool Activate_value = 0;
		// if (v1 == 0) Activate_value = 0;
		// else	Activate_value = 1;
		// cout << "---------ValuingNode0.5" << endl;
		// PrintItems(ValuingNode);
		
		// cout << "A is: " << id_name[A->Id] << endl;
		st = ActivateImplyNAR(A, Afault);
		// cout << "---------ValuingNode1" << endl;
		// PrintItems(ValuingNode);
		// cout << "---------JTable2" << endl;
		// PrintItems(JTable);
		
		if (st == 1){
			// perform logic implication for node
			st_free_table(&JTable);
			JTable = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			
			st = imply_backward(node, node);
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			
			if (st == 1){
				for(unsigned k = 0; k < node->vFanouts.size(); k++){
                    fo = Gntk->Obj_Ptr[node->vFanouts[k]];
                    st = imply_forward(node, fo);
                    if(st == 0){
                        break;
                    }
					// cout << "---------ValuingNode2.25" << endl;
					// PrintItems(ValuingNode);
                }
				if (st == 1){
#if RECUR
					st = RecurImply();
#endif
				}
			}	
		}
		// cout << "st: " << st << endl;
		// cout << "---------JTable3" << endl;
		// PrintItems(JTable);
		if (st == 0) {
			// refresh_value_nodeInfotable();
            // st_free_table(&ValuingNode);
			// LoadNodeValue(implied_set);
			
            // // st_free_table(&JTable);
            // // st_free_table(&implied_set);
			// // st_free_table(&JTableTmp);
			// // st_free_table(&LearnN);
			// //ValuingNode = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
			// ValuingNode = st_copy(implied_set);
			// UpdateJTable(JTableTmp);
			// // cout << "Countinue1" << endl;
			// continue;

			if (direct == 0){
				if (Afault == 0) phase = 0;
				else phase = 1;
			}else{
				if (Afault == 0) phase = 1;
				else phase = 0;
			}		
			refresh_value_nodeInfotable();
            st_free_table(&ValuingNode);
            st_free_table(&JTable);
            st_free_table(&implied_set);
			st_free_table(&JTableTmp);
			st_free_table(&LearnN);

			replaced_count++;
			// cout << "This node can be merge in NodeAR.cpp!!(OR)" << endl;
			// cout << "Node is: " << id_name[node->Id] << ", Node Nf1 is: " << id_name[A->Id] << endl;
#if SIMPLIFY 
			merge_simplify(node, A, phase);
#endif
			return 2;
		}	
		// if (st == 0){
		// 	if (direct == 0){
		// 		if (Afault == 1) phase = 0;
		// 		else phase = 1;
		// 	}else{
		// 		if (Afault == 1) phase = 1;
		// 		else phase = 0;
		// 	}		
			// refresh_value_nodeInfotable();
            // st_free_table(&ValuingNode);
            // st_free_table(&JTable);
            // st_free_table(&implied_set);
			// st_free_table(&JTableTmp);
			// st_free_table(&LearnN);

		// 	replaced_count++;
		// 	cout << "This node can be merge in NodeAR.cpp!!" << endl;
		// 	cout << "Node is: " << id_name[node->Id] << ", Node Nf1 is: " << id_name[A->Id] << endl; 
		// 	merge_simplify(node, A, phase);
 
		// 	return;
		// }

		// cout << "---------ValuingNode2.5" << endl;
		// PrintItems(ValuingNode);

		// **************************** << We don't choose the fanin of the target node as nf2, nf3 >> ****************************
		st_generator* stgennnn;
		Xmg_Obj_t* temp_node;
		char* value;
		st_foreach_item(ValuingNode, stgennnn, (char **)&temp_node, &value){
			// if (temp_node->Type != Zombie && temp_node->vFanins.size() == 0 && temp_node->vFanouts.size() == 0) {
			// 	temp_node->Type = Zombie;
			// }
			if (st_is_member(nofanin, (char*)temp_node) || temp_node == node 
				|| st_is_member(fanout_cone, (char *)temp_node) || temp_node->Type == Zombie
				|| (st_is_member(fanin_cone_fi0, (char *)temp_node) || st_is_member(fanin_cone_fi1, (char *)temp_node))){
				ir = &nodeInfoTable[temp_node->Id];
				if (temp_node->Id == 0)
					ir->value = _0_;
				else if (temp_node->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&temp_node, NIL(char *));
				// if (temp_node->Type == Zombie) {
				// 	cout << "This node \" " << id_name[temp_node->Id] << " \" is Zombie!! ... OR" << endl;
				// }
			}
			if (st_is_member(implied_set, (char*)temp_node)){
				nodeInfoTable[temp_node->Id].value = (tval_t)(long)value;
			}
		}

		// cout << "---------ValuingNode3" << endl;
		// PrintItems(ValuingNode);
		// cout << "---------implied_set" << endl;
		// PrintItems(implied_set);
		// cout << "---------JTable4" << endl;
		// PrintItems(JTable);

		// refresh_value_nodeInfotable();
		// st_free_table(&ValuingNode);
		// LoadNodeValue(implied_set);
		// ValuingNode = st_copy(implied_set);

		// cout << "---------ValuingNode_after_copy" << endl;
		// PrintItems(ValuingNode);

		// found = 0; ncount = 0;
        // ValuingNodeC = st_copy(ValuingNodeB);

		// if (st_count(ValuingNode) < 3){}	continue;

		// Update & Find Candidate B
		st_foreach_item(ValuingNode, stgenn, (char **)&B, &v2){
			// cout << "---------Ccantuse0" << endl;
			// PrintItems(Ccantuse);
			if (B->Type == PO)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
			if (B == A)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
#if DEPTHPRESERVED
			// depth preservation
			if (B->Level >= node->Level)	{
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				continue;
			}
#endif

			// if (B->op_type == 1)	continue;
			// cout << "---------ValuingNode2" << endl;
			// PrintItems(ValuingNode);
			// cout << "---------Ccantuse2" << endl;
			// PrintItems(Ccantuse);
			// cout << "---------Ccantuse1" << endl;
			// PrintItems(Ccantuse);
			

			// cout << "---------Ccantuse2" << endl;
			// PrintItems(Ccantuse);
			
			if (!st_is_member(implied_set, (char *)B)){
				
                for (unsigned i = 0; i < node->vFanins.size(); i++) {
                    Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                    if (fi == B) {
                        BisFanin = 1;
                        break;
                    }
                }

				todo = 1;

				// Find Candidate B
                // B cannot be the only one fanin which only fanouts to node
				// int cnt = 0;
                // if (BisFanin == 1){
                //     if (B->vFanouts.size() == 1) {
                //         for (unsigned i = 0; i < node->vFanins.size(); i++) {
                //             Xmg_Obj_t* fi = Gntk->Obj_Ptr[node->vFanins[i]];
                //             if (fi->vFanouts.size() == 1)   cnt++;
                //         }
                //     }   
                // }
                // if (cnt == 1)   todo = 0;

				if (todo == 1){
						// cout << "---------Ccantuse3" << endl;
						// PrintItems(Ccantuse);
						// cout << "Nf1: " << id_name[A->Id] << ", Nf2: " << id_name[B->Id] << ", Nf3: " << id_name[C->Id] << endl;
						
						if (st_lookup(CandA, (char *)B, &v3)){
							if ((int)(long)v2 != (int)(long)v3){

								if ((int)(long)v2 == 1) Bfault = 0;
								else Bfault = 1;

								refresh_value_nodeInfotable();
								st_free_table(&ValuingNode);
								st_free_table(&JTable);
								st_free_table(&implied_set);
								st_free_table(&JTableTmp);
								st_free_table(&LearnN);
								
								replaced_count_NAR_OR++;
								node->Simp = 1;
#if SIMPLIFY								
								NodeARSimplify_TWOINPUTS(node, A, Afault, B, Bfault, direct, 1);
#endif
								return 2;
							}
						}
					
				}
				ir = &nodeInfoTable[B->Id];
				if (B->Id == 0)
					ir->value = _0_;
				else if (B->Id == 1)
					ir->value = _1_;
				else
					ir->value = _x_;
				st_delete(ValuingNode, (char **)&B, NIL(char *));
				if (st_is_member(implied_set, (char*)B)){
					nodeInfoTable[B->Id].value = (tval_t)(long)v2;
				}
				// cout << "---------ValuingNode_delete1" << endl;
				// PrintItems(ValuingNode);
				if (!st_is_member(LearnN, (char *)B)) st_insert(LearnN, (char *)B, (char *)v2);
			}
			// cout << "---------ValuingNode_delete2" << endl;
			// PrintItems(ValuingNode);
		}
		UpdateJTable(JTableTmp);
		// cout << "---------ValuingNode_delete" << endl;
		// PrintItems(ValuingNode);
	}	
	refresh_value_nodeInfotable();
	st_free_table(&ValuingNode);
	st_free_table(&JTable);
	st_free_table(&implied_set);
	st_free_table(&JTableTmp);	
	st_free_table(&LearnN);
	st_free_table(&fanin_cone_fi0);
	st_free_table(&fanin_cone_fi1);
	return 1;
}

void NodeAR (Xmg_Obj_t* node, st_table* one_set, st_table* zero_set, bool direct, vector<SideInput_Pair_t> st_sideinputpair, int operation) {
    Xmg_Obj_t *fi, *fo, *onode, *pnode1, *pnode2;
	bool todo;
	st_table *candidate_set_A = NULL;
	char *v1;
	st_generator *stgen;
	st_table *fanin_cone = NULL;
	st_table *fanout_cone = NULL;
	st_table *nofanin = NULL;
	st_table *st_set = NULL;
	node_info *ir; 

	candidate_set_A = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanout_cone = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	fanin_cone = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	nofanin = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	st_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
    fanout_cone = FindFanoutConeDfs(node);
	fanin_cone = FindFaninConeDfs(node);
    // **************************** << Decide whether to perform NAR in this node >> ****************************
    // for optimization, we only perform NAR on the node that has at least one single-fanout fanin
	todo = 0;
    for (unsigned i = 0; i < node->vFanins.size(); i++) {
        fi = Gntk->Obj_Ptr[node->vFanins[i]];
        if (fi->vFanouts.size() == 1 && fi->Type == NODE){
            todo = 1;
            break;
        }
    }
	if (todo == 0) return;
	// **************************** << We don't choose the fanin of the target node as nf1, nf2, nf3 >> ****************************
    for (unsigned i = 0; i < node->vFanins.size(); i++) {
        fi = Gntk->Obj_Ptr[node->vFanins[i]];
		ir = &nodeInfoTable[fi->Id];
        st_insert(nofanin, (char *)fi, (char *)1);
    }
	// **************************** << We don't choose the node which NodeMGR doesn't allow >> ****************************
	st_set = st_init_table((int(*)())st_ptrcmp, (int(*)())st_ptrhash);
	
	// for(unsigned i = 0; i < node->vFanouts.size(); i++){
	// 	fo = Gntk->Obj_Ptr[node->vFanouts[i]];
	// 	for(unsigned j = 0; j < fo->vFanins.size(); j++){
	// 		fi = Gntk->Obj_Ptr[fo->vFanins[j]];
	// 		if(fi != node) st_insert(st_set, (char *)fi, (char *)1);
	// 	}
	// }
	// for(vector<SideInput_Pair_t>::iterator iter = st_sideinputpair.begin(); iter != st_sideinputpair.end(); iter++){
	// 	if (iter->index2 != -1) {	// MAJ
	// 		pnode1 = Gntk->Obj_Ptr[iter->pnode1];
	// 		pnode2 = Gntk->Obj_Ptr[iter->pnode2];
	// 		st_insert(st_set, (char *)pnode1, (char *)1);
	// 		st_insert(st_set, (char *)pnode2, (char *)1);
	// 	} else if (iter->index2 == -1) {	// XOR
	// 		pnode1 = Gntk->Obj_Ptr[iter->pnode1];
	// 		st_insert(st_set, (char *)pnode1, (char *)1);
	// 	}
	// }
    // **************************** << Find Candidate A, i.e., find f1 >> ****************************
	// find each node that is in one_set but not in zero_set, 1 ---> create candidate_set_A
	st_foreach_item(one_set, stgen, (char **)&onode, &v1){
		if (onode->Type == PO) continue;
		if (st_is_member(zero_set, (char *)onode)) continue;
        if (st_is_member(fanin_cone, (char *)onode))  continue;
		if (st_is_member(nofanin, (char *)onode))  continue;
		if (st_is_member(fanout_cone, (char *)onode)) continue;

#if DEPTHPRESERVED
		// depth preservation
		if (onode->Level >= node->Level)	continue;
#endif

		st_insert(candidate_set_A, (char *)onode, (char *)v1);
	}
    // **************************** << Find Candidate B and Candidate C, i.e., find f2 and f3 >> ****************************
	int flag = 0;	// 0: initial flag value, 1: NAR fail, 2: NAR success
	// if (st_count(candidate_set_A) != 0 && operation == 2) flag = FindCandidateB(nofanin, node, candidate_set_A, zero_set, fanout_cone, direct);
	// if (flag != 2){
	// 	// cout << "test000" << endl;
	// 	if (st_count(candidate_set_A) != 0 && operation == 2) {
	// 		// cout << "test111" << endl;
	// 		flag = FindCandidateBC(nofanin, node, candidate_set_A, zero_set, fanout_cone, direct);
	// 	}
	// }
	if (st_count(candidate_set_A) != 0 && operation == 2) flag = FindCandidateBC(nofanin, node, candidate_set_A, zero_set, fanout_cone, direct);
	if (st_count(candidate_set_A) != 0 && operation == 0) flag = FindCandidateB_AND(nofanin, node, candidate_set_A, zero_set, fanout_cone, direct);
	if (st_count(candidate_set_A) != 0 && operation == 1) flag = FindCandidateB_OR(nofanin, node, candidate_set_A, zero_set, fanout_cone, direct);

	st_free_table(&candidate_set_A);
	st_free_table(&fanin_cone);
	st_free_table(&fanout_cone);
	st_free_table(&nofanin);
	st_free_table(&st_set);
}