#ifndef RB_TREE_H
#define RB_TREE_H

//#include "node_hand.h"
#include <ext/aligned_buffer.h>    // 内存对齐的缓冲区
#include <ext/alloc_traits.h>      // 分配器特性
#include <bits/cpp_type_traits.h>  // 类型特性
#include "allocator.h"

template <typename _Val, typename _NodeAlloc>
class Node_handle_common
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
    constexpr Node_handle_common() noexcept : _M_ptr() {}

    ~Node_handle_common()
    {
        if (!empty())
            _M_reset();
    }

    Node_handle_common(Node_handle_common &&__nh) noexcept
        : _M_ptr(__nh._M_ptr)
    {
        if (_M_ptr)
            _M_move(std::move(__nh));
    }

    Node_handle_common &
    operator=(Node_handle_common &&__nh) noexcept
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
            _AllocTraits::destroy(*_M_alloc, _M_ptr->M_valptr());
            _AllocTraits::deallocate(*_M_alloc, _M_ptr, 1);

            _M_alloc = __nh._M_alloc.release(); // assigns if POCMA
            _M_ptr = __nh._M_ptr;
            __nh._M_ptr = nullptr;
        }
        return *this;
    }

    Node_handle_common(typename _AllocTraits::pointer __ptr,
                        const _NodeAlloc &__alloc)
        : _M_ptr(__ptr), _M_alloc(__alloc)
    {
        __glibcxx_assert(__ptr != nullptr);
    }

    void
    _M_swap(Node_handle_common &__nh) noexcept
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
    _M_move(Node_handle_common &&__nh) noexcept
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
        _AllocTraits::destroy(__alloc, _M_ptr->M_valptr());
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

template <typename Key, typename _Value, typename _NodeAlloc>
class Node_handle : public Node_handle_common<_Value, _NodeAlloc>
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
        return *this->_M_ptr->M_valptr();
    }

    void
    swap(Node_handle &__nh) noexcept
    {
        this->_M_swap(__nh);
    }

    friend void
    swap(Node_handle &x, Node_handle &y) noexcept(noexcept(x.swap(y)))
    {
        x.swap(y);
    }

private:
    using _AllocTraits = allocator_traits<_NodeAlloc>;

    Node_handle(typename _AllocTraits::pointer __ptr,
                 const _NodeAlloc &__alloc)
        : Node_handle_common<_Value, _NodeAlloc>(__ptr, __alloc) {}

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
struct Node_insert_return
{
    _Iterator position = _Iterator();
    bool inserted = false;
    _NodeHandle node;
};

enum Rb_tree_color { S_red = false, S_black = true }; // 红黑树节点颜色定义

// 红黑树节点基类，不存储具体数据
struct Rb_tree_node_base{

    using Base_ptr = Rb_tree_node_base*;
    using Const_Base_ptr = const Rb_tree_node_base*;

    Rb_tree_color   M_color;    // 节点颜色（红/黑）
    Base_ptr        M_parent;   // 父节点指针
    Base_ptr        M_left;     // 左子节点指针
    Base_ptr        M_right;    // 右子节点指针

    // 查找子树的最小节点（最左叶子）
    static Base_ptr S_minimum(Base_ptr x){
      while (x->M_left != 0) x = x->M_left;   // 持续向左遍历
      return x;
    }

    static Const_Base_ptr S_minimum(Const_Base_ptr x){
      while (x->M_left != 0) x = x->M_left;
      return x;
    }

    // 查找子树的最大节点（最右叶子）
    static Base_ptr S_maximum(Base_ptr x){
      while (x->M_right != 0) x = x->M_right;   // 持续向右遍历
      return x;
    }

    static Const_Base_ptr S_maximum(Const_Base_ptr x){
      while (x->M_right != 0) x = x->M_right;
      return x;
    }

  };

// 红黑树头结构，管理树的元数据
struct Rb_tree_header{
    Rb_tree_node_base   M_header;       // 头节点（根节点的父节点）
    size_t              M_node_count;   // 树中节点总数

    // 默认构造函数：初始化头节点颜色为红色，并重置树为空
    Rb_tree_header() {
      M_header.M_color = S_red;
      M_reset();
    }

    // 移动构造函数：从另一个头结构转移数据
    Rb_tree_header(Rb_tree_header&& x) noexcept{
        if (x.M_header.M_parent != nullptr){
            M_move_data(x);      // 转移数据
        }
        else{
	        M_header.M_color = S_red;
	        M_reset();             // 源树为空则重置
	    }
    }

    // 数据转移：将源头的根节点、边界和计数复制到当前对象
    void M_move_data(Rb_tree_header& from){
        M_header.M_color = from.M_header.M_color;
        M_header.M_parent = from.M_header.M_parent;  // 转移根节点
        M_header.M_left = from.M_header.M_left;       // 转移左边界
        M_header.M_right = from.M_header.M_right;     // 转移右边界
        M_header.M_parent->M_parent = &M_header;      // 更新根节点的父指针
        M_node_count = from.M_node_count;             // 转移节点计数
        from.M_reset();                               // 清空源树
    }

    // 重置树为空状态：根节点为空，左右边界指向头自身
    void M_reset() {
        M_header.M_parent = 0;
        M_header.M_left = &M_header;
        M_header.M_right = &M_header;
        M_node_count = 0;
    }
};

// 红黑树节点模板类，继承基类并存储具体值
template<typename Val>
struct Rb_tree_node : public Rb_tree_node_base{

    using Link_type = Rb_tree_node<Val>*;

    __gnu_cxx::__aligned_membuf<Val> M_storage;     // 内存对齐的值存储

    // 获取存储值的指针（非const版本）
    Val* M_valptr(){ 
        return M_storage._M_ptr(); 
    }

    const Val* M_valptr() const { 
        return M_storage._M_ptr(); 
    }
};

// 旋转操作实现 
void Rb_tree_rotate_left(Rb_tree_node_base* x, Rb_tree_node_base& header) noexcept {
    Rb_tree_node_base* const y = x->M_right;
    
    x->M_right = y->M_left;
    if (y->M_left)
        y->M_left->M_parent = x;
    y->M_parent = x->M_parent;
    
    if (x == header.M_parent)
        header.M_parent = y;
    else if (x == x->M_parent->M_left)
        x->M_parent->M_left = y;
    else 
        x->M_parent->M_right = y;
    
    y->M_left = x;
    x->M_parent = y;
}
 
void Rb_tree_rotate_right(Rb_tree_node_base* x, Rb_tree_node_base& header) noexcept {
    Rb_tree_node_base* const y = x->M_left;
    
    x->M_left = y->M_right;
    if (y->M_right)
        y->M_right->M_parent = x;
    y->M_parent = x->M_parent;
    
    if (x == header.M_parent)
        header.M_parent = y;
    else if (x == x->M_parent->M_right)
        x->M_parent->M_right = y;
    else 
        x->M_parent->M_left = y;
    
    y->M_right = x;
    x->M_parent = y;
}

// 插入平衡完整实现 
void Rb_tree_insert_and_rebalance(bool insert_left, Rb_tree_node_base* x, Rb_tree_node_base* p,
                                  Rb_tree_node_base& header) noexcept {
    
    if (p == &header) {
        header.M_parent = x;
        header.M_left = x;
        header.M_right = x;
    }

    Rb_tree_node_base *& root = header.M_parent;
 
    x->M_parent = p;
    x->M_left = x->M_right = nullptr;
    x->M_color = S_red;
 
    if (insert_left) {
        p->M_left = x;
        if (p == &header) {
            header.M_parent = x;
            header.M_right = x;
        } else if (p == header.M_left)
            header.M_left = x;
    } else {
        p->M_right = x;
        if (p == header.M_right)
            header.M_right = x;
    }
 
    // 平衡调整 
    while (x != root && x->M_parent->M_color == S_red) {
        Rb_tree_node_base* const xpp = x->M_parent->M_parent;
        if (!xpp) break;
        if (x->M_parent == xpp->M_left) {
            Rb_tree_node_base* const y = xpp->M_right;
            if (y && y->M_color == S_red) {
                x->M_parent->M_color = S_black;
                y->M_color = S_black;
                xpp->M_color = S_red;
                x = xpp;
            } else {
                if (x == x->M_parent->M_right) {
                    x = x->M_parent;
                    Rb_tree_rotate_left(x, header);
                }
                x->M_parent->M_color = S_black;
                xpp->M_color = S_red;
                Rb_tree_rotate_right(xpp, header);
            }
        } else {
            Rb_tree_node_base* const y = xpp->M_left;
            if (y && y->M_color == S_red) {
                x->M_parent->M_color = S_black;
                y->M_color = S_black;
                xpp->M_color = S_red;
                x = xpp;
            } else {
                if (x == x->M_parent->M_left) {
                    x = x->M_parent;
                    Rb_tree_rotate_right(x, header);
                }
                x->M_parent->M_color = S_black;
                xpp->M_color = S_red;
                Rb_tree_rotate_left(xpp, header);
            }
        }
    }
    root->M_color = S_black;
}
 
