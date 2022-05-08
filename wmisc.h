#pragma once

#if !defined(__cpp_inline_variables) || !defined(__cpp_static_assert)
#error
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <comdef.h>
#include <shellapi.h>
#include <eh.h>

#include <cassert>
#include <filesystem>
#include <ostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <streambuf>
#include <system_error>
#include <type_traits>
#include <utility>


#define MACRO_CURRENT_LOCATION() __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")"

template<class CharT>
class debug_streambuf : public std::basic_streambuf<CharT>
{
	CharT * m_buf;

	void output_debug(CharT * p, CharT * q)
	{
		static_assert(false);
	}

public:
	debug_streambuf(std::size_t len) : m_buf(new CharT[len])
	{
		this->setp(m_buf, m_buf + len - 1);
		this->setg(m_buf, m_buf, m_buf + len - 1);
	}

	~debug_streambuf()
	{
		delete[] m_buf;
	}

protected:
	int sync() override
	{
		CharT * p = this->pbase();
		CharT * q = this->pptr();

		output_debug(p, q);

		this->pbump((int) (p - q));
		return 0;
	}

	using int_type = typename std::basic_streambuf<CharT>::int_type;

	int_type overflow(int_type ch) override
	{
		this->sync();
		this->sputc(ch);
		return false;
	}
};

template<>
inline void debug_streambuf<char>::output_debug(char * p, char * q)
{
	*q = '\0';
	::OutputDebugStringA(p);
}

template<>
inline void debug_streambuf<wchar_t>::output_debug(wchar_t * p, wchar_t * q)
{
	*q = L'\0';
	::OutputDebugStringW(p);
}

inline thread_local debug_streambuf<char> g_dbgbuf(4096);
inline thread_local debug_streambuf<wchar_t> g_wdbgbuf(2048);

inline thread_local std::ostream cdbg(&g_dbgbuf);
inline thread_local std::wostream wcdbg(&g_wdbgbuf);


class current_location
{
	const char * m_file;
	const int m_line;

public:
	current_location(const char * file = __builtin_FILE(), int line = __builtin_LINE()) : m_file(file), m_line(line)
	{}

	friend std::ostream & operator<<(std::ostream & os, const current_location & loc)
	{
		return os << loc.m_file << '(' << loc.m_line << "):";
	}

	friend std::wostream & operator<<(std::wostream & os, const current_location & loc)
	{
		return os << loc.m_file << L'(' << loc.m_line << L"):";
	}
};

enum class dump_char_t { sjis };

class put_dump
{
	const void * m_data;
	const std::size_t m_size;
	const dump_char_t m_type;

public:
	put_dump(void * data, std::size_t size, dump_char_t type = dump_char_t::sjis) : m_data(data), m_size(size), m_type(type)
	{}

	std::ostream & output_stream(std::ostream & os) const;
	std::wostream & output_stream(std::wostream & os) const;

	friend std::ostream & operator<<(std::ostream & os, const put_dump & dump)
	{
		return dump.output_stream(os);
	}

	friend std::wostream & operator<<(std::wostream & os, const put_dump & dump)
	{
		return dump.output_stream(os);
	}
};


class CommandLineParameters
{
	int m_count;
	const wchar_t ** m_first;

public:
	CommandLineParameters() : CommandLineParameters(::GetCommandLineW())
	{}

	CommandLineParameters(const wchar_t * commandLine) :
		m_count(0),
		m_first(const_cast<const wchar_t **>(::CommandLineToArgvW(commandLine, &m_count)))
	{}

	~CommandLineParameters()
	{
		::LocalFree(m_first);
	}

	CommandLineParameters(const CommandLineParameters &) = delete;
	CommandLineParameters & operator=(const CommandLineParameters &) = delete;

	const wchar_t ** begin() const
	{
		return m_first;
	}

	const wchar_t ** end() const
	{
		return m_first + m_count;
	}

	std::size_t size() const noexcept
	{
		return m_count;
	}
};


struct structured_exception : std::system_error
{
	structured_exception(unsigned int code)
		: std::system_error(code, std::system_category())
	{}

	~structured_exception() = default;
};

class seh_exception_translator
{
	const _se_translator_function m_translator;

public:
	seh_exception_translator()
		: m_translator(::_set_se_translator([](unsigned int code, EXCEPTION_POINTERS *) { throw structured_exception(code); }))
	{}

	~seh_exception_translator() noexcept
	{
		_set_se_translator(m_translator);
	}
};

