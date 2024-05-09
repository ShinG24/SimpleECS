#pragma once
#include "CommonHeader.h"
#include "ECSCommon.h"


template<class T>
class ComponentArray
{
public:
	ComponentArray(T* begin, u32 size) : begin_(begin), size_(size) { _ASSERT_EXPR(begin && size, L"begin‚ªnullptr‚Ü‚½‚Ísize‚ª0‚Å‚µ‚½"); }

	T& operator[](const int index)
	{
		_ASSERT_EXPR(index < size_, L"Out of Range");
		return begin_[index];
	}

	T* begin() { return begin_; }
	T* end() { return begin_ + size_; }

	u32 size() const { return size_; }
private:

	T* begin_;
	u32 size_;
};