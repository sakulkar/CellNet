/*
 * scheduler.h
 *
 *  Created on: 19-May-2014
 *      Author: pranavs
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "params.h"
#include "cell.h"
#include "user.h"

#include <vector>

class scheduler {
public:
	scheduler();
	virtual ~scheduler();

	// two functions for static user associations and no muting
	void schedule_static_PF(params& p, std::vector<cell *> cells);
	double _schedule_cell(params& p, unsigned int cell_idx, std::vector<user *> users);

	// get max pf for a particular cell from all the users
	double get_max_pf(params& p, unsigned int cell_idx, std::vector<bool> is_muted);

	// initial iterations for a positive average rate : before Muting or DPS starts
	void _schedule_initial_iters(params& p);

	// static user association but coordinated muting (NO DPS)
	void schedule_muting_only(params& p);
	double _schedule_cell_with_muting(params& p, unsigned int cell_idx, std::vector<user *> users, std::vector<bool> is_muted);

	// DPS based functions (NO MUTING)
	void schedule_dps_only(params& p);
	std::vector<user*> _order_by_pf(params& p, unsigned int cell_idx, std::vector<user *> users, std::vector<bool> is_muted);

	// DPS + Muting
	void schedule_dps_and_muting(params& p);
};

#endif /* SCHEDULER_H_ */