// 删除平衡调整函数
Rb_tree_node_base* Rb_tree_rebalance_for_erase(Rb_tree_node_base* const z, Rb_tree_node_base& header) {
    Rb_tree_node_base*& root = header.M_parent;
    Rb_tree_node_base*& leftmost = header.M_left;
    Rb_tree_node_base*& rightmost = header.M_right;
    Rb_tree_node_base* y = z;
    Rb_tree_node_base* x = nullptr;
    Rb_tree_node_base* x_parent = nullptr;

    // 找到替代节点 y
    if (y->M_left == nullptr)
        x = y->M_right;
    else if (y->M_right == nullptr)
        x = y->M_left;
    else {
        y = y->M_right;
        while (y->M_left != nullptr)
            y = y->M_left;
        x = y->M_right;
    }

    if (y != z) {
        if (z->M_left) { 
            z->M_left->M_parent = y; 
            y->M_left = z->M_left; 
        } 
        if (y != z->M_right) {
            x_parent = y->M_parent;
            if (x)
                x->M_parent = y->M_parent;
            y->M_parent->M_left = x;
            y->M_right = z->M_right;
            z->M_right->M_parent = y;
        } else {
            x_parent = y->M_parent;
        }
        if (root == z)
            root = y;
        else if (z->M_parent->M_left == z)
            z->M_parent->M_left = y;
        else
            z->M_parent->M_right = y;
        y->M_parent = z->M_parent;
        std::swap(y->M_color, z->M_color);
        y = z;
    } else {
        x_parent = y->M_parent;
        if (x)
            x->M_parent = y->M_parent;
        if (root == z)
            root = x;
        else if (z->M_parent->M_left == z)
            z->M_parent->M_left = x;
        else
            z->M_parent->M_right = x;
        if (leftmost == z)
            leftmost = (z->M_right == nullptr) ? z->M_parent : Rb_tree_node_base::S_minimum(x);
        if (rightmost == z)
            rightmost = (z->M_left == nullptr) ? z->M_parent : Rb_tree_node_base::S_maximum(x);
    }

    // 如果 y 是黑色节点，进行平衡调整
    if (!y->M_color) {
        while (x != root && (x == nullptr || !x->M_color)) {
            if (x == x_parent->M_left) {
                Rb_tree_node_base* w = x_parent->M_right;
                if (w && w->M_color) {
                    w->M_color = S_red;
                    x_parent->M_color = S_black;
                    Rb_tree_rotate_left(x_parent, header);
                    w = x_parent->M_right;
                }
                if ((w->M_left == nullptr || w->M_left && !w->M_left->M_color) &&
                    (w->M_right == nullptr || w->M_right && !w->M_right->M_color)) {
                    w->M_color = S_black;
                    x = x_parent;
                    x_parent = x_parent->M_parent;
                } else {
                    if ((w->M_right == nullptr || (w->M_right && !w->M_right->M_color))) {
                        if (w->M_left)
                            w->M_left->M_color = S_red;
                        w->M_color = S_black;
                        Rb_tree_rotate_right(w, header);
                        w = x_parent->M_right;
                    }
                    w->M_color = x_parent->M_color;
                    x_parent->M_color = S_red;
                    if (w && w->M_right)
                        w->M_right->M_color = S_red;
                    Rb_tree_rotate_left(x_parent, header);
                    x = root;
                }
            } 
            else {
                Rb_tree_node_base* w = x_parent->M_left;
                if (w && w->M_color) {
                    w->M_color = S_red;
                    x_parent->M_color = S_black;
                    Rb_tree_rotate_right(x_parent, header);
                    w = x_parent->M_left;
                }
                if ((w->M_right == nullptr || w->M_right && !w->M_right->M_color) && 
                    (w->M_left == nullptr || w->M_left && !w->M_left->M_color)) {
                    w->M_color = S_black;
                    x = x_parent;
                    x_parent = x_parent->M_parent;
                } else {
                    if (w->M_left == nullptr || w->M_left && !w->M_left->M_color) {
                        if (w->M_right && w->M_right)
                            w->M_right->M_color = S_red;
                        w->M_color = S_black;
                        Rb_tree_rotate_left(w, header);
                        w = x_parent->M_left;
                    }
                    w->M_color = x_parent->M_color;
                    x_parent->M_color = S_red;
                    if (w->M_left && w->M_left)
                        w->M_left->M_color = S_red;
                    Rb_tree_rotate_right(x_parent, header);
                    x = root;
                }
            
            }
        }
        if (x)
            x->M_color = S_red;
    }
    if (root) root->M_color = S_black;
    return y;
}

// 全局函数声明：迭代器的递增/递减操作
Rb_tree_node_base* Rb_tree_increment(Rb_tree_node_base* x) noexcept{
    if (x->M_right) { 
        x = x->M_right;
        while (x->M_left) x = x->M_left;
    } else {
        Rb_tree_node_base* y = x->M_parent;
        while (x == y->M_right) {
            x = y;
            y = y->M_parent;
        }
        if (x->M_right != y) x = y;
    }
    return x;
}

const Rb_tree_node_base* Rb_tree_increment(const Rb_tree_node_base* x) noexcept{
    return Rb_tree_increment(const_cast<Rb_tree_node_base*>(x));
}

Rb_tree_node_base* Rb_tree_decrement(Rb_tree_node_base* x) noexcept{
    if (x->M_color == S_red && x->M_parent->M_parent == x)
        x = x->M_right;
    else if (x->M_left) { 
        x = x->M_left;
        while (x->M_right) x = x->M_right;
    } else {
        Rb_tree_node_base* y = x->M_parent;
        while (x == y->M_left) {
            x = y;
            y = y->M_parent;
        }
        x = y;
    }
    return x;
}

const Rb_tree_node_base* Rb_tree_decrement(const Rb_tree_node_base* x) noexcept{
    return Rb_tree_decrement(const_cast<Rb_tree_node_base*>(x));
}

// 红黑树迭代器模板类（双向迭代器）
template<typename T>
struct Rb_tree_iterator{

    using value_type = T;
    using reference  = T&;
    using pointer    = T*;

    using iterator_category = bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;

    using Self      = Rb_tree_iterator<T>;
    using Base_ptr  = Rb_tree_node_base::Base_ptr;
    using Link_type = Rb_tree_node<T>*;

    Base_ptr M_node;   // 当前指向的节点

    // 默认构造函数：初始化为空节点
    Rb_tree_iterator() noexcept : M_node() {}

    // 显式构造函数：从基类指针初始化
    explicit Rb_tree_iterator(Base_ptr x) noexcept : M_node(x) {}

    // 解引用操作符：返回节点存储值的引用
    reference operator*() const noexcept { 
        return *static_cast<Link_type>(M_node)->M_valptr(); 
    }

    // 成员访问操作符：返回节点存储值的指针
    pointer operator->() const noexcept { 
        return static_cast<Link_type> (M_node)->M_valptr(); 
    }

    // 前缀递增：移动到中序遍历的下一个节点
    Self& operator++() noexcept {
        M_node = Rb_tree_increment(M_node);  // 调用全局递增函数
        return *this;
    }

    // 后缀递增：返回当前迭代器后递增
    Self operator++(int) noexcept {
        Self tmp = *this;
        M_node = Rb_tree_increment(M_node);
        return tmp;
    }

    // 前缀递减：移动到中序遍历的前一个节点
    Self& operator--() noexcept {
        M_node = Rb_tree_decrement(M_node);  // 调用全局递减函数
        return *this;
    }

