#pragma once

#include "gtest/gtest.h"

#define GP_EXPECT_NEAR(val1, val2) EXPECT_NEAR(val1, val2, val1 * 1e-12);
