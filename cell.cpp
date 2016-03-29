/*
 * cell.cpp
 *
 *  Created on: 14-May-2014
 *      Author: pranavs
 */

//#include <iostream>

#include "cell.h"
#include <algorithm>
#include "misc.h"

cell::cell(unsigned int idx, params* p_in) {
	p				=			p_in;
	radius			= 			p->ISD/sqrt(3);
//	time_idx		=			0;
	index			=			idx;

	centre 			=			p->base_stations[idx];

	edge_UE 		= 			p->edge_users[idx];
	centre_UE		= 			p->centre_users[idx];
	extra_dps_UE	=			0;

	int users_now	=			p->all_users.size();

	double temp_rad, temp_angle;
	std::uniform_real_distribution<double> dist_angle(- PI,PI);

	// code for edge users
	std::uniform_real_distribution<double> dist_rad(4.0 - 2 *sqrt(3),1.0);
	user* temp_user;
	edge_users.reserve(edge_UE);
	for(idx = 0; idx < edge_UE; idx++) {
		temp_rad	=			radius * sqrt(dist_rad(p->generator));
		temp_angle	=			dist_angle(p->generator);
		temp_user	=			new user(p, index, users_now, centre + std::complex<double> (temp_rad * cos(temp_angle),temp_rad * sin(temp_angle)));
		edge_users.push_back(temp_user);
		users_now++;
	}

	// code for centre users
	dist_rad		=			std::uniform_real_distribution<double> (0.0,4.0 - 2 *sqrt(3));
	centre_users.reserve(centre_UE);
	for(idx = 0; idx < centre_UE; idx++) {
		temp_rad	=			radius * sqrt(dist_rad(p->generator));
		temp_angle	=			dist_angle(p->generator);
		temp_user	=			new user(p, index, users_now, centre + std::complex<double> (temp_rad * cos(temp_angle),temp_rad * sin(temp_angle)));
		centre_users.push_back(temp_user);
		users_now++;
	}

	// reserve space for DPS users -- just a thumb rule
	extra_dps_users.reserve(centre_UE);
}

cell::~cell() {
	while(!edge_users.empty()) delete edge_users.back(), edge_users.pop_back();
	while(!centre_users.empty()) delete centre_users.back(), centre_users.pop_back();
}

void cell::schedule_multiple_RB(std::vector<user *> users) {
//	std::cout << "\t scheduling cell " << index << " with " << users.size() << " users" << std::endl;

	std::vector<bool> is_muted(p->cells, false);

	unsigned int base_slots		=		p->total_slots / users.size();
	unsigned int excess_slots	=		p->total_slots % users.size();

	for(unsigned int idx = 0; idx < users.size(); idx++) {
		users[idx]->get_sinr(index, is_muted);
	}

	std::sort(users.begin(), users.end(), misc::comp);

	for(unsigned int idx = 0; idx < users.size(); idx++) {
		unsigned int temp2		=		base_slots + ((idx >= (users.size() - excess_slots ))? 1 : 0);
		double inst_rate		=		log2(1 + users[idx]->inst_sinr) * temp2 / p->total_slots;

		users[idx]->update_rate(index, inst_rate);
//		cout << "average rate after = " << users[idx]->avg_rate << endl;
	}
	return;
}

void cell::reset_sim(){
	// reset all the simulation results in all the users
	for(unsigned int user_idx = 0; user_idx < centre_UE; user_idx++){
		centre_users[user_idx]->reset_sim();
	}

	for(unsigned int user_idx = 0; user_idx < edge_UE; user_idx++){
		edge_users[user_idx]->reset_sim();
	}
}

void cell::add_dps_user(user* dps_user) {
	extra_dps_users.push_back(dps_user);
	extra_dps_UE++;

	return;
}
