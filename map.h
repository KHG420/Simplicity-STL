
#ifndef MAP_H
#define MAP_H

#include <bits/functexcept.h>
#include <bits/concept_check.h>
#include <initializer_list>
#include <tuple>
#include "rb_tree.h"
#include "allocator.h"

template <typename Key, typename Value, typename Compare = less<Key>,
          typename Alloc = Allocator<pair<const Key, Value>>>
class Map{

private:
    using value_type      = std::pair<const Key, Value>;
    using Pair_alloc_type = typename __gnu_cxx::__alloc_traits<Alloc>::template rebind<value_type>::other;
    using Rep_type        = Rb_tree<Key, value_type, _Select1st<value_type>, Compare, Pair_alloc_type>;
    using Alloc_traits    = __gnu_cxx::__alloc_traits<Pair_alloc_type>;

    Rep_type M_t;

    template<typename _Up, typename _Vp = remove_reference_t<_Up>>
	static constexpr bool usable_key = __or_v<is_same<const _Vp, const Key>, 
                                       __and_<is_scalar<_Vp>, is_scalar<Key>>>;

public:

    class value_compare {
        friend class Map<Key, Value, Compare, Alloc>;
    protected:
        Compare comp;

        value_compare(Compare c) : comp(c) {}

    public:
        bool operator()(const value_type& x, const value_type& y) const { 
            return comp(x.first, y.first); 
        }
    };

    using pointer                = typename Alloc_traits::pointer;
    using const_pointer          = typename Alloc_traits::const_pointer;
    using reference              = typename Alloc_traits::reference;
    using const_reference        = typename Alloc_traits::const_reference;
    using iterator               = typename Rep_type::iterator;
    using const_iterator         = typename Rep_type::const_iterator;
    using size_type              = typename Rep_type::size_type;
    using difference_type        = typename Rep_type::difference_type;
    using reverse_iterator       = typename Rep_type::reverse_iterator;
    using const_reverse_iterator = typename Rep_type::const_reverse_iterator;
    using mapped_type            = Value;
    using key_compare            = Compare;
    using key_type               = Key;
    using allocator_type         = Alloc;

    Map() = default;

    explicit Map(const Compare& Comp, const allocator_type& a = allocator_type()) : M_t(Comp, Pair_alloc_type(a)) {}

    Map(const Map& Right) = default;

    Map(Map&& Right) = default;

    Map(initializer_list<value_type> IList, const Compare& Comp = Compare(), const allocator_type& a = allocator_type()) 
     : M_t(Comp, Pair_alloc_type(a)){
        M_t.M_insert_range_unique(IList.begin(), IList.end());
    }

    template <class InputIterator>
    Map(InputIterator First, InputIterator Last)  : M_t() {
        M_t.M_insert_range_unique(First, Last); 
    }

    template <class InputIterator>
    Map(InputIterator First, InputIterator Last, const Compare& Comp, const allocator_type& a = allocator_type())
     : M_t(Comp, Pair_alloc_type(a)) { 
        M_t.M_insert_range_unique(First, Last); 
    }

    mapped_type& at(const key_type& key){
        iterator i = lower_bound(key);
	    if (i == end() || key_comp()(key, (*i).first)){
	        __throw_out_of_range(__N("Map::at"));
        }
	    return (*i).second;
    }

    const mapped_type& at(const key_type& key) const{
        const_iterator i = lower_bound(key);
	    if (i == end() || key_comp()(key, (*i).first)){
	        __throw_out_of_range(__N("Map::at"));
        }
	    return (*i).second;
    }

    iterator begin() noexcept { 
        return M_t.begin(); 
    }

    const_iterator begin() const noexcept { 
        return M_t.begin(); 
    }

    iterator end() noexcept { 
        return M_t.end(); 
    }

    const_iterator end() const noexcept { 
        return M_t.end(); 
    }

    reverse_iterator rbegin() noexcept { 
        return M_t.rbegin(); 
    }

    const_reverse_iterator rbegin() const noexcept { 
        return M_t.rbegin(); 
    }

