////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __CORE_PTR_H__
#define __CORE_PTR_H__


namespace Noesis
{

typedef decltype(nullptr) NullPtrT;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Automatically handles lifetime of BaseRefCounted objects by doing AddReference() and Release().
/// Ptr<> overloads certain operators in order to behave similar to a pointer but ensuring that
/// objects are deleted when they are no longer needed, to avoid memory leaks.
///
/// BaseRefCounted objects start with reference count set to 1. When they are stored inside a
/// Ptr<> the count is increased by 1 again. To avoid having to do a manual Release(), Ptr<>
/// supports being constructed from a reference. In this case the reference counter is not
/// incremented again:
///
///     Ptr<Cube> cube = *new Cube(50.0f);
///
/// Or you can use MakePtr for the same effect:
///
///     Ptr<Cube> cube = MakePtr<Cube>(50.0f);
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T> class Ptr
{
public:
    typedef T Type;

    /// Constructs an empty smart pointer
    Ptr();
    Ptr(NullPtrT);

    /// Constructs a smart pointer from a raw pointer, incrementing the reference count
    explicit Ptr(T* ptr);

    /// Constructor from dereferenced pointer, without incrementing the reference count.
    /// Very useful for assigning from new operator: Ptr<Cube> cube = *new Cube(50.0f);
    /// MakePtr is a more flexible alternative to this pattern.
    Ptr(T& ptr);

    /// Copy constructors
    Ptr(const Ptr& ptr);
    template<class S> Ptr(const Ptr<S>& ptr);

    /// Move constructors
    Ptr(Ptr&& ptr);
    template<class S> Ptr(Ptr<S>&& ptr);

    /// Destructor
    ~Ptr();

    /// Copy operators
    Ptr& operator=(const Ptr& ptr);
    template<class S> Ptr& operator=(const Ptr<S>& ptr);

    /// Move operators
    Ptr& operator=(Ptr&& ptr);
    template<class S> Ptr& operator=(Ptr<S>&& ptr);

    /// Copy from from dereferenced pointer, without incrementing the reference count.
    /// Very useful for assigning from new operator: cube = *new Cube(50.0f);
    Ptr& operator=(T& ptr);

    /// Resets to null pointer
    void Reset();

    /// Resets to pointer, increments reference count
    void Reset(T* ptr);

    /// Clears stored pointer without decrementing reference count
    T* GiveOwnership();

    /// Dereferences the stored pointer
    T* operator->() const;

    /// Returns the stored pointer
    T* GetPtr() const;

    /// Implicit  conversion to pointer
    operator T*() const;

private:
    T* mPtr;
};

/// Constructs an object of type T and wraps it in a Ptr<>
template<class T, class... Args> Ptr<T> MakePtr(Args&&... args);

}

#include <NsCore/Ptr.inl>

#endif
