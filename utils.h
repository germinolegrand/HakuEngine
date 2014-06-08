#ifndef UTILS
#define UTILS

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

#endif // UTILS
