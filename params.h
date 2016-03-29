/*
 * params.h
 *
 *  Created on: 15-May-2014
 *      Author: pranavs
 */

#ifndef PARAMS_H_
#define PARAMS_H_

#include <cmath>
#include <vector>
#include <complex>
#include <random>
#include "cell.h"
#include "user.h"

class cell;

class params {
public:
	params();
	virtual ~params();

	// paramters
	double radius, noise_pow, tx_pow; 	// cell radius, noise power per slot and tx power per slot
	static constexpr double carrier 			= 				20000000;
	static constexpr double ISD				= 				500; // inter site distance (in meters)
	static const int cells 			= 				19;  // total cells in a cluster

	// users per cell
	static const int edge_users[cells];//			=				{4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static const int centre_users[cells];//			=				{10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

	static const int total_slots 		= 				10;  // slots per channel realization
	static constexpr double slot_bw 			= 				100000; // 100 kHz
	static constexpr double noise_psd		= 				-174; 	// dBm/Hz
	static constexpr double total_tx_pow		= 				46; 	// dBm
	static const int realizations				=				1000;	// number of channel realizations

	std::vector< std::complex <double> > base_stations;
	std::vector<cell*> all_cells;			// vector of pointers to all the cells
	std::vector<user*> all_users;			// vector of all the users in all the cells
	std::default_random_engine generator;

	// this resets all the simulation results
	// Begin simulations with different assumption but with same UE and Cells
	void reset_sim();

	// log whatever params you want
	void log_params();
};

#endif /* PARAMS_H_ */
