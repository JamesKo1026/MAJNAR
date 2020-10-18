#include "include.h"
#include "struct.h"
#include "ReadNtk.h"
#include "WriteNtk.h"
#include "NodeMGR.h"

Xmg_Ntk_t_ *Ontk;
Xmg_Ntk_t_ *Gntk;

clock_t start;

int id_index = 0;

int replaced_count = 0;
int replaced_maj_count = 0;
int replaced_xor_count = 0;
int replaced_count_NAR = 0;
int replaced_count_NAR_AND = 0;
int replaced_count_NAR_OR = 0;

int remove_count = 0;
int removed_maj_count = 0;
int removed_xor_count = 0;

int substitute_count = 0;
int MAreuse = 0;

vector<int> remove_table;
vector<node_pair> replaced_table;

map<int,string> O_id_name;
map<string,int> O_name_id;
map<int,string> id_name;
map<string,int> name_id;

map<int,int> PI_judge;
map<int,int> PO_judge;

st_table *ValuingNode = NULL;
st_table *JTable = NULL;

node_info *nodeInfoTable;
int *validTable;
vector<int> *EachNodeDom; // Each node's dominators
int size = 0;
vector<int> nList;

int Ontk_level = 0;
int Gntk_level = 0;

unsigned global_index = 0;
double duration;

int main(int argc, char *argv[])
{
	
	char *filename = argv[1];
	
	if(!ReadNtk(filename)) {
		cout << "[ERROR] Reading the file." << endl;
		return 0;
	}
		
	
	bool st;
	
	st = check(Gntk);
	if(st==0) {
		cout << "Something is wrong!" << endl;
		return 0;
	} else {
		cout << "Correct!" << endl;
	}
	start = clock();
	NodeMGR();

	//cout << "check before write" << endl;
	if (!WriteGntk(filename)){
		cout << "[ERROR] Writing the file." << endl;
		return 0;
	}
	
	double O_count = (double)compute_gate(Ontk);
	double G_count = (double)compute_gate(Gntk);
	int O_level = compute_level(Ontk);
	int G_level = compute_level(Gntk);

	cout << argv[1] << ": " << endl;
	cout << "PI/PO: " << Ontk->vPis_num << "/" << Ontk->vPos_num << endl;
	cout << "Ontk node count: " << O_count << endl;
	cout << "Gntk node count: " << G_count << endl;
	cout << "replaced_count: " << replaced_count << endl;
	cout << "replaced_count_NAR: " << replaced_count_NAR << endl;
	cout << "replaced_count_NAR_AND: " << replaced_count_NAR_AND << endl;
	cout << "replaced_count_NAR_OR: " << replaced_count_NAR_OR << endl;
	cout << "total_replaced_count: " << replaced_count + replaced_count_NAR + replaced_count_NAR_AND + replaced_count_NAR_OR << endl;
	cout << "removed_count: " << remove_table.size() << endl;
	cout << "substitute_count: " << substitute_count << endl;
	cout << "Ontk max level: " << O_level << endl;
	cout << "Gntk max level: " << G_level << endl;
	cout << "Opt area: " << ((O_count - G_count)/O_count)*100 << endl;
	cout << "Time: " << duration << endl;
	cout << "MAreuse: " << MAreuse << endl;
	cout << "---------------------------" << endl;

	
	ofstream fout3("EXP_table.v",ios::app);
	fout3 << argv[1] << ": " << endl;
	fout3 << "PI/PO: " << Ontk->vPis_num << "/" << Ontk->vPos_num << endl;
	fout3 << "Ontk node count: " << O_count << endl;
	fout3 << "Gntk node count: " << G_count << endl;
	fout3 << "replaced_count: " << replaced_count << endl;
	fout3 << "replaced_count_NAR: " << replaced_count_NAR << endl;
	fout3 << "replaced_count_NAR_AND: " << replaced_count_NAR_AND << endl;
	fout3 << "replaced_count_NAR_OR: " << replaced_count_NAR_OR << endl;
	fout3 << "total_replaced_count: " << replaced_count + replaced_count_NAR + replaced_count_NAR_AND + replaced_count_NAR_OR << endl;
	fout3 << "removed_count: " << remove_table.size() << endl;
	fout3 << "substitute_count: " << substitute_count << endl;
	fout3 << "Ontk max level: " << O_level << endl;
	fout3 << "Gntk max level: " << G_level << endl;
	fout3 << "Opt area: " << ((O_count - G_count)/O_count)*100 << endl;
	fout3 << "Time: " << duration << endl;
	fout3 << "MAreuse: " << MAreuse << endl;
	fout3 << "--------------------------------" << endl;
	fout3.close();
	
	return 0;
}
