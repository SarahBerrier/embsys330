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
    2,
    {
        // Assignment 2 - Add 2 LED patterns here.
        // Pattern 0
       	{2, {{800,500}, {200, 500}}},
    	// Pattern 1
    	{4, {{0, 500}, {1000,500}, {0, 500}, {100,500}}}
    }
};

} // namespace APP
