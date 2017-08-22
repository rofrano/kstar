#ifndef KSTAR_H
#define KSTAR_H

#include "successor_generator.h"
#include "plan_reconstructor.h"
#include "kstar_types.h"

#include "../search_engines/top_k_eager_search.h"

#include <memory>

namespace kstar {

class KStar : public top_k_eager_search::TopKEagerSearch
{
protected:
	int optimal_solution_cost;
	int num_node_expansions;
	std::priority_queue<Node> queue_djkstra;
    std::unordered_map<Node, Node> parent_node;
	std::unordered_set<Edge> cross_edge;
	std::unique_ptr<PlanReconstructor> plan_reconstructor;
	std::shared_ptr<SuccessorGenerator> pg_succ_generator;
    // root of the path graph
   	shared_ptr<Node> pg_root;

	void initialize_djkstra();
	// djkstra search return true if k solutions have been found and false otherwise
	bool djkstra_search();
	bool enough_nodes_expanded();
	void resume_astar();
    void init_tree_heaps(Node node);
	vector<Sap> djkstra_traceback(Node& top_pair);
	vector<Sap> compute_sidetrack_seq(Node& top_pair, vector<Sap>& path);
	void add_plan(Node& p);
	bool enough_plans_found();
	void set_optimal_plan_cost();
	virtual ~KStar() = default;
public:
	KStar (const options::Options &opts);
	void search() override;
};
}

#endif 