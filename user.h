/*
 * user.h
 *
 *  Created on: 15-May-2014
 *      Author: pranavs
 */

#ifndef USER_H_
#define USER_H_

#include <complex>
#include <vector>
#include <cmath>
#include <random>
#include "params.h"
#include "cell.h"

class params;
class cell;

static std::normal_distribution<double> distribution(0,0.5);

class user {
public:
	user(params* p_in, int idx, int user, std::complex<double> loc);
	virtual ~user();
	double get_sinr(int serving_cell, std::vector<bool> is_muted_cell);
	double get_interference(std::vector<bool> non_interfering_cells);
	double get_rayleigh();
	void update_rate(unsigned int index, double inst_rate);
	void reset_channels();
	void reset_sim();
	void handle_dps();
	void update_dps_rate(unsigned int index, double inst_rate);
	double get_pf(int serving_cell, std::vector<bool> is_muted_cell);

	int cell_idx; // cell association based on geography
	int user_idx; //index according to the list in params
	std::complex<double> location;
	std::vector<double> distances; // distances from all the cells
	double avg_rate;			// average data rate over time
	double inst_sinr; 			// instantanous rate if gets scheduled
	unsigned int time_idx;		// time index of scheduling

	bool is_dps;				// flag to show if this user is a dps user
	std::vector<cell *> candidate_cells; 		// candidates cells for dps

	params* p;				// pointer to params
	cell* serving_cell;		// pointer to the current serving cell
	std::vector<double> current_channels;	// current realizations of the channels

private:
	void set_dist(); // set the distances from other cells
	double get_pow_dBm(int cell_idx);
	double dBm2watts(double dBm);

};

#endif /* USER_H_ */