    // 后缀递减：返回当前迭代器后递减
    Self operator--(int) noexcept {
        Self tmp = *this;
        M_node = Rb_tree_decrement(M_node);
        return tmp;
    }

    // 相等比较：检查两个迭代器是否指向同一节点
    friend bool operator==(const Self& x, const Self& y) noexcept { 
        return x.M_node == y.M_node; 
    }

    // 不等比较（C++20前需要显式定义）
#if ! __cpp_lib_three_way_comparison
    friend bool operator!=(const Self& x, const Self& y) noexcept { 
        return x.M_node != y.M_node; 
    }
#endif
};

// 红黑树常量迭代器模板类（逻辑与普通迭代器类似）
template<typename T>
struct Rb_tree_const_iterator{
    using value_type = T;
    using reference  = const T&;
    using pointer    = const T*;

    using iterator   = Rb_tree_iterator<T>;

    using iterator_category = bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;

    using Self      = Rb_tree_const_iterator<T>;
    using Base_ptr  = Rb_tree_node_base::Const_Base_ptr;
    using Link_type = const Rb_tree_node<T>*;

    Base_ptr M_node;

    Rb_tree_const_iterator() noexcept : M_node() {}

    explicit Rb_tree_const_iterator(Base_ptr x) noexcept : M_node(x) {}

    Rb_tree_const_iterator(const iterator& it) noexcept : M_node(it.M_node) {}

    iterator M_const_cast() const noexcept{ 
        return iterator(const_cast<typename iterator::Base_ptr>(M_node)); 
    }

    reference operator*() const noexcept{ 
        return *static_cast<Link_type>(M_node)->M_valptr(); 
    }

    pointer operator->() const noexcept{ 
        return static_cast<Link_type>(M_node)->M_valptr(); 
    }

    Self& operator++() noexcept{
	    M_node = Rb_tree_increment(M_node);
	    return *this;
    }

    Self operator++(int) noexcept{
	    Self tmp = *this;
	    M_node = Rb_tree_increment(M_node);
	    return tmp;
    }

    Self& operator--() noexcept {
	    M_node = Rb_tree_decrement(M_node);
	    return *this;
    }

    Self operator--(int) noexcept {
	    Self tmp = *this;
	    M_node = Rb_tree_decrement(M_node);
	    return tmp;
    }

    friend bool operator==(const Self& x, const Self& y) noexcept { 
        return x.M_node == y.M_node; 
    }

#if ! __cpp_lib_three_way_comparison
    friend bool operator!=(const Self& x, const Self& y) noexcept { 
        return x.M_node != y.M_node; 
    }
#endif
};

// 实现合并操作
template<typename Tree1, typename Cmp2>
struct Rb_tree_merge_helper {};

// 为红黑树提供键比较函数对象的封装
template<typename Key_compare>
struct Rb_tree_key_compare{
      Key_compare M_key_compare;

      Rb_tree_key_compare() noexcept(is_nothrow_default_constructible<Key_compare>::value)
      : M_key_compare(){}

      Rb_tree_key_compare(const Key_compare& comp) : M_key_compare(comp) {}

      Rb_tree_key_compare(const Rb_tree_key_compare&) = default;

      Rb_tree_key_compare(Rb_tree_key_compare&& x) noexcept(is_nothrow_copy_constructible<Key_compare>::value)
      : M_key_compare(x.M_key_compare) {}
};

// 红黑树体
template<typename Key, typename Val, typename KeyOfValue,
        typename Compare, typename Alloc = Allocator<Val> >
class Rb_tree{

    using Node_allocator = typename __gnu_cxx::__alloc_traits<Alloc>::template 
	                       rebind<Rb_tree_node<Val> >::other;        // 节点分配器类型

    using Alloc_traits = __gnu_cxx::__alloc_traits<Node_allocator>;  // 节点指针类型

protected:
    using Base_ptr = Rb_tree_node_base*;
    using Const_Base_ptr = const Rb_tree_node_base*;
    using Link_type = Rb_tree_node<Val>* ;
    using Const_Link_type = const Rb_tree_node<Val>*;

private:
    // 在红黑树的操作中，实现节点的重用或者分配新节点
    struct Reuse_or_alloc_node{
	    Reuse_or_alloc_node(Rb_tree& t) : M_root(t.M_root()), M_nodes(t.M_rightmost()), M_t(t){
	        if (M_root){
	            M_root->M_parent = 0;
	            if (M_nodes->M_left){
		            M_nodes = M_nodes->M_left;
	            }
            }
	        else{
	            M_nodes = 0;
	        }
        }

	    Reuse_or_alloc_node(const Reuse_or_alloc_node&) = delete;


	    ~Reuse_or_alloc_node() { 
            M_t.M_erase(static_cast<Link_type>(M_root)); 
        }

	    template<typename Arg>
	    Link_type operator()(Arg&& arg){
	        Link_type node = static_cast<Link_type>(M_extract());
	        if (node){
		        M_t.M_destroy_node(node);
		        M_t.M_construct_node(node, std::forward<Arg>(arg));
		        return node;
	        }
	        return M_t.M_create_node(std::forward<Arg>(arg));
	    }

    private:
	    Base_ptr M_extract(){
	        if (!M_nodes){
	            return M_nodes;
            }
	        Base_ptr node = M_nodes;
	        M_nodes = M_nodes->M_parent;
	        if (M_nodes){
	            if (M_nodes->M_right == node){
		            M_nodes->M_right = 0;
		            if (M_nodes->M_left){
		                M_nodes = M_nodes->M_left;
		                while (M_nodes->M_right){
			                M_nodes = M_nodes->M_right;
                        }
		                if (M_nodes->M_left){
			                M_nodes = M_nodes->M_left;
                        }
                    }
		        }
	            else { // 节点在左边
		            M_nodes->M_left = 0;
	            }
            }
	        else{
	            M_root = 0;
            }

	        return node;
	    }

        Base_ptr M_root;
        Base_ptr M_nodes;
        Rb_tree& M_t;
    };

    // 在红黑树中分配新节点
    struct Alloc_node{
	    Alloc_node(Rb_tree& t) : M_t(t) {}

	    template<typename Arg>
	    Link_type operator()(Arg&& arg) const { 
            return M_t.M_create_node(std::forward<Arg>(arg)); 
        }

        private:
	    Rb_tree& M_t;
    };

public:

    using key_type        = Key;
    using value_type      = Val;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type  = Alloc;

    // 获取红黑树类中使用的节点分配器
    Node_allocator& M_get_Node_allocator() noexcept { 
        return this->M_impl;
    }

    const Node_allocator& M_get_Node_allocator() const noexcept { 
        return this->M_impl; 
    }

    // 获取分配器副本
    allocator_type
    get_allocator() const noexcept { 
        return allocator_type(M_get_Node_allocator()); 
    }

protected:
    // 内部辅助函数：分配节点内存
    Link_type M_get_node() { 
        return Alloc_traits::allocate(M_get_Node_allocator(), 1); 
    }

    // 将节点内存归还给分配器
    void M_put_node(Link_type p) noexcept { 
        Alloc_traits::deallocate(M_get_Node_allocator(), p, 1); 
    }

    // 构造一个新的节点
    template<typename... Args>
	void M_construct_node(Link_type node, Args&&... args) {
	    try{
	      ::new(node) Rb_tree_node<Val>;
	      Alloc_traits::construct(M_get_Node_allocator(),
				                  node->M_valptr(),
				                  std::forward<Args>(args)...);
	    }
	    catch(...){
	      node->~Rb_tree_node<Val>();
	      M_put_node(node);
	      throw;
	    }
	}

    // 创建一个新的节点
    template<typename... Args>
	Link_type M_create_node(Args&&... args) {
	    Link_type tmp = M_get_node();
	    M_construct_node(tmp, std::forward<Args>(args)...);
	    return tmp;
	}

    // 销毁节点中的元素
    void M_destroy_node(Link_type p) noexcept {
	    Alloc_traits::destroy(M_get_Node_allocator(), p->M_valptr());
	    p->~Rb_tree_node<Val>();
    }

    // 销毁节点
    void M_drop_node(Link_type p) noexcept {
	    M_destroy_node(p);
	    M_put_node(p);
    }

