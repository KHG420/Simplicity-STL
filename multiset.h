#ifndef MULTISET_H
#define MULTISET_H

#include <bits/concept_check.h>
#include <initializer_list>
#include "rb_tree.h"
#include "set.h"

template <typename Key, typename Compare, typename Alloc>
class Set;

template <typename Key, typename Compare = std::less<Key>, typename Alloc = Allocator<Key>>
class Multiset{
    static_assert(is_same<typename remove_cv<Key>::type, Key>::value,
                  "std::Multiset must have a non-const, non-volatile value_type");

public:
    using key_type       = Key;
    using value_type     = Key;
    using key_compare    = Compare;
    using value_compare  = Compare;
    using allocator_type = Alloc;

private:
    using Key_alloc_type = typename __gnu_cxx::__alloc_traits<Alloc>::template rebind<Key>::other;
    using Rep_type = Rb_tree<key_type, value_type, _Identity<value_type>, key_compare, Key_alloc_type>;

    Rep_type M_t;

    using Alloc_traits = __gnu_cxx::__alloc_traits<Key_alloc_type>;

public:
    using pointer                = typename Alloc_traits::pointer;
    using const_pointer          = typename Alloc_traits::const_pointer;
    using reference              = typename Alloc_traits::reference;
    using const_reference        = typename Alloc_traits::const_reference;
    using iterator               = typename Rep_type::iterator;
    using const_iterator         = typename Rep_type::const_iterator;
    using reverse_iterator       = typename Rep_type::reverse_iterator;
    using const_reverse_iterator = typename Rep_type::const_reverse_iterator;
    using size_type              = typename Rep_type::size_type;
    using difference_type        = typename Rep_type::difference_type;
    using node_type              = typename Rep_type::node_type;

    Multiset() = default;

    explicit Multiset(const Compare &comp, const allocator_type &a = allocator_type()) : M_t(comp, Key_alloc_type(a)) {}

    template <typename InputIterator>
    Multiset(InputIterator first, InputIterator last) : M_t(){
        M_t.M_insert_range_equal(first, last);
    }

    template <typename InputIterator>
    Multiset(InputIterator first, InputIterator last, const Compare &comp,
             const allocator_type &a = allocator_type())
        : M_t(comp, Key_alloc_type(a)){
        M_t.M_insert_range_equal(first, last);
    }

    Multiset(const Multiset &) = default;

    Multiset(Multiset &&) = default;

    Multiset(initializer_list<value_type> l, const Compare &comp = Compare(),
             const allocator_type &a = allocator_type())
        : M_t(comp, Key_alloc_type(a)){
        M_t.M_insert_range_equal(l.begin(), l.end());
    }

    explicit Multiset(const allocator_type &a) : M_t(Key_alloc_type(a)) {}

    Multiset(const Multiset &m, const __type_identity_t<allocator_type> &a) : M_t(m.M_t, Key_alloc_type(a)) {}

    Multiset(Multiset &&m, const __type_identity_t<allocator_type> &a) noexcept(is_nothrow_copy_constructible<Compare>::value && Alloc_traits::_S_always_equal())
        : M_t(std::move(m.M_t), Key_alloc_type(a)) {}

    Multiset(initializer_list<value_type> l, const allocator_type &a) : M_t(Key_alloc_type(a)){
        M_t.M_insert_range_equal(l.begin(), l.end());
    }

    template <typename InputIterator>
    Multiset(InputIterator first, InputIterator last, const allocator_type &a) : M_t(Key_alloc_type(a)){
        M_t.M_insert_range_equal(first, last);
    }

    ~Multiset() = default;

    Multiset &operator=(const Multiset &) = default;

    Multiset &operator=(Multiset &&) = default;

    Multiset &operator=(initializer_list<value_type> l){
        M_t.M_assign_equal(l.begin(), l.end());
        return *this;
    }

    key_compare key_comp() const{
        return M_t.key_comp();
    }

    value_compare value_comp() const{
        return M_t.key_comp();
    }

    allocator_type get_allocator() const noexcept {
        return allocator_type(M_t.get_allocator());
    }

    const_iterator begin() const noexcept {
        return M_t.begin();
    }

    const_iterator end() const noexcept {
        return M_t.end();
    }

    const_reverse_iterator rbegin() const noexcept {
        return M_t.rbegin();
    }

    const_reverse_iterator rend() const noexcept {
        return M_t.rend();
    }

    const_iterator cbegin() const noexcept {
        return M_t.begin();
    }

    const_iterator cend() const noexcept {
        return M_t.end();
    }

    const_reverse_iterator crbegin() const noexcept {
        return M_t.rbegin();
    }

    const_reverse_iterator crend() const noexcept {
        return M_t.rend();
    }

