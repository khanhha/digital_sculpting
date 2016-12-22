#ifndef BASETYPE_KPAIR_H
#define BASETYPE_KPAIR_H

namespace BaseType{



    template<class T,class T1, class T2>
    class KPair3{
    public:
        KPair3():
            _value1(0),
            _value2(0)
        {

        }
        KPair3(T key, T1* val1, T2* val2):
            _key(key),
            _value1(val1),
            _value2(val2){

        }
        ~KPair3(){}

    public:
        T _key;
        T1* _value1;
        T2* _value2;
    };

    template<class T,class T1, class T2>
    struct KPairAscendingSort
    {

        bool operator()(KPair3<T,T1,T2> const* s_pair, KPair3<T,T1,T2> const* e_pair) const
        {
            if (s_pair->_key < e_pair->_key)
            {
                return true;
            }
            return false;
        }

    };

} // namespace
#endif