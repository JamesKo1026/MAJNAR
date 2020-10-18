#include "ReadNtk.h"
#define CHECK 0

bool Fg_mdl = false;
bool Fg_in = false;
bool Fg_out = false;
bool Fg_wire = false;
bool Fg_func = false;
bool Fg_comment_slash = false;
bool Fg_comment_star = false;
bool Fg_semi = false;
bool Fg_fout = false;
bool Fg_compl = false;
bool Fg_fin = false;

int pre_fanin;

//***********************************************************
// Read Given XMG
//***********************************************************
bool ReadNtk(char *filename)
{
	cout << "In ReadNtk.cpp" << endl;
	ifstream fin(filename, ios::in | ios::binary);
	if(!fin)
	{
		cout << "ReadNtk(): Input file is not found. (Fail in ReadNtk.cpp)" << endl;
		fin.close();
		return false;
	}
	
	// Allocate the object
	Ontk = new Xmg_Ntk_t;
	Gntk = new Xmg_Ntk_t;
	
	cout << "----- First Phase Start -----" << endl;
	// first phase: compute number of PI, PO, NODE, and construct ID+Name mapping table

	char del[] = " ,=()\t";
	char *token;
	char s[200];
	int i;
	
	name_id["0"] = id_index;
	id_name[id_index] = "0";
	id_index++;
	name_id["1"] = id_index;
	id_name[id_index] = "1";
	id_index++;
	
	while(fin >> s){
		token = strtok(s,del);
		
		while(token != NULL){
			
			// Case: "1'b0;" or ";", discard the semicolon (only need "1'b0")
			for(i=0; token[i]!='\0'; i++){
				if(token[i]==';'){
					Fg_semi = true;
					token[i] = '\0';
					break;
				}else{
					Fg_semi = false;
				}
			}

			if(token[0] != '\0'){ // valid token
				if(Fg_in && !name_id[token]){
					Ontk->vPis_num++;
					Gntk->vPis_num++;
					name_id[token] = id_index;
					id_name[id_index] = token;
					PI_judge[id_index] = 1;
					id_index++;
				}else if(Fg_out && !name_id[token]){
					Ontk->vPos_num++;
					Gntk->vPos_num++;
					name_id[token] = id_index;
					id_name[id_index] = token;
					PO_judge[id_index] = 1;
					id_index++;
				}else if(Fg_wire && !name_id[token]){
					Ontk->vNode_num++;
					Gntk->vNode_num++;
					name_id[token] = id_index;
					id_name[id_index] = token;
					id_index++;
				}else if(Fg_mdl){
					Ontk->pName = token;
					Gntk->pName = token;
					Fg_mdl = false;
				}
			}	
	
			// // set flag
			if(!strcmp(token, "input"))
				Fg_in = true;
			else if(!strcmp(token, "output"))
				Fg_out = true;
			else if(!strcmp(token, "wire"))
				Fg_wire = true;
			else if(!strcmp(token, "module"))
				Fg_mdl = true; 
			if(Fg_semi){
			//	fCompl_index = 0;
				Fg_in = false;
				Fg_out = false;
				Fg_wire = false;
				Fg_mdl = false;
				Fg_func = false;
				Fg_fin = false;
				Fg_fout = false;
			//	fout_id = 0;
			}
			token = strtok(NULL,del);
		}
	}
	cout << "Name of this circuit: " << Ontk->pName << endl;
	cout << "# of inputs: " << Ontk->vPis_num << ", # of outputs: " << Ontk->vPos_num << endl;
	cout << "# of nodes: " << Ontk->vNode_num << endl;
	fin.clear();
	fin.seekg(0);
	cout << "----- First Phase End -----" << endl;
	
	cout << "----- Second Phase Start -----" << endl;
	// second phase: initialize data array 		
	
	size = Gntk->vPis_num + Gntk->vPos_num + Gntk->vNode_num+2; // 10 is additional space
	
	Ontk->Obj_Ptr = new Xmg_Obj_t* [size]; // QA: Is it useful?
	Gntk->Obj_Ptr = new Xmg_Obj_t* [size];
	EachNodeDom = new vector<int> [size];
	nodeInfoTable = new node_info [size];
	validTable = new int[size];
	
	//initialize validTable
	for(i = 0; i < size; i++)
		validTable[i] = 1;
	
	cout << "----- Second Phase End -----" << endl;
	
	cout << "----- Third Phase Start -----" << endl;
	// third phase: construct the DAG 
	
	Xmg_Obj_t node;
	vector<int> fos[size];
	
	
	int fout_id = 0;
	int fanin_id = 0;
	int fCompl_index = 0;
	int op = 0; // AND: 1, OR: 2 (for MIG)
	
	Xmg_Obj_t ConstZero;
	Xmg_Obj_t ConstOne;
	
	// construct 0 and 1 XMG objects
	ConstZero.Id = 0;
	ConstZero.Type = CONST;
	ConstZero.fCompl0 = 0;
	ConstZero.fCompl1 = 0;
	ConstZero.fCompl2 = 0;
	ConstZero.Level = 0;
	ConstZero.vFanins.clear();
	ConstZero.vFanouts.clear();
	
	ConstOne.Id = 1;
	ConstOne.Type = CONST;
	ConstOne.fCompl0 = 0;
	ConstOne.fCompl1 = 0;
	ConstOne.fCompl2 = 0;
	ConstOne.Level = 0;
	ConstOne.vFanins.clear();
	ConstOne.vFanouts.clear();
	
	Ontk->vPis.push_back(ConstZero);
	Ontk->vPis.push_back(ConstOne);
	Gntk->vPis.push_back(ConstZero);
	Gntk->vPis.push_back(ConstOne);

	int xor_cnt = 0;
	
	while(fin >> s){
		token = strtok(s,del);
		while(token != NULL){
			
			// Case: "1'b0;" or ";", discard the semicolon (only need "1'b0")
			for(i=0; token[i]!='\0'; i++){
				if(token[i]==';') {
					Fg_semi = true;
					token[i] = '\0';
					break;
				}
				else
					Fg_semi = false;
			}
			if(token[0] != '\0'){ // Valid token
				if(Fg_in){
					node.Id = name_id[token];
					node.Type = PI;
					node.fCompl0 = 0;
					node.fCompl1 = 0;
					node.fCompl2 = 0;
					Ontk->vPis.push_back(node);
					Gntk->vPis.push_back(node);
				}else if(Fg_out){ // QA : needed?
					//.......
				}else if(Fg_wire){ // QA : needed?
					//.......
				}else if(Fg_func){

					// ex: "assign n8 = (x2 & x3) | (x2 & ~x4) | (x3 & ~x4);", 
					// Fg_func and Fg_fout are both 1 means that it's the time to handle the node n8 (output of this func)
					// Fg_func is 1 but Fg_fout is 0 means that it's the time to handle rest of this function
					if(Fg_fout){
						fout_id = name_id[token];
						if(fout_id > 1){
							node.Id = fout_id;
							if(PO_judge[fout_id])
								node.Type = PO;
							else
								node.Type = NODE;
						}else{
							cout << "ReadNtk(): function output is error. (Fail in ReadNtk.cpp)" << endl;
							fin.close();
							return false;
						}
						Fg_fout = false;
					}else{
						if(!strcmp(token, "&")){
							op = AND;
						}else if(!strcmp(token, "|")){
							op = OR;
						}else if(!strcmp(token, "^")){
							xor_cnt++;
							node.op_type = 1; /** this node is a xor gate **/
						}else{
							unsigned inv;
							if(token[0] == '~'){
								inv = 1;
								for(i=0; token[i] != '\0'; i++) // format: ~abc -> abc
										token[i] = token[i+1];
							}else{
								inv = 0;
							}

							Fg_fin = true;
							if(!strcmp(token, "1'b1")){
								fanin_id = 1;
							}else if(!strcmp(token, "1'b0") && xor_cnt > 0){
								fanin_id = 0;
							}else{
								fanin_id = name_id[token];
							}
							//checking fanin (format: assign a = b op b, or a = (b & b) | (b & c) | (b & c))
							if(node.vFanins.size()>0 && node.vFanins.size()<2){ // checking fanin
								if(pre_fanin == fanin_id){
									cout << "Exception: " << id_name[node.Id] << " has redundant fanin." << endl;
									cout << id_name[pre_fanin] << " = " << id_name[fanin_id] << endl;
									getchar();
								}
							}
							pre_fanin = fanin_id;
							for(vector<int>::iterator iter = node.vFanins.begin(); iter != node.vFanins.end(); iter++){
								if(*iter == fanin_id){
									Fg_fin = false;
									break;
								}
							}	
							if(Fg_fin){ // valid fanin
								if (xor_cnt != 2) {
									node.vFanins.push_back(fanin_id); // fanin of node which id = fanin_id
									fos[fanin_id].push_back(fout_id); // store the fout_id as the fanin wire's fanout
									switch(fCompl_index){ // store inverter information
										case 0:
											node.fCompl0 = inv;
											break;
										case 1:
											node.fCompl1 = inv;
											break;
										case 2:
											node.fCompl2 = inv;
											break;
										default:
											break;
									}
									fCompl_index++;
								}
								
							}else{ // QA : needed?
								//....
							}
						}
					}
					// cout << "Node is: " << id_name[node.Id] << ", Id is: " << node.Id << endl;
					// cout << " Fanins(Id) are: ";
					// for (unsigned temp = 0; temp < node.vFanins.size(); temp++)
					// 	cout << node.vFanins[temp] << " ";
					// cout << endl;
				}
			}
			// set flag
			if(!strcmp(token, "input")){
				Fg_in = true;
			}else if(!strcmp(token, "output")){
				Fg_out = true;
			}else if(!strcmp(token, "wire")){
				Fg_wire = true;
			}else if(!strcmp(token, "module")){
				Fg_mdl = true;
			}else if(!strcmp(token, "assign")){
				Fg_func = true;
				Fg_fout = true;
			}
			if(Fg_semi){
				if (xor_cnt == 0)
					node.op_type = 0; /** this node is a MAJ gate **/
				if(Fg_func){
					if(fout_id){ // push into Ontk, Gntk
						int fi_count = node.vFanins.size();
						if(fi_count < 3 && node.op_type == 0){
							if(fi_count == 1){ // only one fanin
								node.fCompl1 = 0;
								node.fCompl2 = 1;
								node.vFanins.push_back(1);
								node.vFanins.push_back(1);
								fos[1].push_back(node.Id);
								fos[1].push_back(node.Id);
							}
							else if(fi_count == 2){ // two fanins
								node.fCompl2 = 0;
								if(op == AND){
									node.fCompl2 = 1;
									node.vFanins.push_back(1);
									fos[1].push_back(node.Id);
								}else if(op == OR){
									node.fCompl2 = 0;
									node.vFanins.push_back(1);
									fos[1].push_back(node.Id);
								}
							}
							else{ // QA : needed?
								//......
							}
						}
						if(node.Type == PO){
							Ontk->vPos.push_back(node);
							Gntk->vPos.push_back(node);
						}
						else{
							Ontk->vObjs.push_back(node);
							Gntk->vObjs.push_back(node);
						}
						node.vFanins.clear();
						node.vFanouts.clear();
					}
				}
				if (xor_cnt == 2)
					xor_cnt = 0;
				fCompl_index = 0;
				Fg_in = false;
				Fg_out = false;
				Fg_wire = false;
				Fg_mdl = false;
				Fg_func = false;
				Fg_fin = false;
				Fg_fout = false;
				fout_id = 0;
			//	pre_fanin = size+10;
			}
			token = strtok(NULL,del);
		}
	}
	// store fanouts to each node
	
	unsigned j;
	
	for(j=0; j< Ontk->vPis.size(); j++){
		Ontk->vPis[j].vFanouts = fos[Ontk->vPis[j].Id];
		Ontk->vPis[j].Level = 0;
		Ontk->Obj_Ptr[Ontk->vPis[j].Id] = &Ontk->vPis[j];
		Ontk->Obj_Ptr[Ontk->vPis[j].Id]->op_type = 2;
	}
	for(j=0; j< Ontk->vObjs.size(); j++){
		Ontk->vObjs[j].vFanouts = fos[Ontk->vObjs[j].Id];
		Ontk->vObjs[j].Level = 0;
		Ontk->Obj_Ptr[Ontk->vObjs[j].Id] = &Ontk->vObjs[j];
		if (Ontk->Obj_Ptr[Ontk->vObjs[j].Id]->op_type == 1)
			Ontk->Obj_Ptr[Ontk->vObjs[j].Id]->fCompl2 = 0;
		Ontk->Obj_Ptr[Ontk->vObjs[j].Id]->Simp = 0;
	}
	
	for(j=0; j< Ontk->vPos.size(); j++){
		Ontk->vPos[j].vFanouts = fos[Ontk->vPos[j].Id];
		Ontk->vPos[j].Level = 0;
		Ontk->Obj_Ptr[Ontk->vPos[j].Id] = &Ontk->vPos[j];	
		Ontk->Obj_Ptr[Ontk->vPos[j].Id]->op_type = 2;	
	}
	
	for(j=0; j< Gntk->vPis.size(); j++){
		Gntk->vPis[j].vFanouts = fos[Gntk->vPis[j].Id];
		Gntk->vPis[j].Level = 0;
		Gntk->Obj_Ptr[Gntk->vPis[j].Id] = &Gntk->vPis[j];
		Gntk->Obj_Ptr[Gntk->vPis[j].Id]->op_type = 2;
	}
	for(j=0; j< Gntk->vObjs.size(); j++){
		Gntk->vObjs[j].vFanouts = fos[Gntk->vObjs[j].Id];
		Gntk->vObjs[j].Level = 0;
		Gntk->Obj_Ptr[Gntk->vObjs[j].Id] = &Gntk->vObjs[j];
		if (Gntk->Obj_Ptr[Gntk->vObjs[j].Id]->op_type == 1)
			Gntk->Obj_Ptr[Gntk->vObjs[j].Id]->fCompl2 = 0;
		Gntk->Obj_Ptr[Gntk->vObjs[j].Id]->Simp = 0;
	}
	
	for(j=0; j< Gntk->vPos.size(); j++){
		Gntk->vPos[j].vFanouts = fos[Gntk->vPos[j].Id];
		Gntk->vPos[j].Level = 0;
		Gntk->Obj_Ptr[Gntk->vPos[j].Id] = &Gntk->vPos[j];
		Gntk->Obj_Ptr[Gntk->vPos[j].Id]->op_type = 2;	
	}
	
	cout << "----- Third Phase End -----" << endl;

#if CHECK
	cout << "===== Check everything! =====" << endl;
	Xmg_Obj_t *n;
	cout << "Input:" << endl;
	for(j=0; j< Ontk->vPis.size(); j++){
		cout << " =====  " << endl;
		cout << "Id is: " << Ontk->vPis[j].Id << ", and Node is: " << id_name[Ontk->vPis[j].Id] << endl;
		cout << "Type is: " << Ontk->vPis[j].Type << ", op_type is: " << Ontk->vPis[j].op_type << endl;
		cout << "Fanout nodes are: ";
		n = Ontk->Obj_Ptr[Ontk->vPis[j].Id];
		for (unsigned k = 0; k < n->vFanouts.size(); k++){
			cout << id_name[Ontk->Obj_Ptr[n->vFanouts[k]]->Id] << " ";
		}
		cout << endl;
	}

	cout << "---------------------------" << endl;
	cout << "Output:" << endl;
	for(j=0; j< Ontk->vPos.size(); j++){
		cout << " =====  " << endl;
		cout << "Id is: " << Ontk->vPos[j].Id << ", and Node is: " << id_name[Ontk->vPos[j].Id] << endl;
		cout << "Type is: " << Ontk->vPos[j].Type << ", op_type is: " << Ontk->vPos[j].op_type << endl;
		cout << "Fanin nodes are: ";
		n = Ontk->Obj_Ptr[Ontk->vPos[j].Id];
		for (unsigned k = 0; k < n->vFanins.size(); k++){
			cout << id_name[Ontk->Obj_Ptr[n->vFanins[k]]->Id] << " ";
		}
		cout << endl;	
	}

	cout << "---------------------------" << endl;
	cout << "Node:" << endl;
	for(j=0; j< Ontk->vObjs.size(); j++){
		cout << " =====  " << endl;
		cout << "Id is: " << Ontk->vObjs[j].Id << ", and Node is: " << id_name[Ontk->vObjs[j].Id] << endl;
		cout << "Type is: " << Ontk->vObjs[j].Type << ", op_type is: " << Ontk->vObjs[j].op_type << endl;
		cout << "Inv are(0, 1, 2): " << Ontk->vObjs[j].fCompl0 << ", " << Ontk->vObjs[j].fCompl1 << ", " << Ontk->vObjs[j].fCompl2 << endl;
		cout << "Fanin nodes are: ";
		n = Ontk->Obj_Ptr[Ontk->vObjs[j].Id];
		for (unsigned k = 0; k < n->vFanins.size(); k++){
			cout << id_name[Ontk->Obj_Ptr[n->vFanins[k]]->Id] << " ";
		}
		cout << endl;
		cout << "Fanout nodes are: ";
		n = Ontk->Obj_Ptr[Ontk->vObjs[j].Id];
		for (unsigned k = 0; k < n->vFanouts.size(); k++){
			cout << id_name[Ontk->Obj_Ptr[n->vFanouts[k]]->Id] << " ";
		}
		cout << endl;
	}
	if (Ontk->vObjs.size() != Gntk->vNode_num) {
		cout << "[ERROR] Wrong number of nodes!!!" << endl;
		getchar();
	}
#endif
	

	
	cout << "----- Fourth Phase Start -----" << endl;
	
	Ontk_level = compute_level(Ontk);
	Gntk_level = compute_level(Gntk);
	
	cout << "Level is: " << Ontk_level << endl;

	O_id_name = id_name;
	O_name_id = name_id;
	
	cout << "----- Fourth Phase End -----" << endl;
	
	cout << "----- Fifth Phase Start -----" << endl; // 20160224 add new structure in struct.h
	
	Xmg_Obj_t *node_ptr;
	Xmg_Obj_t *fi;
	node_info *ir;
	unsigned k;
	int singleFanoutFaninCnt = 0;

	//cout << "Gntk->vObjs.size(): " << Gntk->vObjs.size() << endl;
	for(k = 0; k < Gntk->vObjs.size(); k++){
		node_ptr = Gntk->Obj_Ptr[Gntk->vObjs[k].Id];
		ir = &nodeInfoTable[node_ptr->Id];
		if(node_ptr->Type == NODE){
			for(j = 0; j < node_ptr->vFanins.size(); j++){
				fi = Gntk->Obj_Ptr[node_ptr->vFanins[j]];
				if((fi->Type != CONST) && (fi->vFanouts.size()==1)){
					singleFanoutFaninCnt++;
					//cout << "Node Id is: " << node_ptr->Id << endl;
					ir->OneFanoutFanin = 1;
					break;
				}
			}
		}
		ir->OBS1 = NULL;
		ir->OBS0 = NULL;
	}
	cout << "# of the singleFanoutFanin: " << singleFanoutFaninCnt << endl;
	cout << "----- Fifth Phase End -----" << endl;	
	
	fin.close();
	
	return 1;
}