    reverse_iterator rend() noexcept { 
        return M_t.rend(); 
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

    void clear(){
        M_t.clear();
    }

    size_type count(const Key& key) const{
        return M_t.find(key) == M_t.end() ? 0 : 1;
    }

    template <class... Args>
    pair<iterator, bool> emplace(Args&&... args){
        if constexpr (sizeof...(Args) == 2){
	        if constexpr (is_same_v<allocator_type, allocator<value_type>>) {
		        auto&& [a, v] = pair<Args&...>(args...);
		        if constexpr (usable_key<decltype(a)>) {
		            const key_type& k = a;
		            iterator i = lower_bound(k);
		            if (i == end() || key_comp()(k, (*i).first)) {
			            i = emplace_hint(i, std::forward<Args>(args)...);
			            return {i, true};
		            }
		            return {i, false};
		        }
	        }
        }
	  return M_t._M_emplace_unique(std::forward<Args>(args)...);
    }

    template <class... Args>
    iterator emplace_hint( const_iterator where, Args&&... args){
        return M_t._M_emplace_hint_unique(where, std::forward<Args>(args)...);
    }

    bool empty() const noexcept{
        return M_t.empty();
    }

    pair <const_iterator, const_iterator> equal_range (const Key& key) const{
        return M_t.equal_range(key);
    }

    pair <iterator, iterator> equal_range (const Key& key){
        return M_t.equal_range(key);
    }

    iterator erase(const_iterator Where){
        return M_t.erase(Where);
    }

    iterator erase(const_iterator First, const_iterator Last){
        return M_t.erase(First, Last);
    }

    size_type erase(const key_type& key){
        return M_t.erase(key);
    }

    iterator find(const Key& key){
        return M_t.find(key);
    }

    const_iterator find(const Key& key) const{
        return M_t.find(key);
    }

    allocator_type get_allocator() const noexcept {
        return allocator_type(M_t.get_allocator());
    }

    // (1) single element
    std::pair<iterator, bool> insert(const value_type& Val) {
        return M_t._M_insert_unique(Val);
    }

    // (2) single element, perfect forwarded
    std::pair<iterator, bool> insert(value_type&& Val){
        return M_t._M_insert_unique(std::move(Val));
    }

    // (3) single element with hint
    iterator insert(const_iterator Where, const value_type& Val){
        return M_t._M_insert_unique_(Where, Val);
    }

    // (4) single element, perfect forwarded, with hint
    iterator insert(const_iterator Where, value_type&& Val){
        return M_t._M_insert_unique_(Where, std::move(Val));
    }

    // (5) range
    template <class InputIterator>
    void insert(InputIterator First, InputIterator Last){
        M_t.M_insert_range_unique(First, Last);
    }

    // (6) initializer list
    void insert(initializer_list<value_type> IList){
        insert(IList.begin(), IList.end());
    }

    key_compare key_comp() const {
        return M_t.key_comp();
    }

    iterator lower_bound(const Key& key) {
        return M_t.lower_bound(key);
    }

    template<typename Kt>
	auto lower_bound(const Kt& x) -> decltype(iterator(M_t.M_lower_bound_tr(x))) { 
        return iterator(M_t.M_lower_bound_tr(x)); 
    }

    const_iterator lower_bound(const Key& key) const {
        return M_t.lower_bound(key);
    }

    template<typename Kt>
	auto lower_bound(const Kt& x) const -> decltype(const_iterator(M_t.M_lower_bound_tr(x))) { 
        return const_iterator(M_t.M_lower_bound_tr(x)); 
    }

    size_type max_size() const {
        return M_t.max_size();
    }

    size_type size() const noexcept {
        return M_t.size();
    }

    void swap(Map& right) noexcept(__is_nothrow_swappable<Compare>::value) {
        M_t.swap(right.M_t);
    }

    iterator upper_bound(const Key& key){
        return M_t.upper_bound(key);
    }

    const_iterator upper_bound(const Key& key) const {
        return M_t.upper_bound(key);
    }

    value_compare value_comp() const {
        return value_compare(M_t.key_comp());
    }

    mapped_type& operator[](const Key& key){
        __glibcxx_function_requires(_DefaultConstructibleConcept<mapped_type>)

	    iterator i = lower_bound(key);
	    // i->first is greater than or equivalent to key.
	    if (i == end() || key_comp()(key, (*i).first))
	        i = M_t._M_emplace_hint_unique(i, std::piecewise_construct, std::tuple<const key_type&>(key),
					                        std::tuple<>());
	    return (*i).second;
    }

    mapped_type& operator[](Key&& key){
        // concept requirements
        __glibcxx_function_requires(_DefaultConstructibleConcept<mapped_type>)
        iterator i = lower_bound(key);
        // i->first is greater than or equivalent to key.
        if (i == end() || key_comp()(key, (*i).first))
            i = M_t._M_emplace_hint_unique(i, std::piecewise_construct, std::forward_as_tuple(std::move(key)),
                                            std::tuple<>());
        return (*i).second;
    }

    Map& operator=(const Map&) = default;

    Map& operator=(Map&&) = default;

    Map& operator=(initializer_list<value_type> l) {
	    M_t.M_assign_unique(l.begin(), l.end());
	    return *this;
    }



};

#endif // MAP_H