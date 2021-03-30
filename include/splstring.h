/*******************************************************************************
* MIT License
*
* Copyright (c) 2021 Spirrwell
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
********************************************************************************/

#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <charconv>
#include <vector>
#include <array>
#include <limits>
#include <filesystem>

namespace spl
{
class string
{
	struct buffer_deleter
	{
		void operator()(char memory[])
		{
			std::free(memory);
		}
	};

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

		iterator_base(char *begin) noexcept : mCurrent(begin) {}
		iterator_base(const iterator_base &it) noexcept : mCurrent(it.mCurrent) {}

		bool operator==(const iterator_base &rhs) const noexcept { return mCurrent == rhs.mCurrent; }
		bool operator!=(const iterator_base &rhs) const noexcept { return mCurrent != rhs.mCurrent; }

		char &operator*() { return *mCurrent; }
		const char &operator*() const { return *mCurrent; }

	protected:
		char *mCurrent;
	};

	struct iterator : iterator_base
	{
		using iterator_base::iterator_base;

		iterator &operator++() noexcept
		{
			++mCurrent;
			return *this;
		}

		iterator &operator--() noexcept
		{
			--mCurrent;
		}

		iterator operator++(int)
		{
			iterator retval = *this;
			++(*this);

			return retval;
		}

		iterator operator--(int)
		{
			iterator retval = *this;
			--(*this);

			return retval;
		}

		friend iterator operator+(const iterator &lhs, difference_type count)
		{
			iterator retval = lhs;
			retval.mCurrent += count;

			return retval;
		}

		friend iterator operator-(const iterator &lhs, difference_type count)
		{
			iterator retval = lhs;
			retval.mCurrent -= count;

			return retval;
		}

		bool operator<(const iterator &rhs) const noexcept { return mCurrent < rhs.mCurrent; }
		bool operator<=(const iterator &rhs) const noexcept { return mCurrent <= rhs.mCurrent; }
		bool operator>(const iterator &rhs) const noexcept { return mCurrent > rhs.mCurrent; }
		bool operator>=(const iterator &rhs) const noexcept { return mCurrent >= rhs.mCurrent; }
	};

	struct reverse_iterator : iterator_base
	{
		using iterator_base::iterator_base;

		reverse_iterator &operator++() noexcept
		{
			--mCurrent;
			return *this;
		}

		reverse_iterator &operator--() noexcept
		{
			++mCurrent;
			return *this;
		}

		reverse_iterator operator++(int)
		{
			reverse_iterator retval = *this;
			++(*this);

			return retval;
		}

		reverse_iterator operator--(int)
		{
			reverse_iterator retval = *this;
			--(*this);

			return retval;
		}

		friend reverse_iterator operator+(const reverse_iterator &lhs, difference_type count)
		{
			reverse_iterator retval = lhs;
			retval.mCurrent -= count;

			return retval;
		}

		friend reverse_iterator operator-(const reverse_iterator &lhs, difference_type count)
		{
			reverse_iterator retval = lhs;
			retval.mCurrent += count;

			return retval;
		}

		bool operator<(const reverse_iterator &rhs) const noexcept { return mCurrent > rhs.mCurrent; }
		bool operator<=(const reverse_iterator &rhs) const noexcept { return mCurrent >= rhs.mCurrent; }
		bool operator>(const reverse_iterator &rhs) const noexcept { return mCurrent < rhs.mCurrent; }
		bool operator>=(const reverse_iterator &rhs) const noexcept { return mCurrent <= rhs.mCurrent; }
	};

	using const_iterator = iterator;
	using const_reverse_iterator = reverse_iterator;

	using reference = char&;
	using const_reference = const char&;

	using size_type = std::size_t;

	constexpr static size_type npos = std::numeric_limits<size_type>::max();

	inline string();

	inline string(size_type count, char ch);

	inline string(string &&other) noexcept;
	inline string(const string &other);
	inline string(const std::string &str);
	inline string(const std::string_view &sv);
	inline string(const char *str);
	inline string(const char *str, size_type count);

	inline string(const std::string_view &sv, size_type pos, size_type n);

private:

	void allocate(size_type new_size)
	{
		mBuffer.reset((char*)std::malloc(new_size));
	}

