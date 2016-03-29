/*
 * scheduler.cpp
 *
 *  Created on: 19-May-2014
 *      Author: pranavs
 */

#include "scheduler.h"
#include "misc.h"
#include <iostream>
#include <algorithm>
#include <fstream>

scheduler::scheduler() {
	//  Auto-generated constructor stub
}

scheduler::~scheduler() {
	//  Auto-generated destructor stub
}

void scheduler::schedule_static_PF(params& p, std::vector<cell *> cells) {
/*
 * Centralized non-comp scheduler
 * Works with static user association and does not support muting
 */
	// initial iterations reqd to make the avg rate of all the users non-zero
	_schedule_initial_iters(p);

	std::vector<double> avg_pf;
	avg_pf.reserve(p.cells);
	std::fill(avg_pf.begin(),avg_pf.end(), 0.00);

	std::ofstream output_file;
	output_file.open("pf_static.m");

	for(int iter = 0; iter < p.realizations; iter++) { // number of channel realizations
//		std::cout << "realization number = " << iter << std::endl;
		for(unsigned int idx = 0; idx < cells.size(); idx++) { // loop over all the cells
//			std::cout << "\t cell idx = " << idx << std::endl;
			std::vector<user *> all_users;
			all_users.reserve(cells[idx]->centre_UE + cells[idx]->edge_UE);
			if (cells[idx]->centre_UE > 0)
				all_users.insert(all_users.end(), cells[idx]->centre_users.begin(), cells[idx]->centre_users.end());
			if (cells[idx]->edge_UE > 0)
				all_users.insert(all_users.end(), cells[idx]->edge_users.begin(), cells[idx]->edge_users.end());

			if(all_users.size() > 0) {
				if (0) {
					// for scheduling multiple RBs simultaneously
					cells[idx]->schedule_multiple_RB(all_users);
				}
				else {
					avg_pf[idx]		=	avg_pf[idx] + _schedule_cell(p, idx, all_users);
				}
			}
			if(iter == (p.realizations-1)) {
				avg_pf[idx]		=	avg_pf[idx] / p.realizations;
				output_file << "cell[" << idx+1 << "] = " << avg_pf[idx] << ";" << std::endl;

			}
		}
	}
	output_file.close();
	return;
}

double scheduler::_schedule_cell(params& p, unsigned int cell_idx, std::vector<user *> users) {
/*
 * This function should be called from centralized scheduler for a RB allocation for a cell
 * This is a scheduler per cell, i.e., each cell has independent scheduler non-comp
 *
 */
	std::vector<bool> is_muted(p.cells, false);

	for(unsigned int idx = 0; idx < users.size(); idx++) {
		users[idx]->get_sinr(cell_idx, is_muted);
	}

	std::sort(users.begin(), users.end(), misc::comp);

	double max_pf	=	log2(1 + users[users.size()-1]->inst_sinr) / users[users.size()-1]->avg_rate;

	for(unsigned int idx = 0; idx < users.size(); idx++) {
		double inst_rate		=		(idx == (users.size() - 1))?log2(1 + users[idx]->inst_sinr):0;

		users[idx]->update_rate(cell_idx, inst_rate);
//		std::cout << "cell " << cell_idx << " : avg rate " << users[idx]->avg_rate << std::endl;
	}

	return max_pf;
}

double scheduler::get_max_pf(params& p, unsigned int cell_idx, std::vector<bool> is_muted) {
/*
 * This function should be called from centralized scheduler for asking what is the max pf
 * for a particular cell from all the users with the specified muting pattern
 */
	if (is_muted[cell_idx] == true) {
		return 0;
	}
	else {
		double pf_metric;

		std::vector<user *> all_users;
		all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE);
		if (p.all_cells[cell_idx]->centre_UE > 0)
			all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
		if (p.all_cells[cell_idx]->edge_UE > 0)
			all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());

		for (unsigned int user_idx = 0; user_idx < all_users.size(); user_idx++) {
			double temp_pf		=	all_users[user_idx]->get_pf(cell_idx, is_muted);
			pf_metric			=	(pf_metric < temp_pf)?temp_pf:pf_metric;
		}
		return pf_metric;
	}
}

