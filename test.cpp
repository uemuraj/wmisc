#include "pch.h"
#include "wmisc.h"
#include <iterator>

using namespace std::literals::string_literals;

TEST(DebugConsoleTest, HelloWorld)
{
	cdbg << current_location() << "Hello World." << std::endl;
	wcdbg << current_location() << L"Hello World." << std::endl;
}

TEST(DebugConsoleTest, Overflow)
{
	auto wstr = L"0123456789ABCDEF";

	std::wstring wbuf;

	for (int i = 0; i < (2048 / 16); ++i)
	{
		wbuf.append(wstr);
	}

	wcdbg << current_location() << wbuf << std::endl;
}

TEST(CommandLineParametersTest, DefaultConstruct)
{
	CommandLineParameters params;

	for (auto param : params)
	{
		EXPECT_STREQ(VS_TARGETPATH, param);
		break;
	}

	EXPECT_LE(1u, params.size());
}

TEST(CommandLineParametersTest, OptionBlank)
{
	CommandLineParameters params(L"");

	for (auto param : params)
	{
		EXPECT_STREQ(VS_TARGETPATH, param); // !!!
	}

	EXPECT_EQ(1, params.size());
}

TEST(CommandLineParametersTest, Option1)
{
	CommandLineParameters params(L"/help");

	for (auto param : params)
	{
		EXPECT_STREQ(L"/help", param); // !!!
	}

	EXPECT_EQ(1, params.size());
}

TEST(CommandLineParametersTest, OptionNull)
{
	seh_exception_translator translator;

	try
	{
		CommandLineParameters params(nullptr);
		ADD_FAILURE();
	}
	catch (const structured_exception & e)
	{
		EXPECT_EQ(0xC0000005, e.code().value());
	}
}

TEST(CommandLineParametersTest, Options)
{
	CommandLineParameters params(VS_TARGETPATH L" /uninstall msipatch.msp /package Application.msi /quiet");

	for (auto it = params.begin(); it != params.end(); ++it)
	{
		std::size_t index = std::distance(params.begin(), it);

		switch (index)
		{
		case 0:
			EXPECT_STREQ(VS_TARGETPATH, *it);
			break;
		case 1:
			EXPECT_STREQ(L"/uninstall", *it);
			break;
		case 2:
			EXPECT_STREQ(L"msipatch.msp", *it);
			break;
		case 3:
			EXPECT_STREQ(L"/package", *it);
			break;
		case 4:
			EXPECT_STREQ(L"Application.msi", *it);
			break;
		case 5:
			EXPECT_STREQ(L"/quiet", *it);
			break;
		}
	}

	EXPECT_EQ(6, params.size());
}

TEST(ConvertStringTest, Wide2Multi)
{
	EXPECT_EQ(0, convert(L"‚ ‚ ‚ ").compare("‚ ‚ ‚ "));
	EXPECT_EQ(0, convert(L"‚ ‚ ‚ "s).compare("‚ ‚ ‚ "));
	EXPECT_EQ(0, convert(L"‚ ‚ ‚ ", 3).compare("‚ ‚ ‚ "));
}

TEST(ConvertStringTest, Multi2Wide)
{
	EXPECT_EQ(0, convert("‚ ‚ ‚ ").compare(L"‚ ‚ ‚ "));
	EXPECT_EQ(0, convert("‚ ‚ ‚ "s).compare(L"‚ ‚ ‚ "));
	EXPECT_EQ(0, convert("‚ ‚ ‚ ", 6).compare(L"‚ ‚ ‚ "));
}

TEST(GetEnvironmentStringTest, None)
{
	auto name = L"GetEnvironmentStringTest_None";

	EXPECT_EQ(0, GetEnvironmentStringA(name).size());
	EXPECT_EQ(0, GetEnvironmentStringB(name).size());
}

TEST(GetEnvironmentStringTest, BufferSize)
{
	auto name = L"GetEnvironmentStringTest_BufferSize";

	::SetEnvironmentVariable(name, L"123456");
	EXPECT_EQ(6, GetEnvironmentStringA(name).size());
	EXPECT_EQ(6, GetEnvironmentStringB(name).size());

	::SetEnvironmentVariable(name, L"1234567");
	EXPECT_EQ(7, GetEnvironmentStringA(name).size());
	EXPECT_EQ(7, GetEnvironmentStringB(name).size());

	::SetEnvironmentVariable(name, L"12345678");
	EXPECT_EQ(8, GetEnvironmentStringA(name).size());
	EXPECT_EQ(8, GetEnvironmentStringB(name).size());
}

TEST(GetModuleFilePathTest, BufferSize)
{
	for (int n = 1; n < (_countof(VS_TARGETPATH) + 2); ++n)
	{
		auto path = GetModuleFilePath(n);

		EXPECT_EQ(0, path.native().compare(VS_TARGETPATH));
	}
}

TEST(NarrowCastTest, Success)
{
	EXPECT_EQ(ULONG_MAX, narrow_cast<DWORD>((size_t) ULONG_MAX));
}

TEST(NarrowCastTest, Fail)
{
	EXPECT_THROW(narrow_cast<DWORD>(((size_t) ULONG_MAX) + 1), std::runtime_error);
}
