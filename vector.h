#ifndef VECTOR_H
#define VECTOR_H

#include"allocator.h"
#include<iostream>
#include <algorithm>
#include<initializer_list>
using namespace std;

template <typename T, typename Alloc = Allocator<T>>
class Vector{
private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
    Alloc allocator;

    //辅助函数  用于扩容
    void expand_capacity(size_t min_capacity = 0) {
        size_t new_capacity = std::max(capacity_ * 2, min_capacity);
        if(!new_capacity) new_capacity = 1;
        reserve(new_capacity);
    }

public:

    using allocator_type         = Alloc;
    using iterator               = T*;
    using const_iterator         = const T*;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type        = std::ptrdiff_t;
    using value_type             = T;
    using size_type              = size_t;

    //默认构造函数
    Vector() = default;

    explicit Vector(const Alloc& allocator) : data_(nullptr), size_(0), capacity_(0), allocator(allocator) {}

    //析构函数
    ~Vector(){
        for(size_t i = 0; i < size_; i++){
            allocator.destroy(data_ + i);
        }
        allocator.deallocate(data_, capacity_);
    }

    //构造函数  一个参数  大小
	Vector(size_t size) : Vector(size, T()) {}

	//构造函数  两个参数  大小和数值
	Vector(size_t size, const T& value) : size_(size), capacity_(size) {
        data_ = allocator.allocate(capacity_);
        try {
            for (size_t i = 0; i < size; ++i) {
                allocator.construct(data_ + i, value);
            }
        } catch (...) {
            // 若构造过程中抛出异常，释放已分配的内存
            for (size_t i = 0; i < size; ++i) {
                allocator.destroy(data_ + i);
            }
            allocator.deallocate(data_, capacity_);
            throw;
        }
    }

    Vector(size_t size, const T& value, const Alloc& allocator) : size_(size), capacity_(size), allocator(allocator) {
        data_ = this->allocator.allocate(capacity_);
        try {
            for (size_t i = 0; i < size; ++i) {
                this->allocator.construct(data_ + i, value);
            }
        } catch (...) {
            // 若构造过程中抛出异常，释放已分配的内存
            for (size_t i = 0; i < size; ++i) {
                this->allocator.destroy(data_ + i);
            }
            this->allocator.deallocate(data_, capacity_);
            throw;
        }
    }
 
	//构造函数  列表构造
	Vector(initializer_list<T> list) : Vector(list.begin(), list.end()) {}

    //拷贝构造函数
    Vector(const Vector& other) : size_(other.size_), capacity_(other.capacity_), allocator(other.allocator){
        data_ = allocator.allocate(capacity_);
        for (size_t i = 0; i < size_; ++i) {
            allocator.construct(data_ + i, other.data_[i]);
        }
    }

