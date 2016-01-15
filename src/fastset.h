#ifndef FASTSET_H
#define FASTSET_H

#include <memory>
#include <vector>

/// use for small 'set's with small T objects
template<class T, class Allocator = std::allocator<T> >
class fastset : public std::vector<T, Allocator>
{
public:
    typedef std::vector<T, Allocator> VectorT;
     typename VectorT::iterator find(const T& t){
        for(auto it = this->begin();it!=this->end();it++){
            if(t == *it)
                return it;
        }
        return this->end();
    }
    typename VectorT::const_iterator find(const T& t) const{
        for(auto it = this->begin();it!=this->end();it++){
            if(t == *it)
                return it;
        }
        return this->end();
    }
    bool contains(const T& t) const{
        return find(t) != this->end();
    }
};
#endif // FASTSET_H
