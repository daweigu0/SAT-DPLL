#include<iostream>
#include<vector>
#include<unordered_map>
#include"formula.h"
#include"vtoc.h"
using namespace std;
Vtoc variableId_to_Clauses;
Formula::Formula(const Formula& other) {
	this->clauses = other.clauses;
	this->current_clauses_cnt = other.current_clauses_cnt;
	this->unit_clauses = other.unit_clauses;
	this->variables_cnt = other.variables_cnt;
}
Formula::~Formula() {
	this->clauses.clear();
	this->unit_clauses.clear();
}
void Formula::setCurrentClausesCnt(int cnt)
{
	this->current_clauses_cnt = cnt;
}
void Formula::printFormula() {
	for (int i = 1; i < this->clauses.size(); i++) {
		if (!this->clauses[i].flag) {
			cout << "clause_id: " << i << " length:" << this->clauses[i].length << " variables: ";
			for (int j = 0; j < this->clauses[i].variables.size(); j++) {
				if (!this->clauses[i].variables[j].flag)
					cout << this->clauses[i].variables[j].variable_id << " ";
			}
			cout << endl;
		}
	}
	cout << "unit_clauses: ";
	for (auto& item : this->unit_clauses) {
		cout << item << " ";
	}
	cout << endl;
	//variableId_to_Clauses.pirntVtoc();
	for (int i = 1; i < variableId_to_Clauses.size(); i++) {
		//cout << "i: " << i << endl;
		//!variableId_to_Clauses[i].empty()
		if (!variableId_to_Clauses.at(i).empty()) {
			int var_id = Vtoc::idxToVariableId(i);
			cout << "variable_id: " << var_id << " clause_id: ";
			for (auto& clause_id : variableId_to_Clauses.at(i)) {
				cout << clause_id << " ";
			}
			cout << endl;
		}
	}
	cout << "============printFormulaº¯Êý============" << endl;
}

void Formula::setVariablesCnt(int cnt) {
	this->variables_cnt = cnt;
}