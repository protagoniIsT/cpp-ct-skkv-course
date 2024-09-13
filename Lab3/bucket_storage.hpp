#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

template<typename T>
class BucketStorage {
public:
    typedef T value_type;
    typedef T &reference;
    typedef const T &const_reference;
    typedef std::ptrdiff_t difference_type;
    typedef size_t size_type;
    static constexpr size_type INITIAL_IDENTIFIER = -1;

    explicit BucketStorage(size_type new_block_capacity = 64);
    BucketStorage(const BucketStorage &other);
    BucketStorage(BucketStorage &&other) noexcept;
    ~BucketStorage();

    BucketStorage &operator=(const BucketStorage &other);
    BucketStorage &operator=(BucketStorage &&other) noexcept;

    class iterator;
    class const_iterator;

    template<typename U>
    iterator insert(U &&value);

    iterator erase(iterator it);

    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type capacity() const noexcept;
    void shrink_to_fit() noexcept;
    void clear() noexcept;
    void swap(BucketStorage &other) noexcept;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;
    iterator get_to_distance(iterator it, difference_type distance);

private:
    void alloc_new_block();
    void dealloc_block(size_type block_idx);
    void update_prev_identifiers(difference_type block_idx, difference_type elem_idx);

    value_type **blocks;
    size_type block_capacity;
    size_type num_of_blocks;
    size_type curr_size;
    size_type **identifiers;

    template<bool isConst>
    class BaseIterator {
        friend class BucketStorage<T>;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::conditional<isConst, const T *, T *>::type;
        using reference = typename std::conditional<isConst, const T &, T &>::type;
        using storage_type = typename std::conditional<isConst, const BucketStorage, BucketStorage>::type;

        bool operator==(const BaseIterator &other) const noexcept { return curr_ptr == other.curr_ptr; }

        bool operator!=(const BaseIterator &other) const noexcept { return curr_ptr != other.curr_ptr; }

        template<bool OtherIsConst>
        bool operator==(const BaseIterator<OtherIsConst> &other) const noexcept {
            return curr_ptr == other.curr_ptr;
        }

        template<bool OtherIsConst>
        bool operator!=(const BaseIterator<OtherIsConst> &other) const noexcept {
            return curr_ptr != other.curr_ptr;
        }

        bool operator<(const BaseIterator &other) const noexcept {
            return block_idx < other.block_idx || (block_idx == other.block_idx && elem_idx < other.elem_idx);
        }

        bool operator<=(const BaseIterator &other) const noexcept { return *this < other || *this == other; }

        bool operator>(const BaseIterator &other) const noexcept {
            return block_idx > other.block_idx || (block_idx == other.block_idx && elem_idx > other.elem_idx);
        }

        bool operator>=(const BaseIterator &other) const noexcept { return *this > other || *this == other; }

        reference operator*() const { return *curr_ptr; }

        pointer operator->() const { return curr_ptr; }

        BaseIterator &operator=(const BaseIterator &other) {
            if (this != &other) {
                curr_ptr = other.curr_ptr;
                storage = other.storage;
                block_idx = other.block_idx;
                elem_idx = other.elem_idx;
            }
            return *this;
        }

        BaseIterator &operator++() {
            if constexpr (isConst) {
                if (*this == storage->cend()) {
                    return *this;
                }
            } else {
                if (*this == storage->end()) {
                    return *this;
                }
            }
            if (++elem_idx >= storage->block_capacity) {
                elem_idx = 0;
                ++block_idx;
            }
            while (block_idx < storage->num_of_blocks && storage->identifiers[block_idx][elem_idx] > 0) {
                size_type distance =
                        storage->identifiers[block_idx][elem_idx] == INITIAL_IDENTIFIER ? 1 : storage->identifiers[block_idx][elem_idx];
                for (size_type i = 0; i < distance; i++) {
                    ++elem_idx;
                    if (elem_idx >= storage->block_capacity) {
                        elem_idx = 0;
                        ++block_idx;
                    }
                }
            }
            if (block_idx >= storage->num_of_blocks) {
                curr_ptr = nullptr;
            } else {
                curr_ptr = &storage->blocks[block_idx][elem_idx];
            }
            return *this;
        }

