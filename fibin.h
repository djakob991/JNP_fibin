#ifndef __FIBIN_H__
#define __FIBIN_H__

// JNP1 (II rok informatyki MIMUW) : zadanie nr 4
// Autorzy: Jakub Organa, Łukasz Kamiński

#include <iostream>
#include <type_traits>
#include <typeinfo>

// Struktury stanowiące składnię języka:

template<uint64_t N>
struct Fib {};

struct True {};
struct False {};

template<typename T>
struct Lit {};

template<typename Arg1, typename Arg2, typename... Args>
struct Sum {};

template<typename Arg>
struct Inc1 {};

template<typename Arg>
struct Inc10 {};

template<typename Left, typename Right>
struct Eq {};

template<typename Condition, typename Then, typename Else>
struct If {};

template<uint64_t name, typename Value, typename Expr>
struct Let {};

template<uint64_t name>
struct Ref {};

template<uint64_t name, typename Body>
struct Lambda {};

template<typename Fun, typename Param>
struct Invoke {};


// "Var" zwraca hash argumentu. "terminate_wrong_arg", w przypadku błędnego argumentu
// zostanie wywołana i przerwie kompilację.

void terminate_wrong_arg() {}

constexpr uint64_t Var(const char* identifier) {
    uint64_t p = 313;
    uint64_t name = 0;
    size_t counter = 0;

    for (size_t i = 0; identifier[i] != '\0'; i++) {
        counter++;
        char c = identifier[i];

        if (!(('A' <= c and c <= 'Z') or ('0' <= c and c <= '9') or ('a' <= c and c <= 'z'))) {
            terminate_wrong_arg();
        }

        if (c >= 'a') {
            c = c - ('a' - 'A'); 
        }

        name += c * p;
        p *= p;
    }

    if (counter == 0 or counter > 6) {
        terminate_wrong_arg();
    }

    return name;
}

namespace details {
    
    // CalcFib oblicza N-tą liczbę Fibonacciego, do obliczeń używa typu VT
    template<typename VT, uint64_t N>
    struct CalcFib {
        constexpr static VT value = CalcFib<VT, N-1>::value + CalcFib<VT, N-2>::value;
    };

    template<typename VT>
    struct CalcFib<VT, 1> {
        constexpr static VT value = 1;
    };

    template<typename VT>
    struct CalcFib<VT, 0> {
        constexpr static VT value = 0;
    };

    // Oznacza koniec listy
    struct Nil {};

    // name - hash zmiennej, Value - zewaluowana wartość przypisana danej zmiennej, Tail - ogon
    template<uint64_t name, typename Value, typename Tail>
    struct VariableList {};

    // Znajduje w liście Value przypisane do name 
    template<uint64_t name, typename List>
    struct findInList {};

    template<uint64_t name, typename Value, typename Tail>
    struct findInList<name, VariableList<name, Value, Tail>> {
        using result = Value;
    };

    template<uint64_t name1, uint64_t name2, typename Value, typename Tail>
    struct findInList<name1, VariableList<name2, Value, Tail>> {
        using result = typename findInList<name1, Tail>::result;
    };

    // Zewaluowana funkcja-lambda. List to lista argumentów, która obowiązywała w momencie definicji funkcji.
    template<typename Lbd, typename List>
    struct LambdaClosure {};

    // Wartość zewaluowana wyrażenia logicznego (dla porządku i konsekwencji)
    template<typename Bool>
    struct BoolClosure {};

    // Zewaluowana wartość liczbowa (używa typu liczbowego VT)
    template<typename VT, VT N>
    struct Int {
        constexpr static VT value = N;
    };

    // Suma
    template<typename VT, typename I1, typename I2>
    struct sumOfInts {
        constexpr static VT new_value = I1::value + I2::value;
        using result = Int<VT, new_value>;
    };

    // Czy równe
    template<typename I1, typename I2>
    struct equalInts {};

    template<typename VT, VT v>
    struct equalInts<Int<VT, v>, Int<VT, v>> {
        using result = BoolClosure<True>;
    };

    template<typename VT, VT v1, VT v2>
    struct equalInts<Int<VT, v1>, Int<VT, v2>> {
        using result = BoolClosure<False>;
    };

    // Eval<T, List, Expr> - ewaluuje wyrażenie expr według wartości zmiennych z listy List, używając do obliczeń typu T
    template<typename VT, typename List, typename Expr>
    struct Eval {};

    // Eval dzięki następującym specjalizacjom oblicza wartość wyrażenia, dzieląc rekurencyjnie wyrażenie na mniejsze składowe,
    // obliczając je, a potem łącząc w całość.

