#ifndef POOL_H
#define POOL_H

#include <iostream>
#include <stddef.h>
#include <memory>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include "typedef.h"

template <class V, class P, class R>
class PoolIterator
{
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef V value_type;
    typedef R reference;
    typedef P pointer;
    typedef ptrdiff_t difference_type;

    PoolIterator(pointer other_ptr, size_t *other_roster) : ptr_(other_ptr), roster_ptr_(other_roster)
    {
    }
    reference operator*() const
    {
        return ptr_[*roster_ptr_];
    }
    pointer operator->()
    {
        return &ptr_[*roster_ptr_];
    }
    PoolIterator &operator++()
    {
        ++roster_ptr_;
        return (*this);
    }
    PoolIterator &operator--()
    {
        --roster_ptr_;
        return (*this);
    }
    PoolIterator operator++(s32)
    {
        PoolIterator tmp = *this;
        ++*this;
        return(tmp);
    }
    PoolIterator operator--(s32)
    {
        PoolIterator tmp = *this;
        --*this;
        return(tmp);
    }
    bool operator== (const PoolIterator &other) const
    {
        return (roster_ptr_ == other.roster_ptr_ && ptr_ == other.ptr_);
    }
    bool operator!= (const PoolIterator &other) const
    {
        return (roster_ptr_ != other.roster_ptr_ || ptr_ != other.ptr_);
    }
    size_t &roster_index()
    {
        return *roster_ptr_;
    }
private:
    pointer ptr_;
    size_t *roster_ptr_;
};

template <class T>
class Pool
{
public:
    typedef PoolIterator<T, T*, T&> iterator;
    typedef PoolIterator<T, const T*, const T&> const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;


	Pool() : size_(0), roster_(nullptr), partition_(nullptr), data_(nullptr)
	{

	}
	Pool(size_t s) : size_(0), roster_(nullptr), partition_(nullptr), data_(nullptr)
	{
        allocate(s);
	}
	Pool(Pool &other) : size_(other.size()), roster_(new size_t[other.size()]), partition_(other.partition_), data_(new T[other.size()]())
	{
		for (size_t i = 0; i < other.size(); i++)
		{
			roster_[i] = other.roster_[i];
			data_[i] = other.data_[i];
		}
	}
	void allocate(size_t s)
	{
	    free();

        size_ = s;
        roster_ = new size_t[s];
        partition_ = roster_;
        data_ = new T[s]();
        		for (size_t i = 0; i < size_; i++)
		{
			roster_[i] = i;
		}
	}
	~Pool()
	{
		free();
	}
	void free()
	{
	    if (roster_ != NULL)
            delete[] roster_;
        if (data_ != NULL)
            delete[] data_;
	}
	size_t size() const
	{
		return size_;
	}
	bool empty() const
	{
		return (size_ == 0);
	}
	bool active_size() const
	{
	    return partition_ - roster_;
	}
	T *init()
	{
		if (partition_ >= roster_ + size_)
			return nullptr;

		if (!std::is_sorted(partition_, &roster_[size_ - 1]))
		{
			std::sort(partition_, &roster_[size_ - 1]);
		}

		T *temp = &(data_[*partition_++]);

		return temp;
	}

	void deinit(size_t n)
	{
        size_t *temp_ptr = std::find(&roster_[0], partition_, n);
        if (temp_ptr == partition_)
            throw std::out_of_range("Pool::deinit"); // can't deallocate
        //data_[*temp_ptr].deinit();
        std::swap(*temp_ptr, *(--partition_));

        //debug_print();
	}
	void deinit(iterator it)
	{
        deinit(get_index(&(*it)));
	}
	void deinit(T *ptr)
	{
        deinit(get_index(ptr));
	}
	void deinit_all()
	{
	    //for (size_t *obj = roster_; obj != partition_; obj++)
            //data_[*obj].deinit();
        partition_ = roster_;
	}
	size_t get_index(T *ptr)
	{
	    bool found = false;
	    for (auto it = begin(); it != partition(); it++)
        {
            if (&(*it) == ptr)
                found = true;
        }
	    if (found == false)
            throw std::out_of_range("Pool::get_index");
        else return ptr - data_;
	}
	void clear()
	{
		//for (size_t *i = &roster_[0]; i < partition_; i++)
			//data_[*i].deinit();
		partition_ = roster_;

		//debug_print();
	}
	void debug_print()
	{
		for (size_t *i = &roster_[0]; i < partition_; i++)
		{
			std::cout << *i << ", ";
		}
		std::cout << "| ";
		for (size_t *j = partition_; j < &roster_[size_]; j++)
		{
			std::cout << *j<< ", ";
		}
		std::cout << std::endl;
	}
	T *get(size_t n)
	{
		size_t *temp_ptr = std::find(&roster_[0], partition_, n);
		if (temp_ptr == partition_)
			throw std::out_of_range("Pool::operator[]");
		return &data_[n];
	}