        BaseIterator operator++(int) {
            BaseIterator temp = *this;
            ++(*this);
            return temp;
        }

        BaseIterator &operator--() {
            if constexpr (isConst) {
                if (*this == storage->cbegin()) {
                    return *this;
                }
            } else {
                if (*this == storage->begin()) {
                    return *this;
                }
            }
            if (curr_ptr == nullptr) {
                block_idx = storage->num_of_blocks - 1;
                elem_idx = storage->block_capacity - 1;
                curr_ptr = &storage->blocks[block_idx][elem_idx];
            } else {
                if (elem_idx == 0) {
                    if (block_idx == 0) {
                        curr_ptr = nullptr;
                        return *this;
                    }
                    --block_idx;
                    elem_idx = storage->block_capacity - 1;
                } else {
                    --elem_idx;
                }
            }

            while (block_idx >= (difference_type) 0 && storage->identifiers[block_idx][elem_idx] > 0) {
                size_type distance =
                        storage->identifiers[block_idx][elem_idx] == INITIAL_IDENTIFIER ? 1 : storage->identifiers[block_idx][elem_idx];
                for (size_type i = 0; i < distance; i++) {
                    if (elem_idx == 0) {
                        if (block_idx == 0) {
                            curr_ptr = nullptr;
                            return *this;
                        }
                        --block_idx;
                        elem_idx = storage->block_capacity - 1;
                    } else {
                        --elem_idx;
                    }
                }
            }

            curr_ptr = &storage->blocks[block_idx][elem_idx];
            return *this;
        }

        BaseIterator operator--(int) {
            BaseIterator temp = *this;
            --(*this);
            return temp;
        }

    protected:
        BaseIterator() : curr_ptr(nullptr), storage(nullptr), block_idx(0), elem_idx(0) {}

        BaseIterator(pointer ptr, storage_type *storage, difference_type block_idx, difference_type elem_idx)
                                             : curr_ptr(ptr), storage(storage), block_idx(block_idx), elem_idx(elem_idx) {
        }

        BaseIterator(const BaseIterator<isConst> &it)
                                             : curr_ptr(it.curr_ptr), storage(it.storage), block_idx(it.block_idx), elem_idx(it.elem_idx) {
        }
        pointer curr_ptr;
        storage_type *storage;
        difference_type block_idx;
        difference_type elem_idx;
    };

public:
    class iterator : public BaseIterator<false> {
    public:
        using BaseIterator<false>::BaseIterator;
        iterator(const BaseIterator<false> &base) : BaseIterator<false>(base) {}
    };

    class const_iterator : public BaseIterator<true> {
    public:
        using BaseIterator<true>::BaseIterator;
        const_iterator(const BaseIterator<true> &base) : BaseIterator<true>(base) {}
    };
};

template<typename T>
BucketStorage<T>::BucketStorage(size_type new_block_capacity)
                            : blocks(nullptr), block_capacity(new_block_capacity), num_of_blocks(0), curr_size(0), identifiers(nullptr) {
}

template<typename T>
BucketStorage<T>::BucketStorage(const BucketStorage &other)
                                 : block_capacity(other.block_capacity), num_of_blocks(other.num_of_blocks), curr_size(other.curr_size) {
    try {
        blocks = new T *[num_of_blocks];
        identifiers = new size_type *[num_of_blocks];
        for (size_type i = 0; i < num_of_blocks; ++i) {
            blocks[i] = static_cast<T *>(::operator new[](block_capacity * sizeof(T)));
            for (size_type j = 0; j < block_capacity; ++j) {
                if (other.identifiers[i][j] == 0) {
                    new (&blocks[i][j]) T(other.blocks[i][j]);
                }
            }
            identifiers[i] = new size_type[block_capacity];
            std::copy(other.identifiers[i], other.identifiers[i] + block_capacity, identifiers[i]);
        }
    } catch (...) {
        for (size_type i = 0; i < num_of_blocks; ++i) {
            for (size_type j = 0; j < block_capacity; ++j) {
                if (identifiers[i][j] == 0) {
                    blocks[i][j].~T();
                }
            }
            ::operator delete[](blocks[i]);
            delete[] identifiers[i];
        }

        throw;
    }
}

