// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template<typename Key, typename Value, class Less>
listmap<Key, Value, Less>::node::node(node *next_, node *prev_,
                                      const value_type &value_):
        link(next_, prev_), value(value_) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template<typename Key, typename Value, class Less>
listmap<Key, Value, Less>::~listmap() {
    DEBUGF ('l', reinterpret_cast<const void *> (this));
}


//
// iterator listmap::insert (const value_type&)
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::insert(const value_type &pair) {
    DEBUGF ('l', &pair << "->" << pair);

    node *newNode;
    if (this->anchor_.next == this->anchor() and
    this->anchor_.prev == this->anchor()) {
        newNode = new node(anchor(), anchor(), pair);
        this->anchor_.next = newNode;
        this->anchor_.prev = newNode;
    } else {
        auto bgin = this->anchor_.next;
        while (bgin != this->anchor()){
            if (less(pair.first , bgin->value.first)){
                break;
            }

            bgin = bgin->next;
        }
        newNode = new node( bgin, bgin->prev,  pair);
        bgin->prev->next = newNode;
        bgin->prev = newNode;
    }
    return iterator(newNode);
}

//
// listmap::find(const key_type&)
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::find(const key_type &that) {
    DEBUGF ('l', that);
    node *newNode = nullptr;
    auto bgin = this->anchor_.next;
    while(bgin != this->anchor()){
        if(bgin->value.first == that){
            newNode = bgin;
            break;
        }
        bgin = bgin->next;
    } if (newNode == nullptr){
        return iterator(anchor());
    }
    return iterator(newNode);
}

//
// iterator listmap::erase (iterator position)
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator
listmap<Key, Value, Less>::erase(iterator position) {
    DEBUGF ('l', &*position);
 auto d = position.where;

 auto bgin = this->anchor_.next;
 while(bgin != this->anchor()){
     if (d->value.first == bgin->value.first){
         break;
     }
     bgin = bgin->next;
 }

 auto f = bgin->next;
 bgin->next->prev = bgin->prev;
 bgin->prev->next = bgin->next;
 bgin->next = nullptr;
 bgin->prev = nullptr;

 delete(bgin);
 return iterator(f);
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::value_type &
listmap<Key, Value, Less>::iterator::operator*() {
    DEBUGF ('l', where);
    return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::value_type *
listmap<Key, Value, Less>::iterator::operator->() {
    DEBUGF ('l', where);
    return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator &
listmap<Key, Value, Less>::iterator::operator++() {
    DEBUGF ('l', where);
    where = where->next;
    return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template<typename Key, typename Value, class Less>
typename listmap<Key, Value, Less>::iterator &
listmap<Key, Value, Less>::iterator::operator--() {
    DEBUGF ('l', where);
    where = where->prev;
    return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template<typename Key, typename Value, class Less>
inline bool listmap<Key, Value, Less>::iterator::operator==
        (const iterator &that) const {
    return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template<typename Key, typename Value, class Less>
inline bool listmap<Key, Value, Less>::iterator::operator!=
        (const iterator &that) const {
    return this->where != that.where;
}