    // 移动构造函数
    Vector(Vector&& other) noexcept 
        : data_(other.data_), size_(other.size_), 
          capacity_(other.capacity_), allocator(std::move(other.allocator)) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }

    // 迭代器构造函数
    template <class InputIterator, typename = typename std::enable_if<!std::is_integral<InputIterator>::value>::type>
    Vector(InputIterator first, InputIterator last) : data_(nullptr), size_(0), capacity_(0) {
        try {
            while (first != last) {
                push_back(*first);
                ++first;
            }
        } catch (...) {
            // 若构造过程中抛出异常，释放已分配的内存
            for (size_t i = 0; i < size_; ++i) {
                allocator.destroy(data_ + i);
            }
            allocator.deallocate(data_, capacity_);
            throw;
        }
    }

    // 迭代器构造函数，带分配器
    template <class InputIterator, typename = typename std::enable_if<!std::is_integral<InputIterator>::value>::type>
    Vector(InputIterator first, InputIterator last, const Alloc& allocator) : data_(nullptr), size_(0), capacity_(0), allocator(allocator) {
        try {
            while (first != last) {
                push_back(*first);
                ++first;
            }
        } catch (...) {
            // 若构造过程中抛出异常，释放已分配的内存
            for (size_t i = 0; i < size_; ++i) {
                this->allocator.destroy(data_ + i);
            }
            this->allocator.deallocate(data_, capacity_);
            throw;
        }
    }

    // 拷贝赋值运算符
    Vector& operator=(const Vector& other) {
        if (this == &other) return *this;
        for (size_t i = 0; i < size_; ++i) {
            allocator.destroy(data_ + i);
        }
        allocator.deallocate(data_, size_);
        size_ = other.size_;
        capacity_ = other.capacity_;
        data_ = allocator.allocate(capacity_);
        for (size_t i = 0; i < size_; ++i) {
            allocator.construct(data_ + i, other.data_[i]);
        }
        return *this;
    }

    // 移动赋值运算符
    Vector& operator=(Vector&& other) {
        if (this == &other) return *this;
        for (size_t i = 0; i < size_; ++i) {
            allocator.destroy(data_ + i);
        }
        allocator.deallocate(data_, size_);
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        allocator = std::move(other.allocator);
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    iterator begin(){
        return data_;
    }

    const_iterator begin() const {
        return data_;
    }

    const_iterator cbegin() const {
        return data_;
    }

    iterator end(){
        return &(data_[size_]);
    }

    const_iterator end() const {
        return &(data_[size_]);
    }

    const_iterator cend() const {
        return &(data_[size_]);
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

    void push_back(const T& value) {
        if(size_ == capacity_){
            expand_capacity();
        }
        allocator.construct(data_ + size_, value);
        ++size_;
    }

    template <class... Types>
    void emplace_back(Types&&... args){
        expand_capacity(size_ + sizeof...(args));
        allocator.construct(data_ + size_, forward<Types>(args)...);
        size_++;
    }

    void pop_back() {
        if (size_ > 0) {
            allocator.destroy(data_ + size_ - 1);
            --size_;
        }
    }

    reference back(){
        return *(data_ + size_ - 1);
    }

    const_reference back() const{
        return *(data_ + size_ - 1);
    }
    
    //[]访问
    T& operator[](size_t index) {
        return data_[index];
    }

    const T& operator[](size_t index) const {
        return data_[index];
    }

    //at访问
    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("Vector::at");
        }
        // 添加返回语句
        return data_[pos];
    }

    // 获取指定位置的元素常量引用，进行边界检查
    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("Vector::at");
        }
        // 添加返回语句
        return data_[pos];
    }


    //返回指向向量中第一个元素的指针
    const_pointer data() const{
        return this->data_;
    }

    pointer data(){
        return this->data_;
    }

    // 获取元素数量
    size_t size() const {
        return size_;
    }

    // 获取容器容量
    size_t capacity() const {
        return capacity_;
    }

    // 判断容器是否为空
    bool empty() const {
        return size_ == 0;
    }

    // 清除容器
    void clear() {
        for (size_t i = 0; i < size_; ++i) {
            allocator.destroy(data_ + i);
        }
        size_ = 0;
    }

    reference front(){
        return *data_;
    }

    const_reference front() const{
        return *data_;
    }

    //从指定位置删除向量中的一个元素或一系列元素
    iterator erase(const_iterator position){
        size_t pos = position - begin();
        allocator.destroy(begin() + pos);
        for(size_t i = pos; i < size_; i++){
            allocator.construct(data_ + i, *(data_ + i + 1));
        }
        --size_;
        return data_ + pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        if (first == last) return iterator(first);
        
        size_t start_pos = first - cbegin();
        size_t end_pos = last - cbegin();
        size_t num_erase = end_pos - start_pos;
        
        // 批量移动元素
        std::move(begin() + end_pos, end(), begin() + start_pos);
        
        // 析构多余元素
        for (size_t i = size_ - num_erase; i < size_; ++i) {
            allocator.destroy(data_ + i);
        }
        size_ -= num_erase;
        
        return begin() + start_pos;
    }

    //清除矢量并将指定的元素复制到该空矢量
    void assign(size_type count, const T& value){
        clear();
        while(count--){
            push_back(value);
        }
    }

    void assign(initializer_list<value_type> list){
        clear();
        size_t i = 0;
		for (const T& element : list) {
			data_[i++] = element;
            size_++;
		}
    }

    template <class InputIterator, typename = typename std::enable_if<!std::is_integral<InputIterator>::value>::type>
    void assign(InputIterator first, InputIterator last){
        clear();
        while (first != last) {
            push_back(*first);
            ++first;
        }
    }

    //将就地构造的元素插入到指定位置的向量中
    template <class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        size_t offset = pos - cbegin();
        if (size_ == capacity_) {
            expand_capacity(size_ + sizeof...(args)); // 预扩容
        }
        
        // 批量移动元素（避免逐个移动）
        if (offset < size_) {
            std::move_backward(begin() + offset, end(), end() + 1);
        }
        
        allocator.construct(data_ + offset, std::forward<Args>(args)...);
        ++size_;
        return begin() + offset;
    }

    //返回用于构造矢量的分配器对象的一个副本
    allocator_type get_allocator() const{
        return allocator_type();
    }

    //insert   左值
    iterator insert(const_iterator position, const value_type& value){
        return emplace(position, value);
    }

    //右值
    iterator insert(const_iterator position, T&& value) {
        return emplace(position, value);
    }

    //多个相同值的元素
    iterator insert(const_iterator position, size_type count, const T& value) {
        size_t offset = position - cbegin();
        if (size_ + count > capacity) {
            expand_capacity(size_ + count);
        }

        // 移动元素为新元素腾出空间
        if (offset < size_) {
            std::move_backward(begin() + offset, end(), end() + count);
        }

        // 批量构造新元素
        for (size_t i = 0; i < count; ++i) {
            allocator.construct(data + offset + i, value);
        }

        size_ += count;
        return begin() + offset;
    }

    template <class InputIterator>
    void insert(const_iterator position, InputIterator first, InputIterator last) {
        // 计算插入元素的数量
        auto count = static_cast<size_t>(std::distance(first, last));
        if (count == 0) return;

        size_t pos = static_cast<size_t>(position - data_);

        size_t newSize = size_ + count;
        expand_capacity(newSize);

        std::move_backward(data_ + pos, data_ + size_, data_ + newSize);

        // 批量构造新元素
        std::uninitialized_copy(first, last, data_ + pos);

        // 更新容器大小
        size_ = newSize;
    }

    //为向量对象保留最小的存储长度，必要时为其分配空间
    void reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) return;
        
        T* new_data = allocator.allocate(new_capacity);
        for (size_t i = 0; i < size_; ++i) {
            allocator.construct(new_data + i, std::move(data_[i]));
            allocator.destroy(data_ + i);
        }
        allocator.deallocate(data_, capacity_);
        data_ = new_data;
        capacity_ = new_capacity;
    }

    // 为矢量指定新的大小
    void resize(size_type _Newsize) {
        if (_Newsize > capacity_) {  // 如果新大小超过当前容量，需要重新分配内存
            reserve(_Newsize);  // 直接扩容到新大小
        }
        if (size_ > _Newsize) {  // 如果当前大小大于新大小，销毁多余的元素
            for (size_t i = _Newsize; i < size_; ++i) {
                allocator.destroy(data_ + i);
            }
        } else if (size_ < _Newsize) {  // 如果当前大小小于新大小，构造新元素
            for (size_t i = size_; i < _Newsize; ++i) {
                allocator.construct(data_ + i, T());
            }
        }
        size_ = _Newsize;
    }

    void resize(size_type _Newsize, value_type value) {
        if (_Newsize > capacity_) {  // 如果新大小超过当前容量，需要重新分配内存
            reserve(_Newsize);  // 直接扩容到新大小
        }
        if (size_ > _Newsize) {  // 如果当前大小大于新大小，销毁多余的元素
            for (size_t i = _Newsize; i < size_; ++i) {
                allocator.destroy(data_ + i);
            }
        } else if (size_ < _Newsize) {  // 如果当前大小小于新大小，构造新元素
            for (size_t i = size_; i < _Newsize; ++i) {
                allocator.construct(data_ + i, value);
            }
        }
        size_ = _Newsize;  // 更新大小
    }

    //放弃额外容量
    void shrink_to_fit(){
        if(size_ < capacity_){
            pointer new_data = allocator.allocate(size_);
            for(size_t i = 0; i < size_; i++){
                allocator.construct(new_data + i, move(data_[i]));
                allocator.destroy(data_ + i);
            }
            allocator.deallocate(data_, capacity_);
            data_ = new_data;
            capacity_ = size_;
        }
    }

    // 添加 swap 成员函数
    void swap(Vector<T, Alloc>& other) {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

};

// 在 Vector 类外部定义非成员 swap 函数
template <typename T, typename Alloc>
void swap(Vector<T, Alloc>& left, Vector<T, Alloc>& right) {
    left.swap(right);
}


#endif // VECTOR_H