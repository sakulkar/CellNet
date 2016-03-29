class misc {
	/*
	 * This class Misc is used for storing "comp" method which is used for comparing the users in a cell
	 * based on their PF metric
	 */
public:
	static bool comp(const user* a, const user* b) {
		bool temp;

		if((a->avg_rate == 0.00) & (b->avg_rate == 0.00)) {
			temp = (a->inst_sinr < b->inst_sinr);
		}
		else if (a->avg_rate == 0.00) {
			temp = false;
		}
		else if(b->avg_rate == 0.00) {
			temp = true;
		}
		else {
			temp = ((log2(1 + a->inst_sinr)/a->avg_rate) < (log2(1 + b->inst_sinr)/b->avg_rate));
		}
		return temp;
}
};
