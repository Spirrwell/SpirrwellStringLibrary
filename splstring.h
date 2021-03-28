#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <charconv>
#include <vector>

namespace spl
{
class string
{
public:
	enum struct split_side
	{
		left,
		right
	};

	struct iterator_base
	{
		using iterator_category = std::input_iterator_tag;
		using value_type = char;
		using difference_type = ptrdiff_t;
		using pointer = char*;
		using reference = char&;

		iterator_base(char *begin, char *end) : mCurrent(begin), mEnd(end) {}
		iterator_base(const iterator_base &it) : mCurrent(it.mCurrent), mEnd(it.mEnd) {}

		bool operator==(const iterator_base &rhs) const { return mCurrent == rhs.mCurrent; }
		bool operator !=(const iterator_base &rhs) const { return mCurrent != rhs.mCurrent; }

		char &operator*() { return *mCurrent; }
		const char &operator*() const { return *mCurrent; }

	protected:
		char *mCurrent;
		char *mEnd;
	};

	struct iterator : iterator_base
	{
		using iterator_base::iterator_base;

		iterator &operator++()
		{
			if (mCurrent)
			{
				if (++mCurrent > mEnd)
					mCurrent = nullptr;
			}

			return *this;
		}

		iterator operator++(int)
		{
			iterator retval = *this;
			++(*this);

			return retval;
		}
	};

	struct reverse_iterator : iterator_base
	{
		using iterator_base::iterator_base;

		reverse_iterator &operator++()
		{
			if (mCurrent)
			{
				if (--mCurrent < mEnd)
					mCurrent = nullptr;
			}

			return *this;
		}

		reverse_iterator operator++(int)
		{
			reverse_iterator retval = *this;
			++(*this);

			return retval;
		}
	};

	using const_iterator = iterator;
	using const_reverse_iterator = reverse_iterator;

	using reference = char&;
	using const_reference = const char&;

	using size_type = std::size_t;

	string() = default;

	string(size_type count, char ch);

	string(string &&other) noexcept;
	string(const string &other);
	string(const std::string &str);
	string(const std::string_view &sv);
	string(const char *str);

	string &operator=(const string &rhs)
	{
		if (&rhs == this)
			return *this;

		mLength = rhs.mLength;
		mBuffer = std::make_unique<char[]>(mLength);

		std::memcpy(mBuffer.get(), rhs.data(), mLength);
		return *this;
	}

	string &operator=(string &&rhs) noexcept
	{
		mLength = std::exchange(rhs.mLength, 0);
		mBuffer = std::move(rhs.mBuffer);

		return *this;
	}

	reference at(size_type pos)
	{
		if (pos >= size())
			throw std::out_of_range("invalid string position");

		return mBuffer[pos];
	}

	const_reference at(size_type pos) const
	{
		if (pos >= size())
			throw std::out_of_range("invalid string position");

		return mBuffer[pos];
	}

	reference operator[](size_type pos)
	{
		return mBuffer[pos];
	}

	const_reference operator[](size_type pos) const
	{
		return mBuffer[pos];
	}

	bool operator==(const string &rhs) const noexcept
	{
		const size_type lhs_sz = size();
		const size_type rhs_sz = rhs.size();

		if (lhs_sz != rhs_sz)
			return false;

		return std::char_traits<char>::compare(data(), rhs.data(), size()) == 0;
	}

	bool operator==(const std::string &rhs) const noexcept
	{
		const size_type lhs_sz = size();
		const size_type rhs_sz = rhs.size();

		if (lhs_sz != rhs_sz)
			return false;

		return std::char_traits<char>::compare(data(), rhs.data(), size()) == 0;
	}

	friend bool operator==(const std::string &lhs, const string &rhs)
	{
		const size_type lhs_sz = lhs.size();
		const size_type rhs_sz = rhs.size();

		if (lhs_sz != rhs_sz)
			return false;

		return std::char_traits<char>::compare(lhs.data(), rhs.data(), lhs.size()) == 0;
	}

	bool operator==(const std::string_view &rhs) const noexcept
	{
		const size_type lhs_sz = size();
		const size_type rhs_sz = rhs.size();

		if (lhs_sz != rhs_sz)
			return false;

		return std::char_traits<char>::compare(data(), rhs.data(), size()) == 0;
	}

	friend bool operator==(const std::string_view &lhs, const string &rhs)
	{
		const size_type lhs_sz = lhs.size();
		const size_type rhs_sz = rhs.size();

		if (lhs_sz != rhs_sz)
			return false;

		return std::char_traits<char>::compare(lhs.data(), rhs.data(), lhs.size()) == 0;
	}

	bool operator==(const char *rhs) const noexcept
	{
		const size_type lhs_sz = size();
		const size_type rhs_sz = std::char_traits<char>::length(rhs);

		if (lhs_sz != rhs_sz)
			return false;

		return std::char_traits<char>::compare(data(), rhs, size()) == 0;
	}

#if _MSVC_LANG > 201703L || __cplusplus > 201703L
	auto operator<=>(const string &rhs) const noexcept
	{
		return compare(rhs) <=> 0;
	}
#else
	bool operator<(const string &rhs) const noexcept
	{
		return compare(rhs) < 0;
	}