template<typename T>
BucketStorage<T>::BucketStorage(BucketStorage &&other) noexcept
                                             : blocks(nullptr), num_of_blocks(0), curr_size(0), block_capacity(0), identifiers(nullptr) {
    swap(other);
}

template<typename T>
BucketStorage<T>::~BucketStorage() {
    clear();
    delete[] blocks;
    delete[] identifiers;
}

template<typename T>
BucketStorage<T> &BucketStorage<T>::operator=(const BucketStorage &other) {
    if (this != &other) {
        BucketStorage temp(other);
        swap(temp);
    }
    return *this;
}

template<typename T>
BucketStorage<T> &BucketStorage<T>::operator=(BucketStorage &&other) noexcept {
    if (this != &other) {
        this->clear();
        swap(other);
    }
    return *this;
}

template<typename T>
template<typename U>
typename BucketStorage<T>::iterator BucketStorage<T>::insert(U &&value) {
    if (size() == capacity()) {
        alloc_new_block();
    }
    size_type insertion_block_idx;
    size_type insertion_elem_idx = 0;
    bool found_available = false;

    for (insertion_block_idx = 0; insertion_block_idx < num_of_blocks; insertion_block_idx++) {
        for (insertion_elem_idx = 0; insertion_elem_idx < block_capacity; insertion_elem_idx++) {
            if (identifiers[insertion_block_idx][insertion_elem_idx] != 0) {
                found_available = true;
                break;
            }
        }
        if (found_available) {
            break;
        }
    }

    new (&blocks[insertion_block_idx][insertion_elem_idx]) value_type(std::forward<U>(value));
    identifiers[insertion_block_idx][insertion_elem_idx] = 0;
    ++curr_size;

    update_prev_identifiers(insertion_block_idx, insertion_elem_idx);

    return iterator(&blocks[insertion_block_idx][insertion_elem_idx], this, insertion_block_idx, insertion_elem_idx);
}

template<typename T>
typename BucketStorage<T>::iterator BucketStorage<T>::erase(iterator it) {
    difference_type block_idx = it.block_idx;
    difference_type elem_idx = it.elem_idx;
    if (curr_size != 0 && identifiers[block_idx][elem_idx] == 0) {
        iterator it_copy = it;
        it_copy++;
        blocks[block_idx][elem_idx].~T();
        identifiers[block_idx][elem_idx] = INITIAL_IDENTIFIER;
        curr_size--;
        if (it_copy != it) {
            update_prev_identifiers(it_copy.block_idx, it_copy.elem_idx);
        }
        bool block_is_empty = true;
        for (size_type j = 0; j < block_capacity; j++) {
            if (identifiers[it.block_idx][j] == 0) {
                block_is_empty = false;
                break;
            }
        }
        if (block_is_empty) {
            dealloc_block(it.block_idx);
        }
        return it_copy;
    }
    return end();
}

template<typename T>
bool BucketStorage<T>::empty() const noexcept {
    return curr_size == 0;
}

template<typename T>
typename BucketStorage<T>::size_type BucketStorage<T>::size() const noexcept {
    return curr_size;
}

template<typename T>
typename BucketStorage<T>::size_type BucketStorage<T>::capacity() const noexcept {
    return block_capacity * num_of_blocks;
}

template<typename T>
void BucketStorage<T>::shrink_to_fit() noexcept {
    if (empty()) {
        return;
    }
    iterator it = begin();

    size_type new_block_idx = 0;
    size_type new_elem_idx = 0;
    for (size_type i = 0; i < num_of_blocks; i++) {
        for (size_type j = 0; j < block_capacity; j++) {
            if (identifiers[i][j] == 0) {
                if (new_block_idx != i || new_elem_idx != j) {
                    new (&blocks[new_block_idx][new_elem_idx]) T(std::move(blocks[i][j]));
                    blocks[i][j].~T();
                    identifiers[i][j] = INITIAL_IDENTIFIER;
                }
                identifiers[new_block_idx][new_elem_idx] = 0;
                if (++new_elem_idx >= block_capacity) {
                    new_elem_idx = 0;
                    ++new_block_idx;
                }
            }
        }
    }
    size_type new_num_of_blocks = std::ceil(static_cast<double>(curr_size) / block_capacity);
    for (size_type i = new_num_of_blocks; i < num_of_blocks; i++) {
        bool block_is_empty = true;
        for (size_type j = 0; j < block_capacity; j++) {
            if (identifiers[i][j] == 0) {
                block_is_empty = false;
                break;
            }
        }
        if (block_is_empty) {
            dealloc_block(i);
        }
    }
    num_of_blocks = new_num_of_blocks;
}

