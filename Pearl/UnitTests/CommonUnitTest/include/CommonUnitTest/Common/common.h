#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Avoid depth tests in Release because the asserts are disabled there
#if !defined(PR_ASSERTENABLE)
	#define EXPECT_DEATH() 
#endif