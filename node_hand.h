

#ifndef _NODE_HANDLE
#define _NODE_HANDLE 1

#if __cplusplus >= 201703L
#define __cpp_lib_node_extract 201606L

#include <new>
#include <bits/alloc_traits.h>
#include <bits/ptr_traits.h>

/// Base class for node handle types of maps and sets.
template <typename _Val, typename _NodeAlloc>
class _Node_handle_common
{
    using _AllocTraits = allocator_traits<_NodeAlloc>;

public:
    using allocator_type = __alloc_rebind<_NodeAlloc, _Val>;

    allocator_type
    get_allocator() const noexcept
    {
        __glibcxx_assert(!this->empty());
        return allocator_type(_M_alloc._M_alloc);
    }

    explicit operator bool() const noexcept { return _M_ptr != nullptr; }

    [[nodiscard]] bool empty() const noexcept { return _M_ptr == nullptr; }

    /// @cond undocumented
protected:
    constexpr _Node_handle_common() noexcept : _M_ptr() {}

    ~_Node_handle_common()
    {
        if (!empty())
            _M_reset();
    }

    _Node_handle_common(_Node_handle_common &&__nh) noexcept
        : _M_ptr(__nh._M_ptr)
    {
        if (_M_ptr)
            _M_move(std::move(__nh));
    }

    _Node_handle_common &
    operator=(_Node_handle_common &&__nh) noexcept
    {
        if (empty())
        {
            if (!__nh.empty())
                _M_move(std::move(__nh));
        }
        else if (__nh.empty())
            _M_reset();
        else
        {
            // Free the current node before replacing the allocator.
            _AllocTraits::destroy(*_M_alloc, _M_ptr->_M_valptr());
            _AllocTraits::deallocate(*_M_alloc, _M_ptr, 1);

            _M_alloc = __nh._M_alloc.release(); // assigns if POCMA
            _M_ptr = __nh._M_ptr;
            __nh._M_ptr = nullptr;
        }
        return *this;
    }

    _Node_handle_common(typename _AllocTraits::pointer __ptr,
                        const _NodeAlloc &__alloc)
        : _M_ptr(__ptr), _M_alloc(__alloc)
    {
        __glibcxx_assert(__ptr != nullptr);
    }

    void
    _M_swap(_Node_handle_common &__nh) noexcept
    {
        if (empty())
        {
            if (!__nh.empty())
                _M_move(std::move(__nh));
        }
        else if (__nh.empty())
            __nh._M_move(std::move(*this));
        else
        {
            using std::swap;
            swap(_M_ptr, __nh._M_ptr);
            _M_alloc.swap(__nh._M_alloc); // swaps if POCS
        }
    }

private:
    // Moves the pointer and allocator from __nh to *this.
    // Precondition: empty() && !__nh.empty()
    // Postcondition: !empty() && __nh.empty()
    void
    _M_move(_Node_handle_common &&__nh) noexcept
    {
        ::new (std::__addressof(_M_alloc)) _NodeAlloc(__nh._M_alloc.release());
        _M_ptr = __nh._M_ptr;
        __nh._M_ptr = nullptr;
    }

    // Deallocates the node, destroys the allocator.
    // Precondition: !empty()
    // Postcondition: empty()
    void
    _M_reset() noexcept
    {
        _NodeAlloc __alloc = _M_alloc.release();
        _AllocTraits::destroy(__alloc, _M_ptr->_M_valptr());
        _AllocTraits::deallocate(__alloc, _M_ptr, 1);
        _M_ptr = nullptr;
    }

    // Destroys the allocator. Does not deallocate or destroy the node.
    // Precondition: !empty()
    // Postcondition: empty()
    void
    release() noexcept
    {
        _M_alloc.release();
        _M_ptr = nullptr;
    }

protected:
    typename _AllocTraits::pointer _M_ptr;

private:
    // A simplified, non-copyable std::optional<_NodeAlloc>.
    // Call release() before destruction iff the allocator member is active.
    union _Optional_alloc
    {
        _Optional_alloc() {}
        ~_Optional_alloc() {}

        _Optional_alloc(_Optional_alloc &&) = delete;
        _Optional_alloc &operator=(_Optional_alloc &&) = delete;

        _Optional_alloc(const _NodeAlloc &__alloc) noexcept
            : _M_alloc(__alloc)
        {
        }

        // Precondition: _M_alloc is the active member of the union.
        void
        operator=(_NodeAlloc &&__alloc) noexcept
        {
            using _ATr = _AllocTraits;
            if constexpr (_ATr::propagate_on_container_move_assignment::value)
                _M_alloc = std::move(__alloc);
            else if constexpr (!_AllocTraits::is_always_equal::value)
                __glibcxx_assert(_M_alloc == __alloc);
        }

        // Precondition: _M_alloc is the active member of both unions.
        void
        swap(_Optional_alloc &__other) noexcept
        {
            using std::swap;
            if constexpr (_AllocTraits::propagate_on_container_swap::value)
                swap(_M_alloc, __other._M_alloc);
            else if constexpr (!_AllocTraits::is_always_equal::value)
                __glibcxx_assert(_M_alloc == __other._M_alloc);
        }

        // Precondition: _M_alloc is the active member of the union.
        _NodeAlloc &operator*() noexcept { return _M_alloc; }

        // Precondition: _M_alloc is the active member of the union.
        _NodeAlloc release() noexcept
        {
            _NodeAlloc __tmp = std::move(_M_alloc);
            _M_alloc.~_NodeAlloc();
            return __tmp;
        }

