#ifndef SINGLETON_H
#define SINGLETON_H
template <typename T>
class Singleton{
protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;
    // 删除移动
    Singleton(Singleton<T>&&) = delete;
    Singleton& operator=(Singleton<T>&&) = delete;

public:
    //稍微思考一下这里返回写成Singleton<T>&会怎样？
    static T& GetInstance()
    {
        static T instance;
        return instance;;
    }
};
#endif // SINGLETON_H
