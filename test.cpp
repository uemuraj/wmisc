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
	EXPECT_EQ(0, convert(L"あああ").compare("あああ"));
	EXPECT_EQ(0, convert(L"あああ"s).compare("あああ"));
	EXPECT_EQ(0, convert(L"あああ", 3).compare("あああ"));
}

TEST(ConvertStringTest, Multi2Wide)
{
	EXPECT_EQ(0, convert("あああ").compare(L"あああ"));
	EXPECT_EQ(0, convert("あああ"s).compare(L"あああ"));
	EXPECT_EQ(0, convert("あああ", 6).compare(L"あああ"));
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

TEST(NarrowCastTest, Signed)
{
	EXPECT_EQ(LONG_MIN, narrow<LONG>((LONG64) LONG_MIN));
	EXPECT_EQ(LONG_MAX, narrow<LONG>((LONG64) LONG_MAX));

	EXPECT_THROW(narrow<LONG>((LONG64) LONG_MIN - 1), std::runtime_error);
	EXPECT_THROW(narrow<LONG>((LONG64) LONG_MAX + 1), std::runtime_error);
}

TEST(NarrowCastTest, Unsigned)
{
	EXPECT_EQ(0, narrow<ULONG>((ULONG64) 0));
	EXPECT_EQ(ULONG_MAX, narrow<ULONG>((ULONG64) ULONG_MAX));

	EXPECT_THROW(narrow<ULONG>((ULONG64) -1), std::runtime_error);
	EXPECT_THROW(narrow<ULONG>((ULONG64) ULONG_MAX + 1), std::runtime_error);
}

TEST(NarrowCastTest, SignedToUnsigned)
{
	EXPECT_EQ(0, narrow<ULONG>((LONG64) 0));
	EXPECT_EQ(ULONG_MAX, narrow<ULONG>((LONG64) ULONG_MAX));

	EXPECT_THROW(narrow<ULONG>((LONG64) -1), std::runtime_error);
	EXPECT_THROW(narrow<ULONG>((LONG64) ULONG_MAX + 1), std::runtime_error);
}

TEST(NarrowCastTest, UnsignedToSigned)
{
	EXPECT_EQ(0, narrow<LONG>((ULONG64) 0));
	EXPECT_EQ(LONG_MAX, narrow<LONG>((ULONG64) LONG_MAX));

	EXPECT_THROW(narrow<LONG>((ULONG64) - 1), std::runtime_error);
	EXPECT_THROW(narrow<LONG>((ULONG64) LONG_MAX + 1), std::runtime_error);
}

TEST(NarrowCastTest, None)
{
	EXPECT_EQ(0, narrow<LONG>((LONG) 0));
	EXPECT_EQ(0, narrow<LONG>((ULONG) 0));
	EXPECT_EQ(0, narrow<ULONG>((LONG) 0));
	EXPECT_EQ(0, narrow<ULONG>((ULONG) 0));

	EXPECT_EQ(-1, narrow<LONG>((LONG) -1));
	EXPECT_EQ(-1, narrow<LONG>((ULONG) -1));
	EXPECT_EQ(-1, narrow<ULONG>((LONG) -1));
	EXPECT_EQ(-1, narrow<ULONG>((ULONG) -1));
#if 0
	// コンパイルエラーとなる
	EXPECT_EQ(0, narrow<LONG64>((LONG) 0));
	EXPECT_EQ(0, narrow<LONG64>((ULONG) 0));
	EXPECT_EQ(0, narrow<ULONG64>((LONG) 0));
	EXPECT_EQ(0, narrow<ULONG64>((ULONG) 0));
#endif
}
