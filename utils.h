#ifndef UTILS
#define UTILS

#include <algorithm>

inline namespace utils
{

/**
    at_scope_exit
**/

template<class T>
struct at_scope_exit_t: public T
{
    at_scope_exit_t(T&& t): T(std::forward<T>(t))
    {}
    ~at_scope_exit_t(){ (*this)(); }
};

template<class T>
at_scope_exit_t<T> at_scope_exit(T&& t)
{
    return {std::forward<T>(t)};
}

#define AT_SCOPE_EXIT(F) auto at_scope_exit##__LINE__ = at_scope_exit( F );

/**
    <algorithm> extensions
**/

template<class iterator, class F>
void foreach_append_separated(std::string& str, iterator first, iterator last, F&& f, std::string const& separator)
{
    for(auto it = first; it != last;)
    {
        str += f(*it);

        if(++it != last)
            str += separator;
    }
}

///Extract

template<class iterator, class A, class B>
std::pair<iterator, iterator> extract(iterator itBegin, iterator itEnd, A const& a, B const& b)
{
    itBegin = std::find(itBegin, itEnd, a);

    iterator sub_itEnd = std::find(itBegin, itEnd, b);

    if(sub_itEnd != itEnd)
        ++sub_itEnd;

    return {itBegin, sub_itEnd};
}

template<class iterator, class A, class B>
std::pair<iterator, iterator> extract_if(iterator itBegin, iterator itEnd, A a, B b)
{
    itBegin = std::find_if(itBegin, itEnd, a);

    iterator sub_itEnd = std::find_if(itBegin, itEnd, b);

    if(sub_itEnd != itEnd)
        ++sub_itEnd;

    return {itBegin, sub_itEnd};
}

template<class iterator, class A, class B>
std::pair<iterator, iterator> extract_between(iterator itBegin, iterator itEnd, A const& a, B const& b)
{
    itBegin = std::find(itBegin, itEnd, a);

    if(itBegin != itEnd)
        ++itBegin;

    return {itBegin, std::find(itBegin, itEnd, b)};
}

template<class iterator, class A, class B>
std::pair<iterator, iterator> extract_between_if(iterator itBegin, iterator itEnd, A a, B b)
{
    itBegin = std::find_if(itBegin, itEnd, a);

    if(itBegin != itEnd)
        ++itBegin;

    return {itBegin, std::find_if(itBegin, itEnd, b)};
}


///implementations for std::isgraph (seems to be missing in libstdc++)

inline bool isgraph(int c)
{
    return !(c >= 0 && c < 33) && c != 127;
}

inline bool isnotgraph(int c)
{ return !isgraph(c); }

}

#endif // UTILS
