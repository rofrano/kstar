#include "kstar_types.h"

namespace kstar {

class PlanReconstructor {
	std::unordered_map<Node, Node> &parent_node;
	std::unordered_set<Edge> &cross_edge;
	StateID goal_state;	
	StateRegistry* state_registry;
	SearchSpace* search_space;

public:
	PlanReconstructor(std::unordered_map<Node, Node>& parent_sap,
					   std::unordered_set<Edge>& cross_edge,
					   StateID goal_state,
					   StateRegistry* state_registry,
					   SearchSpace* search_space);
	
	virtual ~PlanReconstructor() = default;
	std::vector<Node> djkstra_traceback(Node node);
	std::vector<Node> compute_sidetrack_seq(std::vector<Node>& path);
	void extract_plan(vector<Node>& seq, Plan &plan, StateSequence &state_seq);
	// Checks whether an extracted plan is simple
	bool is_simple_plan(StateSequence seq, StateRegistry* state_registry);
    //bool is_simple_path(Node node);
	void set_goal_state(StateID goal_state);
	void add_plan(Node node, std::vector<Plan>& top_k_plans);
};
}