    template<typename VT, typename List, uint64_t N>
    struct Eval<VT, List, Lit<Fib<N>>> {
        constexpr static VT new_value = CalcFib<VT, N>::value;
        using result = Int<VT, new_value>;
    };

    template<typename VT, typename List>
    struct Eval<VT, List, Lit<True>> {
        using result = BoolClosure<True>;
    };

    template<typename VT, typename List>
    struct Eval<VT, List, Lit<False>> {
        using result = BoolClosure<False>;
    };

    template<typename VT, typename List, typename Arg1, typename Arg2, typename... Args>
    struct Eval<VT, List, Sum<Arg1, Arg2, Args...>> {
        using result =
            typename sumOfInts<
                VT,
                typename Eval<VT, List, Arg1>::result,
                typename Eval<VT, List, Sum<Arg2, Args...>>::result
            >::result;
    };

    template<typename VT, typename List, typename Arg1, typename Arg2>
    struct Eval<VT, List, Sum<Arg1, Arg2>> {
        using result =
            typename sumOfInts<
                VT,
                typename Eval<VT, List, Arg1>::result,
                typename Eval<VT, List, Arg2>::result
            >::result;
    };

    template<typename VT, typename List, typename Arg>
    struct Eval<VT, List, Inc1<Arg>> {
        using result =
            typename sumOfInts<
                VT,
                typename Eval<VT, List, Arg>::result,
                Int<VT, CalcFib<VT, 1>::value>
            >::result;
    };

    template<typename VT, typename List, typename Arg>
    struct Eval<VT, List, Inc10<Arg>> {
        using result =
            typename sumOfInts<
                VT,
                typename Eval<VT, List, Arg>::result,
                Int<VT, CalcFib<VT, 10>::value>
            >::result;
    };

    template<typename VT, typename List, typename Left, typename Right>
    struct Eval<VT, List, Eq<Left, Right>> {
        using result =
            typename equalInts<
                typename Eval<VT, List, Left>::result,
                typename Eval<VT, List, Right>::result
            >::result;
    };

    template<typename VT, typename List, typename Cond, typename Then, typename Else>
    struct Eval<VT, List, If<Cond, Then, Else>> {
        using result =
            typename Eval<
                VT, List,
                If<typename Eval<VT, List, Cond>::result, Then, Else>
            >::result;
    };

    template<typename VT, typename List, typename Then, typename Else>
    struct Eval<VT, List, If<BoolClosure<True>, Then, Else>> {
        using result = typename Eval<VT, List, Then>::result;
    };

    template<typename VT, typename List, typename Then, typename Else>
    struct Eval<VT, List, If<BoolClosure<False>, Then, Else>> {
        using result = typename Eval<VT, List, Else>::result;
    };


    template<typename VT, typename List, uint64_t name, typename Value, typename Expr>
    struct Eval<VT, List, Let<name, Value, Expr>> {
        using result =
            typename Eval<
                VT,
                VariableList<name, typename Eval<VT, List, Value>::result, List>,
                Expr
            >::result;
    };

    template<typename VT, typename List, uint64_t name>
    struct Eval<VT, List, Ref<name>> {
        using result = typename findInList<name, List>::result;
    };


    template<typename VT, typename List, uint64_t name, typename Body>
    struct Eval<VT, List, Lambda<name, Body>> {
        using result = LambdaClosure<Lambda<name, Body>, List>;
    };

    template<typename VT, typename List, typename Fun, typename Param>
    struct Eval<VT, List, Invoke<Fun, Param>> {
        using result = 
            typename Eval<
                VT,
                List,
                Invoke<typename Eval<VT, List, Fun>::result, Param>
            >::result;
    };

    template<typename VT, typename List, typename OldList, uint64_t name, typename Body, typename Param>
    struct Eval<VT, List, Invoke<LambdaClosure<Lambda<name, Body>, OldList>, Param>> {
        using evalParam = typename Eval<VT, List, Param>::result;
        using oldExpList = VariableList<name, evalParam, OldList>;
        using result = 
            typename Eval<        
                VT,
                oldExpList,
                Body
            >::result;
    };
}


template<typename ValueType, typename Enable = void>
class Fibin {

public:
    template<typename Expr>
    constexpr static void eval() {
        std::cout << "Fibin doesn't support: "
                  << typeid(ValueType).name() << "\n";
    }
};

// Użyta zostanie specjalizacja, jeśli ValueType jest typem liczbowym.

template<typename ValueType>
class Fibin<ValueType, std::enable_if_t<std::is_integral_v<ValueType>>> {

public:
    template<typename Expr>
    constexpr static ValueType eval() {
        return details::Eval<ValueType, details::Nil, Expr>::result::value;
    }
};

#endif /* __FIBIN_H__ */