    // 克隆一个红黑树节点
    template<bool MoveValue, typename NodeGen>
	Link_type M_clone_node(Link_type x, NodeGen& node_gen) {
	    using Vp = __conditional_t<MoveValue, value_type&&, const value_type&>;
	    Link_type tmp = node_gen(std::forward<Vp> (*x->M_valptr()));
	    tmp->M_color = x->M_color;
	    tmp->M_left = 0;
	    tmp->M_right = 0;
	    return tmp;
	}

protected:
    // 内部实现结构体，封装分配器、键比较器和头信息
    template<typename Key_compare, bool = __is_pod(Key_compare)>
    struct Rb_tree_impl : public Node_allocator, public Rb_tree_key_compare<Key_compare>, 
                          public Rb_tree_header{
        using Basekey_compare = Rb_tree_key_compare<Key_compare>;

        // 默认构造：初始化分配器和比较器
	    Rb_tree_impl() _GLIBCXX_NOEXCEPT_IF(
		                is_nothrow_default_constructible<Node_allocator>::value && 
                        is_nothrow_default_constructible<Basekey_compare>::value )
	                    : Node_allocator() {}

        // 拷贝构造：复制分配器和比较器
	    Rb_tree_impl(const Rb_tree_impl& x) : Node_allocator(Alloc_traits::_S_select_on_copy(x)), 
                                              Basekey_compare(x.M_key_compare), Rb_tree_header() {}

        // 移动构造：转移数据（noexcept根据类型特性）
	    Rb_tree_impl(Rb_tree_impl&&) noexcept( is_nothrow_move_constructible<Basekey_compare>::value) = default;

	    explicit Rb_tree_impl(Node_allocator&& a) : Node_allocator(std::move(a)) {}

	    Rb_tree_impl(Rb_tree_impl&& x, Node_allocator&& a) : Node_allocator(std::move(a)),
	                                                         Basekey_compare(std::move(x)),
	                                                         Rb_tree_header(std::move(x)) {}

	    Rb_tree_impl(const Key_compare& comp, Node_allocator&& a) : Node_allocator(std::move(a)), 
                                                                    Basekey_compare(comp) {}

	};

    Rb_tree_impl<Compare> M_impl;  // 核心数据成员

protected:
    Base_ptr& M_root() noexcept { // 根节点访问函数
        return this->M_impl.M_header.M_parent; 
    }

    Const_Base_ptr M_root() const noexcept { 
        return this->M_impl.M_header.M_parent; 
    }

    Base_ptr& M_leftmost() noexcept { // 最左节点访问函数
        return this->M_impl.M_header.M_left; 
    }

    Const_Base_ptr M_leftmost() const noexcept { 
        return this->M_impl.M_header.M_left; 
    }

    Base_ptr& M_rightmost() noexcept { // 最右节点访问函数
        return this->M_impl.M_header.M_right; 
    }

    Const_Base_ptr M_rightmost() const noexcept { 
        return this->M_impl.M_header.M_right; 
    }

    Link_type M_mbegin() const noexcept { //  开始节点访问函数
        return static_cast<Link_type>(this->M_impl.M_header.M_parent); 
    }

    Link_type M_begin() noexcept { 
        return M_mbegin(); 
    }

    Const_Link_type M_begin() const noexcept {
	    return static_cast<Const_Link_type>(this->M_impl.M_header.M_parent);
    }

    Base_ptr M_end() noexcept { // 结束节点访问函数
        return &this->M_impl.M_header; 
    }

    Const_Base_ptr M_end() const noexcept { 
        return &this->M_impl.M_header; 
    }

    // 获取键值的静态函数
    static const Key& S_key(Const_Link_type x) {
	    static_assert(__is_invocable<Compare&, const Key&, const Key&>{},
		                "comparison object must be invocable "
		                "with two arguments of key type");
	    if constexpr (__is_invocable<Compare&, const Key&, const Key&>{})
	    static_assert(is_invocable_v<const Compare&, const Key&, const Key&>,
	                    "comparison object must be invocable as const");

	    return KeyOfValue()(*x->M_valptr());
    }

    static const Key& S_key(Const_Base_ptr x) { 
        return S_key(static_cast<Const_Link_type>(x)); 
    }

    // 左右子节点的访问静态函数
    static Link_type S_left(Base_ptr x) noexcept { 
        return static_cast<Link_type>(x->M_left); 
    }

    static Const_Link_type S_left(Const_Base_ptr x) noexcept { 
        return static_cast<Const_Link_type>(x->M_left); 
    }

    static Link_type S_right(Base_ptr x) noexcept { 
        return static_cast<Link_type>(x->M_right); 
    }

    static Const_Link_type S_right(Const_Base_ptr x) noexcept { 
        return static_cast<Const_Link_type>(x->M_right); 
    }

    //最小和最大节点的访问静态函数
    static Base_ptr S_minimum(Base_ptr x) noexcept { 
        return Rb_tree_node_base::S_minimum(x); 
    }

    static Const_Base_ptr S_minimum(Const_Base_ptr x) noexcept { 
        return Rb_tree_node_base::S_minimum(x); 
    }

    static Base_ptr S_maximum(Base_ptr x) noexcept { 
        return Rb_tree_node_base::S_maximum(x); 
    }

    static Const_Base_ptr S_maximum(Const_Base_ptr x) noexcept { 
        return Rb_tree_node_base::S_maximum(x); 
    }

public:
    // 迭代器定义
    using iterator               = Rb_tree_iterator<value_type>;
    using const_iterator         = Rb_tree_const_iterator<value_type>;
    using reverse_iterator       = std::reverse_iterator<iterator> ;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using node_type              = Node_handle<Key, Val, Node_allocator>;
    using insert_return_type     = Node_insert_return<__conditional_t<is_same_v<Key, Val>, const_iterator, iterator>,
    node_type>;

    // 查找唯一插入位置：返回插入点及是否允许插入
    std::pair<Base_ptr, Base_ptr> M_get_insert_unique_pos(const key_type& k){
        typedef std::pair<Base_ptr, Base_ptr> Res;
        Link_type x = M_begin();    // 从根节点开始
        Base_ptr y = M_end();       // 初始父节点为头节点
        bool comp = S_black;   
        while (x != 0) {
            y = x;
            comp = M_impl.M_key_compare(k, S_key(x));  // 比较键值
            x = comp ? S_left(x) : S_right(x);         // 向左或右子树移动
        }
        iterator j = iterator(y);
        if (comp) {
            if (j == begin()) 
                return {x, y};  // 插入到最左侧
            else
                --j;  // 调整到前驱节点
        }

        if (M_impl.M_key_compare(S_key(j.M_node), k)){
            return Res(x, y);     // 允许插入
        }
        return Res(j.M_node, 0);  // 键已存在，禁止插入
    }

    std::pair<Base_ptr, Base_ptr> M_get_insert_equal_pos(const key_type& k){
        typedef std::pair<Base_ptr, Base_ptr> Res;
        Link_type x = M_begin();
        Base_ptr y = M_end();
        while (x != 0) {
            y = x;
            x = M_impl.M_key_compare(k, S_key(x)) ?
            S_left(x) : S_right(x);
        }
        return Res(x, y);
    }

    std::pair<Base_ptr, Base_ptr> M_get_insert_hint_unique_pos(const_iterator position, const key_type& k){
        iterator pos = position.M_const_cast();
        typedef std::pair<Base_ptr, Base_ptr> Res;

        if (pos.M_node == M_end()){
            if (size() > 0 && M_impl.M_key_compare(S_key(M_rightmost()), k)){
                return Res(0, M_rightmost());
            }
            else{
                return M_get_insert_unique_pos(k);
            }
        }
        else if (M_impl.M_key_compare(k, S_key(pos.M_node))) {

            iterator before = pos;
            if (pos.M_node == M_leftmost()) {
                return Res(M_leftmost(), M_leftmost());
            }
            else if (M_impl.M_key_compare(S_key((--before).M_node), k)) {
                if (S_right(before.M_node) == 0){
                    return Res(0, before.M_node);
                }
                else{
                    return Res(pos.M_node, pos.M_node);
                }
            }
            else{
            return M_get_insert_unique_pos(k);
            }
        }
        else if (M_impl.M_key_compare(S_key(pos.M_node), k)) {

            iterator after = pos;
            if (pos.M_node == M_rightmost()){
                return Res(0, M_rightmost());
            }
            else if (M_impl.M_key_compare(k, S_key((++after).M_node))) {
                if (S_right(pos.M_node) == 0){
                    return Res(0, pos.M_node);
                }
                else{
                    return Res(after.M_node, after.M_node);
                }
            }
            else{
                return M_get_insert_unique_pos(k);
            }
        }
        else{
            return Res(pos.M_node, 0);
        }
    }

