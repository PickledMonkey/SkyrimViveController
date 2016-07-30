#include <string.h>
#include <cctype>
#include <algorithm>

struct StringHelper
{
	static bool Contains(std::string str, std::string search)
	{
		if (search.empty())
			return true;
		if (search.length() > str.length())
			return false;

		std::transform(str.begin(), str.end(), str.begin(), std::toupper);
		std::transform(search.begin(), search.end(), search.begin(), std::toupper);

		std::string::const_iterator it = std::search(
			str.begin(), str.end(),
			search.begin(), search.end()
			);
		if (it != str.end()) return true;
		return false;
	}

	static bool StartsWith(std::string str, std::string search)
	{
		if (search.empty())
			return true;
		if (search.length() > str.length())
			return false;

		return !_strnicmp(str.c_str(), search.c_str(), search.length());
	}

	static bool EndsWith(std::string str, std::string search)
	{
		if (search.empty())
			return true;
		if (search.length() > str.length())
			return false;

		str = str.substr(str.length() - search.length());
		return !_stricmp(str.c_str(), search.c_str());
	}

	static std::string Trim(std::string str)
	{
		return TrimEnd(TrimStart(str));
	}

	static std::string TrimEnd(std::string str)
	{
		int idx = str.length() - 1;
		while (idx >= 0)
		{
			if (isspace(str[idx]))
				idx--;
			else
				break;
		}

		idx++;
		return str.substr(0, idx);
	}

	static std::string TrimStart(std::string str)
	{
		int idx = 0;
		int len = str.length();
		while (idx < len)
		{
			if (isspace(str[idx]))
				idx++;
			else
				break;
		}

		return str.substr(idx);
	}

	static int IndexOf(std::string str, std::string search)
	{
		if (search.empty())
			return 0;
		if (search.length() > str.length())
			return -1;

		std::transform(str.begin(), str.end(), str.begin(), std::toupper);
		std::transform(search.begin(), search.end(), search.begin(), std::toupper);

		std::string::iterator it = std::search(
			str.begin(), str.end(),
			search.begin(), search.end()
			);
		if (it != str.end())
			return std::distance(str.begin(), it);
		return -1;
	}

	static int LastIndexOf(std::string str, std::string search)
	{
		if (search.empty())
			return 0;
		if (search.length() > str.length())
			return -1;

		std::transform(str.begin(), str.end(), str.begin(), std::toupper);
		std::transform(search.begin(), search.end(), search.begin(), std::toupper);

		std::string::iterator it = std::find_end(
			str.begin(), str.end(),
			search.begin(), search.end()
			);
		if (it != str.end())
			return std::distance(str.begin(), it);
		return -1;
	}

	static bool Equals(std::string str, std::string search)
	{
		str = StringHelper::Trim(str);

		return _stricmp(str.c_str(), search.c_str()) == 0;
	}

	static bool ToInt(std::string str, int& value, bool mustHex = false)
	{
		const char * strVal = str.c_str();
		char * endVal = NULL;
		long ret = strtol(strVal, &endVal, !mustHex ? 0 : 16);
		if (ret == LONG_MAX || ret == LONG_MIN || (int)endVal != (int)strVal + strlen(strVal))
			return false;
		
		value = ret;
		return true;
	}
	
	static bool ToFloat(std::string str, float& value)
	{
		const char * strVal = str.c_str();
		char * endVal = NULL;
		float ret = strtof(strVal, &endVal);
		if ((int)endVal != (int)strVal + strlen(strVal))
			return false;
		
		value = ret;
		return true;
	}

	static bool ToBool(std::string str, bool& value)
	{
		if (StringHelper::Equals(str, "true") || StringHelper::Equals(str, "1") || StringHelper::Equals(str, "yes"))
		{
			value = true;
			return true;
		}
		else if (StringHelper::Equals(str, "false") || StringHelper::Equals(str, "0") || StringHelper::Equals(str, "no"))
		{
			value = false;
			return true;
		}

		return false;
	}

	static bool ParseWord(std::string& str, std::string& result)
	{
		if (str.empty())
			return false;

		if (isspace(str[0]))
			str = Trim(str);

		int idx = 0;
		int len = str.length();
		while (idx < len && isalnum(str[idx]))
			idx++;

		if (idx == 0)
			return false;

		result = str.substr(0, idx);
		str = Trim(str.substr(idx));
		return true;
	}

	static bool ParseInteger(std::string& str, std::string& result)
	{
		if (str.empty())
			return false;

		if (isspace(str[0]))
			str = Trim(str);

		bool hadNeg = false;
		int hadHex = 0;

		int idx = 0;
		int len = str.length();
		while (idx < len && (isdigit(str[idx]) || (!hadNeg && str[idx] == '-') || (!hadNeg && str[idx] == '+') || (hadHex == 1 && str[idx] == 'x' || str[idx] == 'X') || (hadHex == 2 && isxdigit(str[idx]))))
		{
			if (str[idx] == '-' || str[idx] == '+')
				hadNeg = true;
			else if (str[idx] == '0' && hadHex == 0)
				hadHex = 1;
			else if ((str[idx] == 'x' || str[idx] == 'X') && hadHex == 1)
				hadHex = 2;
			else
				hadHex = 2;

			idx++;
		}

		if (idx == 0)
			return false;

		result = str.substr(0, idx);
		str = Trim(str.substr(idx));
		return true;
	}

	static bool ParseString(std::string& str, std::string& result)
	{
		if (str.empty())
		{
			result = std::string();
			return true;
		}

		if (isspace(str[0]))
			str = Trim(str);

		if (!str.empty() && str[0] == '"')
		{
			result = str.substr(1);
			int idx = IndexOf(result, "\"");
			if (idx < 0)
			{
				str = std::string();
				return true;
			}

			result = result.substr(0, idx);
			str = str.substr(idx + 2);
			return true;
		}

		return ParseNonSpace(str, result);
	}

	static bool ParseNonSpace(std::string& str, std::string& result)
	{
		if (str.empty())
		{
			result = std::string();
			return true;
		}

		if (isspace(str[0]))
			str = Trim(str);

		int idx = 0;
		int len = str.length();
		while (idx < len && !isspace(str[idx]))
			idx++;

		result = str.substr(0, idx);
		str = Trim(str.substr(idx));
		return true;
	}

	static std::string Replace(std::string str, std::string first, std::string second)
	{
		if (first.empty() || str.empty())
			return str;

		size_t pos = 0;
		while ((pos = str.find(first, pos)) != std::string::npos)
		{
			str.replace(pos, first.length(), second);
			pos += second.length();
		}
		return str;
	}
};