    bool empty() const noexcept {
        return M_t.empty();
    }

    size_type size() const noexcept {
        return M_t.size();
    }

    size_type max_size() const noexcept {
        return M_t.max_size();
    }

    void swap(Multiset &x) noexcept(__is_nothrow_swappable<Compare>::value) {
        M_t.swap(x.M_t);
    }

    template <typename... Args> 
    iterator emplace(Args &&...args) {
        return M_t._M_emplace_equal(std::forward<Args>(args)...);
    }

    template <typename... Args> iterator
    emplace_hint(const_iterator pos, Args &&...args) {
        return M_t._M_emplace_hint_equal(pos, std::forward<Args>(args)...);
    }

    iterator insert(const value_type &x) {
        return M_t._M_insert_equal(x);
    }

    iterator insert(value_type &&x) {
        return M_t._M_insert_equal(std::move(x));
    }

    iterator insert(const_iterator position, const value_type &x) {
        return M_t._M_insert_equal_(position, x);
    }

    iterator insert(const_iterator position, value_type &&x) {
        return M_t._M_insert_equal_(position, std::move(x));
    }

    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        M_t.M_insert_range_equal(first, last);
    }

    void insert(initializer_list<value_type> l) {
        this->insert(l.begin(), l.end());
    }

    node_type extract(const_iterator pos) {
        return M_t.extract(pos);
    }

    node_type extract(const key_type &x) {
        return M_t.extract(x);
    }

    iterator insert(node_type &&nh) {
        return M_t.M_reinsert_node_equal(std::move(nh));
    }

    iterator insert(const_iterator hint, node_type &&nh) {
        return M_t._M_reinsert_node_hint_equal(hint, std::move(nh));
    }

    template <typename, typename>
    friend struct Rb_tree_merge_helper;

    template <typename Compare1>
    void merge(Multiset<Key, Compare1, Alloc> &source) {
        using Merge_helper = Rb_tree_merge_helper<Multiset, Compare1>;
        M_t.M_merge_equal(Merge_helper::S_get_tree(source));
    }

    template <typename Compare1>
    void merge(Multiset<Key, Compare1, Alloc> &&source) {
        merge(source);
    }

    template <typename Compare1>
    void merge(Set<Key, Compare1, Alloc> &source) {
        using Merge_helper = Rb_tree_merge_helper<Multiset, Compare1>;
        M_t.M_merge_equal(Merge_helper::S_get_tree(source));
    }

    template <typename Compare1>
    void merge(Set<Key, Compare1, Alloc> &&source) {
        merge(source);
    }

    iterator erase(const_iterator position) {
        return M_t.erase(position);
    }

    void erase(iterator position) {
        M_t.erase(position);
    }

    size_type erase(const key_type &x) {
        return M_t.erase(x);
    }

    iterator erase(const_iterator first, const_iterator last) {
        return M_t.erase(first, last);
    }

    void erase(iterator first, iterator last) {
        M_t.erase(first, last);
    }

    void clear() noexcept {
        M_t.clear();
    }

    size_type count(const key_type &x) const {
        return M_t.count(x);
    }

    template <typename Kt>
    auto count(const Kt &x) const -> decltype(M_t.M_count_tr(x)) {
        return M_t.M_count_tr(x);
    }

    iterator find(const key_type &x) {
        return M_t.find(x);
    }

    const_iterator find(const key_type &x) const {
        return M_t.find(x);
    }

    template <typename Kt>
    auto find(const Kt &x)->decltype(iterator{M_t.M_find_tr(x)}) {
        return iterator{M_t.M_find_tr(x)};
    }

    template <typename Kt>
    auto find(const Kt &x) const->decltype(const_iterator{M_t.M_find_tr(x)}) {
        return const_iterator{M_t.M_find_tr(x)};
    }

    iterator lower_bound(const key_type &x) {
        return M_t.lower_bound(x);
    }

    const_iterator lower_bound(const key_type &x) const {
        return M_t.lower_bound(x);
    }

    template <typename Kt>
    auto lower_bound(const Kt &x)->decltype(iterator(M_t.M_lower_bound_tr(x))) {
        return iterator(M_t.M_lower_bound_tr(x));
    }

    template <typename Kt>
    auto lower_bound(const Kt &x) const -> decltype(iterator(M_t.M_lower_bound_tr(x))) {
        return iterator(M_t.M_lower_bound_tr(x));
    }

    iterator upper_bound(const key_type &x) {
        return M_t.upper_bound(x);
    }

    const_iterator upper_bound(const key_type &x) const {
        return M_t.upper_bound(x);
    }

    template <typename Kt>
    auto upper_bound(const Kt &x) -> decltype(iterator(M_t.M_upper_bound_tr(x))) {
        return iterator(M_t.M_upper_bound_tr(x));
    }

    template <typename Kt>
    auto upper_bound(const Kt &x) const -> decltype(iterator(M_t.M_upper_bound_tr(x))) {
        return iterator(M_t.M_upper_bound_tr(x));
    }

    std::pair<iterator, iterator> equal_range(const key_type &x) {
        return M_t.equal_range(x);
    }

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type &x) const {
        return M_t.equal_range(x);
    }

    template <typename Kt>
    auto equal_range(const Kt &x) -> decltype(pair<iterator, iterator>(M_t.M_equal_range_tr(x))) {
        return pair<iterator, iterator>(M_t.M_equal_range_tr(x));
    }

    template <typename Kt>
    auto equal_range(const Kt &x) const -> decltype(pair<iterator, iterator>(M_t.M_equal_range_tr(x))) {
        return pair<iterator, iterator>(M_t.M_equal_range_tr(x));
    }

    template <typename K1, typename C1, typename A1>
    friend bool operator==(const Multiset<K1, C1, A1> &, const Multiset<K1, C1, A1> &);

    template <typename K1, typename C1, typename A1>
    friend bool operator<(const Multiset<K1, C1, A1> &, const Multiset<K1, C1, A1> &);
};