void scheduler::schedule_muting_only(params& p) {
/*
 * This function should be called from central script to schedule all cells in centrally
 * coordinated manner with muting decision decision taken on the fly based on maximization
 * of sum pf metric
 * NO DPS
 */

	// initial iterations for a postive average rate
	_schedule_initial_iters(p);

	std::vector<double> avg_pf;
	avg_pf.reserve(p.cells);
	std::fill(avg_pf.begin(),avg_pf.end(), 0.00);

	std::ofstream output_file;
	output_file.open("pf_muting_only.m");

	/*
	 * At this time all users have +ve avg rate
	 * From now we use centralized coordinated method
	 * Candidate cells for muting are tier 0 and 1, i.e. 7 over all
	 */
	unsigned int mute_candidates		=		7;
	std::vector<double> all_pf_metric;
	all_pf_metric.reserve(mute_candidates + 1);

	for(int iter = 0; iter < p.realizations; iter++) {
//		std::cout << "realization number = " << iter << "\t\t"; //<< std::endl;
//		std::cout << p.all_cells[0]->centre_users[0]->time_idx << "\t\t";

		std::fill(all_pf_metric.begin(),all_pf_metric.end(), 0.00);

		/*
		 * For every realization, choosing the best muting is imp
		 * This is what is happening in this section
		 */
		// everyone is muted one by one
		int mute_choice = 0;
		double best_mute_pf = 0.00;
		for (unsigned int mute_idx = 0; mute_idx <= mute_candidates; mute_idx++){
			std::vector<bool> is_muted(p.cells, false);

			if(mute_idx != 0) {
				is_muted[mute_idx - 1]	=	true;
			}

			for(unsigned int cell_iter = 0; cell_iter < p.cells; cell_iter++){ //p.cells TODO
				// for every cell find the best r/R user
				if (cell_iter < mute_candidates)
					all_pf_metric[mute_idx]	=		all_pf_metric[mute_idx] + get_max_pf(p,cell_iter,is_muted);
			}

			if(all_pf_metric[mute_idx] > best_mute_pf) {
				best_mute_pf		=	all_pf_metric[mute_idx];
				mute_choice		=	mute_idx;
			}
//			std::cout << "pf with mute index " << mute_idx <<" is " << all_pf_metric[mute_idx] << std::endl;
		}

		std::vector<bool> is_muted(p.cells, false);
		if(mute_choice != 0) {
			is_muted[mute_choice - 1]	=	true;
		}
//		std::cout << "muting cell " << mute_choice -1 << std::endl;

		for (int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			//schedule all the cells one by one with the new found muting pattern
			std::vector<user *> all_users;
			all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE);
			if (p.all_cells[cell_idx]->centre_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
			if (p.all_cells[cell_idx]->edge_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());

			if(cell_idx == (mute_choice-1)) {
				for(unsigned int idx = 0; idx < all_users.size(); idx++) {
					all_users[idx]->update_rate(cell_idx, 0);
				}
			}
			else {
				avg_pf[cell_idx]		=	avg_pf[cell_idx] + _schedule_cell_with_muting(p, cell_idx, all_users, is_muted);
			}
			if(iter == (p.realizations-1)) {
				avg_pf[cell_idx]		=	avg_pf[cell_idx] / p.realizations;
				output_file << "cell[" << cell_idx+1 << "] = " << avg_pf[cell_idx] << ";" << std::endl;
			}
		}
	}
	output_file.close();
	return;
}

double scheduler::_schedule_cell_with_muting(params& p, unsigned int cell_idx, std::vector<user *> users, std::vector<bool> is_muted){
	/*
	 * Schedule the cell with a specific muting pattern
	 */
	for(unsigned int idx = 0; idx < users.size(); idx++) {
		users[idx]->get_sinr(cell_idx, is_muted);
	}

	std::sort(users.begin(), users.end(), misc::comp);
	double max_pf	=	log2(1 + users[users.size()-1]->inst_sinr) / users[users.size()-1]->avg_rate;

	for(unsigned int idx = 0; idx < users.size(); idx++) {
		double inst_rate		=	(idx == (users.size() -1))?log2(1 + users[idx]->inst_sinr):0;

		users[idx]->update_rate(cell_idx, inst_rate);
	}
	return max_pf;
}

