#include "SP_globals.h"
#include "iforks_abstraction.h"
#include "op_hash_var_proj_mapping.h"
#include "op_hash_mapping.h"
#include "var_proj_mapping.h"
#include <string>
#include "../causal_graph.h"


IforksAbstraction::IforksAbstraction() {

	is_singleton = false;
	is_empty = false;
	is_var_singleton = false;
	set_abstraction_type(INVERTED_FORK);

}

IforksAbstraction::IforksAbstraction(int v){
	is_singleton = false;
	is_empty = false;
	is_var_singleton = false;
	var = v;
//	set_variable(v);
	set_abstraction_type(INVERTED_FORK);

}


IforksAbstraction::~IforksAbstraction() {
	// TODO Auto-generated destructor stub
}

int IforksAbstraction::get_root() const {
	return var;
}

void IforksAbstraction::get_parents(vector<int>& vars) const {
	vars = parents;
}

bool IforksAbstraction::is_empty_abstraction(){
	return is_empty;

}

bool IforksAbstraction::is_singleton_abstraction(){
	return is_singleton;

}


void IforksAbstraction::create(const Problem* p) {

	// Creating abstract variables, initial state.
	const vector<string> &var_name = p->get_variable_names();

	const vector<int> &var_domain = p->get_variable_domains();

	vector<int> successors = p->get_causal_graph()->get_successors(var);
	vector<int> predecessors = p->get_causal_graph()->get_predecessors(var);

	if (0 == successors.size() && 0 == predecessors.size()) {
		is_var_singleton = true;
	}

	vector<int> abs_vars;
	vector<int> orig_vars;
	orig_vars.push_back(var);

	for (int i=0; i<predecessors.size(); i++) {
		parents.push_back(predecessors[i]);  //saving the leafs with goals
		orig_vars.push_back(predecessors[i]);
	}

	if (1==orig_vars.size()) {
		is_singleton = true;
	}

	//Creating the empty mapping
	OpHashVarProjMapping* map = new OpHashVarProjMapping();
	map->set_original(p);
	map->set_original_vars(orig_vars);

	abs_vars.assign(var_name.size(),-1);

	int var_count = orig_vars.size();

	vector<string> new_var_name;
	vector<int> new_var_domain;

	new_var_name.resize(var_count);
	new_var_domain.resize(var_count);

	for(int i = 0; i < var_count; i++) {
		abs_vars[orig_vars[i]] = i;
		new_var_name[i] = var_name[orig_vars[i]];
		new_var_domain[i] = var_domain[orig_vars[i]];
	}

	map->set_abstract_vars(abs_vars);

	state_var_t* buf = new state_var_t[var_count];
	for(int i = 0; i < var_count; i++)
		buf[i] = p->get_initial_state()->get_buffer()[orig_vars[i]];

//	const State* init_state = new State(buf, var_count);
	const State* init_state = new State(buf);

	// Creating abstract goal
	vector<pair<int, int> > orig_goal;
	p->get_goal(orig_goal);
	vector<pair<int, int> > g;
	for(int i = 0; i < orig_goal.size() ; i++){
		if (abs_vars[orig_goal[i].first] != -1){
			pair<int, int> new_goal;
			new_goal.first = abs_vars[orig_goal[i].first];
			new_goal.second = orig_goal[i].second;
			g.push_back(new_goal);
		}
	}

	// Creating abstract action
	const vector<Operator*> &orig_ops = p->get_operators();

//	int num_orig_ops = orig_ops.size();

	// Adding axioms as zero-cost operators
	vector<Operator*> ops, axioms;
	vector<pair<Operator*, Operator*> > ops_to_add;

	vector<Operator> axi = p->get_axioms();
	for(int i = 0; i < axi.size(); i++){
		axioms.push_back(&axi[i]);
	}

	abstract_actions(abs_vars,orig_ops,ops,ops_to_add);
	abstract_actions(abs_vars,axioms,ops,ops_to_add);

	vector<Operator> no_axioms;
	Problem* abs_prob = new Problem(new_var_name, new_var_domain, init_state, g, ops, no_axioms, false);

	map->set_abstract(abs_prob);
	map->initialize();

	for (int i = 0; i < ops_to_add.size(); i++)
		map->add_abs_operator(ops_to_add[i].first,ops_to_add[i].second);

	set_mapping(map);
}