template <typename InputIterator, typename Compare = less<typename iterator_traits<InputIterator>::value_type>,
          typename Allocator = Allocator<typename iterator_traits<InputIterator>::value_type>,
          typename = _RequireInputIter<InputIterator>,
          typename = _RequireNotAllocator<Compare>, 
          typename = _RequireAllocator<Allocator>>
Multiset(InputIterator, InputIterator, Compare = Compare(), Allocator = Allocator())
        -> Multiset<typename iterator_traits<InputIterator>::value_type, Compare, Allocator>;

template <typename Key, typename Compare = less<Key>, typename Allocator = Allocator<Key>,
          typename = _RequireNotAllocator<Compare>, typename = _RequireAllocator<Allocator>>
Multiset(initializer_list<Key>, Compare = Compare(), Allocator = Allocator())
        -> Multiset<Key, Compare, Allocator>;

template <typename InputIterator, typename Allocator, typename = _RequireInputIter<InputIterator>,
          typename = _RequireAllocator<Allocator>>
Multiset(InputIterator, InputIterator, Allocator) -> Multiset<typename iterator_traits<InputIterator>::value_type,
                less<typename iterator_traits<InputIterator>::value_type>, Allocator>;

template <typename Key, typename Allocator, typename = _RequireAllocator<Allocator>>
Multiset(initializer_list<Key>, Allocator) -> Multiset<Key, less<Key>, Allocator>;

template <typename Key, typename Compare, typename Alloc>
inline bool operator==(const Multiset<Key, Compare, Alloc> &x, const Multiset<Key, Compare, Alloc> &y) {
    return x.M_t == y.M_t;
}

template <typename Key, typename Compare, typename Alloc>
inline bool operator<(const Multiset<Key, Compare, Alloc> &x, const Multiset<Key, Compare, Alloc> &y) {
    return x.M_t < y.M_t;
}

template <typename Key, typename Compare, typename Alloc>
inline bool operator!=(const Multiset<Key, Compare, Alloc> &x, const Multiset<Key, Compare, Alloc> &y) {
    return !(x == y);
}

template <typename Key, typename Compare, typename Alloc>
inline bool operator>(const Multiset<Key, Compare, Alloc> &x, const Multiset<Key, Compare, Alloc> &y) {
    return y < x;
}

template <typename Key, typename Compare, typename Alloc>
inline bool operator<=(const Multiset<Key, Compare, Alloc> &x, const Multiset<Key, Compare, Alloc> &y) {
    return !(y < x);
}

template <typename Key, typename Compare, typename Alloc>
inline bool operator>=(const Multiset<Key, Compare, Alloc> &x, const Multiset<Key, Compare, Alloc> &y) {
    return !(x < y);
}

template <typename Key, typename Compare, typename Alloc>
inline void swap(Multiset<Key, Compare, Alloc> &x, Multiset<Key, Compare, Alloc> &y) noexcept(noexcept(x.swap(y))) {
    x.swap(y);
}

template <typename Val, typename Cmp1, typename Alloc, typename Cmp2>
struct Rb_tree_merge_helper<Multiset<Val, Cmp1, Alloc>, Cmp2> {
private:
    friend class Multiset<Val, Cmp1, Alloc>;

    static auto& S_get_tree(Set<Val, Cmp2, Alloc> &set) {
        return set.M_t;
    }

    static auto & S_get_tree(Multiset<Val, Cmp2, Alloc> &set) {
        return set.M_t;
    }
};

#endif //MULTISET_H