inline void convert(const char * mbs, size_t len, std::wstring & wcs)
{
	if (int cch = ::MultiByteToWideChar(CP_THREAD_ACP, 0, mbs, (int) len, nullptr, 0); cch > 0)
	{
		wcs.resize(cch);

		::MultiByteToWideChar(CP_THREAD_ACP, 0, mbs, (int) len, wcs.data(), cch);
	}
}

inline std::wstring convert(const char * mbs, size_t len)
{
	std::wstring wcs;
	convert(mbs, len, wcs);
	return wcs;
}

inline std::wstring convert(const char * mbs)
{
	std::wstring wcs;
	convert(mbs, std::strlen(mbs), wcs);
	return wcs;
}

inline std::wstring convert(const std::string_view & mbs)
{
	std::wstring wcs;
	convert(mbs.data(), mbs.size(), wcs);
	return wcs;
}


inline void convert(const wchar_t * wcs, size_t len, std::string & mbs)
{
	if (int cch = ::WideCharToMultiByte(CP_THREAD_ACP, 0, wcs, (int) len, nullptr, 0, nullptr, nullptr); cch > 0)
	{
		mbs.resize(cch);

		::WideCharToMultiByte(CP_THREAD_ACP, 0, wcs, (int) len, mbs.data(), cch, nullptr, nullptr);
	}
}

inline std::string convert(const wchar_t * wcs, size_t len)
{
	std::string mbs;
	convert(wcs, len, mbs);
	return mbs;
}

inline std::string convert(const wchar_t * wcs)
{
	std::string mbs;
	convert(wcs, std::wcslen(wcs), mbs);
	return mbs;
}

inline std::string convert(const std::wstring_view & wcs)
{
	std::string mbs;
	convert(wcs.data(), wcs.size(), mbs);
	return mbs;
}


template<typename T, typename U>
constexpr T narrow_cast(U && u)
{
	static_assert(std::is_unsigned<std::remove_reference<T>::type>::value);
	static_assert(std::is_unsigned<std::remove_reference<U>::type>::value);

	if (u <= std::numeric_limits<T>::max())
	{
		return static_cast<T>(std::forward<U>(u));
	}

	throw std::runtime_error(MACRO_CURRENT_LOCATION());
}


// wstring::data() は長さ size() + 1 の配列を返し NULL で終わることが保証されている
// 
// * https://cpprefjp.github.io/reference/string/basic_string/data.html
// * https://docs.microsoft.com/ja-jp/cpp/standard-library/basic-string-class?view=msvc-170#data
//
// 最後の NULL を変更することは許されないが、NULL を書き込むのは許される
// 
// * https://qiita.com/yumetodo/items/24d21d97e04977b78b45

inline std::wstring GetEnvironmentStringA(const wchar_t * name)
{
	// (A) 呼び出しを１回で済ませるのを諦めて、最初に長さを調べる

	std::wstring buf;

	if (auto size = ::GetEnvironmentVariable(name, nullptr, 0); size > 0)
	{
		buf.resize(size - 1);

		::GetEnvironmentVariable(name, buf.data(), size);
	}

	return buf;
}

static const auto g_wstring_initital_capacity = std::wstring().capacity();

inline std::wstring GetEnvironmentStringB(const wchar_t * name)
{
	// (B) 呼び出しが１回で済むかもしれない可能性に賭けて、ある程度の長さを想定しておく

	std::wstring buf(g_wstring_initital_capacity, L'\0');

	DWORD size{};

	while ((size = ::GetEnvironmentVariable(name, buf.data(), narrow_cast<DWORD>(buf.size() + 1))) > buf.size())
	{
		buf.resize(size - 1);
	}

	if (size < buf.size())
	{
		buf.resize(size);
	}

	return buf;
}


// このモジュールのインスタンスハンドルを取得する
//
// * 参照 https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483
//
extern "C" IMAGE_DOS_HEADER __ImageBase;


inline std::filesystem::path GetModuleFilePath(const std::wstring::size_type n = MAX_PATH)
{
	std::wstring buff(n, L'\0');

	for (;;)
	{
		const auto length = ::GetModuleFileName((HINSTANCE) &__ImageBase, buff.data(), narrow_cast<DWORD>(buff.size() + 1));
		const auto error = ::GetLastError();

		if (length == 0)
		{
			throw std::system_error(error, std::system_category(), MACRO_CURRENT_LOCATION());
		}

		if (length > buff.size())
		{
			assert(error == ERROR_INSUFFICIENT_BUFFER);
			::SetLastError(NO_ERROR);

			// バッファが不足した場合 buff.size() + 1 が返される（必要なサイズではない）
			buff.resize(buff.size() + n);
		}
		else
		{
			// 正常終了した場合 NULL を含まない文字数が返される
			buff.resize(length);
			break;
		}
	}

	return buff;
}