	bool operator<=(const string &rhs) const noexcept
	{
		return compare(rhs) <= 0;
	}

	bool operator>(const string &rhs) const noexcept
	{
		return compare(rhs) > 0;
	}

	bool operator>=(const string &rhs) const noexcept
	{
		return compare(rhs) >= 0;
	}
#endif

	int compare(const string &str) const noexcept
	{
		const size_type lhs_sz = size();
		const size_type rhs_sz = str.size();

		const int result = std::char_traits<char>::compare(data(), str.data(), std::min(lhs_sz, rhs_sz));

		if (result != 0)
			return result;
		if (lhs_sz < rhs_sz)
			return -1;
		if (lhs_sz > rhs_sz)
			return 1;

		return 0;
	}

	bool contains(const std::string_view &str) const
	{
		if (str.size() > size())
			return false;

		// Note: It seems like empty strings are still considered substrings of any string
		if (str.empty())
			return true;

		const size_type difference = size() - str.size();

		for (size_type i = 0; i <= difference; ++i)
		{
			if (mBuffer[i] == str[0] && std::char_traits<char>::compare(&mBuffer[i], str.data(), str.size()) == 0)
				return true;
		}

		return false;
	}

	bool contains(const char *str) const
	{
		return contains(std::string_view(str, std::char_traits<char>::length(str)));
	}

	bool contains(char c) const
	{
		for (size_type i = 0; i < size(); ++i)
		{
			if (mBuffer[i] == c)
				return true;
		}

		return false;
	}

	iterator begin() noexcept { return mBuffer.get() ? iterator(mBuffer.get(), &mBuffer[mLength - 1]) : end(); }
	iterator end() noexcept { return iterator(nullptr, nullptr); }

	const_iterator begin() const noexcept { return mBuffer.get() ? const_iterator(mBuffer.get(), &mBuffer[mLength - 1]) : end(); }
	const_iterator end() const noexcept { return const_iterator(nullptr, nullptr); }

	const_iterator cbegin() const noexcept { return mBuffer.get() ? const_iterator(mBuffer.get(), &mBuffer[mLength - 1]) : end(); }
	const_iterator cend() const noexcept { return const_iterator(nullptr, nullptr); }

	reverse_iterator rbegin() noexcept { return mBuffer.get() ? reverse_iterator(&mBuffer[mLength - 1], mBuffer.get()) : rend(); }
	reverse_iterator rend() noexcept { return reverse_iterator(nullptr, nullptr); }

	const_reverse_iterator rbegin() const noexcept { return mBuffer.get() ? const_reverse_iterator(&mBuffer[mLength - 1], mBuffer.get()) : rend(); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(nullptr, nullptr); }

	const_reverse_iterator crbegin() const noexcept { return mBuffer.get() ? const_reverse_iterator(&mBuffer[mLength - 1], mBuffer.get()) : crend(); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(nullptr, nullptr); }

	operator std::string() const { return std_string(); }
	operator std::string_view() const noexcept { return view(); }

	const char *data() const noexcept { return mBuffer.get(); }
	std::string_view view() const noexcept { return { data(), size() }; }
	std::string std_string() const { return { data(), size() }; }

	bool empty() const noexcept { return mLength == 0; }
	size_type size() const noexcept { return mLength; }
	size_type length() const noexcept { return mLength; }

	void clear() noexcept
	{
		mLength = 0;
		mBuffer.reset();
	}

	string lower() const
	{
		string low = *this;
		std::transform(low.cbegin(), low.cend(), low.begin(), ::tolower);

		return low;
	}

	string upper() const
	{
		string up = *this;

		std::transform(up.cbegin(), up.cend(), up.begin(), ::toupper);
		return up;
	}

	string reverse() const
	{
		if (empty())
			return {};

		string str(size(), '\0');

		for (size_type i = 0; i < size(); ++i)
			str[i] = mBuffer[size() - i - 1];

		return str;
	}

	std::string_view split(char ch, size_type offset = 0, split_side side = split_side::left) const
	{
		// Note: This also serves as an empty() check
		if (offset >= size())
			return {};

		for (size_type i = offset; i < size(); ++i)
		{
			if (mBuffer[i] == ch)
			{
				switch (side)
				{
				case split_side::left:
					return std::string_view(&mBuffer[offset], i - offset);
				case split_side::right:
					if (++i < size())
						return std::string_view(&mBuffer[i], size() - i);
					else
						return {};
				}
			}
		}

		switch (side)
		{
		case split_side::left:
			return std::string_view(&mBuffer[offset], size() - offset);
		case split_side::right:
			return {}; // Hit the end, nothing to return on the right
		}

		return {};
	}

