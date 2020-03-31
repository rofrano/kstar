#ifndef KSTAR_PLAN_RECONSTRUCTOR_H
#define KSTAR_PLAN_RECONSTRUCTOR_H

#include "kstar_types.h"

namespace kstar {

class PlanReconstructor {
    std::unordered_map<Node, Node> &parent_node;
    std::unordered_set<Edge> &cross_edge;
    StateID goal_state;
    StateRegistry* state_registry;
    SearchSpace* search_space;
    Verbosity verbosity;

    std::string fact_to_pddl(std::string fact) const;
    std::string restructure_fact(std::string fact) const;
    void dump_state_with_actions(const StateID& state, const OperatorSet& actions, std::ostream& os);
    void dump_action_json(const GlobalOperator *op, std::ostream& os);
    void dump_state_json(const StateID& state, std::ostream& os);
    void action_name_parsing(std::string op_name, std::vector<std::string>& parsed);

public:
    PlanReconstructor(std::unordered_map<Node, Node>& parent_sap,
                       std::unordered_set<Edge>& cross_edge,
                       StateID goal_state,
                       StateRegistry* state_registry,
                       SearchSpace* search_space,     
                       Verbosity verbosity);

    virtual ~PlanReconstructor() = default;
    std::vector<Node> djkstra_traceback(Node node);
    std::vector<Node> compute_sidetrack_seq(std::vector<Node>& path);
    void extract_plan(vector<Node>& seq, Plan &plan, StateSequence &state_seq);
    bool is_simple_plan(StateSequence seq, StateRegistry* state_registry);
    void set_goal_state(StateID goal_state);
    void add_plan(Node node, std::vector<Plan>& top_k_plans, std::vector<StateSequence>& top_k_plans_states, bool simple_plans_only);
    void save_plans(std::vector<Plan>& top_k_plans, bool dump_plans);
    void dump_dot_plan(const Plan& plan);
    void preprocess_and_dump_state_action_pairs_to_json(std::vector<Plan>& top_k_plans, std::vector<StateSequence>& top_k_plans_states, std::string file_name);

};
}

#endif
