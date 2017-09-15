//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//  Copyright (c) 2017 by
//       __      _     _         _____
//    /\ \ \__ _| |__ (_) __ _  /__   \_ __ _   _  ___  _ __   __ _
//   /  \/ / _` | '_ \| |/ _` |   / /\/ '__| | | |/ _ \| '_ \ / _` |
//  / /\  / (_| | | | | | (_| |  / /  | |  | |_| | (_) | | | | (_| |
//  \_\ \/ \__, |_| |_|_|\__,_|  \/   |_|   \__,_|\___/|_| |_|\__, |
//         |___/                                              |___/
//
//  <nghiatruong.vn@gmail.com>
//  All rights reserved.
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

#include <vector>
#include <atomic>
#include <limits>
#include <tbb/tbb.h>

#undef min
#undef max

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
namespace ParallelObjects
{
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class SpinLock
{
public:
    SpinLock() = default;
    SpinLock(const SpinLock&) {}
    SpinLock& operator =(const SpinLock&) { return *this; }

    void lock()
    {
        while(m_Lock.test_and_set(std::memory_order_acquire))
            ;
    }

    void unlock()
    {
        m_Lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_Lock = ATOMIC_FLAG_INIT;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Scalar, class VectorType>
class VectorMinMaxVectorElements
{
public:
    VectorMinMaxVectorElements(const std::vector<VectorType>& vec) :
        m_ResultMin(VectorType(std::numeric_limits<Scalar>::max())), m_ResultMax(VectorType(std::numeric_limits<Scalar>::min())), m_Vector(vec)
    {}
    VectorMinMaxVectorElements(VectorMinMaxVectorElements<Scalar, VectorType>& vmme, tbb::split) :
        m_ResultMin(VectorType(std::numeric_limits<Scalar>::max())), m_ResultMax(VectorType(std::numeric_limits<Scalar>::min())), m_Vector(vmme.m_Vector)
    {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            const auto& vec = m_Vector[i];
            for(int j = 0; j < 3; ++j)
            {
                m_ResultMin[j] = (m_ResultMin[j] < vec[j]) ? m_ResultMin[j] : vec[j];
                m_ResultMax[j] = (m_ResultMax[j] > vec[j]) ? m_ResultMax[j] : vec[j];
            }
        }
    }

    void join(VectorMinMaxVectorElements<Scalar, VectorType>& vmme)
    {
        for(int j = 0; j < 3; ++j)
        {
            m_ResultMin[j] = (m_ResultMin[j] < vmme.m_ResultMin[j]) ? m_ResultMin[j] : vmme.m_ResultMin[j];
            m_ResultMax[j] = (m_ResultMax[j] > vmme.m_ResultMax[j]) ? m_ResultMax[j] : vmme.m_ResultMax[j];
        }
    }

    const VectorType& getMin() const noexcept { return m_ResultMin; }
    const VectorType& getMax() const noexcept { return m_ResultMax; }

private:
    VectorType m_ResultMin;
    VectorType m_ResultMax;

    const std::vector<VectorType>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}   // end namespace ParallelObjects
