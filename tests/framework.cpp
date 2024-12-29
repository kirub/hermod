#include "framework.h"
#include <string>
#include <iostream>

namespace unit
{

	std::string ParseUnitTestName(const std::source_location& unit_test_info)
	{
		const char* func_name = unit_test_info.function_name();
		std::string UnitTestName(func_name);
		std::size_t StartUnitTestName = UnitTestName.find("UnitTest");
		std::size_t StartName = UnitTestName.find_first_of("_", StartUnitTestName) + 1;
		std::size_t EndName = UnitTestName.find_first_of("(", StartName);

		return std::string(UnitTestName.begin() + StartName, UnitTestName.begin() + EndName);
	}

	bool CheckPassed(bool (*UnitTest)(std::source_location&))
	{
		std::source_location unit_test_info;
		if (UnitTest(unit_test_info))
		{
			std::string UnitTestCategoryName = ParseUnitTestName(unit_test_info);
			std::cout << "UnitTest " << UnitTestCategoryName << " Passed" << std::endl;
			return true;
		}

		return false;
	}

}