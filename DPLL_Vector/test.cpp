#include<iostream>
#include"input_class.cpp"
#include"formula.h"
#include"dpll.h"
#include"vtoc.h"
#include <chrono>
using namespace std;
extern Vtoc variableId_to_Clauses;
//'C:\\Users\\l1768\\Desktop\\sat\\DPLL培训\\DPLL培训\\SAT测试备选算例\\基准算例\\功能测试\\unsat-5cnf-30.cnf'
//C:\\Users\\l1768\\Desktop\\sat\\DPLL培训\\DPLL培训\\SAT测试备选算例\\基准算例\\功能测试\\sat-20.cnf
void test1() {
	string filename = "C:\\Users\\l1768\\Desktop\\sat\\DPLL培训\\DPLL培训\\SAT测试备选算例\\Beijing\\enddr2-10-by-5-1.cnf";
	cout << filename << endl;
	Input input(filename);
	Formula* f_ptr = new Formula();
	f_ptr->setCurrentClausesCnt(input.getClauseCnt());
	f_ptr->setVariablesCnt(input.getBoolVarCnt());
	variableId_to_Clauses.setVtocSize(input.getBoolVarCnt());
	input.readClauses(*f_ptr);
	//f_ptr->printFormula();
	auto start = chrono::high_resolution_clock::now();
	int result = DPLL::solver(*f_ptr);
	auto stop = chrono::high_resolution_clock::now();
	DPLL::showResult(result);
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
	cout << "耗时：" << duration.count() / 1000.0 << "s" << endl;
}
void test2(string filename) {
	cout << filename << endl;
	Input input(filename);
	Formula* f_ptr = new Formula();
	f_ptr->setCurrentClausesCnt(input.getClauseCnt());
	f_ptr->setVariablesCnt(input.getBoolVarCnt());
	variableId_to_Clauses.setVtocSize(input.getBoolVarCnt());
	input.readClauses(*f_ptr);
	//f_ptr->printFormula();
	auto start = chrono::high_resolution_clock::now();
	int result = DPLL::solver(*f_ptr);
	auto stop = chrono::high_resolution_clock::now();
	DPLL::showResult(result);
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
	cout << duration.count() / 1000.0 << endl;
}
int main(int argc,char* argv[]) {
	test2(argv[1]);
}