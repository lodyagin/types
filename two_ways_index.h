/**
 * @file 
 * Implements equivalence relation between sparse and
 * sequentional structure (e.g. allow sequentional access to
 * hashtable).
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (c) 2015 Asset Control International B.V.
 */

#ifndef TYPES_TWO_WAYS_INDEX_H
#define TYPES_TWO_WAYS_INDEX_H

#include "types/dynarray.h"
#include "types/traits.h"

namespace types
{

template<
    class Integer, 
    class IndexContainer, // must contain Size+1 elements
    size_t Size
>
struct two_ways_index
{
    typedef IndexContainer index_type;

    // should point to the last (unreachable) element
    constexpr Integer end_value = Size; 

    constexpr bool is_wait_free = is_wait_free<IndexContainer>::value;

    //! maps sequential numbers (from 0) to sparse index
    index_type seq2sparse;

    //! maps sparse indexes to sequential numbers (from 0);
    //! contains `no_value' in unused cells
    index_type sparse2seq;

    two_ways_index() noexcept : sparse2seq(Size+1, end_value)
    {
    }

    class sparse_iterator
    {
    public:
        typedef Integer value_type;
        typedef value_type size_type;
        typedef std::random_access_iterator_tag iterator_category;

        //! Constructs some invalid iterator.
        //! NB zero initialization just makes sure
        //! we will get SIGSEGV on uninitialized access.
        //! See how it used in e.g. wf_unordered_map::iterator
        sparse_iterator() noexcept : index(nullptr), seq_idx(0) 
        {
        }

        value_type operator*() const
        {
            return index->seq2sparse[seq_idx];
        }

        sparse_iterator& operator++()
        {
            ++seq_idx;
            return *this;
        }

        sparse_iterator operator++()
        {
            sparse_iterator copy = *this;
            ++(*this);
            return copy;
        }

        sparse_iterator& operator+=(size_type offs)
        {
            seq_idx += offs;
            return *this;
        }

        sparse_iterator operator+(size_type offs) const
        {
            sparse_iterator res = *this;
            return res += offs;
        }

        bool operator==(const sparse_iterator& o) const
        {
            return sparse_idx() == o.sparse_idx()
                && __builtin_expect(index == o.index, 1);
            //NB when return true seq_idx can be still not equal 
            // (i.e. for the end() iterator)
        }

        bool operator!=(const sparse_iterator& o) const
        {
            return !operator==(o);
        }

    protected:
        const two_ways_index* index;
        Integer seq_idx;

        sparse_iterator(two_ways_index& idx, Integer seq) 
            : index(idx), seq_idx(seq)
        {
            assert(si >= 0);
            assert(si <= Size); // == Size for the end iterator
        }

        Integer sparse_idx() const
        {
            return idx.seq2sparse[seq_idx];
        }
    };

    sparse_iterator sparse_begin() const
    {
        return sparse_iterator(*this, 0);
    }

    sparse_iterator sparse_end() const
    {
        return sparse_iterator(*this, end_value);
    }
};


} // types

#endif

