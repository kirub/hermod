#include "framework.h"

#include "unit_serialization.h"
#include "unit_fragments.h"

int main()
{
	unit::CheckPassed(UnitTests_Serialization);
	unit::CheckPassed(UnitTests_Fragments);

	return 0;
}