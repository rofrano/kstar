#ifndef SEARCH_ENGINES_TOP_K_EAGER_SEARCH_H
#define SEARCH_ENGINES_TOP_K_EAGER_SEARCH_H

#include "../search_engine.h"
#include "../option_parser.h"
#include "../open_lists/open_list.h"
#include "../state_action_pair.h"

#include <memory>
#include <algorithm>
#include <vector>
#include <queue>

class GlobalOperator;
class Heuristic;
class PruningMethod;
class ScalarEvaluator;
namespace options {
class Options;
}

namespace top_k_eager_search {
typedef shared_ptr<StateActionPair> Sap;

struct SearchControl {
	bool interrupt_immediatly = false;
    int optimal_solution_cost = std::numeric_limits<int>::max();
	int d = -1;
	int f_u = -1;
	bool check_interrupt() {
		if (interrupt_immediatly || optimal_solution_cost + d <= f_u) {
			return true;
		}
		return false;
	}
};



class TopKEagerSearch : public SearchEngine {
    const bool reopen_closed_nodes;
protected:
	const int number_of_plans;
    std::unique_ptr<StateOpenList> open_list;
    ScalarEvaluator *f_evaluator;
    std::vector<Heuristic *> heuristics;
    std::vector<Heuristic *> preferred_operator_heuristics;
    std::shared_ptr<PruningMethod> pruning_method;
	bool interrupt_search;
	bool first_plan_found;
	long int num_saps;
	StateID goal_state = StateID::no_state;
    bool all_nodes_expanded = false;
    SearchControl search_control;
	std::vector<Plan> top_k_plans;
   	PerStateInformation<vector<Sap>> incomming_heap;
	PerStateInformation<vector<Sap>> tree_heap;

	int get_f_value(StateID id);
    std::pair<SearchNode, bool> fetch_next_node();
    void start_f_value_statistics(EvaluationContext &eval_context);
    void update_f_value_statistics(const SearchNode &node);
    void reward_progress();
    void print_checkpoint_line(int g) const;
    virtual void initialize() override;
    virtual SearchStatus step() override;
	void output_plans();
	void print_plan(Plan plan, bool generates_multiple_plan_files);
	void interrupt();
	void resume(SearchControl &search_control);
	void add_incomming_edge(SearchNode node, const GlobalOperator *op,
                             SearchNode succ_node);
	void remove_tree_edge(GlobalState s);
    void sort_and_remove(GlobalState  s);
    // TODO: move to util
	std::string get_node_label(StateActionPair &edge);
	std::string get_node_name(StateActionPair &edge);

public:
    explicit TopKEagerSearch(const options::Options &opts);
    virtual ~TopKEagerSearch() = default;
    virtual void print_statistics() const override;
	void init_tree_heap(GlobalState& state);
};

void add_top_k_option(OptionParser &parser);
void add_pruning_option(OptionParser &parser);
}

#endif