template<typename T>
void BucketStorage<T>::clear() noexcept {
    if (empty())
        return;

    for (size_type i = 0; i < num_of_blocks; ++i) {
        for (size_type j = 0; j < block_capacity; ++j) {
            if (identifiers[i][j] == 0) {
                blocks[i][j].~T();
            }
        }

        ::operator delete[](blocks[i]);
        delete[] identifiers[i];
    }
    delete[] blocks;
    delete[] identifiers;

    blocks = nullptr;
    identifiers = nullptr;
    num_of_blocks = 0;
    curr_size = 0;
}

template<typename T>
void BucketStorage<T>::swap(BucketStorage &other) noexcept {
    using std::swap;
    swap(blocks, other.blocks);
    swap(block_capacity, other.block_capacity);
    swap(num_of_blocks, other.num_of_blocks);
    swap(curr_size, other.curr_size);
    swap(identifiers, other.identifiers);
}

template<typename T>
void BucketStorage<T>::alloc_new_block() {
    size_type new_num_of_blocks = num_of_blocks + 1;
    T **new_blocks = nullptr;
    size_type **new_identifiers = nullptr;

    try {
        new_blocks = new T *[new_num_of_blocks];
        new_identifiers = new size_type *[new_num_of_blocks];
        for (size_type i = 0; i < num_of_blocks; i++) {
            new_blocks[i] = blocks[i];
            new_identifiers[i] = identifiers[i];
        }
        new_blocks[num_of_blocks] = static_cast<T *>(::operator new[](block_capacity * sizeof(T)));
        new_identifiers[num_of_blocks] = new size_type[block_capacity];
        std::fill_n(new_identifiers[num_of_blocks], block_capacity, INITIAL_IDENTIFIER);

        delete[] blocks;
        delete[] identifiers;

        blocks = new_blocks;
        identifiers = new_identifiers;
        num_of_blocks = new_num_of_blocks;

    } catch (...) {
        std::cerr << "Block memory allocation failed.";
        if (new_blocks) {
            for (size_type i = 0; i < new_num_of_blocks; i++) {
                if (new_blocks[i]) {
                    for (size_type j = 0; j < block_capacity; j++) {
                        new_blocks[i][j].~T();
                    }
                    ::operator delete[](new_blocks[i]);
                }
            }
            delete[] new_blocks;
        }

        if (new_identifiers) {
            for (size_type i = 0; i < new_num_of_blocks; i++) {
                if (new_identifiers[i]) {
                    delete[] new_identifiers[i];
                }
            }
            delete[] new_identifiers;
        }

        throw;
    }
}

template<typename T>
void BucketStorage<T>::dealloc_block(size_type block_idx) {
    if (block_idx >= num_of_blocks) {
        return;
    }
    for (size_type j = 0; j < block_capacity; j++) {
        if (identifiers[block_idx][j] == 0) {
            blocks[block_idx][j].~T();
        }
    }
    ::operator delete[](blocks[block_idx]);
    delete[] identifiers[block_idx];
    for (size_type i = block_idx; i < num_of_blocks - 1; ++i) {
        blocks[i] = blocks[i + 1];
        identifiers[i] = identifiers[i + 1];
    }
    num_of_blocks--;
}

template<typename T>
typename BucketStorage<T>::iterator BucketStorage<T>::begin() noexcept {
    if (num_of_blocks == 0 || empty()) {
        return end();
    }

    for (size_type block_idx = 0; block_idx < num_of_blocks; ++block_idx) {
        for (size_type elem_idx = 0; elem_idx < block_capacity; ++elem_idx) {
            if (identifiers[block_idx][elem_idx] == 0) {
                return iterator(&blocks[block_idx][elem_idx], this, block_idx, elem_idx);
            }
        }
    }
    return end();
}