void IforksAbstraction::abstract_action(const vector<int>& abs_vars, Operator* op, vector<Operator*>& abs_op){

	//TODO: Currently the operator is decomposed by effects, and not by affected variables.
	// Change of this code will require also a change of the BoundedIfork class.

	int i = root_unconditional_prepost_index(op);
	if (i>=0) {
		vector<Prevail> new_prv, empty_prv;
		vector<Prevail> prv = op->get_prevail();
		vector<PrePost> pre = op->get_pre_post();

		//Constructing the prevail vector from the original prevails and the post-conditions of
		// other unconditional preposts
		for(int j = 0; j< prv.size(); j++) {
			assert(-1 != abs_vars[prv[j].var]);
			new_prv.push_back(Prevail(abs_vars[prv[j].var], prv[j].prev));
		}

		for(int j = 0; j< pre.size(); j++) {
			if (i!=j) {
				assert(-1 != abs_vars[pre[j].var]);
				if (pre[j].cond.size() == 0) {
					new_prv.push_back(Prevail(abs_vars[pre[j].var], pre[j].post));
				}
			}
		}

		for(int j = 0; j< pre.size(); j++) {

			string nm;
#ifdef DEBUGMODE

			nm = op->get_name();
			const int max_str_len(42);
			char my_string[max_str_len+1];
			snprintf(my_string, max_str_len, "::ifork1::%d::%#x",j, (unsigned int) this);
			nm += my_string;
#endif

			vector<PrePost> new_pre;
			// if pre value is not defined, then condition on that variable (if defined) will
			// provide the pre value.
			int new_var_pre = pre[j].pre;
			if (-1 == new_var_pre) {
				new_var_pre = get_value_for_var(pre[j].var,pre[j].cond);
			}
			new_pre.push_back(PrePost(abs_vars[pre[j].var], new_var_pre, pre[j].post, empty_prv));

			assert(-1 != abs_vars[pre[j].var]);
			//new_pre.push_back(PrePost(abs_vars[pre[j].var], pre[j].pre, pre[j].post, pre[j].cond));
			Operator* new_op;
			if (i==j) {
				new_op = new Operator(op->is_axiom(), new_prv, new_pre, nm, op->get_double_cost());
			}
			else {
				new_op = new Operator(op->is_axiom(), empty_prv, new_pre, nm, op->get_double_cost());
			}
			abs_op.push_back(new_op);
		}
		return;
	}

	vector<PrePost> pre = op->get_pre_post();
	vector<Prevail> empty_prv;

	for(int j = 0; j< pre.size(); j++) {
		if (abs_vars[pre[j].var] >= 0) {
			string nm;
#ifdef DEBUGMODE

			nm = op->get_name();
			const int max_str_len(42);
			char my_string[max_str_len+1];
			snprintf(my_string, max_str_len, "::ifork2::%d::%#x",j, (unsigned int) this);
			nm += my_string;
#endif

			vector<PrePost> new_pre;
			// if pre value is not defined, then condition on that variable (if defined) will
			// provide the pre value.
			int new_var_pre = pre[j].pre;
			if (-1 == new_var_pre) {
				new_var_pre = get_value_for_var(pre[j].var,pre[j].cond);
			}
			new_pre.push_back(PrePost(abs_vars[pre[j].var], new_var_pre, pre[j].post, empty_prv));

			//new_pre.push_back(PrePost(abs_vars[pre[j].var], pre[j].pre, pre[j].post, pre[j].cond));
			Operator* new_op;
			vector<Prevail> new_prv;

			if (pre[j].var == var) { // when changing the root variable
				vector<Prevail> prv = op->get_prevail();
				vector<PrePost> pre = op->get_pre_post();

				for(int i=0; i<pre.size(); i++) {
					if (i!=j) {
						assert(-1 != abs_vars[pre[i].var]);
						if (pre[i].cond.size() == 0) {
							// Prevail consists of the effects of
							// unconditioned variables. Also, if condition on this variable
							// is defined, it should be used as a prevail instead of the effect.
							int new_prv_val = -1;
							if (pre[i].var != var) {
								new_prv_val = get_value_for_var(pre[i].var,pre[j].cond);
							}
							if (-1 == new_prv_val) {
								new_prv_val = pre[i].post;
							}
							new_prv.push_back(Prevail(abs_vars[pre[i].var], new_prv_val));
						}
					}
				}
				// Prevail consists also of the original prevail.
				for(int i=0; i<prv.size(); i++) {
					assert(-1 != abs_vars[prv[i].var]);
					new_prv.push_back(Prevail(abs_vars[prv[i].var], prv[i].prev));
				}
			}

			new_op = new Operator(op->is_axiom(), new_prv, new_pre, nm, op->get_double_cost());
			abs_op.push_back(new_op);
		}
	}
}

int IforksAbstraction::root_unconditional_prepost_index(Operator* op){

	vector<PrePost> pre = op->get_pre_post();
	for(int i = 0; i < pre.size() ; i++)
		if ((pre[i].var == var) && (pre[i].cond.size() == 0))
			return i;
	return -1;
}


int IforksAbstraction::root_prepost_index(Operator* op){

	vector<PrePost> pre = op->get_pre_post();
	for(int i = 0; i < pre.size() ; i++)
		if (pre[i].var == var)
			return i;
	return -1;
}

int IforksAbstraction::get_value_for_var(int v, vector<Prevail>& prv){

	for(int ind = 0; ind < prv.size(); ind++) {
		if (prv[ind].var == v) {
			return prv[ind].prev;
		}
	}
	return -1;
}
