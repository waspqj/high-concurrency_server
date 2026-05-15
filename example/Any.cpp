#include <iostream>
#include <cassert>
#include <typeinfo>
#include <any>

class Any
{
    class Holder
    {
    public:
        Holder(){}
        virtual const std::type_info &Type() = 0;
        virtual Holder *Clone() = 0;
        virtual ~Holder(){}
    };
    template <typename T>
    class PlaceHolder : public Holder
    {
    public:
        T _val;
    public:
        PlaceHolder(T &val) : _val(val){}
        const std::type_info &Type() override
        {
            return typeid(T);
        }
        Holder *Clone() override
        {
            return new PlaceHolder(_val);
        }

        ~PlaceHolder() override = default;
    };
private:
    Holder *_holder;
public:
    Any() : _holder(nullptr){}
    template <class T>
    Any(T &&val) : _holder(new PlaceHolder<T>(val)){}
    Any(Any &other) : _holder(other._holder ? other._holder->Clone() : nullptr){}
    ~Any(){if(_holder) delete _holder;}
    Any &swap(Any &other)
    {
        std::swap(_holder, other._holder);
        return *this;
    }
    template <class T>
    T *get()
    {
        if (!_holder) return nullptr;
        assert(typeid(T) == _holder->Type());
        return &((PlaceHolder<T> *)_holder)->_val;
    }
    template<class T>
    Any &operator=(T &&val)
    {
        //构建临时Any对象，交换到有效数据后，临时any会带着要丢弃的数据销毁
        Any(std::forward<T>(val)).swap(*this);
        return *this;
    }
    Any &operator=(Any &other)
    {
        Any(other).swap(*this);
        return *this;
    }
};

class Test
{
public:
    Test()
    {std::cout << "create" << std::endl;}
    Test(const Test &)
    {std::cout << "copy" << std::endl;}
    ~Test()
    {std::cout << "destroy" << std::endl;}
};

int main()
{
    //c++17 提供的通用any
    std::any a; 
    a = Test();
    a = 1;
    std::cout << std::any_cast<int>(a) << std::endl;
    a = 1.1;
    std::cout << std::any_cast<double>(a) << std::endl;
    a = std::string("hello");
    std::cout << std::any_cast<std::string>(a) << std::endl;


    // Any a;
    // a = Test();
    
    // a = 1;
    // std::cout << *a.get<int>() << std::endl;
    // a = 1.1;
    // std::cout << *a.get<double>() << std::endl;
    // a = std::string("hello");
    // std::cout << *a.get<std::string>() << std::endl;
    return 0;
}