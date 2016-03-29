/*
 * cell.h
 *
 *  Created on: 14-May-2014
 *      Author: pranavs
 */

#ifndef CELL_H_
#define CELL_H_

#define PI 3.141592

#include <cmath>
#include <complex>
#include <vector>
//#include <deque>
#include <random>
#include "params.h"
#include "user.h"

class params;
class user;

class cell {
public:
	cell(unsigned int idx, params* p_in);
	virtual ~cell();

	void schedule_multiple_RB(std::vector<user *> users); // old per cell PF for multiple RBs

	// parameters
	double radius; 	// radius of the cell
	unsigned int index; 		// index of the cell
	std::complex<double> centre; 	// centre of the cell

	unsigned int edge_UE, centre_UE, extra_dps_UE;
	std::vector<user *> edge_users;			// vector of edge users
	std::vector<user *> centre_users;			// vector of centre users
	std::vector<user *> extra_dps_users;		// vector of dps candidates from other cells
	//user *extra_dps_users[20];

	params* p;

	void add_dps_user(user* dps_user);
	void reset_sim();
};

#endif /* CELL_H_ */
