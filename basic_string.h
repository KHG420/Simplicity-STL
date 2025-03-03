#ifndef BASIC_STRING_H
#define BASIC_STRING_H

#include <bits/char_traits.h>
#include <unordered_set>
#include "vector.h"
#include "allocator.h"


template <class CharType, class Traits = char_traits<CharType>, class Alloc = Allocator<CharType>>
class Basic_string{

private:
    CharType* data_  = nullptr;
    size_t size_     = 0;
    size_t capacity_ = 0;
    Alloc allocator_;

    size_t strlen(const char *str) const {
        const char *p = str;
        while (*p != '\0') {
            p++;
        }
        return p - str;
    }

        // 扩容策略：双倍当前容量（至少为1
    void expand_capacity() {
        size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
        CharType* new_data = allocator_.allocate(new_capacity);
        
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] =  std::move(data_[i]);
            allocator_.destroy(data_ + i);
        }
        
        allocator_.deallocate(data_, capacity_);
        data_ = new_data;
        capacity_ = new_capacity;
    }

    // 通用构造辅助函数
    template <typename Input>
    void construct_from_range(Input first, Input last) {
        for(Input it = first; it != last; it++){
            ++size_;
            ++capacity_;
        }

        data_ = allocator_.allocate(capacity_);
        
        for (size_t i = 0; first != last; ++first, ++i) {
            allocator_.construct(data_ + i, *first);
        }
    }

    // 迭代器基类模板
    template <bool IsConst>
    class IteratorBase {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = CharType;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::conditional_t<IsConst, const CharType*, CharType*>;
        using reference         = std::conditional_t<IsConst, const CharType&, CharType&>;

        explicit IteratorBase(pointer ptr) : ptr_(ptr) {}

        reference       operator*() const { return *ptr_; }
        pointer         operator->() const { return ptr_; }
        IteratorBase&   operator++()    { ++ptr_; return *this; }
        IteratorBase    operator++(int) { auto tmp = *this; ++ptr_; return tmp; }
        IteratorBase&   operator--()    { --ptr_; return *this; }
        IteratorBase    operator--(int) { auto tmp = *this; --ptr_; return tmp; }
        IteratorBase    operator+(size_t n) const { return IteratorBase(ptr_ + n); }
        IteratorBase    operator-(size_t n) const { return IteratorBase(ptr_ - n); }
        difference_type operator-(const IteratorBase& other) const {return ptr_ - other.ptr_;}
        bool            operator==(const IteratorBase& other) const { return ptr_ == other.ptr_; }
        bool            operator!=(const IteratorBase& other) const { return !(*this == other); }

        pointer getData() const { return ptr_; }

    protected:
        pointer ptr_;
    };