	void reallocate(size_type new_size)
	{
		mBuffer.reset((char*)std::realloc(mBuffer.release(), new_size));
	}

public:

	string &operator=(const string &rhs)
	{
		if (&rhs == this)
			return *this;

		mLength = rhs.mLength;
		
		reallocate(mLength + 1);
		mBuffer[mLength] = '\0';

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

	char &front()
	{
		return mBuffer[0];
	}

	const char &front() const
	{
		return mBuffer[0];
	}

	char &back()
	{
		return mBuffer[size() - 1];
	}

	const char &back() const
	{
		return mBuffer[size() - 1];
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

	string &operator+=(const std::string_view &str)
	{
		return append(str);
	}

	string &operator+=(const string &str)
	{
		return append(str);
	}

	string &operator+=(char ch)
	{
		return append(1, ch);
	}

	string &operator+=(const char *str)
	{
		return operator+=(std::string_view(str));
	}

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

	void resize(size_type count)
	{
		return resize(count, char());
	}

	void resize(size_type count, char ch)
	{
		if (count > max_size())
			throw std::length_error("string too long");

		if (count <= mLength)
		{
			mLength = count;
			mBuffer[mLength] = '\0';
		}
		else
		{
			reallocate(count + 1);
			mBuffer[count] = '\0';

			std::fill_n(&mBuffer[mLength], count - mLength, ch);
			mLength = count;
		}
	}

	iterator begin() noexcept { return iterator(mBuffer.get()); }
	iterator end() noexcept { return iterator(mBuffer.get() + mLength); }

	const_iterator begin() const noexcept { return const_iterator(mBuffer.get()); }
	const_iterator end() const noexcept { return const_iterator(mBuffer.get() + mLength); }

	const_iterator cbegin() const noexcept { return const_iterator(mBuffer.get()); }
	const_iterator cend() const noexcept { return const_iterator(mBuffer.get() + mLength); }

	reverse_iterator rbegin() noexcept { return reverse_iterator(mBuffer.get() + mLength - 1); }
	reverse_iterator rend() noexcept { return reverse_iterator(mBuffer.get() - 1); }

	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(mBuffer.get() + mLength - 1); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(mBuffer.get() - 1); }

	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(mBuffer.get() + mLength - 1); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(mBuffer.get() - 1); }

	operator std::string() const { return std_string(); }
	operator std::string_view() const noexcept { return view(); }
	operator std::filesystem::path() const { return { view() }; }

	char *data() { return &mBuffer[0]; }
	const char *data() const { return &mBuffer[0]; }
	const char *c_str() const { return &mBuffer[0]; }

	std::string_view view() const noexcept { return { data(), size() }; }
	std::string std_string() const { return { data(), size() }; }

