#pragma once

#include <vector>

#define luby_unit 16000U

/**
 * @brief Restart scheduler proposed in [Luby et al., 1993].
 *        The code below is modified from Tinisat. 
 *        Original copyright is shown below.
 * @copyright Copyright 2007 Jinbo Huang.
 *            This project is released under the GNU General Public License.
 */
class Luby {

    std::vector<unsigned> seq;
    unsigned index;
	unsigned k;

public:

	Luby(): index(0), k(1) {}

    unsigned next() {
		if(++index == ((1U << k) - 1U))
			seq.push_back(1U << (k++ - 1U));
		else
			seq.push_back(seq[index - (1U << (k - 1U))]);
		return seq.back() * luby_unit;
	}
};