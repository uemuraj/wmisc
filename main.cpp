#include "pch.h"
#include "wmisc.h"
#include <locale>
#include <comdef.h>

// http://opencv.jp/googletestdocs/primer.html#primer-writing-the-main
// http://opencv.jp/googletestdocs/advancedguide.html#adv-global-setup-teardown

class CoInitializedEnvironment : public testing::Environment
{
	void SetUp() override
	{
		if (auto hr = ::CoInitializeEx(0, COINIT_MULTITHREADED); FAILED(hr))
		{
			throw std::system_error(hr, std::system_category(), MACRO_CURRENT_LOCATION());
		}
	}

	void TearDown() override
	{
		::CoUninitialize();
	}
};

int main(int argc, char ** argv)
{
	std::locale::global(std::locale(""));

	::testing::AddGlobalTestEnvironment(new CoInitializedEnvironment);

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