public:

    // 迭代器类型定义
    using iterator               = IteratorBase<false>;
    using const_iterator         = IteratorBase<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator_type         = Alloc;
    using pointer                = CharType*;
    using const_pointer          = const CharType*;
    using reference              = CharType&;
    using const_reference        = const CharType&;
    using difference_type        = std::ptrdiff_t;
    using value_type             = CharType;
    using size_type              = size_t;

    static const size_type npos = -1;

    Basic_string() = default;

    explicit Basic_string(const Alloc& alloc) : allocator_(alloc) {}
    
    Basic_string(const Basic_string& other) : 
    size_(other.size_), capacity_(other.size_), allocator_(other.allocator_) {
        data_ = allocator_.allocate(capacity_);
        size_t i = 0;
        for(typename Basic_string<CharType>::const_iterator it = other.cbegin(); it != other.cend(); ++it){
            data_[i] = *it;
            i++;
        }
    }

    Basic_string(Basic_string&& other) noexcept : 
    data_(other.data_), size_(other.size_), capacity_(other.capacity_), allocator_(std::move(other.allocator_)) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }

    Basic_string(const Basic_string& right, size_type right_offset, size_type count = npos) 
        : allocator_(right.allocator_) { // 显式传递原分配器
        if (right_offset > right.size()) return;
        if (count == npos) {
            count = right.size() - right_offset;
        }
        size_ = count;
        capacity_ = count;
        data_ = allocator_.allocate(capacity_);
        for(size_type i = 0; i < count; i++) {
            data_[i] = right.data_[i + right_offset];
        }
    }

    Basic_string(const Basic_string& right, size_type right_offset, size_type count, const Alloc& alloc_type)
    : allocator_(alloc_type){
        if(right_offset > right.size()) return;
        if(count == npos){
            count = right.size() - right_offset;
        } 
        size_ = count;
        capacity_ = count;
        data_ = allocator_.allocate(capacity_);
        for(size_type i = 0; i < count; i++){
            data_[i] = right[i + right_offset];
        }
    }

    Basic_string(const value_type* ptr, size_type count) : size_(count), capacity_(count) {
        data_ = allocator_.allocate(capacity_);
        for(size_type i = 0; i < count; i++){
            data_[i] = ptr[i];
        }
    }

    Basic_string(const value_type* ptr, size_type count, const Alloc& alloc_type) :
    size_(count), capacity_(count), allocator_(alloc_type) {
        data_ = allocator_.allocate(capacity_);
        for(size_type i = 0; i < count; i++){
            data_[i] = ptr[i];
        }
    }

    Basic_string(size_type count, value_type char_value) : size_(count), capacity_(count) {
        data_ = allocator_.allocate(capacity_);
        for(size_t i = 0; i < count; i++){
            data_[i] = char_value;
        }
    }

    Basic_string(size_type count, value_type char_value, const Alloc& alloc_type): 
    size_(count), capacity_(count), allocator_(alloc_type){
        data_ = allocator_.allocate(capacity_);
        for(size_t i = 0; i < count; i++){
            data_[i] = char_value;
        }
    }

    template <typename Input>
    Basic_string(Input first, Input last, const Alloc& alloc = Alloc()) : allocator_(alloc) {
        construct_from_range(first, last);
    }

    Basic_string(const value_type* str, const Alloc& alloc = Alloc()) : allocator_(alloc) {
        construct_from_range(str, str + strlen(str));
    }

    ~Basic_string(){
        clear();
    }

    //对字符串的内容赋新的字符值
    Basic_string& operator=(value_type char_value){
        clear();
        push_back(char_value);
        return *this;
    }

    Basic_string& operator=(const value_type* ptr){
        clear();
        return append(ptr, strlen(ptr));
    }

    Basic_string& operator=(Basic_string& other){
        clear();
        size_ = other.size_; 
        capacity_ = other.size_;
        allocator_ = other.allocator_;
        data_ = allocator_.allocate(capacity_);
        size_t i = 0;
        for(typename Basic_string<CharType>::const_iterator it = other.cbegin(); it != other.cend(); ++it){
            data_[i] = *it;
            i++;
        }
        return *this;
    }

    Basic_string& operator=(Basic_string&& right){
        clear();
        data_ = right.data_;
        size_ = right.size_;
        capacity_ = right.capacity_;
        allocator_ = right.allocator_;
        right.data_ = nullptr;
        right.capacity_ = right.size_ = 0;
        return *this;
    }

    //向字符串追加字符
    Basic_string& operator+=(value_type char_value){
        return append(1, char_value);
    }

    Basic_string& operator+=(const value_type* ptr){
        return append(ptr);
    }

    Basic_string& operator+=(const Basic_string<CharType, Traits, Alloc>& right){
        return append(right);
    }

    // 元素访问
    CharType& operator[](size_t pos) { return data_[pos]; }
    const CharType& operator[](size_t pos) const { return data_[pos]; }

    //at访问
    CharType& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Basic_string::at: index out of range");
        }
        return data_[index];
    }

    const CharType& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Basic_string::at: index out of range");
        }
        return data_[index];
    }

    // 迭代器
    iterator begin() noexcept { 
        return iterator(data_); 
    }
    const_iterator cbegin() const noexcept { 
        return const_iterator(data_);
    }
    iterator end() noexcept { 
        return iterator(data_ + size_); 
    }
    const_iterator cend() const noexcept { 
        return const_iterator(data_ + size_); 
    }
    reverse_iterator rbegin() noexcept { 
        return reverse_iterator(end()); 
    }
    const_reverse_iterator crbegin() const noexcept { 
        return const_reverse_iterator(cend()); 
    }
    reverse_iterator rend() noexcept { 
        return reverse_iterator(begin()); 
    }
    const_reverse_iterator crend() const noexcept { 
        return const_reverse_iterator(cbegin()); 
    }

    reference back(){
        return *(data_ + size_ - 1);
    }

    const_reference back() const{
        return *(data_ + size_ - 1);
    }

    reference front(){
        return *data_;
    }

    const_reference front() const{
        return *data_;
    }

    void clear() noexcept {
        for (size_t i = 0; i < size_; ++i) {
            allocator_.destroy(data_ + i);
        }
        allocator_.deallocate(data_, capacity_);
        data_ = nullptr;
        size_ = capacity_ = 0;
    }

    size_t size() const { return size_; }

    size_t capacity() const { return capacity_; }

    // 容量操作
    void reserve(size_t new_capacity = 0) {
        if (new_capacity <= capacity_) return;
        
        CharType* new_data = allocator_.allocate(new_capacity);
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
            allocator_.destroy(data_ + i);
        }
        
        allocator_.deallocate(data_, capacity_);
        data_ = new_data;
        capacity_ = new_capacity;
    }

    //向字符串的末尾添加字符
    Basic_string<CharType, Traits, Alloc>& append(const value_type* ptr){
        return append(ptr, strlen(ptr));
    }

    Basic_string<CharType, Traits, Alloc>& append(const value_type* ptr, size_type count){
        if (size_ + count > capacity_) reserve(size_ + count);
        
        for (size_t i = 0; i < count; ++i) {
            data_[i + size_] = ptr[i];
        }
        size_ += count;
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& append(const Basic_string<CharType, Traits, Alloc>& str, 
    size_type offset, size_type count){
        count = count > str.size_ ? str.size_ : count;
        while(size_ + count > capacity_) { 
            expand_capacity();
        }
        for(size_type i = 0; i < count && i < str.size(); i++){
            data_[i + size_] = str[i + offset];
        }
        size_ += count;
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& append(const Basic_string<CharType, Traits, Alloc>& str){
        return this->append(str, 0, str.size());
    }

    Basic_string<CharType, Traits, Alloc>& append(size_type count, value_type char_value){
        while(size_ + count > capacity_) { 
            expand_capacity();
        }
        for(size_type i = 0; i < count; i++){
            data_[i + size_] = char_value;
        }
        size_ += count;
        return *this;
    }

    template <class InputIt>
    Basic_string<CharType, Traits, Alloc>& append(InputIt first, InputIt last){
        size_t len = last - first;
        while(size_ + len > capacity_) { 
            expand_capacity();
        }

        size_t i = 0;
        while(first != last){
            data_[i++ + size_] =  *first++;
        }
        size_ += len;
        return *this;
    }

    //对字符串的内容赋新的字符值
    Basic_string<CharType, Traits, Alloc>& assign(const value_type* ptr){
        clear();
        return append(ptr);
    }

    Basic_string<CharType, Traits, Alloc>& assign(const value_type* ptr, size_type count){
        clear();
        return append(ptr, count);
    }

    Basic_string<CharType, Traits, Alloc>& assign(const Basic_string<CharType, Traits, Alloc>& str,
    size_type off,
    size_type count){
        clear();
        return append(str.c_str(), off, count);
    }

    Basic_string<CharType, Traits, Alloc>& assign(const Basic_string<CharType, Traits, Alloc>& str){
        clear();
        return append(str.c_str());
    }

    Basic_string<CharType, Traits, Alloc>& assign(size_type count, value_type char_value){
        clear();
        return append(count, char_value);
    }

    template <class InIt>
    Basic_string<CharType, Traits, Alloc>& assign(InIt first, InIt last){
        clear();
        return append(first, last);
    }

    //将字符串的内容转换为以 null 结尾的 C 样式字符串
    const value_type* c_str() const {
        value_type* newData = allocator_.allocate(size_ + 1);

        for (size_t i = 0; i < size_; ++i) {
            newData[i] = data_[i];
        }

        newData[size_] = '\0';
        return newData;
    }

    //与指定字符串进行区分大小写的比较，以确定两个字符串是否相等或按字典顺序一个字符串是否小于另一个
    int compare(const Basic_string<CharType, Traits, Alloc>& str) const{
        return compare(0, size_, str, 0, str.size());
    }

    int compare(size_type position_1, size_type number_1,
    const Basic_string<CharType, Traits, Alloc>& str) const{
        return compare(position_1, number_1, str, 0, str.size());
    }

    int compare(size_type position_1,size_type number_1, const Basic_string<CharType, Traits, Alloc>& str,
    size_type position_2, size_type number_2) const{
        CharType* str_pointer = str.data_;
        size_t str_len = str.size();

        for(size_t i = 0; i + position_1 < size_ && i + position_2 < str_len && i < number_1 && i < number_2; i++, str_pointer++){
            if(*(data_ + i + position_1) > *(str_pointer + position_2)){
                return 1;
            }
            if(*(data_ + i + position_1) < *(str_pointer + position_2)){
                return -1;
            }
        }

        if((number_2 > str_len ? str_len : number_2) > (number_1 > size_ ? size_ : number_1)){
            return -1;
        }
        else if((number_2 > str_len ? str_len : number_2) < (number_1 > size_ ? size_ : number_1)){
            return 1;
        }
        else{
            return 0;
        }
    }

    int compare(const value_type* ptr) const{
        return compare(0, size_, ptr, strlen(ptr));
    }

    int compare(size_type position_1, size_type number_1, const value_type* ptr) const{
        return compare(position_1, number_1, ptr, strlen(ptr));
    }

    int compare(size_type position_1, size_type number_1, const value_type* str, size_type number_2) const{

        size_t str_len = strlen(str);
        for(size_t i = 0; i + position_1 < size_ && i < str_len && i < number_1 && i < number_2; i++){
            if(*(data_ + i + position_1) > str[i]){
                return 1;
            }
            if(*(data_ + i + position_1) < str[i]){
                return -1;
            }
        }

        if(number_2 > number_1){
            return -1;
        }
        else if(number_2 < number_1){
            return 1;
        }
        else{
            return 0;
        }
    }

    //测试字符串是否包含字符
    bool empty() const{
        return size_ == 0;
    }

    //检查字符串是否以指定的后缀结尾
    bool ends_with(const CharType c) const noexcept{
        return data_[size_ - 1] == c;
    }

    bool ends_with(const CharType* const x) const noexcept{
        size_t x_len = strlen(x);
        for(size_t i = 0; i < x_len; i++){
            if(data_[size_ - 1 - i] != x[x_len - 1 - i])
                return false;
        }
        return true;
    }

    bool ends_with(const Basic_string<CharType, Traits, Alloc>& x) const noexcept{
        size_t x_len = x.size();
        for(size_t i = 0; i < x_len; i++){
            if(data_[size_ - 1 - i] != x[x_len - 1 - i])
                return false;
        }
        return true;
    }

    //从字符串中的指定位置删除一个或一系列元素
    iterator erase(iterator first, iterator last){
        size_t begin_position = &(*first) - data_;
        size_t end_position = &(*last) - data_;
        size_t tail_datas_count = (data_ + size_) - &(*last);
        //销毁元素
        while(first != last){
            allocator_.destroy(&(*first++));
        }
        //搬运剩余元素
        for(size_t i = 0; i < tail_datas_count; i++){
            data_[i + begin_position] = std::move(data_[i + end_position]);
            allocator_.destroy(data_ + i + end_position);
        }

        size_ -= end_position - begin_position;
        return iterator(data_ + begin_position);
    }

    iterator erase(iterator iter){
        return erase(iter, iterator(data_ + size_));
    }

    Basic_string<CharType, Traits, Alloc>& erase(size_type offset = 0, size_type count = npos){
        if (offset >= size_) {
            throw std::out_of_range("Basic_string::at: index out of range");
        }

        if(count == npos || count + offset > size_)
            count = size_ - offset;
        //销毁元素
        for(size_t i = offset; i != offset + count; i++){
            allocator_.destroy(data_ + i);
        }
        //搬运剩余元素
        for(size_t i = 0; i < size_ - offset - count; i++){
            data_[i + offset] = std::move(data_[i + offset + count]);
            allocator_.destroy(data_ + i + offset + count);
        }

        size_ -= count;
        return *this;
    }

    //向前搜索字符串，搜索与指定字符序列匹配的第一个子字符串
    size_type find(value_type char_value, size_type offset = 0) const{
        for (size_t i = offset; i < size_; ++i) {
            if (data_[i] == char_value) {
                return i;
            }
        }
        return npos;
    }

    // 使用 KMP 算法查找子字符串的 find 函数
    size_t find(const CharType* str, size_t offset = 0) const {
        return find(str, offset, strlen(str));
    }

    size_type find(const value_type* str, size_type offset, size_type count) const{
        return find_impl(str, offset, count);
    }

    size_type find(const Basic_string<CharType, Traits, Alloc>& str, size_type offset = 0) const{
        return find(str.c_str(), offset);
    }

    //在字符串中搜索不属于指定字符串中元素的第一个字符
    size_type find_first_not_of(value_type char_value, size_type offset = 0) const{
        for (size_t i = offset; i < size_; ++i) {
            if (data_[i] != char_value) {
                return i;
            }
        }
        return npos;
    }

    size_type find_first_not_of(const value_type* ptr, size_type offset = 0) const{
        return find_first_not_of(ptr, offset, strlen(ptr)); 
    }

    size_type find_first_not_of(const value_type* ptr, size_type offset, size_type count) const{
        if (offset >= size_) {
            return npos;
        }
        std::unordered_set<CharType> charSet;
        size_t s_len = 0;
        while (ptr[s_len] != '\0' && count--) {
            charSet.insert(ptr[s_len]);
            ++s_len;
        }
        for (size_t i = offset; i < size_; ++i) {
            if (charSet.find(data_[i]) == charSet.end()) {
                return i;
            }
        }
        return npos;
    }

    size_type find_first_not_of(const Basic_string<CharType, Traits, Alloc>& str, size_type offset = 0) const{
        return find_first_not_of(str.c_str(), offset);
    }

    //在字符串中搜索与指定字符串中任何元素匹配的第一个字符
    size_type find_first_of(value_type char_value, size_type offset = 0) const{
        for (size_t i = offset; i < size_; ++i) {
            if (data_[i] == char_value) {
                return i;
            }
        }
        return npos;
    }

    size_type find_first_of(const value_type* ptr, size_type offset = 0) const{
        return find_first_of(ptr, offset, strlen(ptr));
    }

    size_type find_first_of(const value_type* ptr, size_type offset, size_type count) const{
        if (offset >= size_) {
            return npos;
        }
        std::unordered_set<CharType> charSet;
        size_t s_len = 0;
        while (ptr[s_len] != '\0' && count--) {
            charSet.insert(ptr[s_len]);
            ++s_len;
        }
        for (size_t i = offset; i < size_; ++i) {
            if (charSet.find(data_[i]) != charSet.end()) {
                return i;
            }
        }
        return npos;
    }

    size_type find_first_of(const Basic_string<CharType, Traits, Alloc>& str, size_type offset = 0) const{
        return find_first_of(str.c_str(), offset);
    }

    //在字符串中搜索不属于指定字符串中任何元素的最后一个字符
    size_type find_last_not_of(value_type char_value, size_type offset = npos) const{
        offset = offset == npos ? size_ - 1 : offset;
        for (size_t i = offset; i > -1; --i) {
            if (data_[i] == char_value) {
                return i;
            }
        }
        return npos;
    }

    size_type find_last_not_of(const value_type* ptr, size_type offset = npos) const{
        return find_last_not_of(ptr, offset, strlen(ptr));
    }

    size_type find_last_not_of(const value_type* ptr, size_type offset, size_type count) const {
        return find_last_of_base(ptr, offset, count, true);
    }

    size_type find_last_not_of(const Basic_string<CharType, Traits, Alloc>& str, size_type offset = npos) const{
        return find_last_not_of(str.c_str(), offset);
    }

    //在字符串中搜索与指定字符串中任何元素匹配的最后一个字符
    size_type find_last_of(value_type char_value, size_type offset = npos) const{
        offset = offset == npos ? size_ - 1 : offset;
        for (size_t i = offset; i > -1; --i) {
            if (data_[i] != char_value) {
                return i;
            }
        }
        return npos;
    }

    size_type find_last_of(const value_type* ptr, size_type offset = npos) const{
        return find_last_of(ptr, offset, strlen(ptr));
    }

    size_type find_last_of(const value_type* ptr, size_type offset, size_type count) const{
        return find_last_of_base(ptr, offset, count, false);
    }

    size_type find_last_of(const Basic_string<CharType, Traits, Alloc>& str, size_type offset = npos) const{
        return find_last_of(str.c_str(), offset);
    }

    //返回用于构造字符串的分配器对象的一个副本
    allocator_type get_allocator() const{
        return allocator_type();
    }

    //将一个、多个或一系列元素插入到指定位置的字符串中
    Basic_string<CharType, Traits, Alloc>& insert(size_type position, const value_type* ptr){
        return insert(position, ptr, strlen(ptr));;
    }

    Basic_string& insert(size_t pos, const CharType* str, size_t count) {
        if (pos > size_) return *this;
        
        reserve(size_ + count);
        
        // 移动现有元素
        for (size_t i = size_; i > pos; --i) {
            allocator_.construct(data_ + i + count - 1, std::move(data_[i - 1]));
            allocator_.destroy(data_ + i - 1);
        }
        
        // 插入新元素
        for (size_t i = 0; i < count; ++i) {
            allocator_.construct(data_ + pos + i, str[i]);
        }
        
        size_ += count;
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& insert(size_type position, const Basic_string<CharType, Traits, Alloc>& str){
        return insert(position, str.data_, str.size_);
    }

    Basic_string<CharType, Traits, Alloc>& insert(size_type position, const Basic_string<CharType, Traits, Alloc>& str,
    size_type offset, size_type count){
        if (offset > str.size_) return *this;
        count = (count == npos) ? str.size_ - offset : std::min(count, str.size_ - offset);
        return insert(position, str.data_ + offset, count);
    }

    Basic_string<CharType, Traits, Alloc>& insert(size_type position, size_type count, value_type char_value){
        if (position > size_) return *this;
        size_type required_size = size_ + count;

        // 扩容直到容量足够
        while (required_size > capacity_) {
            expand_capacity();
        }

        // 后移原数据
        for (size_type i = size_; i > position; --i) {
            data_[i + count - 1] = std::move(data_[i - 1]);
            allocator_.destroy(data_ + i - 1);
        }

        // 插入新字符
        for (size_type i = 0; i < count; ++i) {
            allocator_.construct(data_ + position + i, char_value);
        }

        size_ += count;
        return *this;
    }

    iterator insert(iterator iter, value_type char_value){
        size_type pos = iter.getData() - data_;
        insert(pos, 1, char_value);
        return iterator(data_ + pos);
    }

    template <class InputIterator>
    void insert(iterator iter, InputIterator first, InputIterator last){
        size_type pos = iter.getData() - data_;
        size_type count = last - first;
        insert(pos, count, value_type()); // 预扩容
        for (size_type i = 0; first != last; ++i, ++first) {
            data_[pos + i] = *first;
        }
    }

    void insert(iterator iter, size_type count, value_type char_value){
        size_type pos = iter.getData() - data_;
        insert(pos, count, char_value);
    }

    //返回字符串中元素的当前数目
    size_type length() const{ return size_; }

    void pop_back(){
        allocator_.destroy(data_ + size_ - 1);
        --size_;
    }

    void push_back(value_type char_value){
        if(size_ + 1 > capacity_) expand_capacity();
        data_[size_] = char_value;
        ++size_;
    }

    //用指定字符或者从其他范围、字符串或 C 字符串复制的字符来替代字符串中指定位置的元素
    Basic_string<CharType, Traits, Alloc>& replace(size_type position_1, size_type number_1, const value_type* ptr){
        erase(position_1, number_1);
        insert(position_1, ptr);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(size_type position_1, size_type number_1,
    const Basic_string<CharType, Traits, Alloc>& str){
        return replace(position_1, number_1, str, 0, str.size());
    }

    Basic_string<CharType, Traits, Alloc>& replace(size_type position_1, size_type number_1,
    const value_type* ptr, size_type number_2){
        erase(position_1, number_1);
        insert(position_1, ptr, 0, number_2);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(size_type position_1, size_type number_1,
    const Basic_string<CharType, Traits, Alloc>& str, size_type position_2, size_type number_2){
        erase(position_1, number_1);
        insert(position_1, str, position_2, number_2);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(size_type position_1, size_type number_1, 
    size_type count, value_type char_value){
        erase(position_1, number_1);
        insert(position_1, count, char_value);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(iterator first0, iterator last0, const value_type* ptr){
        erase(first0, last0);
        insert(first0 - iterator(data_), ptr);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(iterator first0, iterator last0,
    const Basic_string<CharType, Traits, Alloc>& str){
        erase(first0, last0);
        insert(first0 - iterator(data_), str);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(iterator first0, iterator last0,
    const value_type* ptr, size_type number_2){
        erase(first0, last0);
        insert(first0 - iterator(data_), ptr, 0, number_2);
        return *this;
    }

    Basic_string<CharType, Traits, Alloc>& replace(iterator first0, iterator last0,
    size_type count, value_type char_value){
        erase(first0, last0);
        insert(first0 - iterator(data_), count, char_value);
        return *this;
    }

    template <class InputIterator>
    Basic_string<CharType, Traits, Alloc>& replace(iterator first0, iterator last0,
    InputIterator first, InputIterator last){
        erase(first0, last0);
        insert(first0, first, last);
        return *this;
    }

    //根据要求追加或删除元素，为字符串指定新的大小
    void resize(size_type new_size){
        while(size_ > new_size){
            pop_back();
        }
        while(size_ < new_size){
            push_back(CharType());
        }
    }

    void resize(size_type new_size, value_type char_value){
        while(size_ > new_size){
            pop_back();
        }
        while(size_ < new_size){
            push_back(CharType(char_value));
        }
    }

    // 查找字符
    size_type rfind(value_type char_value, size_type offset = npos) const {
        if (offset == npos) {
            offset = length();
        }
        while(offset >= 0){
            if(data_[offset] == char_value)
                return offset;
            offset--;
        }
        return npos;
    }

    // 查找 C 风格字符串
    size_type rfind(const value_type* ptr, size_type offset = npos) const {
        return rfind(ptr, offset, strlen(ptr));
    }

    // 查找 C 风格字符串的前 count 个字符
    size_type rfind(const value_type* ptr, size_type offset, size_type count) const {
        if (offset == npos || offset > size_ - 1) {
            offset = length();
        }
        if (offset + 1 < count) {
            return npos;
        }
        for (size_type i = offset + 1 - count; i >= 0; --i) {
            bool found = true;
            for (size_type j = 0; j < count; ++j) {
                if (data_[i + j] != ptr[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return i;
            }
        }
        return npos;
    }

    // 查找另一个 basic_string
    size_type rfind(const Basic_string<CharType, Traits, Alloc>& str, size_type offset = npos) const {
        return rfind(str.c_str(), offset, str.size());
    }

    //放弃字符串的超出容量
    void shrink_to_fit(){
        if(size_ < capacity_){
            pointer new_data = allocator_.allocate(size_);
            for(size_t i = 0; i < size_; i++){
                allocator_.construct(new_data + i, move(data_[i]));
                allocator_.destroy(data_ + i);
            }
            allocator_.deallocate(data_, capacity_);
            data_ = new_data;
            capacity_ = size_;
        }
    }

    //检查字符串是否以指定的前缀开始
    bool starts_with(const CharType c) const noexcept{
        if(!size_) return false;
        return data_[0] == c;
    }

    bool starts_with(const CharType* const x) const noexcept{
        size_type x_len = strlen(x);
        if(size_ < x_len) return false;
        for(size_type i = 0; i < x_len; i++){
            if(data_[i] != x[i]){
                return false;
            }
        }
        return true;
    }

    //从字符串起始处的指定位置复制最多某个数目的字符的子字符串
    Basic_string<CharType, Traits, Alloc> substr(size_type offset = 0, size_type count = npos) const{
        return Basic_string<CharType, Traits, Alloc> (*this, offset, count);
    }

private:

    // KMP实现细节
    size_t find_impl(const CharType* pattern, size_t pos, size_t pattern_len) const {
        if (pattern_len == 0) return pos <= size_ ? pos : npos;
        
        Vector<size_t> lps(pattern_len);
        compute_lps(pattern, pattern_len, lps.data());
        
        size_t i = pos, j = 0;
        while (i < size_) {
            if (Traits::eq(data_[i], pattern[j])) {
                ++i; ++j;
                if (j == pattern_len) return i - j;
            } else if (j > 0) {
                j = lps[j-1];
            } else {
                ++i;
            }
        }
        return npos;
    }

    void compute_lps(const CharType* pattern, size_t len, size_t* lps) const {
        size_t prev = 0;
        lps[0] = 0;
        
        for (size_t i = 1; i < len; ) {
            if (Traits::eq(pattern[i], pattern[prev])) {
                lps[i++] = ++prev;
            } else if (prev != 0) {
                prev = lps[prev-1];
            } else {
                lps[i++] = 0;
            }
        }
    }

    //find_last_not_of和find_last_of函数
    size_type find_last_of_base(const value_type* ptr, size_type offset, size_type count, bool not_or_in) const{
        if (size_ == 0) return npos;

        if (offset >= size_) {
            offset = size_ - 1;
        }

        size_t ptr_len = strlen(ptr);
        std::unordered_set<CharType> charSet;

        size_t ptr_position = 0;
        while (ptr_position < ptr_len && count-- > 0) {
            charSet.insert(ptr[ptr_position]);
            ++ptr_position;
        }
        if(not_or_in){
            //find_last_not_of函数
            for (long long i = static_cast<long long>(offset); i >= 0; --i) {
                if (charSet.find(data_[i]) == charSet.end()) {
                    return static_cast<size_type>(i);
                }
            }
        }
        else{
            //find_last_of函数
            for (long long i = static_cast<long long>(offset); i >= 0; --i) {
                if (charSet.find(data_[i]) != charSet.end()) {
                    return static_cast<size_type>(i);
                }
            }
        }

        return npos;
    }

};

//重载<<
template <class CharType, class Traits, class Alloc>
std::ostream& operator<<(std::ostream& os, const Basic_string<CharType, Traits, Alloc>& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        os << str[i];
    }
    return os;
}


/// A string of @c char
using String = Basic_string<char>;

/// A string of @c wchar_t
using WString = Basic_string<wchar_t>;

#ifdef _GLIBCXX_USE_CHAR8_T
/// A string of @c char8_t
using U8String = Basic_string<char8_t>;
#endif

#if __cplusplus >= 201103L
/// A string of @c char16_t
using U16String = Basic_string<char16_t>;

/// A string of @c char32_t
using U32String = Basic_string<char32_t>;
#endif

#endif // BASIC_STRING_H