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
template<class ... T>
class VectorDotProduct;

template<class T>
class VectorDotProduct<T>
{
public:
    VectorDotProduct(const std::vector<T>& vec1, const std::vector<T>& vec2) : m_Vec1(vec1), m_Vec2(vec2), m_Result(0) {}
    VectorDotProduct(VectorDotProduct& vdp, tbb::split) : m_Vec1(vdp.m_Vec1), m_Vec2(vdp.m_Vec2), m_Result(0) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
            m_Result += m_Vec1[i] * m_Vec2[i];
    }

    void join(VectorDotProduct& vdp)
    {
        m_Result += vdp.m_Result;
    }

    T getResult() const noexcept { return m_Result; }

private:
    T m_Result;

    const std::vector<T>& m_Vec1;
    const std::vector<T>& m_Vec2;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T, class VectorType>
class VectorDotProduct<T, VectorType>
{
public:
    VectorDotProduct(const std::vector<VectorType>& vec1, const std::vector<VectorType>& vec2) : m_Vec1(vec1), m_Vec2(vec2), m_Result(0) {}
    VectorDotProduct(VectorDotProduct& vdp, tbb::split) : m_Vec1(vdp.m_Vec1), m_Vec2(vdp.m_Vec1), m_Result(0) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
            m_Result += glm::dot(m_Vec1[i], m_Vec2[i]);
    }

    void join(VectorDotProduct& vdp)
    {
        m_Result += vdp.m_Result;
    }

    T getResult() const noexcept { return m_Result; }

private:
    T m_Result;

    const std::vector<VectorType>& m_Vec1;
    const std::vector<VectorType>& m_Vec2;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T>
class VectorMinElement
{
public:
    VectorMinElement(const std::vector<T>& vec) : m_Vector(vec), m_Result(std::numeric_limits<T>::max()) {}
    VectorMinElement(VectorMinElement& vme, tbb::split) : m_Vector(vme.m_Vector), m_Result(std::numeric_limits<T>::max()) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            m_Result = m_Result < m_Vector[i] ? m_Result : m_Vector[i];
        }
    }

    void join(VectorMinElement& vme)
    {
        m_Result = m_Result < vme.m_Result ? m_Result : vme.m_Result;
    }

    T getResult() const noexcept { return m_Result; }

private:
    T m_Result;

    const std::vector<T>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T>
class VectorMaxElement
{
public:
    VectorMaxElement(const std::vector<T>& vec) : m_Vector(vec), m_Result(std::numeric_limits<T>::min()) {}
    VectorMaxElement(VectorMaxElement& vme, tbb::split) : m_Vector(vme.m_Vector), m_Result(std::numeric_limits<T>::min()) {}

    // overload () so it does finding max
    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            m_Result = m_Result > m_Vector[i] ? m_Result : m_Vector[i];
        }
    }

    void join(VectorMaxElement& vme)
    {
        m_Result = m_Result > vme.m_Result ? m_Result : vme.m_Result;
    }

    T getResult() const noexcept { return m_Result; }

private:
    T m_Result;

    const std::vector<T>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class ... T>
class VectorMaxAbs;

template<class T>
class VectorMaxAbs<T>
{
public:
    VectorMaxAbs(const std::vector<T>& vec) : m_Vector(vec), m_Result(0) {}
    VectorMaxAbs(VectorMaxAbs& vma, tbb::split) : m_Vector(vma.m_Vector), m_Result(0) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            T tmp = fabs(m_Vector[i]);
            m_Result = m_Result > tmp ? m_Result : tmp;
        }
    }

    void join(VectorMaxAbs& vma)
    {
        m_Result = m_Result > vma.m_Result ? m_Result : vma.m_Result;
    }

    T getResult() const noexcept { return m_Result; }

private:
    T m_Result;

    const std::vector<T>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T, class VectorType>
class VectorMaxAbs<T, VectorType>
{
public:
    VectorMaxAbs(const std::vector<VectorType>& vec) : m_Vector(vec), m_Result(0) {}
    VectorMaxAbs(VectorMaxAbs& vma, tbb::split) : m_Vector(vma.m_Vector), m_Result(0) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            T tmp = std::abs(m_Vector[i][0]);
            for(auto k = 1; k < m_Vector[i].length(); ++k)
            {
                tmp = tmp < std::abs(m_Vector[i][k]) ? std::abs(m_Vector[i][k]) : tmp;
            }

            m_Result = m_Result > tmp ? m_Result : tmp;
        }
    }

    void join(VectorMaxAbs& vma)
    {
        m_Result = m_Result > vma.m_Result ? m_Result : vma.m_Result;
    }

    T getResult() const noexcept { return m_Result; }

private:
    T                              m_Result;
    const std::vector<VectorType>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T, class VectorType>