	bool empty() const noexcept { return mLength == 0; }
	size_type size() const noexcept { return mLength; }
	size_type length() const noexcept { return mLength; }
	size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() - 1; } // -1 for null terminator or npos

	void clear()
	{
		mLength = 0;
		reallocate(1);

		mBuffer[mLength] = '\0';
	}

	string &erase(size_type index, size_type count = npos)
	{
		// Note: According to cppreference, std::string throws only if > size()
		// Erasing at an index of size() should make the count 0 and erase nothing
		// But this is still stupid IMHO
		if (index > size())
			throw std::out_of_range("invalid string position");

		count = std::min(count, size() - index);
		const size_type end = index + count;

		for (size_type i = end; i < size(); ++i)
			mBuffer[i - count] = mBuffer[i];

		mLength -= count;
		mBuffer[mLength] = '\0';

		return *this;
	}

	iterator erase(const_iterator position)
	{
		if (position == cend())
			return end();

		iterator current = position;
		iterator next = position + 1;

		for (;next != end(); ++next, ++current)
			*current = *next;

		mLength -= 1;
		mBuffer[mLength] = '\0';

		return empty() ? end() : position;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		const auto range = std::distance(first, last);

		iterator current = first;
		iterator next = last;

		for (;next != end(); ++next, ++current)
			*current = *next;

		mLength -= range;
		mBuffer[mLength] = '\0';

		return empty() ? end() : first;
	}

	void pop_back()
	{
		if (!empty())
		{
			--mLength;
			mBuffer[mLength] = '\0';
		}
	}

	void push_back(char ch)
	{
		resize(size() + 1, ch);
	}

	string &append(size_type count, char ch)
	{
		resize(size() + count, ch);
		return *this;
	}

	string &append(const std::string_view &str)
	{
		if (!str.empty())
		{
			const size_type old_size = size();
			resize(old_size + str.size());

			std::memcpy(&mBuffer[old_size], str.data(), str.size());
		}

		return *this;
	}

	string &append(const string &str)
	{
		if (!str.empty())
		{
			const size_type old_size = size();
			resize(old_size + str.size());

			std::memcpy(&mBuffer[old_size], str.data(), str.size());
		}
		
		return *this;
	}

	bool starts_with(const std::string_view &sv) const noexcept
	{
		return sv.size() > size() ? false :
			std::char_traits<char>::compare(data(), sv.data(), sv.size()) == 0;
	}

	bool starts_with(char c) const noexcept
	{
		return empty() ? false : mBuffer[0] == c;
	}

	bool starts_with(const char *str) const
	{
		return starts_with(std::string_view(str, std::char_traits<char>::length(str)));
	}

	bool ends_with(const std::string_view &sv) const noexcept
	{
		return sv.size() > size() ? false :
			std::char_traits<char>::compare(&mBuffer[size()] - sv.size(), sv.data(), sv.size()) == 0;
	}

	bool ends_with(char c) const noexcept
	{
		return empty() ? false : mBuffer[size() - 1] == c;
	}

	bool ends_with(const char *str) const
	{
		return ends_with(std::string_view(str, std::char_traits<char>::length(str)));
	}

	string &lowered()
	{
		std::transform(cbegin(), cend(), begin(), ::tolower);
		return *this;
	}

	string lower() const
	{
		string low = *this;
		std::transform(low.cbegin(), low.cend(), low.begin(), ::tolower);

		return low;
	}

	string &uppered()
	{
		std::transform(cbegin(), cend(), begin(), ::toupper);
		return *this;
	}

	string upper() const
	{
		string up = *this;

		std::transform(up.cbegin(), up.cend(), up.begin(), ::toupper);
		return up;
	}

	string &reversed()
	{
		if (size() > 1)
		{
			const size_type even = size() % 2;

			for (size_type i = 0; i < size(); ++i)
			{
				const size_type back_index = size() - i - 1;
				std::swap(mBuffer[i], mBuffer[back_index]);

				if (i + 1 == back_index - even)
					break;
			}
		}

		return *this;
	}

	string reverse() const
	{
		if (empty())
			return {};

		string str(size(), char());

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

private:

	template<typename T>
	void split_into_vector(char ch, std::vector<T> &out, size_type offset = 0) const
	{
		// Note: This also serves as an empty() check
		if (offset >= size())
			return;

		size_type last_split = offset;

		for (size_type i = offset; i < size(); ++i)
		{
			if (mBuffer[i] == ch)
			{
				out.emplace_back(T(&mBuffer[last_split], i - last_split));
				last_split = i + 1;
			}
		}

		if (last_split < size())
			out.emplace_back(T(&mBuffer[last_split], size() - last_split));
	}

public:
	void split(char ch, std::vector<string> &out, size_type offset = 0) const
	{
		return split_into_vector(ch, out, offset);
	}

	void split(char ch, std::vector<std::string> &out, size_type offset = 0) const
	{
		return split_into_vector(ch, out, offset);
	}

	void split(char ch, std::vector<std::string_view> &out, size_type offset = 0) const
	{
		return split_into_vector(ch, out, offset);
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
						return std::string_view(&mBuffer[0], real_index);
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

	template <typename T>
	T get_as() const
	{
		T value = {};

		// Note: Using data() + mLength here because using &mBuffer[mLength] feels wrong even though it's fine.
		if (data())
			std::from_chars(data(), data() + mLength, value);

		return value;
	}

	friend std::ostream &operator<<(std::ostream &os, const string &str)
	{
		return os << str.view();
	}

	friend spl::string operator+(const std::string_view &lhs, const std::string_view &rhs)
	{
		string str(lhs.size() + rhs.size(), char());

		std::memcpy(str.data(), lhs.data(), lhs.size());
		std::memcpy(str.data() + lhs.size(), rhs.data(), rhs.size());

		return str;
	}

	friend spl::string operator+(const std::string_view &lhs, char rhs)
	{
		string str(lhs.size() + 1, char());

		std::memcpy(str.data(), lhs.data(), lhs.size());
		std::memcpy(str.data() + lhs.size(), &rhs, sizeof(rhs));

		return str;
	}

private:
	size_type mLength = 0;
	std::unique_ptr<char[], buffer_deleter> mBuffer;
};

inline string::string()
{
	allocate(1);
	mBuffer[mLength] = '\0';
}

inline string::string(size_type count, char ch)
{
	mLength = count;

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::fill_n(&mBuffer[0], mLength, ch);
}

inline string::string(string &&other) noexcept :
	mLength(std::exchange(other.mLength, 0)),
	mBuffer(std::move(other.mBuffer))
{
}

inline string::string(const string &other)
{
	mLength = other.size();

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::memcpy(&mBuffer[0], other.data(), mLength);
}

inline string::string(const std::string &str)
{
	mLength = str.size();

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::memcpy(&mBuffer[0], str.data(), mLength);
}

inline string::string(const std::string_view &sv)
{
	mLength = sv.size();

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::memcpy(&mBuffer[0], sv.data(), mLength);
}

inline string::string(const char *str)
{
	mLength = std::char_traits<char>::length(str);

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::memcpy(&mBuffer[0], str, mLength);
}

inline string::string(const char *str, size_type count)
{
	mLength = count;

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::memcpy(&mBuffer[0], str, mLength);
}

// TODO: Safety?
inline string::string(const std::string_view &sv, size_type pos, size_type n)
{
	mLength = n;

	allocate(mLength + 1);
	mBuffer[mLength] = '\0';

	std::memcpy(&mBuffer[0], &sv[pos], n);
}

template<typename T>
string to_string(T value)
{
	constexpr std::size_t maxDigits = 35;
	std::array<char, maxDigits> str;

	auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), value);
	return string(std::string_view(str.data(), p - str.data()));
}