    std::pair<Base_ptr, Base_ptr> M_get_insert_hint_equal_pos(const_iterator position, const key_type& k){
        iterator pos = position.M_const_cast();
        typedef std::pair<Base_ptr, Base_ptr> Res;

        if (pos.M_node == M_end()){
            if (size() > 0 && !M_impl.M_key_compare(k, S_key(M_rightmost()))){
                return Res(0, M_rightmost());
            }
            else{
                return M_get_insert_equal_pos(k);
            }
        }
        else if (!M_impl.M_key_compare(S_key(pos.M_node), k)) {
            iterator before = pos;
            if (pos.M_node == M_leftmost()) {
                return Res(M_leftmost(), M_leftmost());
            }
            else if (!M_impl.M_key_compare(k, S_key((--before).M_node))) {
                if (S_right(before.M_node) == 0){
                    return Res(0, before.M_node);
                }
                else{
                    return Res(pos.M_node, pos.M_node);
                }
            }
            else{
                return M_get_insert_equal_pos(k);
            }
        }
        else {
            iterator after = pos;
            if (pos.M_node == M_rightmost()){
                return Res(0, M_rightmost());
            }
            else if (!M_impl.M_key_compare(S_key((++after).M_node), k)) {
                if (S_right(pos.M_node) == 0){
                    return Res(0, pos.M_node);
                }
                else{
                return Res(after.M_node, after.M_node);
                }
            }
        else{
            return Res(0, 0);
            }
        }
    }

private:

    // 插入节点并调整树结构
    template<typename Arg, typename NodeGen>
	iterator M_insert(Base_ptr x, Base_ptr y, Arg&& v, NodeGen& node_gen){
        bool insert_left = (x != 0 || y == M_end() || M_impl.M_key_compare(KeyOfValue()(v), S_key(y)));
	    Link_type z = node_gen(std::forward<Arg>(v));   // 创建新节点
	    Rb_tree_insert_and_rebalance(insert_left, z, y, this->M_impl.M_header);   // 调整平衡
	    ++M_impl.M_node_count;                          // 更新节点计数
        return iterator(z);                             // 返回新节点的迭代器
    }

    iterator M_insert_node(Base_ptr x, Base_ptr y, Link_type z){
        bool insert_left = (x != 0 || y == M_end() || M_impl.M_key_compare(S_key(z), S_key(y)));

        Rb_tree_insert_and_rebalance(insert_left, z, y, this->M_impl.M_header);
        ++M_impl.M_node_count;
        return iterator(z);
    }

    template<typename Arg>
	iterator M_insert_lower(Base_ptr y, Arg&& v){
        bool insert_left = (y == M_end() || !M_impl.M_key_compare(S_key(y), KeyOfValue()(v)));
        Link_type z = M_create_node(std::forward<Arg>(v));
        Rb_tree_insert_and_rebalance(insert_left, z, y, this->M_impl.M_header);
        ++M_impl.M_node_count;
        return iterator(z);
    }

    template<typename Arg>
	iterator M_insert_equal_lower(Arg&& v){
        Link_type x = M_begin();
        Base_ptr y = M_end();
        while (x != 0) {
            y = x;
            x = !M_impl.M_key_compare(S_key(x), KeyOfValue()(v)) ? S_left(x) : S_right(x);
        }
        return M_insert_lower(y, std::forward<Arg>(v));
    }

    iterator M_insert_lower_node(Base_ptr p, Link_type z){
        bool insert_left = (p == M_end() || !M_impl.M_key_compare(S_key(p), S_key(z)));

        Rb_tree_insert_and_rebalance(insert_left, z, p, this->M_impl.M_header);
        ++M_impl.M_node_count;
        return iterator(z);
    }

    iterator M_insert_equal_lower_node(Link_type z){
        Link_type x = M_begin();
        Base_ptr y = M_end();
        while (x != 0) {
            y = x;
            x = !M_impl.M_key_compare(S_key(x), S_key(z)) ?
            S_left(x) : S_right(x);
        }
        return M_insert_lower_node(y, z);
    }

    enum { as_lvalue, as_rvalue };

    // 递归复制子树（用于深拷贝）
    template<bool MoveValues, typename NodeGen>
	Link_type M_copy(Link_type x, Base_ptr p, NodeGen& node_gen){
        // 结构复制, x和p不能为null。
        Link_type top = M_clone_node<MoveValues>(x, node_gen);  // 克隆当前节点
        top->M_parent = p;
        try {
            if (x->M_right){
                top->M_right = M_copy<MoveValues>(S_right(x), top, node_gen);  // 递归复制右子树
            }
            p = top;
            x = S_left(x);
            while (x != 0) {   // 递归复制左子树
                Link_type y = M_clone_node<MoveValues>(x, node_gen);
                p->M_left = y;
                y->M_parent = p;
                if (x->M_right){
                    y->M_right =M_copy<MoveValues>(S_right(x), y, node_gen);
                }
                p = y; 
                x = S_left(x);
            }
        }
        catch(...) {
            M_erase(top);  // 异常时回滚已分配节点
            throw;
        }
        return top;
    }

    template<bool MoveValues, typename NodeGen>
	Link_type M_copy(const Rb_tree& x, NodeGen& gen) {
	    Link_type root = M_copy<MoveValues>(x.M_mbegin(), M_end(), gen);
	    M_leftmost() = S_minimum(root);
	    M_rightmost() = S_maximum(root);
	    M_impl.M_node_count = x.M_impl.M_node_count;
	    return root;
	}

    Link_type M_copy(const Rb_tree& x) {
	    Alloc_node an(*this);
	    return M_copy<as_lvalue>(x, an);
    }

    void M_erase(Link_type x){
        // 擦除而不重新平衡。
        while (x != 0) {
            M_erase(S_right(x));
            Link_type y = S_left(x);
            M_drop_node(x);
            x = y;
        }
    }

    iterator M_lower_bound(Link_type x, Base_ptr y, const Key& k){
        while (x != 0){
            if (!M_impl.M_key_compare(S_key(x), k)) {
                y = x, x = S_left(x);
            }
            else{
                x = S_right(x);
            }
        }
        return iterator(y);
    }

    const_iterator M_lower_bound(Const_Link_type x, Const_Base_ptr y, const Key& k) const{
        while (x != 0){
            if (!M_impl.M_key_compare(S_key(x), k)){
                y = x, x = S_left(x);
            }
            else{
                x = S_right(x);}
            }
        return const_iterator(y);
    }

    iterator M_upper_bound(Link_type x, Base_ptr y, const Key& k){
        while (x != 0){
            if (M_impl.M_key_compare(k, S_key(x))){
                y = x, x = S_left(x);
            }
            else{
                x = S_right(x);
            }
        }
        return iterator(y);
    }

    const_iterator M_upper_bound(Const_Link_type x, Const_Base_ptr y, const Key& k) const{
        while (x != 0){
            if (M_impl.M_key_compare(k, S_key(x))){
                y = x, x = S_left(x);
            }
            else{
                x = S_right(x);
            }
        }
        return const_iterator(y);
    }

public:

    Rb_tree() = default;

    Rb_tree(const Compare& comp, const allocator_type& a = allocator_type())
      : M_impl(comp, Node_allocator(a)) {}

    Rb_tree(const Rb_tree& x) : M_impl(x.M_impl) {
	    if (x.M_root() != 0)
	    M_root() = M_copy(x);
    }

    Rb_tree(const allocator_type& a) : M_impl(Node_allocator(a)) {}

    Rb_tree(const Rb_tree& x, const allocator_type& a) : M_impl(x.M_impl.M_key_compare, Node_allocator(a)) {
	    if (x.M_root() != nullptr)
	    M_root() = M_copy(x);
    }

    Rb_tree(Rb_tree&&) = default;