template<typename T>
typename BucketStorage<T>::const_iterator BucketStorage<T>::begin() const noexcept {
    if (num_of_blocks == 0 || empty()) {
        return cend();
    }

    for (size_type block_idx = 0; block_idx < num_of_blocks; ++block_idx) {
        for (size_type elem_idx = 0; elem_idx < block_capacity; ++elem_idx) {
            if (identifiers[block_idx][elem_idx] == 0) {
                return const_iterator(&blocks[block_idx][elem_idx], this, block_idx, elem_idx);
            }
        }
    }
    return cend();
}

template<typename T>
typename BucketStorage<T>::const_iterator BucketStorage<T>::cbegin() const noexcept {
    if (num_of_blocks == 0 || empty()) {
        return cend();
    }
    for (size_type block_idx = 0; block_idx < num_of_blocks; ++block_idx) {
        for (size_type elem_idx = 0; elem_idx < block_capacity; ++elem_idx) {
            if (identifiers[block_idx][elem_idx] == 0) {
                return const_iterator(&blocks[block_idx][elem_idx], const_cast<BucketStorage<T> *>(this), block_idx, elem_idx);
            }
        }
    }
    return cend();
}

template<typename T>
typename BucketStorage<T>::iterator BucketStorage<T>::end() noexcept {
    if (num_of_blocks == 0 || empty()) {
        return iterator(nullptr, this, num_of_blocks, 0);
    }
    for (difference_type block_idx = num_of_blocks - 1; block_idx >= 0; --block_idx) {
        for (difference_type elem_idx = block_capacity - 1; elem_idx >= 0; --elem_idx) {
            if (identifiers[block_idx][elem_idx] == 0) {
                return iterator(nullptr, this, num_of_blocks, 0);
            }
        }
    }
    return iterator(nullptr, this, num_of_blocks, 0);
}

template<typename T>
typename BucketStorage<T>::const_iterator BucketStorage<T>::end() const noexcept {
    if (num_of_blocks == 0 || empty()) {
        return const_iterator(nullptr, this, num_of_blocks, 0);
    }
    for (difference_type block_idx = num_of_blocks - 1; block_idx >= 0; --block_idx) {
        for (difference_type elem_idx = block_capacity - 1; elem_idx >= 0; --elem_idx) {
            if (identifiers[block_idx][elem_idx] == 0) {
                return const_iterator(nullptr, this, num_of_blocks, 0);
            }
        }
    }
    return const_iterator(nullptr, this, num_of_blocks, 0);
}

template<typename T>
typename BucketStorage<T>::const_iterator BucketStorage<T>::cend() const noexcept {
    return const_iterator(nullptr, const_cast<BucketStorage<T> *>(this), num_of_blocks, 0);
}

template<typename T>
typename BucketStorage<T>::iterator BucketStorage<T>::get_to_distance(iterator it, difference_type distance) {
    iterator it_copy = it;
    if (distance >= 0) {
        for (difference_type i = 0; i < distance; i++) {
            it_copy++;
        }
    } else {
        for (difference_type i = 0; i < -distance; i++) {
            it_copy--;
        }
    }
    return it_copy;
}

template<typename T>
void BucketStorage<T>::update_prev_identifiers(difference_type block_idx, difference_type elem_idx) {
    difference_type temp_block_idx = block_idx;
    difference_type temp_elem_idx = elem_idx;
    if (!(block_idx == 0 && elem_idx == 0)) {
        temp_elem_idx--;
        if (temp_elem_idx < 0) {
            temp_block_idx--;
            temp_elem_idx = block_capacity - 1;
        }
        while (temp_block_idx >= 0 && identifiers[temp_block_idx][temp_elem_idx] != 0) {
            identifiers[temp_block_idx][temp_elem_idx] = (block_idx - temp_block_idx) * block_capacity + (elem_idx - temp_elem_idx);
            if (temp_elem_idx == 0) {
                temp_block_idx--;
                temp_elem_idx = block_capacity - 1;
            } else {
                temp_elem_idx--;
            }
        }
    }
}