void scheduler::schedule_dps_only(params& p) {
/*
 * This functions performs scheduling per RB basis
 * DPS is considered but no muting
 */
	for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
		for(unsigned int user_idx = 0; user_idx < p.all_cells[cell_idx]->edge_UE; user_idx++) {
			p.all_cells[cell_idx]->edge_users[user_idx]->handle_dps();
		}
	}

	// initial iterations for a positive average rate
	_schedule_initial_iters(p);

	std::vector<double> avg_pf;
	avg_pf.reserve(p.cells);
	std::fill(avg_pf.begin(),avg_pf.end(), 0.00);

	std::ofstream output_file;
	output_file.open("pf_dps_only.m");

	/*
	 * Now that every user has a +ve avg rate, we start with the actual DPS code
	 */
	std::vector<bool> is_muted(p.cells, false);
	for (int iter = 0; iter < p.realizations; iter++) {
//		std::cout << "realization number = " << iter << std::endl; //"\t\t";//  //<< std::endl;
		std::vector<user *> best_user_choices;
		best_user_choices.reserve(p.cells);
		std::vector<user *> next_user_choices;
		next_user_choices.reserve(p.cells);
		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			std::vector<user *> all_users;
			all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE + p.all_cells[cell_idx]->extra_dps_UE);
			if (p.all_cells[cell_idx]->centre_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
			if (p.all_cells[cell_idx]->edge_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());
			if (p.all_cells[cell_idx]->extra_dps_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->extra_dps_users.begin(), p.all_cells[cell_idx]->extra_dps_users.end());

			all_users			=		_order_by_pf(p, cell_idx, all_users, is_muted);
			best_user_choices.push_back(all_users[all_users.size()-1]);
			next_user_choices.push_back(all_users[all_users.size()-2]);
		}
		/*
		 * make sure if this choice is actually the best
		 * i.e. conflict resolution
		 */
		std::vector<user *> final_user_choices;
		final_user_choices.reserve(p.cells);
		for(unsigned int cell_idx = 0; cell_idx < best_user_choices.size(); cell_idx++) {
			if(best_user_choices[cell_idx]->is_dps) {
//				std::cout<< "dps user " << std::endl;
				int cell_idx0	=	best_user_choices[cell_idx]->candidate_cells[0]->index;
				int cell_idx1	=	best_user_choices[cell_idx]->candidate_cells[1]->index;
				if (best_user_choices[cell_idx0] == best_user_choices[cell_idx1]) {
//					std::cout << "resolving conflict" << std::endl;
					// conflict resolution logic
					std::vector<bool> is_muted(p.cells, false);
					double pf_1		= best_user_choices[cell_idx0]->get_pf(cell_idx0, is_muted) + next_user_choices[cell_idx1]->get_pf(cell_idx1, is_muted);
					double pf_2		= best_user_choices[cell_idx1]->get_pf(cell_idx1, is_muted) + next_user_choices[cell_idx0]->get_pf(cell_idx0, is_muted);
					if (pf_1 > pf_2) {
						final_user_choices[cell_idx0] = best_user_choices[cell_idx0];
						final_user_choices[cell_idx1] = next_user_choices[cell_idx1];
					}
					else{
						final_user_choices[cell_idx1] = best_user_choices[cell_idx1];
						final_user_choices[cell_idx0] = next_user_choices[cell_idx0];
					}
				}
				else {
					final_user_choices[cell_idx] = best_user_choices[cell_idx];
				}
			}
			else {
//				std::cout << "non - dps" << std::endl;
				final_user_choices[cell_idx] = best_user_choices[cell_idx];
			}
		}

		std::vector<double> best_sinr;
		best_sinr.reserve(p.cells);
		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			/*
			 * Gather all the best SINRs in an array
			 */
			best_sinr.push_back(final_user_choices[cell_idx]->get_sinr(cell_idx,is_muted));
		}

		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			/*
			 * update all the rates for all the users to '0'
			 */
	//		std::cout << "scheduling with DPS, cell " << cell_idx << std::endl;

			std::vector<user *> all_users;
			all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE + p.all_cells[cell_idx]->extra_dps_UE);
			if (p.all_cells[cell_idx]->centre_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
			if (p.all_cells[cell_idx]->edge_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());

			for(unsigned int user_idx = 0; user_idx < all_users.size(); user_idx++) {
				all_users[user_idx]->update_rate(cell_idx, 0);
			}
		}

		/*
		 * Update the actual rates for the DPS users without changing the time index
		 */
		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			/*
			 * Update the actual rates for the DPS users without changing the time index
			 */
			final_user_choices[cell_idx]->update_dps_rate(cell_idx, log2(1 + best_sinr[cell_idx]));
			avg_pf[cell_idx]	=	avg_pf[cell_idx] + log2(1 + best_sinr[cell_idx]) / final_user_choices[cell_idx]->avg_rate;

			if (iter == (p.realizations-1)) {
				avg_pf[cell_idx]		=	avg_pf[cell_idx] / p.realizations;
				output_file << "cell[" << cell_idx+1 << "] = " << avg_pf[cell_idx] << ";" << std::endl;
			}
		}
	}
	output_file.close();
	return;
}

