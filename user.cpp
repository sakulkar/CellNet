/*
 * user.cpp
 *
 *  Created on: 15-May-2014
 *      Author: pranavs
 */

#include "user.h"

#include <iostream>

user::user(params* p_in, int idx, int user, std::complex<double> loc) {

	p					=		p_in;
	cell_idx			= 		idx;
	user_idx			=		user;
	serving_cell		=		p->all_cells[idx];
	location			= 		loc;
	avg_rate			=		0.00;
	inst_sinr			=		0.00;
	time_idx			=		0;

	set_dist();
	current_channels.reserve(p->cells);
	reset_channels();

	is_dps			=		false;
	candidate_cells.reserve(0);
}

user::~user() {
/*	delete p;
	delete serving_cell;

	p = nullptr;
	serving_cell = nullptr;
*/
}

void user::set_dist() {
	distances.reserve(p->cells);

	for(int idx =0; idx < p->cells; idx++) {
		distances.push_back(abs(location - p->base_stations[idx]));
	}
}

double user::dBm2watts(double dBm) {
	double watts		=		pow(10, (dBm - 30) / 10);
	return watts;
}

double user::get_pow_dBm(int cell_idx) {
	double pow		=		p->tx_pow - (128.1 + 37.6 * log10(distances[cell_idx] / 1000));
	return pow;
}

double user::get_interference(std::vector<bool> non_interfering_cells) {
	double interference		=		0;

	for(unsigned int idx = 0; idx < non_interfering_cells.size(); idx++) {
		if(non_interfering_cells[idx] == false)
			interference		=		interference + dBm2watts(get_pow_dBm(idx)) * current_channels[idx];
	}

	return interference;
}

double user::get_sinr(int serving_cell, std::vector<bool> is_muted_cell) {
	double signal_pow		= 		dBm2watts(get_pow_dBm(serving_cell)) * current_channels[serving_cell];

	is_muted_cell[serving_cell]		=		true;

	double interference		=		get_interference(is_muted_cell);

	inst_sinr				=		signal_pow / (interference + dBm2watts(p->noise_pow));

	return inst_sinr;
}

double user::get_rayleigh() {
	double nx		=	distribution(p->generator);
	double ny		=	distribution(p->generator);

	double rayleigh =	sqrt(nx * nx + ny * ny);
	return rayleigh;
}

void user::update_rate(unsigned int index, double inst_rate) {
/*	cout << "average rate before = " << avg_rate;
	cout << " instantaneous rate = " << inst_rate << " ratio = " << inst_rate / avg_rate << endl;
*/
	serving_cell		=		p->all_cells[index];
	time_idx++;
	avg_rate			=			(avg_rate * (time_idx - 1) + inst_rate ) / time_idx;

	reset_channels();
}

void user::update_dps_rate(unsigned int index, double inst_rate) {
/*	time index is not incremented
 * this function is called only after update_rate is called for all the users with '0' during DPS
*/
	serving_cell		=		p->all_cells[index];
	avg_rate			=			(avg_rate * time_idx + inst_rate ) / time_idx;
	reset_channels();
}

void user::reset_channels() {
	for (int idx =0; idx < p->cells; idx++) {
		current_channels[idx]		=		get_rayleigh();
	}
	return;
}

void user::reset_sim() {
	time_idx	=	0;
	avg_rate	=	0.00;
	inst_sinr	=	0.00;

	return;
}

void user::handle_dps() {
	/*
	 * Ways to handle the DPS users
	 */
	is_dps			=		true;
	candidate_cells.reserve(2);
	// set the candidates
	candidate_cells.push_back(p->all_cells[cell_idx]);

	// following code for finding out the 2nd candidate cell for dps
	int idx2;		// 2nd minimum distance cell
	double dist2	= 		10 * p->radius;	// 2nd min distance
	for (int iter = 0; iter < p->cells; iter++){
		if (iter == cell_idx) {
			continue;
		}
		if (distances[iter] < dist2) {
			dist2		=		distances[iter];
			idx2		=		iter;
		}
	}
	candidate_cells.push_back(p->all_cells[idx2]);
	p->all_cells[idx2]->add_dps_user(this);

	std::cout << "DPS user with candidate cell " << idx2;
	std::cout << " at " << location << std::endl;
}

double user::get_pf(int serving_cell, std::vector<bool> is_muted_cell) {
	double sinr		=	get_sinr(serving_cell, is_muted_cell);
	double pf		=	log2(1 + sinr) / avg_rate;

	return pf;
}
