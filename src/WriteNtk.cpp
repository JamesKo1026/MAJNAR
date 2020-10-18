#include "WriteNtk.h"

//***********************************************************
bool WriteGntk(char *filename)
{
	cout << "In WriteNtk.cpp " << endl;
	string new_filename;
	new_filename = "Opt_";
	new_filename += filename;
	cout << new_filename << endl;
	ofstream fout(new_filename.c_str(), ios::out | ios::binary);
	if(!fout)
	{
		cout << "WriteNtk(): Output file is not found. (Fail in WriteNtk.cpp)" << endl;
		fout.close();
		return false;
	}

	int op_count;
	unsigned i,j;
	int index;
	int bound;
	
	int one_count;
	int zero_count;
	
	vector<int> Id;
	vector<int> fCompl;
	
	// print module
	fout << "module " << Gntk->pName << "(";
	index = 0;
	bound = Gntk->vPis_num + Gntk->vPos_num;
	for(vector<Xmg_Obj_t>::iterator pi = Gntk->vPis.begin(); pi != Gntk->vPis.end(); pi++){
		if(pi->Type == PI){
			fout << id_name[pi->Id] << ", ";
			if((index+1) % 10 == 0)
				fout << "\n\t\t\t";
			index++;
		}
	}
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		if(po->Type == PO){
			if(index == bound-1){
				fout << id_name[po->Id] << ");" << endl;
			}
			else
				fout << id_name[po->Id] << ", ";
			if((index + 1) % 10 == 0)
				fout << "\n\t\t\t";
			index++;
		}
	}
	
	// print input
	fout << "  input ";
	index = 0;
	bound = Gntk->vPis_num;
	for(vector<Xmg_Obj_t>::iterator pi = Gntk->vPis.begin(); pi != Gntk->vPis.end(); pi++){
		if(pi->Type == PI){
			if(index == bound-1){
				fout << id_name[pi->Id] << ";" << endl;
			}
			else
				fout << id_name[pi->Id] << ", ";
			if((index + 1) % 10 == 0)
				fout << "\n\t\t\t";
			index++;
		}
	}
	
	// print output
	fout << "  output ";
	index = 0;
	bound = Gntk->vPos_num;
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		if(po->Type == PO){
			if(index == bound-1){
				fout << id_name[po->Id] << ";" << endl;
			}
			else
				fout << id_name[po->Id] << ", ";
			if((index + 1) % 10 == 0)
				fout << "\n\t\t\t";
			index++;
		}
	}
	
	// print wire
	bound = 0;
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){
		if(pn->Type == NODE && pn->vFanins.size() > 0)
			bound++;
	}
	fout << "  wire ";
	index = 0;
	int wire_cnt = 0;
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){
		if(pn->Type == NODE && pn->vFanins.size() > 0){
			if(index == bound - 1){
				fout << id_name[pn->Id] << ";" << endl;
			}
			else
				fout << id_name[pn->Id] << ", ";
			//cout << "Node is: " << id_name[pn->Id] << ", wire_cnt to " << wire_cnt << endl;
			if((index + 1) % 10 == 0)
				fout << "\n\t\t\t";
			index++;
			wire_cnt++;
		}
	}
	//cout << "wire_cnt: " << wire_cnt << endl;
	// print function (Obj)	
	for(vector<Xmg_Obj_t>::iterator pn = Gntk->vObjs.begin(); pn != Gntk->vObjs.end(); pn++){
		if(pn->Type == NODE && pn->vFanins.size() > 0 && pn->vFanouts.size() > 0){
			if (pn->op_type == 0) {
				one_count = 0;
				zero_count = 0;
				index = 0;
				Id.clear();
				fCompl.clear();
				fout << "  assign " << id_name[pn->Id] << " = ";	
				for(vector<int>::iterator fi_id = pn->vFanins.begin(); fi_id != pn->vFanins.end(); fi_id++){
					if(index == 0){
						if(id_name[*fi_id] == "0"){
							if(pn->fCompl0 == 0)
								zero_count++;
							else
								one_count++;
						}
						else if(id_name[*fi_id] == "1"){
							if(pn->fCompl0 == 0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(pn->fCompl0);
					}else if(index == 1){
						if(id_name[*fi_id] == "0"){
							if(pn->fCompl1 == 0)
								zero_count++;
							else
								one_count++;
						}else if(id_name[*fi_id]=="1"){
							if(pn->fCompl1 == 0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(pn->fCompl1);
					}else if(index == 2){
						if(id_name[*fi_id] == "0"){
							if(pn->fCompl2 == 0)
								zero_count++;
							else
								one_count++;
						}else if(id_name[*fi_id] == "1"){
							if(pn->fCompl2 == 0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(pn->fCompl2);
					}				
					index++;
				}
				if(zero_count + one_count == 0){
					op_count = 0;
					for(i = 0; i < Id.size(); i++){
						for(j = i+1; j < Id.size(); j++){
							fout << "(";
							if(fCompl[i]==1)
								fout << "~";
							fout << id_name[Id[i]] << " & ";
							if(fCompl[j]==1)
								fout << "~";
							fout << id_name[Id[j]] << ")";
							if(op_count < 2)
								fout << " | ";
							op_count++;
						}
					}
					fout << ";" << endl;
				}else if(zero_count >= 2){
					fout << "1'b0;" << endl;
				}else if(one_count >= 2){
					fout << "1'b1;" << endl;
				}else if(zero_count == 1 && one_count == 1){
					for(i = 0; i < Id.size(); i++){
						if(id_name[Id[i]] != "0" && id_name[Id[i]] != "1"){
							if(fCompl[i] == 1)
								fout << "~";
							fout << id_name[Id[i]];
							break;
						}
					}
					fout << ";" << endl;
				}else if(zero_count == 1){
					op_count = 0;
					for(i = 0; i < Id.size(); i++){
						if(id_name[Id[i]] != "0" && id_name[Id[i]] != "1"){
							if(fCompl[i] == 1)
								fout << "~";
							fout << id_name[Id[i]];
							if(op_count < 1)
								fout << " & ";
							op_count++;
						}
					}
					fout << ";" << endl;
				}else if(one_count == 1){
					op_count = 0;
					for(i=0; i<Id.size(); i++){
						if(id_name[Id[i]] != "0" && id_name[Id[i]] != "1"){
							if(fCompl[i]==1)
								fout << "~";
							fout << id_name[Id[i]];
							if(op_count < 1)
								fout << " | ";
							op_count++;
						}
					}
					fout << ";" << endl;
				}			
			} else if (pn->op_type == 1) {
				one_count = 0;
				zero_count = 0;
				index = 0;
				Id.clear();
				fCompl.clear();
				fout << "  assign " << id_name[pn->Id] << " = ";
				for(vector<int>::iterator fi_id = pn->vFanins.begin(); fi_id != pn->vFanins.end(); fi_id++){
					if(index == 0){
						if(id_name[*fi_id] == "0"){
							if(pn->fCompl0 == 0)
								zero_count++;
							else
								one_count++;
						}
						else if(id_name[*fi_id] == "1"){
							if(pn->fCompl0 == 0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(pn->fCompl0);
					}else if(index == 1){
						if(id_name[*fi_id] == "0"){
							if(pn->fCompl1 == 0)
								zero_count++;
							else
								one_count++;
						}else if(id_name[*fi_id]=="1"){
							if(pn->fCompl1 == 0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(pn->fCompl1);
					}else if(index == 2){
						if(id_name[*fi_id] == "0"){
							if(pn->fCompl2 == 0)
								zero_count++;
							else
								one_count++;
						}else if(id_name[*fi_id]=="1"){
							if(pn->fCompl2 == 0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(pn->fCompl2);
					}			
					index++;
				}
				if(zero_count + one_count < 2){
					op_count = 0;
					for(i = 0; i < Id.size(); i++){
						if(fCompl[i] == 1)
							fout << "~";

						if (id_name[Id[i]] == "0")
							fout << "1'b0";
						else if (id_name[Id[i]] == "1")
							fout << "1'b1";
						else
							fout << id_name[Id[i]];
						
						if (i != Id.size()-1)
							fout << " ^ ";

						op_count++;
					}
					fout << ";\n";
					//fout << "1'b0;" << endl;
				}else if(zero_count == 2){
					fout << "1'b0;" << endl;
				}else if(one_count == 2){
					fout << "1'b0;" << endl;
				}else if(zero_count == 1 && one_count == 1){
					fout << "1'b1;" << endl;
				}
			}
		}
	}	
		
	// print function (PO)
	for(vector<Xmg_Obj_t>::iterator po = Gntk->vPos.begin(); po != Gntk->vPos.end(); po++){
		if(po->Type == PO){
			if (po->op_type == 2) {
				one_count = 0;
				zero_count = 0;
				index = 0;
				Id.clear();
				fCompl.clear();
				fout << "  assign " << id_name[po->Id] << " = ";
				for(vector<int>::iterator fi_id = po->vFanins.begin(); fi_id != po->vFanins.end(); fi_id++){
					if(index == 0){
						if(id_name[*fi_id]=="0"){
							if(po->fCompl0==0)
								zero_count++;
							else
								one_count++;
						}
						else if(id_name[*fi_id]=="1"){
							if(po->fCompl0==0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(po->fCompl0);
					}else if(index == 1){
						if(id_name[*fi_id]=="0"){
							if(po->fCompl1==0)
								zero_count++;
							else
								one_count++;
						}else if(id_name[*fi_id]=="1"){
							if(po->fCompl1==0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(po->fCompl1);
					}else if(index == 2){
						if(id_name[*fi_id]=="0"){
							if(po->fCompl2==0)
								zero_count++;
							else
								one_count++;
						}else if(id_name[*fi_id]=="1"){
							if(po->fCompl2==0)
								one_count++;
							else
								zero_count++;
						}
						Id.push_back(*fi_id);
						fCompl.push_back(po->fCompl2);
					}				
					index++;
				}
				if(zero_count+one_count == 0)
				{
					op_count = 0;
					for(i=0; i<Id.size(); i++){
						for(j=i+1; j<Id.size(); j++){
							fout << "(";
							if(fCompl[i]==1)
								fout << "~";
							fout << id_name[Id[i]] << " & ";
							if(fCompl[j]==1)
								fout << "~";
							fout << id_name[Id[j]] << ")";
							if(op_count < 2)
								fout << " | ";
							op_count++;
						}
					}
					fout << ";" << endl;
				}else if(zero_count>=2){
					fout << "1'b0;" << endl;
				}else if(one_count>=2){
					fout << "1'b1;" << endl;
				}else if(zero_count == 1 && one_count == 1){
					for(i=0; i<Id.size(); i++){
						if(id_name[Id[i]] != "0" && id_name[Id[i]] != "1"){
							if(fCompl[i]==1)
								fout << "~";
							fout << id_name[Id[i]];
							break;
						}
					}
					fout << ";" << endl;
				}else if(zero_count == 1){
					op_count = 0;
					for(i=0; i<Id.size(); i++){
						if(id_name[Id[i]] != "0" && id_name[Id[i]] != "1"){
							if(fCompl[i]==1)
								fout << "~";
							fout << id_name[Id[i]];
							if(op_count < 1)
								fout << " & ";
							op_count++;
						}
					}
					fout << ";" << endl;
				}else if(one_count == 1){
					op_count = 0;
					for(i=0; i<Id.size(); i++){
						if(id_name[Id[i]] != "0" && id_name[Id[i]] != "1"){
							if(fCompl[i]==1)
								fout << "~";
							fout << id_name[Id[i]];
							if(op_count < 1)
								fout << " | ";
							op_count++;
						}
					}
					fout << ";" << endl;
				}
			} 
			// else if (po->op_type == 1) {
			// 	one_count = 0;
			// 	zero_count = 0;
			// 	index = 0;
			// 	Id.clear();
			// 	fCompl.clear();
			// 	fout << "  assign " << id_name[po->Id] << " = ";
			// 	for(vector<int>::iterator fi_id = po->vFanins.begin(); fi_id != po->vFanins.end(); fi_id++){
			// 		if(index == 0){
			// 			if(id_name[*fi_id] == "0"){
			// 				if(po->fCompl0 == 0)
			// 					zero_count++;
			// 				else
			// 					one_count++;
			// 			}
			// 			else if(id_name[*fi_id] == "1"){
			// 				if(po->fCompl0 == 0)
			// 					one_count++;
			// 				else
			// 					zero_count++;
			// 			}
			// 			Id.push_back(*fi_id);
			// 			fCompl.push_back(po->fCompl0);
			// 		}else if(index == 1){
			// 			if(id_name[*fi_id] == "0"){
			// 				if(po->fCompl1 == 0)
			// 					zero_count++;
			// 				else
			// 					one_count++;
			// 			}else if(id_name[*fi_id]=="1"){
			// 				if(po->fCompl1 == 0)
			// 					one_count++;
			// 				else
			// 					zero_count++;
			// 			}
			// 			Id.push_back(*fi_id);
			// 			fCompl.push_back(po->fCompl1);
			// 		}else if(index == 2){
			// 			if(id_name[*fi_id] == "0"){
			// 				if(po->fCompl2 == 0)
			// 					zero_count++;
			// 				else
			// 					one_count++;
			// 			}else if(id_name[*fi_id]=="1"){
			// 				if(po->fCompl2 == 0)
			// 					one_count++;
			// 				else
			// 					zero_count++;
			// 			}
			// 			Id.push_back(*fi_id);
			// 			fCompl.push_back(po->fCompl2);
			// 		}				
			// 		index++;
			// 	}
			// 	if(zero_count + one_count < 2){
			// 		op_count = 0;
			// 		for(i = 0; i < Id.size(); i++){
			// 			if(fCompl[i] == 1)
			// 				fout << "~";

			// 			if (id_name[Id[i]] == "0")
			// 				fout << "1'b0";
			// 			else if (id_name[Id[i]] == "1")
			// 				fout << "1'b1";
			// 			else
			// 				fout << id_name[Id[i]];
						
			// 			if (i != Id.size()-1)
			// 				fout << " ^ ";

			// 			op_count++;
			// 		}
			// 		fout << ";\n";
			// 	}else if(zero_count == 2){
			// 		fout << "1'b0;" << endl;
			// 	}else if(one_count == 2){
			// 		fout << "1'b0;" << endl;
			// 	}else if(zero_count == 1 && one_count == 1){
			// 		fout << "1'b1;" << endl;
			// 	}
			// }				
		}
	}
	// print endmodule
	fout << "endmodule\n";
	
	fout.close();
	return 1;
}