        [[__no_unique_address__]] _NodeAlloc _M_alloc;
    };

    [[__no_unique_address__]] _Optional_alloc _M_alloc;

    template <typename _Key2, typename _Value2, typename _KeyOfValue,
              typename _Compare, typename _ValueAlloc>
    friend class Rb_tree;

    template <typename _Key2, typename _Value2, typename _ValueAlloc,
              typename _ExtractKey, typename _Equal,
              typename _Hash, typename _RangeHash, typename _Unused,
              typename _RehashPolicy, typename _Traits>
    friend class _Hashtable;

    /// @endcond
};

/// Node handle type for maps.
// template <typename _Key, typename _Value, typename _NodeAlloc>
// class Node_handle : public _Node_handle_common<_Value, _NodeAlloc>
// {
// public:
//     constexpr Node_handle() noexcept = default;
//     ~Node_handle() = default;
//     Node_handle(Node_handle &&) noexcept = default;

//     Node_handle &
//     operator=(Node_handle &&) noexcept = default;

//     using key_type = _Key;
//     using mapped_type = typename _Value::second_type;

//     key_type &
//     key() const noexcept
//     {
//         __glibcxx_assert(!this->empty());
//         return *_M_pkey;
//     }

//     mapped_type &
//     mapped() const noexcept
//     {
//         __glibcxx_assert(!this->empty());
//         return *_M_pmapped;
//     }

//     void
//     swap(Node_handle &__nh) noexcept
//     {
//         this->_M_swap(__nh);
//         using std::swap;
//         swap(_M_pkey, __nh._M_pkey);
//         swap(_M_pmapped, __nh._M_pmapped);
//     }

//     friend void
//     swap(Node_handle &__x, Node_handle &__y) noexcept(noexcept(__x.swap(__y)))
//     {
//         __x.swap(__y);
//     }

// private:
//     using _AllocTraits = allocator_traits<_NodeAlloc>;

//     Node_handle(typename _AllocTraits::pointer __ptr,
//                  const _NodeAlloc &__alloc)
//         : _Node_handle_common<_Value, _NodeAlloc>(__ptr, __alloc)
//     {
//         if (__ptr)
//         {
//             auto &__key = const_cast<_Key &>(__ptr->_M_valptr()->first);
//             _M_pkey = _S_pointer_to(__key);
//             _M_pmapped = _S_pointer_to(__ptr->_M_valptr()->second);
//         }
//         else
//         {
//             _M_pkey = nullptr;
//             _M_pmapped = nullptr;
//         }
//     }

//     template <typename _Tp>
//     using __pointer = __ptr_rebind<typename _AllocTraits::pointer,
//                                    remove_reference_t<_Tp>>;

//     __pointer<_Key> _M_pkey = nullptr;
//     __pointer<typename _Value::second_type> _M_pmapped = nullptr;

//     template <typename _Tp>
//     __pointer<_Tp>
//     _S_pointer_to(_Tp &__obj)
//     {
//         return pointer_traits<__pointer<_Tp>>::pointer_to(__obj);
//     }

//     const key_type &
//     _M_key() const noexcept { return key(); }

//     template <typename _Key2, typename _Value2, typename _KeyOfValue,
//               typename _Compare, typename _ValueAlloc>
//     friend class Rb_tree;

//     template <typename _Key2, typename _Value2, typename _ValueAlloc,
//               typename _ExtractKey, typename _Equal,
//               typename _Hash, typename _RangeHash, typename _Unused,
//               typename _RehashPolicy, typename _Traits>
//     friend class _Hashtable;
// };

/// Node handle type for sets.
template <typename _Value, typename _NodeAlloc>
class Node_handle<_Value, _Value, _NodeAlloc>
    : public _Node_handle_common<_Value, _NodeAlloc>
{
public:
    constexpr Node_handle() noexcept = default;
    ~Node_handle() = default;
    Node_handle(Node_handle &&) noexcept = default;

    Node_handle &
    operator=(Node_handle &&) noexcept = default;

    using value_type = _Value;

    value_type &
    value() const noexcept
    {
        __glibcxx_assert(!this->empty());
        return *this->_M_ptr->_M_valptr();
    }

    void
    swap(Node_handle &__nh) noexcept
    {
        this->_M_swap(__nh);
    }

    friend void
    swap(Node_handle &__x, Node_handle &__y) noexcept(noexcept(__x.swap(__y)))
    {
        __x.swap(__y);
    }

private:
    using _AllocTraits = allocator_traits<_NodeAlloc>;

    Node_handle(typename _AllocTraits::pointer __ptr,
                 const _NodeAlloc &__alloc)
        : _Node_handle_common<_Value, _NodeAlloc>(__ptr, __alloc) {}

    const value_type &
    _M_key() const noexcept { return value(); }

    template <typename _Key, typename _Val, typename _KeyOfValue,
              typename _Compare, typename _Alloc>
    friend class Rb_tree;

    template <typename _Key2, typename _Value2, typename _ValueAlloc,
              typename _ExtractKey, typename _Equal,
              typename _Hash, typename _RangeHash, typename _Unused,
              typename _RehashPolicy, typename _Traits>
    friend class _Hashtable;
};

/// Return type of insert(node_handle&&) on unique maps/sets.
template <typename _Iterator, typename _NodeHandle>
struct _Node_insert_return
{
    _Iterator position = _Iterator();
    bool inserted = false;
    _NodeHandle node;
};

/// @}

#endif // C++17
#endif
