#include"AdvancedDPLL.h"
#include<stack>
#include<random>
#include"vtoc.h"
#include"RecordChange.h"
extern Vtoc variableId_to_Clauses;
enum Result
{
	NORMAL = -1,
	UNSAT = 0,
	SAT = 1,
	COMPLETE = 2
};
AdvancedDPLL::AdvancedDPLL()
{

}

AdvancedDPLL::~AdvancedDPLL()
{
}

void AdvancedDPLL::deleteClause(AdvancedFormula& f, int clause_idx, RecordChange& rc) {
	if (!f.clauses[clause_idx].flag) {
		rc.delete_clauses.push_back(clause_idx);
		f.clauses[clause_idx].flag = !f.clauses[clause_idx].flag;
		f.current_clauses_cnt--;
		//int need_delete_item_idx = -1;
		for (int i = 0; i < f.unit_clauses.size(); i++) {
			if (f.unit_clauses[i] == clause_idx) {
				//need_delete_item_idx = i;
				rc.unit_clauses_remove.push_back(clause_idx);
				f.unit_clauses[i] = f.unit_clauses.back();
				f.unit_clauses.pop_back();
				break;
			}
		}
	}
}

bool AdvancedDPLL::deleteVariableByClauseId(AdvancedFormula& f, int clause_idx, int variable_id, RecordChange& rc) {
	bool flag = false;//为true代表删除variable_id变元导致该子句成为了空子句。
	if (!f.clauses[clause_idx].flag) {
		for (auto& item : f.clauses[clause_idx].variables) {
			if (!item.flag && item.variable_id == variable_id) {
				item.flag = !item.flag;
				rc.delete_variables.push_back(make_pair(clause_idx, item.variable_id));//可优化
				break;
			}
		}
		int res = --f.clauses[clause_idx].length;
		if (res == 0) {
			f.clauses[clause_idx].flag = !f.clauses[clause_idx].flag;
			f.current_clauses_cnt--;
			rc.delete_clauses.push_back(clause_idx);
			flag = true;
		}
		if (res == 1) {
			f.unit_clauses.push_back(clause_idx);
			rc.unit_clauses_insert.push_back(clause_idx);
		}
	}
	return flag;
}

bool AdvancedDPLL::isClausesEmpty(AdvancedFormula& f) {
	if (f.current_clauses_cnt > 0) return false;
	return true;
}

int AdvancedDPLL::findNoDeleteVariableByClauseId(AdvancedFormula& f, int clause_id) {
	for (auto& var : f.clauses[clause_id].variables) {
		if (!var.flag) return var.variable_id;
	}
	return 0;
}

int AdvancedDPLL::applyVariableAssign(AdvancedFormula& f, int variable_id, bool value,RecordChange& rc) {
	AdvancedFormula::flip_flag_ptr[variable_id]++;
	AdvancedFormula::variables_assign_ptr[variable_id] = value;
	rc.assign_var_id.push_back(variable_id);
	int var_id = value ? variable_id : -variable_id;
	if (variableId_to_Clauses.find(var_id) != 0) {
		for (auto& clause_id : variableId_to_Clauses[var_id]) {
			AdvancedDPLL::deleteClause(f, clause_id,rc);
			if (AdvancedDPLL::isClausesEmpty(f)) return SAT;
		}
	}
	var_id = -var_id;
	if (variableId_to_Clauses.find(var_id) != 0) {
		for (auto& clause_id : variableId_to_Clauses[var_id]) {
			bool flag = AdvancedDPLL::deleteVariableByClauseId(f, clause_id, var_id, rc);
			if (flag) return UNSAT;
		}
	}
	return NORMAL;
}

void AdvancedDPLL::pureLiteralSimplify(AdvancedFormula& f) {
	RecordChange* rc = new RecordChange(RecordChange::moms_flag);
	for (int i = 1; i < variableId_to_Clauses.size(); i += 2) {
		if (!variableId_to_Clauses.at(i).empty() && variableId_to_Clauses.at(i + 1).empty()) {
			int var_id = Vtoc::idxToVariableId(i);
			for (auto& clause_id : variableId_to_Clauses.at(i)) {
				bool value = var_id > 0 ? true : false;
				AdvancedDPLL::applyVariableAssign(f, abs(var_id), value, *rc);
				if (AdvancedDPLL::isClausesEmpty(f)) {
					delete rc;
					return;
				}
			}
		}
		if (variableId_to_Clauses.at(i).empty() && !variableId_to_Clauses.at(i + 1).empty()) {
			int var_id = Vtoc::idxToVariableId(i + 1);
			for (auto& clause_id : variableId_to_Clauses.at(i + 1)) {
				bool value = var_id > 0 ? true : false;
				AdvancedDPLL::applyVariableAssign(f, abs(var_id), value, *rc);
				if (AdvancedDPLL::isClausesEmpty(f)) {
					delete rc;
					return;
				} 
			}
		}
	}
	delete rc;
}