    Rb_tree(Rb_tree&& x, const allocator_type& a) : Rb_tree(std::move(x), Node_allocator(a)) {}

private:
    Rb_tree(Rb_tree&& x, Node_allocator&& a, true_type) noexcept(is_nothrow_default_constructible<Compare>::value)
    : M_impl(std::move(x.M_impl), std::move(a)) {}

    Rb_tree(Rb_tree&& x, Node_allocator&& a, false_type) : M_impl(x.M_impl.M_key_compare, std::move(a)) {
	    if (x.M_root() != nullptr)
	    M_move_data(x, false_type{});
    }

public:
    Rb_tree(Rb_tree&& x, Node_allocator&& a) noexcept( noexcept( 
        Rb_tree(std::declval<Rb_tree&&>(), std::declval<Node_allocator&&>(),
		std::declval<typename Alloc_traits::is_always_equal>())) )
      : Rb_tree(std::move(x), std::move(a),
		typename Alloc_traits::is_always_equal{}) {}

    Rb_tree& operator=(const Rb_tree& x){
        if (this != std::__addressof(x)) {
            // Key 可能是一个常量类型
            if (Alloc_traits::_S_propagate_on_copy_assign()) {
                auto& this_alloc = this->M_get_Node_allocator();
                auto& that_alloc = x.M_get_Node_allocator();
                if (!Alloc_traits::_S_always_equal() && this_alloc != that_alloc) {
                // 替换分配器无法释放现有存储，我们需要首先擦除节点
                    clear();
                    std::__alloc_on_copy(this_alloc, that_alloc);
                }
            }
            Reuse_or_alloc_node roan(*this);
            M_impl.M_reset();
            M_impl.M_key_compare = x.M_impl.M_key_compare;
            if (x.M_root() != 0){
                M_root() =M_copy<as_lvalue>(x, roan);
            }
        }
        return *this;
    }

    Compare key_comp() const { 
        return M_impl.M_key_compare; 
    }

    iterator begin() noexcept { 
        return iterator(this->M_impl.M_header.M_left); 
    }

    const_iterator begin() const noexcept { 
        return const_iterator(this->M_impl.M_header.M_left); 
    }

    iterator end() noexcept { 
        return iterator(&this->M_impl.M_header); 
    }

    const_iterator end() const noexcept { 
        return const_iterator(&this->M_impl.M_header); 
    }

    reverse_iterator rbegin() noexcept { 
        return reverse_iterator(end()); 
    }

    const_reverse_iterator rbegin() const noexcept { 
        return const_reverse_iterator(end()); 
    }

    reverse_iterator rend() noexcept { 
        return reverse_iterator(begin()); 
    }

    const_reverse_iterator rend() const noexcept { 
        return const_reverse_iterator(begin()); 
    }

    bool empty() const noexcept { 
        return M_impl.M_node_count == 0; 
    }

    size_type size() const noexcept { 
        return M_impl.M_node_count; 
    }

    size_type max_size() const noexcept { 
        return Alloc_traits::max_size(M_get_Node_allocator()); 
    }

    void swap(Rb_tree& t) noexcept(__is_nothrow_swappable<Compare>::value){
        if (M_root() == 0) {
            if (t.M_root() != 0){
                M_impl.M_move_data(t.M_impl);
            }
        }
        else if (t.M_root() == 0){
            t.M_impl.M_move_data(M_impl);
        }
        else {
            std::swap(M_root(), t.M_root());
            std::swap(M_leftmost(), t.M_leftmost());
            std::swap(M_rightmost(), t.M_rightmost());
            M_root()->M_parent = M_end();
            t.M_root()->M_parent = t.M_end();
            std::swap(this->M_impl.M_node_count, t.M_impl.M_node_count);
        }

        // 无需交换头文件的颜色，因为它不会改变
        std::swap(this->M_impl.M_key_compare, t.M_impl.M_key_compare);
        Alloc_traits::_S_on_swap(M_get_Node_allocator(), t.M_get_Node_allocator());
    }

    // 唯一插入
    template<typename Arg>
	std::pair<iterator, bool> _M_insert_unique(Arg&& v){
        using Res = std::pair<iterator, bool>;
        std::pair<Base_ptr, Base_ptr> res = M_get_insert_unique_pos(KeyOfValue()(v));

        if (res.second) {
            Alloc_node an(*this);
            return Res(M_insert(res.first, res.second, std::forward<Arg>(v), an), S_black);
        }

        return Res(iterator(res.first), S_red);
    }

    // 可重复插入
    template<typename Arg>
	iterator _M_insert_equal(Arg&& v){
        std::pair<Base_ptr, Base_ptr> res = M_get_insert_equal_pos(KeyOfValue()(v));
        Alloc_node an(*this);
        return M_insert(res.first, res.second, std::forward<Arg>(v), an);
    }

    // 带位置提示的唯一插入
    template<typename Arg, typename NodeGen>
	iterator _M_insert_unique_(const_iterator pos, Arg&& v, NodeGen& node_gen){
        std::pair<Base_ptr, Base_ptr> res = M_get_insert_hint_unique_pos(pos, KeyOfValue()(v));

        if (res.second){
            return M_insert(res.first, res.second, std::forward<Arg>(v), node_gen);
        }
        return iterator(res.first);
    }

    // 带位置提示的唯一插入
    template<typename Arg>
	iterator _M_insert_unique_(const_iterator pos, Arg&& x) {
	    Alloc_node an(*this);
	    return _M_insert_unique_(pos, std::forward<Arg>(x), an);
	}

    // 带位置提示的可重复插入
    template<typename Arg, typename NodeGen>
	iterator _M_insert_equal_(const_iterator pos, Arg&& v, NodeGen& node_gen){
        std::pair<Base_ptr, Base_ptr> res = M_get_insert_hint_equal_pos(pos, KeyOfValue()(v));

        if (res.second){
        return M_insert(res.first, res.second, std::forward<Arg>(v), node_gen);
        }
        return M_insert_equal_lower(std::forward<Arg>(v));
    }

    // 带位置提示的可重复插入
    template<typename Arg>
	iterator _M_insert_equal_(const_iterator pos, Arg&& x) {
	    Alloc_node an(*this);
	    return _M_insert_equal_(pos, std::forward<Arg>(x), an);
	}

    // 原地构造唯一插入
    template<typename... Args>
	std::pair<iterator, bool> _M_emplace_unique(Args&&... args){
        Auto_node z(*this, std::forward<Args>(args)...);
        auto res = M_get_insert_unique_pos(z._M_key());
        if (res.second){
            return {z.M_insert(res), S_black};
        }
        return {iterator(res.first), S_red};
    }

    // 原地构造可重复插入
    template<typename... Args>
	iterator _M_emplace_equal(Args&&... args){
        Auto_node z(*this, std::forward<Args>(args)...);
        auto res = M_get_insert_equal_pos(z._M_key());
        return z.M_insert(res);
    }

    // 带位置提示的原地构造唯一插入
    template<typename... Args>
	iterator _M_emplace_hint_unique(const_iterator pos, Args&&... args){
        Auto_node z(*this, std::forward<Args>(args)...);
        auto res = M_get_insert_hint_unique_pos(pos, z._M_key());
        if (res.second){
            return z.M_insert(res);
        }
        return iterator(res.first);
    }

    // 带位置提示的原地构造可重复插入
    template<typename... Args>
	iterator _M_emplace_hint_equal(const_iterator pos, Args&&... args){
        Auto_node z(*this, std::forward<Args>(args)...);
        auto res = M_get_insert_hint_equal_pos(pos, z._M_key());
        if (res.second){
            return z.M_insert(res);
        }
        return z.M_insert_equal_lower();
    }

    template<typename _Iter>
	using same_value_type = is_same<value_type, typename iterator_traits<_Iter>::value_type>;

    // 以下是一些范围插入
    template<typename InputIterator>
	__enable_if_t<same_value_type<InputIterator>::value>
	M_insert_range_unique(InputIterator first, InputIterator last) {
	    Alloc_node an(*this);
	    for (; first != last; ++first)
	    _M_insert_unique_(end(), *first, an);
	}

    template<typename InputIterator>
	__enable_if_t<!same_value_type<InputIterator>::value>
	M_insert_range_unique(InputIterator first, InputIterator last) {
	    for (; first != last; ++first)
	    _M_emplace_unique(*first);
	}

