/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <cstddef>

namespace AP_HAL {

/* Poor's man std::unique_ptr
 *
 * OwnPtr is a container for a pointer declaring ownership.
 *
 * The goal is to allow to own object, pass it around and automatically delete
 * the pointer and the container goes out of scope.
 *
 * In order to pass it around the move constructor or move assignment operator
 * must be used. operator*, operator-> and the get() method can be used to get
 * the pointer contained in the OwnPtr container.
 *
 * The OwnPtr name comes from similar class on WebKit, before they switched to
 * std::unique_ptr. The implementation is different/simpler.
 */
template<typename T>
class OwnPtr {
public:
    OwnPtr() : _ptr(nullptr) { }
    OwnPtr(std::nullptr_t) : _ptr(nullptr) { }

    /* non-copyable */
    OwnPtr(const OwnPtr<T> &other) = delete;

    /* Allow construction from a derived class U */
    template<typename U>
    OwnPtr(OwnPtr<U>&& other) : _ptr(other.leak()) { }

    OwnPtr(T *ptr) : _ptr(ptr) { }

    OwnPtr<T>& operator=(std::nullptr_t) { clear(); return *this; }

    template<typename U>
    OwnPtr<T>& operator=(OwnPtr<U>&& other)
    {
        T *old = _ptr;
        _ptr = other.leak();
        delete old;
        return *this;
    }

    ~OwnPtr() { delete _ptr; }

    void clear()
    {
        T *old = _ptr;
        _ptr = nullptr;
        delete old;
    }

    T *leak()
    {
        T *old = _ptr;
        _ptr = nullptr;
        return old;
    }

    T *get() const
    {
        return _ptr;
    }

    T& operator*() const { return *_ptr; }
    T *operator->() const { return _ptr; }
    bool operator !() const { return !_ptr; }

private:
    T *_ptr;
};

template<typename T>
inline bool operator==(T* a, const OwnPtr<T>& b)
{
    return a == b.get();
}

template<typename T>
inline bool operator==(const OwnPtr<T>& a, T* b)
{
    return a.get() == b;
}

template<typename T>
inline bool operator!=(T* a, const OwnPtr<T>& b)
{
    return a != b.get();
}

template<typename T>
inline bool operator!=(const OwnPtr<T>& a, T* b)
{
    return a.get() != b;
}

}
