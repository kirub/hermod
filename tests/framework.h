#pragma once
#include <source_location>

#define DECLARE_UNIT_TEST(TestCategory) bool UnitTests_##TestCategory(std::source_location&)
#define DEFINE_UNIT_TEST(TestCategory) bool UnitTests_##TestCategory(std::source_location& location) 

#define REGISTER_LOCATION location = std::source_location::current()

namespace unit
{
	__declspec(noinline) bool CheckPassed(bool (*UnitTest)(std::source_location&));
}