    template<typename InputIterator>
	__enable_if_t<same_value_type<InputIterator>::value>
	M_insert_range_equal(InputIterator first, InputIterator last) {
	    Alloc_node an(*this);
	    for (; first != last; ++first)
	    _M_insert_equal_(end(), *first, an);
	}

    template<typename InputIterator>
	__enable_if_t<!same_value_type<InputIterator>::value>
	M_insert_range_equal(InputIterator first, InputIterator last) {
	    for (; first != last; ++first)
	    _M_emplace_equal(*first);
	}

private:
    // 删除节点并调整树结构
    void M_erase_aux(const_iterator position){
        // 调整平衡并获取被删除节点
        Link_type y = static_cast<Link_type>(
            Rb_tree_rebalance_for_erase(const_cast<Base_ptr>(position.M_node),  M_impl.M_header));
        M_drop_node(y);                // 销毁节点并释放内存
        --M_impl.M_node_count;         // 更新节点计数
    }

    void M_erase_aux(const_iterator first, const_iterator last){
        if (first == begin() && last == end()){
            clear();
        }
        else{
            while (first != last){
                M_erase_aux(first++);
            }
        }
    }

public:
    iterator erase(const_iterator position) { // 删除单个元素
	    __glibcxx_assert(position != end());
	    const_iterator result = position;
	    ++result;
	    M_erase_aux(position);
	    return result.M_const_cast();
    }

    __attribute ((__abi_tag__ ("cxx11")))
    iterator erase(iterator position) { 
	    iterator result = position;
	    ++result;
	    M_erase_aux(position);
	    return result;
    }

    // 根据键删除元素
    size_type erase(const key_type& x) {
        std::pair<iterator, iterator> p = equal_range(x);
        const size_type old_size = size();
        M_erase_aux(p.first, p.second);
        return old_size - size();
    }

    __attribute ((__abi_tag__ ("cxx11")))
    iterator erase(const_iterator first, const_iterator last) { // 删除范围元素
	    M_erase_aux(first, last);
	    return last.M_const_cast();
    }

    iterator find(const key_type& k){
        iterator j = M_lower_bound(M_begin(), M_end(), k);
        return (j == end() || M_impl.M_key_compare(k, S_key(j.M_node))) ? end() : j;  
    }

    const_iterator find(const key_type& k) const{
        const_iterator j = M_lower_bound(M_begin(), M_end(), k);
        return (j == end() || M_impl.M_key_compare(k, S_key(j.M_node))) ? end() : j;   
    }

    size_type count(const key_type& k) const {
        std::pair<const_iterator, const_iterator> p = equal_range(k);
        const size_type n = std::distance(p.first, p.second);
        return n;
    }

    iterator lower_bound(const key_type& k) { // 下界查找
        return M_lower_bound(M_begin(), M_end(), k); 
    }

    const_iterator lower_bound(const key_type& k) const { 
        return M_lower_bound(M_begin(), M_end(), k); 
    }

    iterator upper_bound(const key_type& k) { // 上界查找
        return M_upper_bound(M_begin(), M_end(), k); 
    }

    const_iterator upper_bound(const key_type& k) const { 
        return M_upper_bound(M_begin(), M_end(), k); 
    }

    // 等值范围查找
    std::pair<iterator, iterator> equal_range(const key_type& k){
        Link_type x = M_begin();
        Base_ptr y = M_end();
        while (x != 0) {
            if (M_impl.M_key_compare(S_key(x), k)){
                x = S_right(x);
            }
            else if (M_impl.M_key_compare(k, S_key(x))){
                y = x, x = S_left(x);
            }
            else {
                Link_type xu(x);
                Base_ptr yu(y);
                y = x, x = S_left(x);
                xu = S_right(xu);
                return std::pair<iterator, iterator>(M_lower_bound(x, y, k), M_upper_bound(xu, yu, k));
            }
        }
        return std::pair<iterator, iterator>(iterator(y), iterator(y));
    }

    std::pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
        Const_Link_type x = M_begin();
        Const_Base_ptr y = M_end();
        while (x != 0){
            if (M_impl.M_key_compare(S_key(x), k)){
                x = S_right(x);
            }
            else if (M_impl.M_key_compare(k, S_key(x))){
                y = x, x = S_left(x);
            }
            else {
                Const_Link_type xu(x);
                Const_Base_ptr yu(y);
                y = x, x = S_left(x);
                xu = S_right(xu);
                return std::pair<const_iterator, const_iterator>(M_lower_bound(x, y, k), M_upper_bound(xu, yu, k));
            }
        }
        return std::pair<const_iterator, const_iterator>(const_iterator(y), const_iterator(y));
    }

    void clear() noexcept {
	    M_erase(M_begin());
	    M_impl.M_reset();
    }

    // 以下是透明查找相关函数
    // 允许使用不同类型的键进行查找。提供了查找、统计、下界、上界和等值范围查找的功能
    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	iterator M_find_tr(const Kt& k) {
	    const Rb_tree* const_this = this;
	    return const_this->M_find_tr(k).M_const_cast();
	}

    // 透明查找：支持异构键类型（需比较器定义 is_transparent）
    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	const_iterator M_find_tr(const Kt& k) const {
	    auto j = M_lower_bound_tr(k);  // 查找下界
        if (j != end() && M_impl.M_key_compare(k, S_key(j.M_node))) {
            j = end();  // 未找到匹配项
        }
        return j;
	}

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	size_type M_count_tr(const Kt& k) const {
	    auto p = M_equal_range_tr(k);
	    return std::distance(p.first, p.second);
	}

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	iterator M_lower_bound_tr(const Kt& k) {
	    const Rb_tree* const_this = this;
	    return const_this->M_lower_bound_tr(k).M_const_cast();
    }

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	const_iterator M_lower_bound_tr(const Kt& k) const {
	    auto x = M_begin();
	    auto y = M_end();
	    while (x != 0){
	        if (!M_impl.M_key_compare(S_key(x), k)){
		        y = x;
		        x = S_left(x);
	        }
	        else{
	            x = S_right(x);
            }
        }
	    return const_iterator(y);
	}

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	iterator M_upper_bound_tr(const Kt& k) {
	    const Rb_tree* const_this = this;
	    return const_this->M_upper_bound_tr(k).M_const_cast();
	}

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	const_iterator M_upper_bound_tr(const Kt& k) const {
	    auto x = M_begin();
	    auto y = M_end();
	    while (x != 0){
	        if (M_impl.M_key_compare(k, S_key(x))){
		        y = x;
		        x = S_left(x);
	        }
            else{
	            x = S_right(x);
            }
        }
	    return const_iterator(y);
	}

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	std::pair<iterator, iterator> M_equal_range_tr(const Kt& k) {
	    const Rb_tree* const_this = this;
	    auto ret = const_this->M_equal_range_tr(k);
	    return { ret.first.M_const_cast(), ret.second.M_const_cast() };
	}

    template<typename Kt, typename Req = __has_is_transparent_t<Compare, Kt>>
	std::pair<const_iterator, const_iterator> M_equal_range_tr(const Kt& k) const {
	    auto low = M_lower_bound_tr(k);
	    auto high = low;
	    auto& cmp = M_impl.M_key_compare;
	    while (high != end() && !cmp(k, S_key(high.M_node))){
	        ++high;
        }
	    return { low, high };
	}

    Rb_tree& operator=(Rb_tree&& x) noexcept(Alloc_traits::_S_nothrow_move() && 
                                           is_nothrow_move_assignable<Compare>::value){
        M_impl.M_key_compare = std::move(x.M_impl.M_key_compare);
        M_move_assign(x, __bool_constant<Alloc_traits::_S_nothrow_move()>());
        return *this;
    }

    template<typename Iterator>
	void M_assign_unique(Iterator first, Iterator last){
        Reuse_or_alloc_node roan(*this);
        M_impl.M_reset();
        for (; first != last; ++first){
            M_insert_unique_(end(), *first, roan);
        }
    }

    template<typename Iterator> 
    void M_assign_equal(Iterator first, Iterator last){
        Reuse_or_alloc_node roan(*this);
        M_impl.M_reset();
        for (; first != last; ++first){
            M_insert_equal_(end(), *first, roan);
        }
    }

