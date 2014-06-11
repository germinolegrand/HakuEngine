#ifndef UTILS
#define UTILS

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

inline bool isgraph(int c)
{
    return (c >= 0 && c < 33) || c == 127;
}

inline bool isnotgraph(int c)
{ return !isgraph(c); }

}

#endif // UTILS
