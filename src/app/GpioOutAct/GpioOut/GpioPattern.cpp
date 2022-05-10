#include "GpioPattern.h"

namespace APP {

// Illustration only.
/*
GpioInterval testInterval = {1, 2};

GpioPattern testPattern = {
    2, {{800,500}, {0, 500}}
};
*/

GpioPatternSet const TEST_GPIO_PATTERN_SET = {
    3,
    {
        // Pattern 0
       	{2, {{800,500}, {200, 500}}},
    	// Pattern 1
    	{4, {{0, 500}, {1000,500}, {0, 500}, {100,500}}},
    	// Pattern 1
    	{21,
    			{
    					{10, 50}, {20, 50}, {30, 50}, {50, 50}, {70, 50}, {100, 50}, {200, 50}, {400, 50}, {500, 50}, {900, 50},  // ramp up
						{900,2000}, 																							  // constant high
    					{900, 50}, {500, 50}, {400, 50}, {200, 50}, {100, 50}, {70, 50}, {50, 50}, {30, 50}, {20, 50}, {10, 50}  // ramp up
    			}
    	}
    }
};

} // namespace APP