int AdvancedDPLL::up(AdvancedFormula& f,RecordChange& rc) {
	bool unit_clause_find = false;
	vector<int>& uc = f.unit_clauses;
	do
	{
		unit_clause_find = false;
		if (!uc.empty()) {
			unit_clause_find = true;
			int clause_id = uc.back();
			rc.unit_clauses_remove.push_back(clause_id);
			uc.pop_back();
			int var_id = AdvancedDPLL::findNoDeleteVariableByClauseId(f, clause_id);
			bool value = var_id > 0 ? true : false;
			int result = AdvancedDPLL::applyVariableAssign(f, abs(var_id), value,rc);
			if (result == SAT || result == UNSAT) return result;
		}
	} while (unit_clause_find);
	return NORMAL;
}

int AdvancedDPLL::randomSelectVariableIdInVector(vector<int>& v) {
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

int AdvancedDPLL::momsSelectVariable(AdvancedFormula& f) {
	//cout << "momsSelectVariable函数" << endl;
	int min_clause_length = INT_MAX;
	for (auto it = (++f.clauses.begin()); it != f.clauses.end(); it++) {
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
	fill(bucket_count, bucket_count + n, 0);
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
	return AdvancedDPLL::randomSelectVariableIdInVector(vars);
}
void AdvancedDPLL::addUnitClause(AdvancedFormula& f, int var_id) {
	f.current_clauses_cnt++;
	variable var;
	var.variable_id = var_id;
	clause c;
	c.length = 1;
	c.variables.push_back(var);
	f.clauses.push_back(c);
	variableId_to_Clauses[var_id].push_back(f.clauses.size()-1);
	f.unit_clauses.push_back(f.clauses.size() - 1);
}

void AdvancedDPLL::deleteUnitClause(AdvancedFormula& f, RecordChange& rc) {
	AdvancedDPLL::upBackTrackingHelpFun(f, rc);
	f.current_clauses_cnt--;
	int var_id = f.clauses.rbegin()->variables.begin()->variable_id;
	int clause_id = f.clauses.size() - 1;
	f.clauses.pop_back();
	variableId_to_Clauses[var_id].pop_back();
	for (auto it = f.unit_clauses.rbegin(); it != f.unit_clauses.rend(); it++) {
		if (*it == clause_id) {
			*it = f.unit_clauses.back();
			f.unit_clauses.pop_back();
			break;
		}
	}
	AdvancedFormula::flip_flag_ptr[abs(var_id)] = -1;
	AdvancedFormula::variables_assign_ptr[abs(var_id)] = -1;
}

void AdvancedDPLL::upBackTrackingHelpFun(AdvancedFormula& f, RecordChange& rc) {
	for (const auto& clause_id : rc.delete_clauses) {
		f.clauses[clause_id].flag = false;
		f.current_clauses_cnt++;
	}
	for (const auto& pair_item : rc.delete_variables) {
		for (auto& var : f.clauses[pair_item.first].variables) {
			if (var.variable_id == pair_item.second) {
				var.flag = false;
				f.clauses[pair_item.first].length++;
				break;
			}
		}
	}
	for (auto& clause_id : rc.unit_clauses_remove) {
		f.unit_clauses.push_back(clause_id);
	}
	for (auto& clause_id : rc.unit_clauses_insert) {
		for (auto it = f.unit_clauses.rbegin(); it != f.unit_clauses.rend(); it++) {
			if (*it == clause_id) {
				*it = f.unit_clauses.back();
				f.unit_clauses.pop_back();
			}
		}
	}
	for (auto& var_id : rc.assign_var_id) {
		if (AdvancedFormula::flip_flag_ptr[var_id] == 0)
			AdvancedFormula::variables_assign_ptr[var_id] = -1;
		if (AdvancedFormula::flip_flag_ptr[var_id] == 1) {
			AdvancedFormula::variables_assign_ptr[var_id] = AdvancedFormula::variables_assign_ptr[var_id] == 1 ? 0 : 1;
		}
		AdvancedFormula::flip_flag_ptr[var_id]--;
	}
}
int AdvancedDPLL::momsBackTrackingHelpFun(AdvancedFormula& f, RecordChange& rc) {
	for (const auto& clause_id : rc.delete_clauses) {
		f.clauses[clause_id].flag = false;
		f.current_clauses_cnt++;
	}
	for (const auto& pair_item : rc.delete_variables) {
		for (auto& var : f.clauses[pair_item.first].variables) {
			if (var.variable_id == pair_item.second) {
				var.flag = false;
				f.clauses[pair_item.first].length++;
				break;
			}
		}
	}
	for (auto& clause_id : rc.unit_clauses_remove) {
		f.unit_clauses.push_back(clause_id);
	}
	for (auto& clause_id : rc.unit_clauses_insert) {
		for (auto it = f.unit_clauses.rbegin(); it != f.unit_clauses.rend(); it++) {
			if (*it == clause_id) {
				*it = f.unit_clauses.back();
				f.unit_clauses.pop_back();
			}
		}
	}
	char flag = AdvancedFormula::flip_flag_ptr[*rc.assign_var_id.begin()];
	if (flag == 0) {
		return *rc.assign_var_id.begin();
	}
	if (flag == 1) {
		AdvancedFormula::flip_flag_ptr[*rc.assign_var_id.begin()] = -1;
	}
	return -1;
}

int AdvancedDPLL::upFailBackTracking(AdvancedFormula& f, stack<RecordChange*>& s) {
	RecordChange*& rc_top = s.top();
	s.pop();
	if (rc_top->up_or_moms_flag == s.top()->up_or_moms_flag) {
		AdvancedDPLL::deleteUnitClause(f, *rc_top);
		delete rc_top;
	}
	else {
		s.push(rc_top);
	}
	while (!(!(s.top()->up_or_moms_flag) && s.size() == 1)) {
		RecordChange*& up_rc = s.top();
		s.pop();
		AdvancedDPLL::upBackTrackingHelpFun(f, *up_rc);
		delete up_rc;
		RecordChange*& moms_rc = s.top();
		s.pop();
		int var_id = AdvancedDPLL::momsBackTrackingHelpFun(f, *moms_rc);
		delete moms_rc;
		if (var_id != -1) return var_id;
	}
	return NORMAL;
}
int AdvancedDPLL::momsFailBackTracking(AdvancedFormula& f, stack<RecordChange*>& s) {
	while (!(!(s.top()->up_or_moms_flag) && s.size() == 1)) {
		RecordChange*& moms_rc = s.top();
		s.pop();
		int var_id = AdvancedDPLL::momsBackTrackingHelpFun(f, *moms_rc);
		//f.print();
		if (var_id != -1) return var_id;
		if (var_id == -1) {//unordered_map实现的代码这个地方可能有问题，回头要用代码的时候要检查一下。
			RecordChange*& up_rc = s.top();
			AdvancedDPLL::upBackTrackingHelpFun(f, *up_rc);
			delete up_rc;
		}
		delete moms_rc;
	}
	return -1;
}

int AdvancedDPLL::backTracking(AdvancedFormula& f, stack<RecordChange*>& s, bool up_fail_or_moms_fail_flag) {
	if (!up_fail_or_moms_fail_flag) return upFailBackTracking(f, s);
	else return momsFailBackTracking(f, s);
}

int AdvancedDPLL::solverByIncrementalUpdate(AdvancedFormula& f) {
	if (AdvancedDPLL::isClausesEmpty(f)) return COMPLETE;
	AdvancedDPLL::pureLiteralSimplify(f);
	if (AdvancedDPLL::isClausesEmpty(f)) return COMPLETE;
	stack<RecordChange*> stack_rc;
	do
	{
		RecordChange* rc_up = new RecordChange(RecordChange::up_flag);
		int result_up = AdvancedDPLL::up(f,*rc_up);
		stack_rc.push(rc_up);
		int var_id = -1;
		bool value = true;
		if (result_up == SAT) return COMPLETE;
		if (result_up == UNSAT) {
			int result_back = AdvancedDPLL::backTracking(f, stack_rc, RecordChange::up_flag);
			//f.printFormula();
			if (result_back == NORMAL) {
				//cout << "1这里返回的-1\n";
				return NORMAL;
			}
			var_id = result_back;
			value = !AdvancedFormula::variables_assign_ptr[var_id];
		}
		if (result_up == NORMAL) {
			var_id = AdvancedDPLL::momsSelectVariable(f);
			value = var_id > 0 ? true : false;
		}
		RecordChange* rc_moms = new RecordChange(RecordChange::moms_flag);
		//f.printFormula();
		int result_apply_variable = AdvancedDPLL::applyVariableAssign(f, abs(var_id), value, *rc_moms);
		stack_rc.push(rc_moms);
		if (result_apply_variable == SAT) return COMPLETE;
		if (result_apply_variable == UNSAT) {
			int result_back = AdvancedDPLL::backTracking(f, stack_rc, RecordChange::moms_flag);
			if (result_back == NORMAL) {
				//cout << "2这里返回的-1\n";
				return NORMAL;
			}
			int variable_id = AdvancedFormula::variables_assign_ptr[result_back] == 1 ? 0 - result_back : result_back;
			AdvancedDPLL::addUnitClause(f, variable_id);
		}
	} while (!(stack_rc.size() == 1 && ((stack_rc.top())->up_or_moms_flag == RecordChange::up_flag)));
	//cout << "这里返回的-1\n";
	return NORMAL;
}

void AdvancedDPLL::showResult(int result) {
	//cout << "result: " << result << endl;
	if (result == COMPLETE) cout << "SAT\n";
	if (result == NORMAL) cout << "UNSAT\n";
}