// Extra logic for standard strings

inline bool contains(const std::string_view &str, const std::string_view &substring)
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

inline std::string &lowered(std::string &str)
{
	std::transform(str.cbegin(), str.cend(), str.begin(), ::tolower);
	return str;
}

inline std::string lower(const std::string_view &view)
{
	std::string low(view);
	std::transform(low.cbegin(), low.cend(), low.begin(), ::tolower);

	return low;
}

inline std::string &uppered(std::string &str)
{
	std::transform(str.cbegin(), str.cend(), str.begin(), ::toupper);
	return str;
}

inline std::string upper(const std::string_view &view)
{
	std::string up(view);

	std::transform(up.cbegin(), up.cend(), up.begin(), ::toupper);
	return up;
}

inline std::string &reversed(std::string &str)
{
	if (str.size() > 1)
	{
		const std::size_t even = str.size() % 2;

		for (std::size_t i = 0; i < str.size(); ++i)
		{
			const std::size_t back_index = str.size() - i - 1;
			std::swap(str[i], str[back_index]);

			if (i + 1 == back_index - even)
				break;
		}
	}

	return str;
}

inline std::string reverse(const std::string_view &view)
{
	if (view.empty())
		return {};

	std::string str(view.size(), char());

	for (std::size_t i = 0; i < view.size(); ++i)
		str[i] = view[view.size() - i - 1];

	return str;
}

inline void split(const std::string_view &view, char ch, std::vector<std::string_view> &out, std::size_t offset = 0)
{
	// Note: This also serves as an empty() check
	if (offset >= view.size())
		return;

	std::size_t last_split = offset;

	for (std::size_t i = offset; i < view.size(); ++i)
	{
		if (view[i] == ch)
		{
			out.emplace_back(std::string_view(&view[last_split], i - last_split));
			last_split = i + 1;
		}
	}

	if (last_split < view.size())
		out.emplace_back(std::string_view(&view[last_split], view.size() - last_split));
}

}

// Hashing so spl::string can be used in containers like std::unordered_map

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