void scheduler::_schedule_initial_iters(params& p) {
	/*
	 * Initially average rate is not set for all the users (since they start from '0')
	 * At that time schedule the cells independently till all users have +ve avg rate
	 */
	for(unsigned int iter = 0; iter < p.all_cells[0]->centre_UE + p.all_cells[0]->edge_UE; iter++) {
		for(unsigned int idx = 0; idx < p.all_cells.size(); idx++) { // loop over all the cells
			std::vector<user *> all_users;
			all_users.reserve(p.all_cells[idx]->centre_UE + p.all_cells[idx]->edge_UE);
			if (p.all_cells[idx]->centre_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[idx]->centre_users.begin(), p.all_cells[idx]->centre_users.end());
			if (p.all_cells[idx]->edge_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[idx]->edge_users.begin(), p.all_cells[idx]->edge_users.end());

			if(all_users.size() > 0) {
				if (0) {
					// for scheduling multiple RBs simultaneously
					p.all_cells[idx]->schedule_multiple_RB(all_users);
				}
				else {
					// schedule each RB independently
					_schedule_cell(p, idx, all_users);
				}
			}
		}
	}
	return;
}

std::vector<user*> scheduler::_order_by_pf(params& p, unsigned int cell_idx, std::vector<user *> users, std::vector<bool> is_muted) {

	for(unsigned int idx = 0; idx < users.size(); idx++) {
		users[idx]->get_sinr(cell_idx, is_muted);
	}

	std::sort(users.begin(), users.end(), misc::comp);

	return users;
}

