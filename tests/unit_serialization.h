#pragma once
#include <source_location>

#define DECLARE_UNIT_TEST(TestCategory) bool UnitTests_##TestCategory(std::source_location&)


#define DEFINE_UNIT_TEST(TestCategory) bool UnitTests_##TestCategory(std::source_location& location) 

DECLARE_UNIT_TEST(Serialization);