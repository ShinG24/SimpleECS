#pragma once

#include <Windows.h>
#include "CommonHeader.h"

class PerformanceCounter
{
public:

    inline static u32 Begin()
    {
        if(cpu_frequency_.QuadPart == 0) QueryPerformanceFrequency(&cpu_frequency_);

	    u32 ret{};
        if(!free_indices_.empty())
        {
	        ret = free_indices_.back();
            free_indices_.pop_back();
        }
        else
        {
	        ret = count_begin_.size();
            count_begin_.emplace_back();
        }

        LARGE_INTEGER count_begin{};
        QueryPerformanceCounter(&count_begin);
        count_begin_.at(ret) = count_begin;
        return ret;
    }

   inline  static double End(u32 clock_index)
    {
        LARGE_INTEGER count_end{};
        QueryPerformanceCounter(&count_end);

        const LARGE_INTEGER count_begin{ count_begin_.at(clock_index) };

        return  1000.0 * (static_cast<double>(count_end.QuadPart - count_begin.QuadPart) / static_cast<double>(cpu_frequency_.QuadPart));
    }

private:
    inline static LARGE_INTEGER cpu_frequency_{ 0 };
    inline static Vector<LARGE_INTEGER> count_begin_{};
    inline static Vector<u32> free_indices_{};
};