class VectorMaxNorm2
{
public:
    VectorMaxNorm2(const std::vector<VectorType>& vec) : m_Vector(vec), m_ResultMax(std::numeric_limits<T>::min()) {}
    VectorMaxNorm2(VectorMaxNorm2& vmmn, tbb::split) : m_Vector(vmmn.m_Vector), m_ResultMax(std::numeric_limits<T>::min()) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            T mag2 = glm::length2(m_Vector[i]);
            m_ResultMax = m_ResultMax > mag2 ? m_ResultMax : mag2;
        }
    }

    void join(VectorMaxNorm2& vmmn)
    {
        m_ResultMax = m_ResultMax > vmmn.m_ResultMax ? m_ResultMax : vmmn.m_ResultMax;
    }

    T getResult() const noexcept { return m_ResultMax; }

private:
    T                              m_ResultMax;
    const std::vector<VectorType>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T>
class VectorMinMaxElements
{
public:
    VectorMinMaxElements(const std::vector<T>& vec) : m_Vector(vec), m_ResultMin(std::numeric_limits<T>::max()), m_ResultMax(std::numeric_limits<T>::min()) {}
    VectorMinMaxElements(VectorMinMaxElements<T>& vmme, tbb::split) : m_Vector(vmme.m_Vector), m_ResultMin(std::numeric_limits<T>::max()), m_ResultMax(std::numeric_limits<T>::min()) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            m_ResultMin = m_ResultMin < m_Vector[i] ? m_ResultMin : m_Vector[i];
            m_ResultMax = m_ResultMax > m_Vector[i] ? m_ResultMax : m_Vector[i];
        }
    }

    void join(VectorMinMaxElements<T>& vmme)
    {
        m_ResultMin = m_ResultMin < vmme.m_ResultMin ? m_ResultMin : vmme.m_ResultMin;
        m_ResultMax = m_ResultMax > vmme.m_ResultMax ? m_ResultMax : vmme.m_ResultMax;
    }

    T getMin() const noexcept { return m_ResultMin; }
    T getMax() const noexcept { return m_ResultMax; }

private:
    T m_ResultMin;
    T m_ResultMax;

    const std::vector<T>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class Scalar, class VectorType>
class VectorMinMaxVectorElements
{
public:
    VectorMinMaxVectorElements(const std::vector<VectorType>& vec) :
        m_Vector(vec), m_ResultMin(VectorType(std::numeric_limits<Scalar>::max())), m_ResultMax(VectorType(std::numeric_limits<Scalar>::min())) {}
    VectorMinMaxVectorElements(VectorMinMaxVectorElements<Scalar, VectorType>& vmme, tbb::split) :
        m_Vector(vmme.m_Vector), m_ResultMin(VectorType(std::numeric_limits<Scalar>::max())), m_ResultMax(VectorType(std::numeric_limits<Scalar>::min())) {}

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
template<class T, class VectorType>
class VectorMinMaxNorm2
{
public:
    VectorMinMaxNorm2(const std::vector<VectorType>& vec) : m_Vector(vec), m_ResultMin(std::numeric_limits<T>::max()), m_ResultMax(std::numeric_limits<T>::min()) {}
    VectorMinMaxNorm2(VectorMinMaxNorm2& vmmn, tbb::split) : m_Vector(vmmn.m_Vector), m_ResultMin(std::numeric_limits<T>::max()), m_ResultMax(std::numeric_limits<T>::min()) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            T mag2 = glm::length2(m_Vector[i]);
            m_ResultMin = m_ResultMin < mag2 ? m_ResultMin : mag2;
            m_ResultMax = m_ResultMax > mag2 ? m_ResultMax : mag2;
        }
    }

    void join(VectorMinMaxNorm2& vmmn)
    {
        m_ResultMin = m_ResultMin < vmmn.m_ResultMin ? m_ResultMin : vmmn.m_ResultMin;
        m_ResultMax = m_ResultMax > vmmn.m_ResultMax ? m_ResultMax : vmmn.m_ResultMax;
    }

    T getMin() const noexcept { return m_ResultMin; }
    T getMax() const noexcept { return m_ResultMax; }

private:
    T m_ResultMin;
    T m_ResultMax;

    const std::vector<VectorType>& m_Vector;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
template<class T>
class VectorSum
{
public:
    VectorSum(const std::vector<T>& vec) : m_Vector(vec), m_Result(0) {}
    VectorSum(VectorSum& vsum, tbb::split) : m_Vector(vsum.v), m_Result(0) {}

    void operator ()(const tbb::blocked_range<size_t>& r)
    {
        for(size_t i = r.begin(); i != r.end(); ++i)
        {
            m_Result += m_Vector[i];
        }
    }

    void join(VectorSum& vsum)
    {
        m_Result += vsum.m_Result;
    }

    T getSum() const noexcept { return m_Result; }
private:
    const std::vector<T>& m_Vector;
    T                     m_Result;
};


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
}   // end namespace ParallelObjects