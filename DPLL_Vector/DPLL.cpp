#include"dpll.h"
#include<stack>
#include<random>
#include"vtoc.h"
extern Vtoc variableId_to_Clauses;
enum Result
{
	NORMAL = -1,
	UNSAT = 0,
	SAT = 1,
	COMPLETE = 2
};
DPLL::DPLL()
{
}

DPLL::~DPLL()
{
}

void DPLL::deleteClause(Formula& f, int clause_idx) {
	if (!f.clauses[clause_idx].flag) {
		f.clauses[clause_idx].flag = !f.clauses[clause_idx].flag;
		f.current_clauses_cnt--;
		int need_delete_item_idx = -1;
		for (int i = 0; i < f.unit_clauses.size(); i++) {
			if (f.unit_clauses[i] == clause_idx) {
				need_delete_item_idx = i;
				break;
			}
		}
		if (need_delete_item_idx != -1 && f.unit_clauses.size() > 1) {
			f.unit_clauses[need_delete_item_idx] = f.unit_clauses.back();
			f.unit_clauses.pop_back();
		}
		if (need_delete_item_idx != -1 && f.unit_clauses.size() == 1) {
			f.unit_clauses.pop_back();
		}
	}
}

bool DPLL::deleteVariableByClauseId(Formula& f, int clause_idx, int variable_id) {
	bool flag = false;//为true代表删除variable_id变元导致该子句成为了空子句。
	if (!f.clauses[clause_idx].flag) {
		for (auto& item : f.clauses[clause_idx].variables) {
			if (!item.flag && item.variable_id == variable_id) {
				item.flag = !item.flag;
				break;
			}
		}
		int res = --f.clauses[clause_idx].length;
		if (res == 0) {
			f.clauses[clause_idx].flag = !f.clauses[clause_idx].flag;
			f.current_clauses_cnt--;
			flag = true;
		}
		if (res == 1) {
			f.unit_clauses.push_back(clause_idx);
		}
	}
	return flag;
}

bool DPLL::isClausesEmpty(Formula& f) {
	if (f.current_clauses_cnt>0) return false;
	return true;
}

int DPLL::findNoDeleteVariableByClauseId(Formula& f, int clause_id) {
	for (auto& var : f.clauses[clause_id].variables) {
		if (!var.flag) return var.variable_id;
	}
	return 0;
}

int DPLL::applyVariableAssign(Formula& f, int variable_id, bool value) {
	int var_id = value ? variable_id : -variable_id;
	if (variableId_to_Clauses.find(var_id) != 0) {
		for (auto& clause_id : variableId_to_Clauses[var_id]) {
			DPLL::deleteClause(f, clause_id);
			if (DPLL::isClausesEmpty(f)) return SAT;
		}
	}
	var_id = -var_id;
	if (variableId_to_Clauses.find(var_id) != 0) {
		for (auto& clause_id : variableId_to_Clauses[var_id]) {
			bool flag = DPLL::deleteVariableByClauseId(f,clause_id, var_id);
			if (flag) return UNSAT;
		}
	}
	return NORMAL;
}

void DPLL::pureLiteralSimplify(Formula& f) {
	for (int i = 1; i < variableId_to_Clauses.size(); i+=2) {
		if (!variableId_to_Clauses.at(i).empty() && variableId_to_Clauses.at(i + 1).empty()) {
			int var_id = Vtoc::idxToVariableId(i);
			for (auto& clause_id : variableId_to_Clauses.at(i)) {
				bool value = var_id > 0 ? true : false;
				DPLL::applyVariableAssign(f, abs(var_id), value);
				if (DPLL::isClausesEmpty(f)) return;
			}
		}
		if (variableId_to_Clauses.at(i).empty() && !variableId_to_Clauses.at(i + 1).empty()) {
			int var_id = Vtoc::idxToVariableId(i+1);
			for (auto& clause_id : variableId_to_Clauses.at(i+1)) {
				bool value = var_id > 0 ? true : false;
				DPLL::applyVariableAssign(f, abs(var_id), value);
				if (DPLL::isClausesEmpty(f)) return;
			}
		}
	}
}

int DPLL::up(Formula& f) {
	bool unit_clause_find = false;
	vector<int>& uc = f.unit_clauses;
	do
	{
		unit_clause_find = false;
		if (!uc.empty()) {
			unit_clause_find = true;
			int clause_id = uc.back();
			uc.pop_back();
			int var_id = DPLL::findNoDeleteVariableByClauseId(f, clause_id);
			bool value = var_id > 0 ? true : false;
			int result = DPLL::applyVariableAssign(f,abs(var_id), value);
			if (result == SAT || result == UNSAT) return result;
		}
	} while (unit_clause_find);
	return NORMAL;
}