void scheduler::schedule_dps_and_muting(params& p) {
/*
 * This function performs scheduling when DPS and muting can both take place
 */
	for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
		/*
		 * Enabling the DPS flag and making users available for DPS
		 */
		for(unsigned int user_idx = 0; user_idx < p.all_cells[cell_idx]->edge_UE; user_idx++) {
			p.all_cells[cell_idx]->edge_users[user_idx]->handle_dps();
		}
	}

	// initial iterations for a positive average rate
	_schedule_initial_iters(p);

	std::vector<double> avg_pf;
	avg_pf.reserve(p.cells);
	std::fill(avg_pf.begin(),avg_pf.end(), 0.00);

	std::ofstream output_file;
	output_file.open("pf_dps_and_muting.m");

	/*
	 * Now that every user has a +ve avg rate, we start with the actual DPS code
	 */
	int mute_candidates		=		7;
	std::vector<double> all_pf_metric;
	all_pf_metric.reserve(mute_candidates + 1);

	for(int iter = 0; iter < p.realizations; iter++) {
//		std::cout << "realization number = " << iter << "\t\t"; //<< std::endl;

		std::fill(all_pf_metric.begin(),all_pf_metric.end(), 0.00);

		/*
		 * For every realization, choosing the best muting is imp
		 * This is what is happening in this section
		 */
		// everyone is muted one by one
		int mute_choice = 0;
		double best_mute_pf = 0.00;
		for (int mute_idx = 0; mute_idx <= mute_candidates; mute_idx++){
			std::vector<bool> is_muted(p.cells, false);

			if(mute_idx != 0) {
				is_muted[mute_idx - 1]	=	true;
			}

			/*
			 * DPS code here for the is_muted muting pattern
			 * store the best dps pf in all_pf_metric[mute_idx]
			 */
			std::vector<user *> best_user_choices;
			best_user_choices.reserve(p.cells);
			std::vector<user *> next_user_choices;
			next_user_choices.reserve(p.cells);
			for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
				std::vector<user *> all_users;
				all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE + p.all_cells[cell_idx]->extra_dps_UE);
				if (p.all_cells[cell_idx]->centre_UE > 0)
					all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
				if (p.all_cells[cell_idx]->edge_UE > 0)
					all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());
				if (p.all_cells[cell_idx]->extra_dps_UE > 0)
					all_users.insert(all_users.end(), p.all_cells[cell_idx]->extra_dps_users.begin(), p.all_cells[cell_idx]->extra_dps_users.end());

				all_users			=		_order_by_pf(p, cell_idx, all_users, is_muted);
				best_user_choices.push_back(all_users[all_users.size()-1]);
				next_user_choices.push_back(all_users[all_users.size()-2]);
			}
			/*
			 * make sure if this choice is actually the best
			 * i.e. conflict resolution
			 */
			std::vector<user *> final_user_choices;
			final_user_choices.reserve(p.cells);
			for(unsigned int cell_idx = 0; cell_idx < best_user_choices.size(); cell_idx++) {
				if(best_user_choices[cell_idx]->is_dps) {
	//				std::cout<< "dps user " << std::endl;
					int cell_idx0	=	best_user_choices[cell_idx]->candidate_cells[0]->index;
					int cell_idx1	=	best_user_choices[cell_idx]->candidate_cells[1]->index;
					if (best_user_choices[cell_idx0] == best_user_choices[cell_idx1]) {
//						std::cout << "resolving conflict" << std::endl;
						// conflict resolution logic
						std::vector<bool> is_muted(p.cells, false);
						double pf_1		= best_user_choices[cell_idx0]->get_pf(cell_idx0, is_muted) + next_user_choices[cell_idx1]->get_pf(cell_idx1, is_muted);
						double pf_2		= best_user_choices[cell_idx1]->get_pf(cell_idx1, is_muted) + next_user_choices[cell_idx0]->get_pf(cell_idx0, is_muted);
						if (pf_1 > pf_2) {
							final_user_choices[cell_idx0] = best_user_choices[cell_idx0];
							final_user_choices[cell_idx1] = next_user_choices[cell_idx1];
						}
						else{
							final_user_choices[cell_idx1] = best_user_choices[cell_idx1];
							final_user_choices[cell_idx0] = next_user_choices[cell_idx0];
						}
					}
					else {
						final_user_choices[cell_idx] = best_user_choices[cell_idx];
					}
				}
				else {
	//				std::cout << "non - dps" << std::endl;
					final_user_choices[cell_idx] = best_user_choices[cell_idx];
				}
			}

			for(int cell_idx = 0; cell_idx < mute_candidates; cell_idx++) { //  TODO mute_candidates p.cells
				if((mute_idx-1)	!= cell_idx)
					all_pf_metric[mute_idx]		=	all_pf_metric[mute_idx] + final_user_choices[cell_idx]->get_pf(cell_idx,is_muted);
			}

			if(all_pf_metric[mute_idx] > best_mute_pf) {
				best_mute_pf		=	all_pf_metric[mute_idx];
				mute_choice		=	mute_idx;
			}
//			std::cout << "pf with mute index " << mute_idx <<" is " << all_pf_metric[mute_idx] << std::endl;
		}

		std::vector<bool> is_muted(p.cells, false);
		if(mute_choice != 0) {
			is_muted[mute_choice - 1]	=	true;
		}
