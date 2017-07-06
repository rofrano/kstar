#include "kstar.h"

#include "../plugin.h"
#include "../option_parser.h"
#include "../search_engines/search_common.h"
#include "../utils/util.h"
#include "../utils/countdown_timer.h"
#include "../utils/system.h"
#include "../utils/timer.h"

namespace kstar{
	KStar::KStar(const options::Options &opts) 
	:TopKEagerSearch(opts), first_solution_found(false) {
			//search_common::create_djkstra_search();
	}

	void KStar::search() {
		initialize();
		utils::CountdownTimer timer(max_time);
		while (status == IN_PROGRESS || status == INTERRUPTED) {
			status = step();
			if (timer.is_expired()) {
				cout << "Time limit reached. Abort search." << endl;
				status = TIMEOUT;
				break;
			}
			
			// Goal state has been reached for the first time	
			if (status == SOLVED && !first_solution_found) {
				interrupt();		
				first_solution_found = true;
			}

			if (status == INTERRUPTED) {
				resume();
				// Dominik: do dijkstra here		
			}
		}
		cout << "Actual search time: " << timer
         << " [t=" << utils::g_timer << "]" << endl;
	}

static SearchEngine *_parse(OptionParser &parser) {
    parser.add_option<ScalarEvaluator *>("eval", "evaluator for h-value");

	top_k_eager_search::add_top_k_option(parser);
	top_k_eager_search::add_pruning_option(parser);
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    KStar *engine = nullptr;
    if (!parser.dry_run()) {
        auto temp = search_common::create_astar_open_list_factory_and_f_eval(opts);
        opts.set("open", temp.first);
        opts.set("f_eval", temp.second);
        opts.set("reopen_closed", true);
		std::vector<Heuristic *> preferred_list;
        opts.set("preferred", preferred_list);
        engine = new KStar(opts);
    }

    return engine;
}
	
static Plugin<SearchEngine> _plugin("kstar", _parse);
}