	std::string_view rsplit(char ch, size_type roffset = 0, split_side side = split_side::right) const
	{
		// Note: This also serves as an empty() check
		if (roffset >= size())
			return {};

		for (size_type i = size() - roffset; i > 0; --i)
		{
			const size_type real_index = i - 1;

			if (mBuffer[real_index] == ch)
			{
				switch (side)
				{
				case split_side::left:
					if (--i > 0)
					{
						return std::string_view(&mBuffer[0], real_index);
					}
					else
						return {};
				case split_side::right:
					if (i == size())
						return {};

					return std::string_view(&mBuffer[i], size() - i - roffset);
				}
			}
		}

		switch (side)
		{
		case split_side::left:
			return {}; // Hit the end, nothing to return on the left
		case split_side::right:
			return std::string_view(&mBuffer[0], size() - roffset);
		}

		return {};
	}

	std::vector<std::string_view> split_views(char ch, size_type offset = 0) const
	{
		// Note: This also serves as an empty() check
		if (offset >= size())
			return {};

		std::vector<std::string_view> views;
		size_type last_split = offset;

		for (size_type i = offset; i < size(); ++i)
		{
			if (mBuffer[i] == ch)
			{
				views.emplace_back(std::string_view(&mBuffer[last_split], i - last_split));
				last_split = i + 1;
			}
		}

		if (last_split < size())
			views.emplace_back(std::string_view(&mBuffer[last_split], size() - last_split));

		return views;
	}

	template <typename T>
	T get_as() const
	{
		T value = {};

		if (mBuffer.get())
			std::from_chars(mBuffer.get(), &mBuffer[mLength - 1], value);

		return value;
	}

	friend std::ostream &operator<<(std::ostream &os, const string &str)
	{
		return os << str.view();
	}

private:
	size_type mLength = 0;
	std::unique_ptr<char[]> mBuffer;
};

string::string(size_type count, char ch)
{
	mLength = count;

	mBuffer = std::make_unique<char[]>(mLength);
	std::fill_n(mBuffer.get(), mLength, ch);
}

string::string(string &&other) noexcept :
	mLength(std::exchange(other.mLength, 0)),
	mBuffer(std::move(other.mBuffer))
{
}

string::string(const string &other)
{
	mLength = other.size();

	mBuffer = std::make_unique<char[]>(mLength);
	std::memcpy(mBuffer.get(), other.data(), mLength);
}

string::string(const std::string &str)
{
	mLength = str.size();

	mBuffer = std::make_unique<char[]>(mLength);
	std::memcpy(mBuffer.get(), str.data(), mLength);
}

string::string(const std::string_view &sv)
{
	mLength = sv.size();

	mBuffer = std::make_unique<char[]>(mLength);
	std::memcpy(mBuffer.get(), sv.data(), mLength);
}

string::string(const char *str)
{
	mLength = std::strlen(str);

	mBuffer = std::make_unique<char[]>(mLength);
	std::memcpy(mBuffer.get(), str, mLength);
}

// Extra logic for standard strings

bool contains(const std::string_view &str, const std::string_view &substring)
{
	if (substring.size() > str.size())
		return false;

	// Note: It seems like empty strings are still considered substrings of any string
	if (substring.empty())
		return true;

	const std::size_t difference = str.size() - substring.size();

	for (std::size_t i = 0; i <= difference; ++i)
	{
		if (str[i] == substring[0] && std::char_traits<char>::compare(&str[i], substring.data(), substring.size()) == 0)
			return true;
	}

	return false;
}

std::string lower(const std::string_view &view)
{
	std::string low(view);
	std::transform(low.cbegin(), low.cend(), low.begin(), ::tolower);

	return low;
}

std::string upper(const std::string_view &view)
{
	std::string up(view);

	std::transform(up.cbegin(), up.cend(), up.begin(), ::toupper);
	return up;
}

std::string reverse(const std::string_view &view)
{
	if (view.empty())
		return {};

	std::string str(view.size(), '\0');

	for (std::size_t i = 0; i < view.size(); ++i)
		str[i] = view[view.size() - i - 1];

	return str;
}

std::vector<std::string_view> split_views(const std::string_view &view, char ch, std::size_t offset = 0)
{
	// Note: This also serves as an empty() check
	if (offset >= view.size())
		return {};

	std::vector<std::string_view> views;
	std::size_t last_split = offset;

	for (std::size_t i = offset; i < view.size(); ++i)
	{
		if (view[i] == ch)
		{
			views.emplace_back(std::string_view(&view[last_split], i - last_split));
			last_split = i + 1;
		}
	}

	if (last_split < view.size())
		views.emplace_back(std::string_view(&view[last_split], view.size() - last_split));

	return views;
}

}

namespace std
{
	template<> struct hash<spl::string>
	{
		std::size_t operator()(const spl::string &str) const noexcept
		{
			// spl::string should be implicitly convertible to std::string_view via operator
			return std::hash<std::string_view>{}(str);
		}
	};
}