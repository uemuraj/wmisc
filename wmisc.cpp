#include "wmisc.h"
#include <mbctype.h>
#include <mbstring.h>

// SJIS コードを想定した文字ダンプ。
//
// * １回の呼び出しにつき 32 バイトまでしか処理しない
// * 漢字の泣き別れが想定される場合のみ 33 バイトを処理する
//
void SjisDump(std::ostream & os, const unsigned char * data, std::size_t size)
{
	os << " | ";

	char buff[32 + 1 + 1]{};

	for (int i = 0; (i < 32) && (size > 0); ++i, ++data, --size)
	{
		if (_ismbblead(data[0]) && (size > 1) && _ismbbtrail(data[1]))
		{
			buff[i++] = *data++, --size;
			buff[i] = *data;
		}
		else if (iscntrl(data[0]))
		{
			buff[i] = '.';
		}
		else if (isprint(data[0]))
		{
			buff[i] = *data;
		}
		else
		{
			buff[i] = ' ';
		}
	}

	os << buff;
}

void SjisDump(std::wostream & os, const unsigned char * data, std::size_t size)
{
	os << L" | ";

	wchar_t buff{}; // １文字ずつ出力する

	for (int i = 0; (i < 32) && (size > 0); ++i, ++data, --size)
	{
		if (_ismbblead(data[0]) && (size > 1) && _ismbbtrail(data[1]))
		{
			mbtowc(&buff, (const char *) data, 2);
			++i, ++data, --size;
		}
		else if (iscntrl(data[0]))
		{
			buff = L'.';
		}
		else if (isprint(data[0]))
		{
			buff = btowc(*data);
		}
		else
		{
			buff = L' ';
		}

		os << buff;
	}
}

template<class CharT>
void HexDump(std::basic_ostream<CharT> & os, const unsigned char * data, std::size_t size, void (*chardump)(std::basic_ostream<CharT> &, const unsigned char *, std::size_t))
{
	const CharT * ascii;

	if constexpr (std::is_same_v<CharT, char>)
	{
		ascii = "0123456789ABCDEF";
	}
	else if constexpr (std::is_same_v<CharT, wchar_t>)
	{
		ascii = L"0123456789ABCDEF";
	}
	else
	{
		static_assert(false);
	}

	CharT buff[72 + 1]{};

	while (size > 0)
	{
		auto p = data;
		auto s = size;

		int i = 0;

		while ((i < 72) && (0 < s))
		{
			if ((i % 9) == 0)
			{
				buff[i++] = ' ';
			}
			else
			{
				buff[i++] = ascii[(*p >> 4)];
				buff[i++] = ascii[(*p & 0x0F)];
				++p, --s;
			}
		}

		while (i < 72)
		{
			buff[i++] = ' ';
		}

		os << buff;
		chardump(os, data, size);
		os << std::endl;

		data = p;
		size = s;
	}
}

std::ostream & put_dump::output_stream(std::ostream & os) const
{
	switch (m_type)
	{
	case dump_char_t::sjis:
		HexDump(os, (const unsigned char *) m_data, m_size, SjisDump);
		break;
	}

	return os;
}

std::wostream & put_dump::output_stream(std::wostream & os) const
{
	switch (m_type)
	{
	case dump_char_t::sjis:
		HexDump(os, (const unsigned char *) m_data, m_size, SjisDump);
		break;
	}

	return os;
}


//
// 文字コード変換の実装。
//
#if 0
template<>
convert<wchar_t>::operator std::string()
{
	std::string buf;

	if (int cch = ::WideCharToMultiByte(CP_THREAD_ACP, 0, m_str.data(), (int) m_str.size(), nullptr, 0, nullptr, nullptr); cch > 0)
	{
		buf.resize(cch);

		::WideCharToMultiByte(CP_THREAD_ACP, 0, m_str.data(), (int) m_str.size(), buf.data(), cch, nullptr, nullptr);
	}

	return buf;
}

template<>
convert<char>::operator std::wstring()
{
	std::wstring buf;

	if (int cch = ::MultiByteToWideChar(CP_THREAD_ACP, 0, m_str.data(), (int) m_str.size(), nullptr, 0); cch > 0)
	{
		buf.resize(cch);

		::MultiByteToWideChar(CP_THREAD_ACP, 0, m_str.data(), (int) m_str.size(), buf.data(), cch);
	}

	return buf;
}
#else

#endif
