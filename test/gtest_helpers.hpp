#pragma once

#include <cmath>

#define GP_EXPECT_NEAR(val1, val2) \
        EXPECT_NEAR(val1, val2, std::abs((val1) * 1e-12));
