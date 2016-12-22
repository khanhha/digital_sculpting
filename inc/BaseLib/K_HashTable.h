/**************************************************************************************************
 * @file    D:\KS-2013\SRC\kstudio\inc\BaseLib\K_HashTable.h
 *
 * @brief   Declares the hash table class.
 * @author: Son
 * @copyright (c) 2014 Kevvox company
 * @history:
 *
 *  Date          | Author         | Note
 *  ---------------------------------------------------------------------------------------
 *  05/27/2014     Son               File created
 *
 **************************************************************************************************/

#ifndef __K_HASHTABLE_H__
#define __K_HASHTABLE_H__

#include "list"
#include "math.h"

const unsigned int DEFAULT_KHASHTBL_SIZE = 256;

template<typename T, unsigned int SIZE>
class HashFncImpl {
public:
    unsigned operator()(const T &val) const {
        std::hash<T> thash;
        unsigned hval = thash(val);
        return (hval % SIZE);
    }
};

template<typename T, unsigned SIZE>
class HashFncImpl<T *, SIZE>
{
public:
    unsigned operator()(const T *val) const{
        return (HashFncImpl<T, SIZE>()(*val));
    }
};

template<unsigned SIZE>
class HashFncImpl<Point3Dd, SIZE> {
public:
    unsigned operator()(const Point3Dd &p) {
        std::hash<double> dhash;
        int n = 5;
        unsigned hval = dhash(ROUND_FLOAT(p.x, n)) + 
                        dhash(ROUND_FLOAT(p.y, n)) + 
                        dhash(ROUND_FLOAT(p.z, n));

        return (hval % SIZE);
    }
};

template <typename T, unsigned int SIZE = DEFAULT_KHASHTBL_SIZE>
class KHashTable {
public:
    typedef std::list<T> ListT;

    KHashTable();
    ~KHashTable();
    unsigned hashFunc(const T &val);
    void addElement(const T &val);
    void removeElement(const T &val);
    T* findElement(const T &val);

    template<typename Functor>
    T* findElement(const T &val, Functor func);
    void clearAll() {
            std::for_each(_htbl, _htbl + SIZE, [&] (ListT& lt) { lt.clear(); });
    }
    unsigned numelement() { return _numElements; }
    friend class HashFncImpl<T, SIZE>;

private:
    ListT _htbl[SIZE];
    unsigned _numElements;
};

template<typename T, unsigned int SIZE>
KHashTable<T, SIZE>::KHashTable(): _numElements(0)
{
}

template<typename T, unsigned int SIZE>
KHashTable<T, SIZE>::~KHashTable()
{
    clearAll();
}

template<typename T, unsigned int SIZE>
unsigned KHashTable<T, SIZE>::hashFunc(const T&val)
{
    return (HashFncImpl<T, SIZE>()(val));
}

template<typename T, unsigned int SIZE>
void KHashTable<T, SIZE>::addElement(const T &val)
{
    unsigned hval = hashFunc(val);
    std::list<T> &lst = _htbl[hval];
    lst.push_back(val);
    _numElements++;
}

template<typename T, unsigned int SIZE>
void KHashTable<T, SIZE>::removeElement(const T &val)
{
    unsigned hval = hashFunc(val);
    std::list<T> &lst = _htbl[hval];
    lst.remove(val);
    _numElements--;
}

template<typename T, unsigned int SIZE>
T * KHashTable<T, SIZE>::findElement(const T &val)
{
    unsigned hval = hashFunc(val);
    std::list<T> &lst = _htbl[hval];
    std::list<T>::iterator it;
    it = std::find_if(lst.begin(), lst.end(), 
                                [&] (const T &t)
                                {
                                    return t == val;
                                });

    if (it != lst.end()) return &*it;

    return NULL;
}

template<typename T, unsigned int SIZE>
    template<typename Functor>
T * KHashTable<T, SIZE>::findElement(const T &val, Functor func)
{
    unsigned hval = hashFunc(val);
    std::list<T> &lst = _htbl[hval];
    std::list<T>::iterator it;
    for (it = lst.begin(); it != lst.end(); ++it)
        if (func(*it, val)) break;

    if (it != lst.end()) return &*it;

    return NULL;
}

//----
// partial specification for T*
//-----
template <typename T, unsigned int SIZE>
class KHashTable<T *, SIZE> {
public:
    typedef std::list<T* > ListT;

    KHashTable();
    ~KHashTable();
    void clearAll() {
            std::for_each(_htbl, _htbl + SIZE, [&] (ListT& lt) { lt.clear(); });
    }

    unsigned hashFunc(const T *val);
    void addElement(T *val);
    void removeElement(T *val);
    T* findElement(const T *val);
    template<typename Functor>
        T* findElement(const T *val, Functor func);
    unsigned numelelment() { return _numElements; }


    friend class HashFncImpl<T *, SIZE>;

private:
    ListT _htbl[SIZE];
    unsigned _numElements;
};

template<typename T, unsigned int SIZE>
KHashTable<T *, SIZE>::KHashTable(): _numElements(0)
{
}

template<typename T, unsigned int SIZE>
KHashTable<T *, SIZE>::~KHashTable()
{
    clearAll();
}

template<typename T, unsigned int SIZE>
unsigned KHashTable<T *, SIZE>::hashFunc(const T *val)
{
    return (HashFncImpl<T *, SIZE>()(val));
}

template<typename T, unsigned int SIZE>
void KHashTable<T *, SIZE>::addElement(T *val)
{
    unsigned hval = hashFunc(val);
    std::list<T *> &lst = _htbl[hval];
    lst.push_back(val);
    _numElements++;
}

template<typename T, unsigned int SIZE>
void KHashTable<T *, SIZE>::removeElement(T *val)
{
    unsigned hval = hashFunc(val);
    std::list<T *> &lst = _htbl[hval];
    lst.remove(val);
    _numElements--;
}

template<typename T, unsigned int SIZE>
T * KHashTable<T *, SIZE>::findElement(const T *val)
{
    unsigned hval = hashFunc(val);
    std::list<T *> &lst = _htbl[hval];
    std::list<T *>::iterator it;
    it = std::find_if(lst.begin(), lst.end(), 
        [&] (const T *t)
    {
        return *t == *val;
    });

    if (it != lst.end()) return *it;

    return NULL;
}

template<typename T, unsigned int SIZE>
    template<typename Functor>
T * KHashTable<T*, SIZE>::findElement(const T *val, Functor func)
{
    unsigned hval = hashFunc(val);
    std::list<T *> &lst = _htbl[hval];
    std::list<T *>::iterator it = lst.begin();
    for (;it != lst.end(); ++it) {
        if (func(val, *it)) break;
    }

    if (it != lst.end()) return *it;

    return NULL;
}
#endif // !__K_HASHTABLE_H__