//		std::cout << "muting cell " << mute_choice -1 << std::endl;

		std::vector<user *> best_user_choices;
		best_user_choices.reserve(p.cells);
		std::vector<user *> next_user_choices;
		next_user_choices.reserve(p.cells);
		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			std::vector<user *> all_users;
			all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE + p.all_cells[cell_idx]->extra_dps_UE);
			if (p.all_cells[cell_idx]->centre_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
			if (p.all_cells[cell_idx]->edge_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());
			if (p.all_cells[cell_idx]->extra_dps_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->extra_dps_users.begin(), p.all_cells[cell_idx]->extra_dps_users.end());

			all_users			=		_order_by_pf(p, cell_idx, all_users, is_muted);
			best_user_choices.push_back(all_users[all_users.size()-1]);
			next_user_choices.push_back(all_users[all_users.size()-2]);
		}
		/*
		 * make sure if this choice is actually the best
		 * i.e. conflict resolution
		 */
		std::vector<user *> final_user_choices;
		final_user_choices.reserve(p.cells);
		for(unsigned int cell_idx = 0; cell_idx < best_user_choices.size(); cell_idx++) {
			if(best_user_choices[cell_idx]->is_dps) {
//				std::cout<< "dps user " << std::endl;
				int cell_idx0	=	best_user_choices[cell_idx]->candidate_cells[0]->index;
				int cell_idx1	=	best_user_choices[cell_idx]->candidate_cells[1]->index;

				if (best_user_choices[cell_idx0] == best_user_choices[cell_idx1]) {

					if((cell_idx0 == (mute_choice -1)) | (cell_idx1 == (mute_choice -1))) {
						// one of the two cells is going to be muted
						final_user_choices[cell_idx1] = best_user_choices[cell_idx1];
						final_user_choices[cell_idx0] = best_user_choices[cell_idx0];
						continue;
					}

//					std::cout << "resolving conflict" << std::endl;
					// conflict resolution logic
					std::vector<bool> is_muted(p.cells, false);
					double pf_1		= best_user_choices[cell_idx0]->get_pf(cell_idx0, is_muted) + next_user_choices[cell_idx1]->get_pf(cell_idx1, is_muted);
					double pf_2		= best_user_choices[cell_idx1]->get_pf(cell_idx1, is_muted) + next_user_choices[cell_idx0]->get_pf(cell_idx0, is_muted);
					if (pf_1 > pf_2) {
						final_user_choices[cell_idx0] = best_user_choices[cell_idx0];
						final_user_choices[cell_idx1] = next_user_choices[cell_idx1];
					}
					else{
						final_user_choices[cell_idx1] = best_user_choices[cell_idx1];
						final_user_choices[cell_idx0] = next_user_choices[cell_idx0];
					}
				}
				else {
					final_user_choices[cell_idx] = best_user_choices[cell_idx];
				}
			}
			else {
//				std::cout << "non - dps" << std::endl;
				final_user_choices[cell_idx] = best_user_choices[cell_idx];
			}
		}

		std::vector<double> best_sinr;
		best_sinr.reserve(p.cells);
		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			/*
			 * Gather all the best SINRs in an array
			 */
			if(cell_idx != (mute_choice -1))
				best_sinr.push_back(final_user_choices[cell_idx]->get_sinr(cell_idx,is_muted));
			else
				best_sinr.push_back(0.00);
		}

		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			/*
			 * update all the rates for all the users to '0'
			 */
	//		std::cout << "scheduling with DPS, cell " << cell_idx << std::endl;

			std::vector<user *> all_users;
			all_users.reserve(p.all_cells[cell_idx]->centre_UE + p.all_cells[cell_idx]->edge_UE + p.all_cells[cell_idx]->extra_dps_UE);
			if (p.all_cells[cell_idx]->centre_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->centre_users.begin(), p.all_cells[cell_idx]->centre_users.end());
			if (p.all_cells[cell_idx]->edge_UE > 0)
				all_users.insert(all_users.end(), p.all_cells[cell_idx]->edge_users.begin(), p.all_cells[cell_idx]->edge_users.end());

			for(unsigned int user_idx = 0; user_idx < all_users.size(); user_idx++) {
				all_users[user_idx]->update_rate(cell_idx, 0);
			}
		}

		/*
		 * Update the actual rates for the DPS users without changing the time index
		 */
		for(int cell_idx = 0; cell_idx < p.cells; cell_idx++) {
			/*
			 * Update the actual rates for the DPS users without changing the time index
			 */
			if(cell_idx != (mute_choice -1)) {
				final_user_choices[cell_idx]->update_dps_rate(cell_idx, log2(1 + best_sinr[cell_idx]));
				avg_pf[cell_idx]	=	avg_pf[cell_idx] + log2(1 + best_sinr[cell_idx]) / final_user_choices[cell_idx]->avg_rate;
			}
			if (iter == (p.realizations-1)) {
				avg_pf[cell_idx]		=	avg_pf[cell_idx] / p.realizations;
				output_file << "cell[" << cell_idx+1 << "] = " << avg_pf[cell_idx] << ";" << std::endl;
			}
		}
	}
	output_file.close();
	return;
}