	const T *get(size_t n) const
	{
		return get(n);
	}
	bool has_room()
	{
	    return (partition() != end());
	}
	iterator begin()
    {
        return iterator(data_, roster_);
    }
    const_iterator begin() const
    {
        return const_iterator(data_, roster_);
    }
    const_iterator cbegin() const
    {
        return const_iterator(data_, roster_);
    }
    reverse_iterator rbegin()
    {
        return reverse_iterator(data_, &roster_[size_ - 1]);
    }
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(data_, &roster_[size_ - 1]);
    }
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(data_, &roster_[size_ - 1]);
    }

    iterator end()
    {
        return iterator(data_, &roster_[size_]);
    }
    const_iterator end() const
    {
        return const_iterator(data_, &roster_[size_]);
    }
    const_iterator cend() const
    {
        return const_iterator(data_, &roster_[size_]);
    }
    reverse_iterator rend()
    {
        return reverse_iterator(data_, &roster_[-1]);
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(data_, &roster_[-1]);
    }
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(data_, &roster_[-1]);
    }
    iterator partition()
    {
        return iterator(data_, partition_);
    }
    const_iterator partition() const
    {
        return const_iterator(data_, partition_);
    }
	protected:
	size_t size_;
	size_t *roster_, *partition_;
	T *data_;
};

/////////////////////////////////////////////////////////////////////////////
//INDEXPOOL
/////////////////////////////////////////////////////////////////////////////

template <class V, class P, class R>
class IndexPoolIterator
{
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef V value_type;
    typedef R reference;
    typedef P pointer;
    typedef ptrdiff_t difference_type;

    IndexPoolIterator(pointer other_ptr) : ptr_(other_ptr)
    {
    }
    reference operator*() const
    {
        return *ptr_;
    }
    pointer operator->()
    {
        return ptr_;
    }
    IndexPoolIterator &operator++()
    {
        ++ptr_;
        return (*this);
    }
    IndexPoolIterator &operator--()
    {
        --ptr_;
        return (*this);
    }
    IndexPoolIterator operator++(s32)
    {
        IndexPoolIterator tmp = *this;
        ++*this;
        return(tmp);
    }
    IndexPoolIterator operator--(s32)
    {
        IndexPoolIterator tmp = *this;
        --*this;
        return(tmp);
    }
    bool operator== (const IndexPoolIterator &other) const
    {
        return (ptr_ == other.ptr_);
    }
    bool operator!= (const IndexPoolIterator &other) const
    {
        return (ptr_ != other.ptr_);
    }
private:
    pointer ptr_;
};


class IndexPool
{
public:
    typedef IndexPoolIterator<memory_index, memory_index*, memory_index&> iterator;
    typedef IndexPoolIterator<memory_index, const memory_index*, const memory_index&> const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

	IndexPool() : size_(0), roster_(nullptr), partition_(nullptr)
	{

	}
	IndexPool(size_t s) : size_(0), roster_(nullptr), partition_(nullptr)
	{
        allocate(s);
	}
	IndexPool(IndexPool &other) : size_(other.size()), roster_(new size_t[other.size()]), partition_(other.partition_)
	{
		for (size_t i = 0; i < other.size(); i++)
		{
			roster_[i] = other.roster_[i];
		}
	}
	void allocate(size_t s)
	{
	    free();

        size_ = s;
        roster_ = new size_t[s];
        partition_ = roster_;
        for (size_t i = 0; i < size_; i++)
		{
			roster_[i] = i;
		}
	}
	~IndexPool()
	{
		free();
	}
	void free()
	{
	    if (roster_ != NULL)
            delete[] roster_;
	}
	size_t size() const
	{
		return size_;
	}
	bool empty() const
	{
		return (size_ == 0);
	}
	bool active_size() const
	{
	    return partition_ - roster_;
	}
	s32 init()
	{
		if (partition_ >= roster_ + size_)
			return -1;

		if (!std::is_sorted(partition_, &roster_[size_ - 1]))
		{
			std::sort(partition_, &roster_[size_ - 1]);
		}

		s32 temp = *partition_;
		partition_++;

		return temp;
	}

	void deinit(size_t n)
	{
        size_t *temp_ptr = std::find(&roster_[0], partition_, n);
        if (temp_ptr == partition_)
            throw std::out_of_range("Pool::deinit"); // can't deallocate
        std::swap(*temp_ptr, *(--partition_));

        //debug_print();
	}
	void deinit(iterator it)
	{
        deinit(*it);
	}
	void deinit(size_t *ptr)
	{
        deinit(ptr);
	}
	void deinit_all()
	{
        partition_ = roster_;
	}

	void clear()
	{
		partition_ = roster_;
	}
	void debug_print()
	{
		for (size_t *i = &roster_[0]; i < partition_; i++)
		{
			std::cout << *i << ", ";
		}
		std::cout << "| ";
		for (size_t *j = partition_; j < &roster_[size_]; j++)
		{
			std::cout << *j<< ", ";
		}
		std::cout << std::endl;
	}
	bool has_room()
	{
	    return (partition() != end());
	}
	iterator begin()
    {
        return iterator(roster_);
    }
    const_iterator begin() const
    {
        return const_iterator(roster_);
    }
    const_iterator cbegin() const
    {
        return const_iterator(roster_);
    }
    reverse_iterator rbegin()
    {
        return reverse_iterator(&roster_[size_ - 1]);
    }
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(&roster_[size_ - 1]);
    }
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(&roster_[size_ - 1]);
    }
    iterator end()
    {
        return iterator(&roster_[size_]);
    }
    const_iterator end() const
    {
        return const_iterator(&roster_[size_]);
    }
    const_iterator cend() const
    {
        return const_iterator(&roster_[size_]);
    }
    reverse_iterator rend()
    {
        return reverse_iterator(&roster_[-1]);
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(&roster_[-1]);
    }
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(&roster_[-1]);
    }
    iterator partition()
    {
        return iterator(partition_);
    }
    const_iterator partition() const
    {
        return const_iterator(partition_);
    }
	protected:
	size_t size_;
	size_t *roster_, *partition_;
};


#endif // POOL_H