private:
    // 将元素从具有相同分配器的容器中移动
    void M_move_data(Rb_tree& x, true_type) { 
        M_impl.M_move_data(x.M_impl); 
    }

    // 将元素从可能具有非相等分配器的容器中移动，可能会复制而不是移动
    void M_move_data(Rb_tree& x, false_type an) {
        if (M_get_Node_allocator() == x.M_get_Node_allocator()){
            M_move_data(x, true_type());
        }
        else {
            constexpr bool move = !__move_if_noexcept_cond<value_type>::value;
            Alloc_node an(*this);
            M_root() = M_copy<move>(x, an);
            if constexpr (move){
                x.clear();
            }
        }
    }

    // 将分配从具有相同分配器的容器中移动
    void M_move_assign(Rb_tree& x, true_type) {
        clear();
        if (x.M_root() != nullptr){
            M_move_data(x, true_type());
        }
        std::__alloc_on_move(M_get_Node_allocator(), x.M_get_Node_allocator());
    }

    // 将元素从可能具有非相等分配器的容器中移动，可能会复制而不是移动
    void M_move_assign(Rb_tree& x, false_type) {
        if (M_get_Node_allocator() == x.M_get_Node_allocator()){
            return M_move_assign(x, true_type{});
        }

        // 尝试移动每个节点，重用现有节点并复制x个节点
        Reuse_or_alloc_node roan(*this);
        M_impl.M_reset();
        if (x.M_root() != nullptr) {
            M_root() =M_copy<as_rvalue>(x, roan);
            x.clear();
        }
    }

public:

    // 重新插入提取的节点
    insert_return_type M_reinsert_node_unique(node_type&& nh) {
	    insert_return_type ret;
	    if (nh.empty()){
	        ret.position = end();
        }
	    else {
	        __glibcxx_assert(M_get_Node_allocator() == *nh._M_alloc);
	        auto res = M_get_insert_unique_pos(nh._M_key());
            if (res.second) {
                ret.position = M_insert_node(res.first, res.second, nh._M_ptr);
                nh.release();
                ret.inserted = S_black;
            }
            else {
                ret.node = std::move(nh);
                ret.position = iterator(res.first);
                ret.inserted = S_red;
            }
	    }
	    return ret;
    }

    // 重新插入提取的节点
    iterator M_reinsert_node_equal(node_type&& nh) {
	    iterator ret;
	    if (nh.empty()){
	        ret = end();
        }
	    else {
	        __glibcxx_assert(M_get_Node_allocator() == *nh._M_alloc);
	        auto res = M_get_insert_equal_pos(nh._M_key());
	        if (res.second){
	            ret = M_insert_node(res.first, res.second, nh._M_ptr);
            }
            else{
	            ret = M_insert_equal_lower_node(nh._M_ptr);
	            nh.release();
            }
	    }
	    return ret;
    }

    // 重新插入提取的节点
    iterator M_reinsert_node_hint_unique(const_iterator hint, node_type&& nh) {
	    iterator ret;
	    if (nh.empty()){
	        ret = end();
        }
	    else {
	        __glibcxx_assert(M_get_Node_allocator() == *nh._M_alloc);
	        auto res = M_get_insert_hint_unique_pos(hint, nh._M_key());
	        if (res.second) {
		        ret = M_insert_node(res.first, res.second, nh._M_ptr);
		        nh.release();
	        }
	        else{
	            ret = iterator(res.first);
	        }
        }
	    return ret;
    }

    // 重新插入提取的节点
    iterator M_reinsert_node_hint_equal(const_iterator hint, node_type&& nh) {
	    iterator ret;
	    if (nh.empty()){
	        ret = end();
        }
	    else {
	        __glibcxx_assert(M_get_Node_allocator() == *nh._M_alloc);
	        auto res = _M_get_insert_hint_equal_pos(hint, nh._M_key());
	        if (res.second){
	            ret = M_insert_node(res.first, res.second, nh._M_ptr);
            }
            else{
	            ret = M_insert_equal_lower_node(nh._M_ptr);
            }
            nh.release();
	    }
	    return ret;
    }

    // 提取一个节点
    node_type extract(const_iterator pos) {
	    auto ptr = Rb_tree_rebalance_for_erase(pos.M_const_cast().M_node, M_impl.M_header);
	    return { static_cast<Link_type>(ptr), M_get_Node_allocator() };
    }

    // 提取一个节点
    node_type extract(const key_type& k) {
	    node_type nh;
	    auto pos = find(k);
	    if( pos != end()){
            nh = extract(const_iterator(pos));
        }
	    return nh;
    }

    template<typename Compare2>
	using Compatible_tree = Rb_tree<Key, Val, KeyOfValue, Compare2, Alloc>;

    template<typename, typename>
	friend struct Rb_tree_merge_helper;

    // 从兼容的容器中合并到具有等效键的容器中
    template<typename Compare2>
	void M_merge_unique(Compatible_tree<Compare2>& src) noexcept {
	    using Merge_helper = Rb_tree_merge_helper<Rb_tree, Compare2>;
	    for (auto i = src.begin(), end = src.end(); i != end;) {
	        auto pos = i++;
	        auto res = M_get_insert_unique_pos(KeyOfValue()(*pos));
	        if (res.second) {
		        auto& src_impl = Merge_helper::S_get_impl(src);
		        auto ptr = Rb_tree_rebalance_for_erase(
		        pos.M_node, src_impl.M_header);
		        --src_impl.M_node_count;
		        M_insert_node(res.first, res.second, static_cast<Link_type>(ptr));
		    }
	    }
	}

    // 从兼容的容器中合并到具有等效键的容器中
    template<typename Compare2>
	void M_merge_equal(Compatible_tree<Compare2>& src) noexcept {
	    using Merge_helper = Rb_tree_merge_helper<Rb_tree, Compare2>;
	    for (auto i = src.begin(), end = src.end(); i != end;) {
	        auto pos = i++;
	        auto res = M_get_insert_equal_pos(KeyOfValue()(*pos));
	        if (res.second) {
		        auto& src_impl = Merge_helper::S_get_impl(src);
		        auto ptr = Rb_tree_rebalance_for_erase(
		        pos.M_node, src_impl.M_header);
		        --src_impl.M_node_count;
		        M_insert_node(res.first, res.second, static_cast<Link_type>(ptr));
		    }
	    }
	}

    friend bool operator==(const Rb_tree& x, const Rb_tree& y) {
	    return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin());
    }

    friend bool operator<(const Rb_tree& x, const Rb_tree& y) {
	    return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }

private:

    // 封装红黑树节点的创建、插入和销毁操作
    struct Auto_node {
	    template<typename... Args>
	    Auto_node(Rb_tree& t, Args&&... args) : 
            M_t(t), M_node(t.M_create_node(std::forward<Args>(args)...)) {}

	    ~Auto_node() {
	    if (M_node)
	        M_t.M_drop_node(M_node);
	    }

	    Auto_node(Auto_node&& n) : M_t(n.M_t), M_node(n.M_node) { 
            n.M_node = nullptr; 
        }

	    const Key& _M_key() const { 
            return S_key(M_node); 
        }

	    iterator M_insert(std::pair<Base_ptr, Base_ptr> p) {
	        auto it = M_t.M_insert_node(p.first, p.second, M_node);
	        M_node = nullptr;
	        return it;
	    }

	    iterator M_insert_equal_lower() {
	        auto it = M_t.M_insert_equal_lower_node(M_node);
	        M_node = nullptr;
	        return it;
	    }

	    Rb_tree& M_t;
	    Link_type M_node;
    };
};

template<typename Key, typename Val, typename KeyOfValue, typename Compare, typename Alloc>
inline void swap(Rb_tree<Key, Val, KeyOfValue, Compare, Alloc>& x,  Rb_tree<Key, Val, KeyOfValue, Compare, Alloc>& y) { 
    x.swap(y);   // 调用成员函数交换数据
}

// 允许访问兼容的 Rb_tree 专门化的内部
template<typename Key, typename Val, typename Sel, typename Cmp1, typename Alloc, typename Cmp2>
struct Rb_tree_merge_helper<Rb_tree<Key, Val, Sel, Cmp1, Alloc>, Cmp2> {
private:
    friend class Rb_tree<Key, Val, Sel, Cmp1, Alloc>;

    static auto& S_get_impl(Rb_tree<Key, Val, Sel, Cmp2, Alloc>& tree) { 
        return tree.M_impl; 
    }
};

#endif //RB_TREE_H