int DPLL::randomSelectVariableIdInVector(vector<int>& v) {
	//cout << "randomSelectVariableIdInVector函数" << endl;
	//for (auto& item : v) cout << item << " ";
	//cout << endl;
	// 创建随机数引擎
	random_device rd;
	mt19937 gen(rd());

	// 创建均匀分布函数
	uniform_int_distribution<> dis(0, v.size() - 1);

	// 随机选择一项
	int randomIndex = dis(gen);
	int var_id = v[randomIndex];
	return var_id;
}

int DPLL::momsSelectVariable(Formula& f) {
	//cout << "momsSelectVariable函数" << endl;
	int min_clause_length = INT_MAX;
	for (auto it = (++f.clauses.begin()); it != f.clauses.end();it++) {
		clause c = *it;
		if (!c.flag) {
			min_clause_length = min(min_clause_length, c.length);
		}
	}
	//cout << "min_clause_length: " << min_clause_length << endl;
	vector<int> min_clauses;
	for (int i = 1; i < f.clauses.size(); i++) {
		if (!f.clauses[i].flag && f.clauses[i].length == min_clause_length) {
			min_clauses.push_back(i);
		}
	}
	int n = (int)variableId_to_Clauses.size() + 1;
	int* bucket_count = new int[n];
	fill(bucket_count, bucket_count+n, 0);
	int max_emerge_cnt = INT_MIN;
	for (auto& clause_id : min_clauses) {
		for (auto& var : f.clauses[clause_id].variables) {
			if (!var.flag) {
				max_emerge_cnt = max(++bucket_count[abs(var.variable_id)], max_emerge_cnt);
			}
		}
	}
	vector<int> vars;
	for (int i = 1; i < n; i++) {
		if (bucket_count[i] == max_emerge_cnt) {
			vars.push_back(i);
		}
	}
	delete[] bucket_count;
	if (vars.size() == 1) return *vars.begin();
	return DPLL::randomSelectVariableIdInVector(vars);
}

int DPLL::solver(Formula& f) {
	if (DPLL::isClausesEmpty(f)) return COMPLETE;
	DPLL::pureLiteralSimplify(f);
	if (DPLL::isClausesEmpty(f)) return COMPLETE;
	int cnt = 0;
	for (int i = 1; i < variableId_to_Clauses.size(); i++) {
		if (!variableId_to_Clauses.at(i).empty()) cnt++;
	}
	stack<Formula*> s;
	s.push(&f);
	while (!s.empty())
	{
		Formula* temp_f_ptr = s.top();
		s.pop();
		//temp_f_ptr->printFormula();
		//cout << "up操作" << endl;
		int result_up = DPLL::up(*temp_f_ptr);
		//temp_f_ptr->printFormula();
		//cout << "result_up: " << result_up << endl;
		if (result_up == SAT) return COMPLETE;
		if (result_up == UNSAT) {
			//temp_f_ptr->printFormula();
			delete temp_f_ptr;
			continue;
		}
		if (result_up == NORMAL) {
			int var_id = DPLL::momsSelectVariable(*temp_f_ptr);
			//cout << "moms挑选的变元为：" << var_id << endl;
			bool value = var_id > 0 ? true : false;
			Formula* copy_f = new Formula(*temp_f_ptr);
			//temp_f_ptr->printFormula();
			//cout << "one赋值操作,value: " << value <<endl;
			int result_branch_one = DPLL::applyVariableAssign(*temp_f_ptr, abs(var_id), value);
			// << "result_branch_one: " << result_branch_one << endl;
			//temp_f_ptr->printFormula();
			if (result_branch_one == SAT) return COMPLETE;
			if (result_branch_one == UNSAT) delete temp_f_ptr;
			if (result_branch_one == NORMAL) s.push(temp_f_ptr);
			//copy_f->printFormula();
			//cout << "two赋值操作,value: " << !value << endl;
			int result_branch_two = DPLL::applyVariableAssign(*copy_f, abs(var_id), !value);
			//cout << "result_branch_two: " << result_branch_two << endl;
			//copy_f->printFormula();
			if (result_branch_two == SAT) return COMPLETE;
			if (result_branch_two == UNSAT) delete copy_f;
			if (result_branch_two == NORMAL) s.push(copy_f);
		}
	}
	return NORMAL;
}

void DPLL::showResult(int result) {
	//cout << "result: " << result << endl;
	if (result == COMPLETE) cout << "SAT\n";
	if (result == NORMAL) cout << "UNSAT\n";
}