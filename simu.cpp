//============================================================================
// Name        : simu.cpp
// Author      : Pranav Sakulkar
// Version     : 1.0.0
// Copyright   : Your copyright notice
//============================================================================

#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <cstdlib>
//#include <cstdio>

#include "cell.h"
#include "scheduler.h"
#include "params.h"

int main() {

	// create a parameters object
	params p;

	scheduler my_scheduler;

	std::vector<cell *> cells = p.all_cells;
	std::ofstream myfile;

	int	is_static			=	1;
	int	is_muting			=	1;
	int	is_dps				=	1;
	int is_dps_and_muting	=	1;

	// log the edge user locations
	p.log_params();

	if(is_dps_and_muting) {
		/*
		 * DPS and muting -- dynamic allocation of users and coordinated muting
		 */
			p.reset_sim();
			my_scheduler.schedule_dps_and_muting(p);
			myfile.open("static_with_dps_and_muting_results.m");

			for(int idx = 0; idx < p.cells; idx++) {
				myfile << "cell{" << idx+1 << "}.centre = [" << std::endl;
				for(unsigned int user_idx = 0; user_idx < cells[idx]->centre_UE; user_idx++) {
	//				std::cout << cells[idx]->centre_users[user_idx]->time_idx << std::endl;
					myfile << "\t\t" << cells[idx]->centre_users[user_idx]->avg_rate << std::endl;
				}
				myfile << "];" << std::endl << std::endl;
				myfile << "cell{" << idx+1 << "}.edge = [" << std::endl;
				for(unsigned int user_idx = 0; user_idx < cells[idx]->edge_UE; user_idx++) {
					myfile << "\t\t" << cells[idx]->edge_users[user_idx]->avg_rate << std::endl;
				}
				myfile << "];" << std::endl << std::endl;
			}
			myfile.close();
	}

	if(is_static) {
	/*
	 * static allocation of users and no muting
	 */
	p.reset_sim();
	my_scheduler.schedule_static_PF(p, cells);
	myfile.open("static_results.m");

	for(int idx = 0; idx < p.cells; idx++) {
		myfile << "cell{" << idx+1 << "}.centre = [" << std::endl;
		for(unsigned int user_idx = 0; user_idx < cells[idx]->centre_UE; user_idx++) {
			myfile << "\t\t" << cells[idx]->centre_users[user_idx]->avg_rate << std::endl;
		}
		myfile << "];" << std::endl << std::endl;
		myfile << "cell{" << idx+1 << "}.edge = [" << std::endl;
		for(unsigned int user_idx = 0; user_idx < cells[idx]->edge_UE; user_idx++) {
			myfile << "\t\t" << cells[idx]->edge_users[user_idx]->avg_rate << std::endl;
		}
		myfile << "];" << std::endl << std::endl;
	}

	myfile.close();
	}

	if(is_muting) {
	/*
	 * Static allocation of users with best muting choice
	 */
		p.reset_sim();
		my_scheduler.schedule_muting_only(p);
		myfile.open("static_with_muting_results.m");

		for(int idx = 0; idx < p.cells; idx++) {
			myfile << "cell{" << idx+1 << "}.centre = [" << std::endl;
			for(unsigned int user_idx = 0; user_idx < cells[idx]->centre_UE; user_idx++) {
				myfile << "\t\t" << cells[idx]->centre_users[user_idx]->avg_rate << std::endl;
			}
			myfile << "];" << std::endl << std::endl;
			myfile << "cell{" << idx+1 << "}.edge = [" << std::endl;
			for(unsigned int user_idx = 0; user_idx < cells[idx]->edge_UE; user_idx++) {
				myfile << "\t\t" << cells[idx]->edge_users[user_idx]->avg_rate << std::endl;
			}
			myfile << "];" << std::endl << std::endl;
		}
		myfile.close();
	}

	if(is_dps) {
	/*
	 * DPS -- dynamic allocation of users (NO MUTING)
	 */
		p.reset_sim();
		my_scheduler.schedule_dps_only(p);
		myfile.open("static_with_dps_results.m");

		for(int idx = 0; idx < p.cells; idx++) {
			myfile << "cell{" << idx+1 << "}.centre = [" << std::endl;
			for(unsigned int user_idx = 0; user_idx < cells[idx]->centre_UE; user_idx++) {
//				std::cout << cells[idx]->centre_users[user_idx]->time_idx << std::endl;
				myfile << "\t\t" << cells[idx]->centre_users[user_idx]->avg_rate << std::endl;
			}
			myfile << "];" << std::endl << std::endl;
			myfile << "cell{" << idx+1 << "}.edge = [" << std::endl;
			for(unsigned int user_idx = 0; user_idx < cells[idx]->edge_UE; user_idx++) {
				myfile << "\t\t" << cells[idx]->edge_users[user_idx]->avg_rate << std::endl;
			}
			myfile << "];" << std::endl << std::endl;
		}
		myfile.close();
	}

	std::cout << "program finished" << std::endl;
	return 0;
}
