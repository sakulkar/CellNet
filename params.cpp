/*
 * params.cpp
 *
 *  Created on: 15-May-2014
 *      Author: pranavs
 */

#include "params.h"
//#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>

const int params::edge_users[cells]			=		{4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};//	{4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};	//
const int params::centre_users[cells]			=				{10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
//const int params::centre_users[cells]			=				{10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

params::params() {
	double s 			= 			sqrt(3);

	radius 				= 			ISD / s;
	tx_pow				= 			total_tx_pow - 10 * log10(total_slots);
	noise_pow 			= 			noise_psd + 10 * log10(slot_bw);

	base_stations.push_back(radius * std::complex<double>(0.0,0.0));
	base_stations.push_back(radius * std::complex<double>(s,0.0));
	base_stations.push_back(radius * std::complex<double>(s/2.0,3/2.0));
	base_stations.push_back(radius * std::complex<double>(-s/2.0,3/2.0));
	base_stations.push_back(radius * std::complex<double>(-s,0.0));
	base_stations.push_back(radius * std::complex<double>(-s/2.0,-3/2.0));
	base_stations.push_back(radius * std::complex<double>(s/2.0,-3/2.0));
	base_stations.push_back(radius * std::complex<double>(2*s,0.0));
	base_stations.push_back(radius * std::complex<double>(3*s/2,3/2.0));
	base_stations.push_back(radius * std::complex<double>(s,3.0));
	base_stations.push_back(radius * std::complex<double>(0,3.0));
	base_stations.push_back(radius * std::complex<double>(-s,3.0));
	base_stations.push_back(radius * std::complex<double>(-3*s/2.0,3.0/2));
	base_stations.push_back(radius * std::complex<double>(-2*s,0.0));
	base_stations.push_back(radius * std::complex<double>(-3*s/2,-3/2.0));
	base_stations.push_back(radius * std::complex<double>(-s,-3.0));
	base_stations.push_back(radius * std::complex<double>(0.0,-3.0));
	base_stations.push_back(radius * std::complex<double>(s,-3.0));
	base_stations.push_back(radius * std::complex<double>(3*s/2,-3/2.0));

	all_users.reserve((centre_users[0] + edge_users[0]) * cells);

	generator.seed(time(NULL));

	all_cells.reserve(cells);
	for (int idx = 0; idx < cells; idx++){
		all_cells.push_back(new cell(idx, this));
	}

}

params::~params() {
	while(!all_cells.empty()) delete all_cells.back(), all_cells.pop_back();
}

void params::reset_sim(){
	for(int cell_idx = 0; cell_idx < cells; cell_idx++) {
		all_cells[cell_idx]->reset_sim();
	}
}

void params::log_params() {
	std::ofstream myfile;
	myfile.open("params.log");

	myfile << "Following are the locations of the edge users" << std::endl << std::endl;

	for(int cell_idx = 0; cell_idx < cells; cell_idx++) {
		myfile << "cell " << cell_idx << std::endl;
		for (unsigned int user_idx = 0; user_idx < all_cells[cell_idx]->edge_UE; user_idx++) {
			myfile << "\t" << user_idx << "\t" << all_cells[cell_idx]->edge_users[user_idx]->location << std::endl;
		}
		myfile << std::endl;
	}
	return;
}
