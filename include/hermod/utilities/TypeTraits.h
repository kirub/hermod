#pragma once

#include <string>

/*-----------------------------------------------------------------------------
 * Macros to abstract the presence of certain compiler intrinsic type traits
 -----------------------------------------------------------------------------*/
#define HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
#define HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
#define HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
#define HAS_TRIVIAL_COPY(T) __has_trivial_copy(T)
#define IS_POD(T) __is_pod(T)
#define IS_ENUM(T) __is_enum(T)
#define IS_EMPTY(T) __is_empty(T)

/*-----------------------------------------------------------------------------
	Type traits similar to TR1 (uses intrinsics supported by VC8)
	Should be updated/revisited/discarded when compiler support for tr1 catches up.
 -----------------------------------------------------------------------------*/

/** Is type DerivedType inherited from BaseType. */
template <typename DerivedType, typename BaseType>
struct is_derived_from {
	// Different size types so we can compare their sizes later.
	typedef char No[1];
	typedef char Yes[2];

	// Overloading Test() s.t. only calling it with something that is
	// a BaseType (or inherited from the BaseType) will return a Yes.
	static Yes &Test(BaseType *);
	static Yes &Test(const BaseType *);
	static No &Test(...);

	// Makes a DerivedType ptr.
	static DerivedType *DerivedTypePtr() { return nullptr; }

public:
	// @third party code - BEGIN RARE Backport FStringView from 4.25 head
	// Test the derived type pointer. If it inherits from BaseType, the Test( BaseType* )
	// will be chosen. If it does not, Test( ... ) will be chosen.
	static const bool Value = sizeof(Test(DerivedTypePtr())) == sizeof(Yes);

	static const bool IsDerived = Value;
	// @third party code - END RARE Backport FStringView from 4.25 head
};

template <class T, T Val>
struct integral_constant {
	static constexpr T Value = Val;

	using value_type = T;
	using type = integral_constant;
};

template <bool _Val>
using bool_constant = integral_constant<bool, _Val>;
using true_type  = bool_constant<true>;
using false_type = bool_constant<false>;

/**
 * is_float_type
 */
template <typename T>
struct is_float_type {
	enum { Value = false };
};

template <>
struct is_float_type<float> {
	enum { Value = true };
};
template <>
struct is_float_type<double> {
	enum { Value = true };
};
template <>
struct is_float_type<long double> {
	enum { Value = true };
};

/**
 * is_integral_type
 */
template <typename T>
struct is_integral_type {
	enum { Value = false };
};

template <>
struct is_integral_type<uint8_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<uint16_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<uint32_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<uint64_t> {
	enum { Value = true };
};

template <>
struct is_integral_type<int8_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<int16_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<int32_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<int64_t> {
	enum { Value = true };
};

template <>
struct is_integral_type<bool> {
	enum { Value = true };
};

template <>
struct is_integral_type<wchar_t> {
	enum { Value = true };
};
template <>
struct is_integral_type<char> {
	enum { Value = true };
};


/**
 * is_signed_integral_type
 */
template <typename T>
struct is_signed_integral_type {
	enum { Value = false };
};

template <>
struct is_signed_integral_type<int8_t> {
	enum { Value = true };
};
template <>
struct is_signed_integral_type<int16_t> {
	enum { Value = true };
};
template <>
struct is_signed_integral_type<int32_t> {
	enum { Value = true };
};
template <>
struct is_signed_integral_type<int64_t> {
	enum { Value = true };
};

/**
 * is_unsigned_integral_type
 */
template <typename T>
struct is_unsigned_integral_type {
	enum { Value = is_integral_type<T>::Value && !is_signed_integral_type<T>::Value };
};

/**
 * is_arithmetic_type
 */
template <typename T>
struct is_arithmetic_type {
	enum { Value = is_integral_type<T>::Value || is_float_type<T>::Value };
};

/**
 * is_same
 *
 *  implementation of std::is_same trait.
 */
template <typename A, typename B>
struct is_same {
	enum { Value = false };
};
template <typename T>
struct is_same<T, T> {
	enum { Value = true };
};

/**
 * are_types_equal
 *
/* Tests whether two typenames refer to the same type.
 */
template <typename A, typename B>
struct are_types_equal;

template <typename, typename>
struct are_types_equal {
	enum { Value = false };
};

template <typename A>
struct are_types_equal<A, A> {
	enum { Value = true };
};

/**
 * is_char_type
 */
template <typename T>
struct is_char_type {
	enum { Value = false };
};
template <>
struct is_char_type<char> {
	enum { Value = true };
};
template <>
struct is_char_type<char16_t> {
	enum { Value = true };
};
template <>
struct is_char_type<wchar_t> {
	enum { Value = true };
};

/**
* is_pointer_type
* @todo - exclude member pointers
*/
template <typename T>
struct is_pointer_type {
	static constexpr bool Value = false;
	//enum { Value = false };
};
template <typename T>
struct is_pointer_type<T *> {
	static constexpr bool Value = true;
	//enum { Value = true };
};
template <typename T>
struct is_pointer_type<const T *> {
	static constexpr bool Value = true;
	//enum { Value = true };
};
template <typename T>
struct is_pointer_type<const T *const> {
	static constexpr bool Value = true;
	//enum { Value = true };
};
template <typename T>
struct is_pointer_type<T *volatile> {
	enum { Value = true };
};
template <typename T>
struct is_pointer_type<T *const volatile> {
	static constexpr bool Value = true;
	//enum { Value = true };
};

/**
 * is_reference_type
 */
template <typename T>
struct is_reference_type {
	enum { Value = false };
};
template <typename T>
struct is_reference_type<T &> {
	enum { Value = true };
};
template <typename T>
struct is_reference_type<T &&> {
	enum { Value = true };
};

/**
 * is_void_type
 */
template <typename T>
struct is_void_type {
	enum { Value = false };
};
template <>
struct is_void_type<void> {
	enum { Value = true };
};
template <>
struct is_void_type<void const> {
	enum { Value = true };
};
template <>
struct is_void_type<void volatile> {
	enum { Value = true };
};
template <>
struct is_void_type<void const volatile> {
	enum { Value = true };
};

/**
 * is_pod_type
 * @todo - POD array and member pointer detection
 */
#if _MSC_VER >= 1900 //VS2017 compiler fixes migrating from VS2015
#pragma warning(push)
#pragma warning(disable : 4647)
#endif // _MSC_VER == 1900
template <typename T>
struct is_pod_type {
	enum { Value = IS_POD(T) || IS_ENUM(T) || is_arithmetic_type<T>::Value || is_pointer_type<T>::Value };
};
#if _MSC_VER >= 1900// VS2017 compiler fixes migrating from VS2015
#pragma warning(pop)
#endif // _MSC_VER == 1900

/**
 * is_fundamental_type
 */
template <typename T>
struct is_fundamental_type {
	enum { Value = is_arithmetic_type<T>::Value || is_void_type<T>::Value };
};

/**
 * is_function
 *
 * Tests is a type is a function.
 */
template <typename T>
struct is_function {
	enum { Value = false };
};

#if PLATFORM_COMPILER_HAS_VARIADIC_TEMPLATES

template <typename RetType, typename... Params>
struct is_function<RetType(Params...)> {
	enum { Value = true };
};

#else

template <typename RetType>
struct is_function<RetType()> {
	enum { Value = true };
};
template <typename RetType, typename P0>
struct is_function<RetType(P0)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1>
struct is_function<RetType(P0, P1)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1, typename P2>
struct is_function<RetType(P0, P1, P2)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1, typename P2, typename P3>
struct is_function<RetType(P0, P1, P2, P3)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1, typename P2, typename P3, typename P4>
struct is_function<RetType(P0, P1, P2, P3, P4)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
struct is_function<RetType(P0, P1, P2, P3, P4, P5)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct is_function<RetType(P0, P1, P2, P3, P4, P5, P6)> {
	enum { Value = true };
};
template <typename RetType, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct is_function<RetType(P0, P1, P2, P3, P4, P5, P6, P7)> {
	enum { Value = true };
};

#endif

/**
 * is_zero_construct_type
 */
template <typename T>
struct is_zero_construct_type {
	enum { Value = IS_ENUM(T) || is_arithmetic_type<T>::Value || is_pointer_type<T>::Value };
};

/**
 * no_destructor_type
 */
template <typename T>
struct no_destructor_type {
	enum { Value = is_pod_type<T>::Value || HAS_TRIVIAL_DESTRUCTOR(T) };
};

/**
 * is_weak_pointer_type
 */
template <typename T>
struct is_weak_pointer_type {
	enum { Value = false };
};

/**
 * is_callable
 */
template <typename T>
class is_callable {
private:
	typedef char yes_type;
	struct no_type { char x[2]; };

	template <typename U> static yes_type Test(void(U::*)());
	template <typename U> static no_type Test(...);

public:
	static constexpr bool Value = sizeof(Test<T>(nullptr)) == sizeof(yes_type);
};

/*-----------------------------------------------------------------------------
	Call traits - Modeled somewhat after boost's interfaces.
-----------------------------------------------------------------------------*/

/**
* Call traits helpers
*/
template <typename T, bool TypeIsSmall>
struct call_traits_param_type_helper {
typedef const T &ParamType;
typedef const T &ConstParamType;
};
template <typename T>
struct call_traits_param_type_helper<T, true> {
	typedef const T ParamType;
	typedef const T ConstParamType;
};
template <typename T>
struct call_traits_param_type_helper<T *, true> {
	typedef T *ParamType;
	typedef const T *ConstParamType;
};

/*-----------------------------------------------------------------------------
	Helper templates for dealing with 'const' in template code
-----------------------------------------------------------------------------*/

/**
 * remove_const<> is modeled after boost's implementation.  It allows you to take a templatized type
 * such as 'const Foo*' and use const_cast to convert that type to 'Foo*' without knowing about Foo.
 *
 *		MutablePtr = const_cast< RemoveConst< ConstPtrType >::Type >( ConstPtr );
 */
template <class T>
struct remove_const {
	typedef T Type;
};
template <class T>
struct remove_const<const T> {
	typedef T Type;
};

/*-----------------------------------------------------------------------------
 * call_traits
 *
 * Same call traits as boost, though not with as complete a solution.
 *
 * The main member to note is ParamType, which specifies the optimal
 * form to pass the type as a parameter to a function.
 *
 * Has a small-value optimization when a type is a POD type and as small as a pointer.
-----------------------------------------------------------------------------*/

/**
 * base class for call traits. Used to more easily refine portions when specializing
 */
template <typename T>
struct call_traitsBase {
private:
	enum { PassByValue = is_arithmetic_type<T>::Value || is_pointer_type<T>::Value || (is_pod_type<T>::Value && sizeof(T) <= sizeof(void *)) };

public:
	typedef T ValueType;
	typedef T &Reference;
	typedef const T &ConstReference;
	typedef typename call_traits_param_type_helper<T, PassByValue>::ParamType ParamType;
	typedef typename call_traits_param_type_helper<T, PassByValue>::ConstParamType ConstPointerType;
};

/**
 * call_traits
 */
template <typename T>
struct call_traits : public call_traitsBase<T> {};

// Fix reference-to-reference problems.
template <typename T>
struct call_traits<T &> {
	typedef T &ValueType;
	typedef T &Reference;
	typedef const T &ConstReference;
	typedef T &ParamType;
	typedef T &ConstPointerType;
};

// Array types
template <typename T, size_t N>
struct call_traits<T[N]> {
private:
	typedef T ArrayType[N];

public:
	typedef const T *ValueType;
	typedef ArrayType &Reference;
	typedef const ArrayType &ConstReference;
	typedef const T *const ParamType;
	typedef const T *const ConstPointerType;
};

// const array types
template <typename T, size_t N>
struct call_traits<const T[N]> {
private:
	typedef const T ArrayType[N];

public:
	typedef const T *ValueType;
	typedef ArrayType &Reference;
	typedef const ArrayType &ConstReference;
	typedef const T *const ParamType;
	typedef const T *const ConstPointerType;
};

/*-----------------------------------------------------------------------------
	Traits for our particular container classes
-----------------------------------------------------------------------------*/

/**
 * Helper for array traits. Provides a common base to more easily refine a portion of the traits
 * when specializing. NeedsCopyConstructor/NeedsMoveConstructor/NeedsDestructor is mainly used by the contiguous storage
 * containers.
 */
template <typename T>
struct type_traits_base {
	typedef typename call_traits<T>::ParamType ConstInitType;
	typedef typename call_traits<T>::ConstPointerType ConstPointerType;
	// WRH - 2007/11/28 - the compilers we care about do not produce equivalently efficient code when manually
	// calling the constructors of trivial classes. In array cases, we can call a single memcpy
	// to initialize all the members, but the compiler will call memcpy for each element individually,
	// which is slower the more elements you have.
	enum { NeedsCopyConstructor = !HAS_TRIVIAL_COPY(T) && !is_pod_type<T>::Value };
	enum { NeedsCopyAssignment = !HAS_TRIVIAL_ASSIGN(T) && !is_pod_type<T>::Value };
	// There's currently no good way to detect the need for move construction/assignment so we'll fake it for
	// now with the copy traits
	enum { NeedsMoveConstructor = NeedsCopyConstructor };
	enum { NeedsMoveAssignment = NeedsCopyAssignment };
	// WRH - 2007/11/28 - the compilers we care about correctly elide the destructor code on trivial classes
	// (effectively compiling down to nothing), so it is not strictly necessary that we have NeedsDestructor.
	// It doesn't hurt, though, and retains for us the ability to skip destructors on classes without trivial ones
	// if we should choose.
	enum { NeedsDestructor = !HAS_TRIVIAL_DESTRUCTOR(T) && !is_pod_type<T>::Value };
	// There's no good way of detecting this so we'll just assume it to be true for certain known types and expect
	// users to customize it for their custom types.
	enum { IsBytewiseComparable = IS_ENUM(T) || is_arithmetic_type<T>::Value || is_pointer_type<T>::Value };
};

/**
 * Traits for types.
 */
template <typename T>
struct type_traits : public type_traits_base<T> {};

/**
 * Traits for containers.
 */
template <typename T>
struct container_traits_base {
	// This should be overridden by every container that supports emptying its contents via a move operation.
	enum { MoveWillEmptyContainer = false };
};

template <typename T>
struct container_traits : public container_traits_base<T> {};

template <typename T, typename U>
struct move_support_traits_base {
	// Param type is not an const lvalue reference, which means it's pass-by-value, so we should just provide a single type for copying.
	// Move overloads will be ignored due to SFINAE.
	typedef U Copy;
};

template <typename T>
struct move_support_traits_base<T, const T &> {
	// Param type is a const lvalue reference, so we can provide an overload for moving.
	typedef const T &Copy;
	typedef T &&Move;
};

/**
 * This traits class is intended to be used in pairs to allow efficient and correct move-aware overloads for generic types.
 * For example:
 *
 * template <typename T>
 * void Func(typename move_support_traits<T>::Copy Obj)
 * {
 *     // Copy Obj here
 * }
 *
 * template <typename T>
 * void Func(typename move_support_traits<T>::Move Obj)
 * {
 *     // Move from Obj here as if it was passed as T&&
 * }
 *
 * Structuring things in this way will handle T being a pass-by-value type (e.g. ints, floats, other 'small' types) which
 * should never have a reference overload.
 */
template <typename T>
struct move_support_traits : move_support_traits_base<T, typename call_traits<T>::ParamType> {
};


template <typename T>
struct remove_cv {
	typedef T Type;
};
template <typename T>
struct remove_cv<const T> {
	typedef T Type;
};
template <typename T>
struct remove_cv<volatile T> {
	typedef T Type;
};
template <typename T>
struct remove_cv<const volatile T> {
	typedef T Type;
};


/**
 * Tests if a type T is bitwise-constructible from a given argument type U.  That is, whether or not
 * the U can be memcpy'd in order to produce an instance of T, rather than having to go
 * via a constructor.
 *
 * Examples:
 * is_bitwise_constructible<PODType,    PODType   >::Value == true  // PODs can be trivially copied
 * is_bitwise_constructible<const int*, int*      >::Value == true  // a non-const Derived pointer is trivially copyable as a const Base pointer
 * is_bitwise_constructible<int*,       const int*>::Value == false // not legal the other way because it would be a const-correctness violation
 * is_bitwise_constructible<int32_t,      uint32_t    >::Value == true  // signed integers can be memcpy'd as unsigned integers
 * is_bitwise_constructible<uint32_t,     int32_t     >::Value == true  // and vice versa
 */

template <typename T, typename Arg>
struct is_bitwise_constructible {
	static_assert(
			!is_reference_type<T>::Value &&
					!is_reference_type<Arg>::Value,
			"is_bitwise_constructible is not designed to accept reference types");

	static_assert(
			are_types_equal<T, typename remove_cv<T>::Type>::Value &&
					are_types_equal<Arg, typename remove_cv<Arg>::Type>::Value,
			"is_bitwise_constructible is not designed to accept qualified types");

	// Assume no bitwise construction in general
	enum { Value = false };
};

template <typename T>
struct is_bitwise_constructible<T, T> {
	// Ts can always be bitwise constructed from itself if it is trivially copyable.
	enum { Value = !type_traits<T>::NeedsCopyConstructor };
};

template <typename T, typename U>
struct is_bitwise_constructible<const T, U> : is_bitwise_constructible<T, U> {
	// Constructing a const T is the same as constructing a T
};

// Const pointers can be bitwise constructed from non-const pointers.
// This is not true for pointer conversions in general, e.g. where an offset may need to be applied in the case
// of multiple inheritance, but there is no way of detecting that at compile-time.
template <typename T>
struct is_bitwise_constructible<const T *, T *> {
	// Constructing a const T is the same as constructing a T
	enum { Value = true };
};

// Unsigned types can be bitwise converted to their signed equivalents, and vice versa.
// (assuming two's-complement, which we are)
template <>
struct is_bitwise_constructible<uint8_t, int8_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<int8_t, uint8_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<uint16_t, int16_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<int16_t, uint16_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<uint32_t, int32_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<int32_t, uint32_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<uint64_t, int64_t> {
	enum { Value = true };
};
template <>
struct is_bitwise_constructible<int64_t, uint64_t> {
	enum { Value = true };
};

#define GENERATE_MEMBER_FUNCTION_CHECK(MemberName, Result, ConstModifier, ...) \
	template <typename T>                                                      \
	class has_member_function_##MemberName {                                    \
		template <typename U, Result (U::*)(__VA_ARGS__) ConstModifier>        \
		struct Check;                                                          \
		template <typename U>                                                  \
		static char MemberTest(Check<U, &U::MemberName> *);                    \
		template <typename U>                                                  \
		static int MemberTest(...);                                            \
                                                                               \
	public:                                                                    \
		enum { Value = sizeof(MemberTest<T>(nullptr)) == sizeof(char) };       \
	};
/*
template <typename T>
struct is_enum {
	enum { Value = IS_ENUM(T) };
};
*/
template <class T, bool = std::is_enum<T>::value>
struct _underlying_struct {
    using type = __underlying_type(T);
};

template <class _Ty>
struct _underlying_struct<_Ty, false> {};

template <class _Ty>
struct underlying_struct : _underlying_struct<_Ty> {}; // determine underlying type for enum

template <class _Ty>
using underlying_type = typename underlying_struct<_Ty>::type;

// Additional Type Traits

template <typename T>
struct is_array {
	static constexpr bool Value = false;
	using Type = T;
	static constexpr int32_t Size = 0;
};

template <typename T, size_t InSize>
struct is_array<T[InSize]> {
	static constexpr bool Value = true;
	using Type = T;
	static constexpr int32_t Size = InSize;
};

template <typename T, size_t InSize>
struct is_array<const T (&)[InSize]> {
	static constexpr bool Value = true;
	using Type = T;
	static constexpr int32_t Size = InSize;
};

template <typename T, size_t InSize>
struct is_array<const T[InSize]> {
	static constexpr bool Value = true;
	using Type = T;
	static constexpr int32_t Size = InSize;
};

template <typename T>
struct unqualified {
	using Type = T;
};

template <typename T>
struct unqualified<const T> {
	using Type = T;
};

template <typename T>
struct unqualified<volatile T> {
	using Type = T;
};

template <typename T>
struct unqualified<const volatile T> {
	using Type = T;
};

template <typename T>
struct unqualified<T *> {
	using Type = typename unqualified<T>::Type;
};

template <typename T>
struct unqualified<const T *> {
	using Type = typename unqualified<T>::Type;
};

template <typename T>
struct unqualified<T &> {
	using Type = typename unqualified<T>::Type;
};

template <typename T>
struct unqualified<T &&> {
	using Type = typename unqualified<T>::Type;
};

template <typename T>
struct unqualified<T *const> {
	using Type = typename unqualified<T>::Type;
};

template <typename T>
struct unqualified<T *const volatile> {
	using Type = typename unqualified<T>::Type;
};

template <typename T>
struct unqualified<T *volatile> {
	using Type = typename unqualified<T>::Type;
};

template <typename T, typename... U>
struct pack_has_type_or_derived_type {
	static constexpr bool Value = false;
};

template <typename Base, typename TypeToCheck, typename... OtherTypesToCheck>
struct pack_has_type_or_derived_type<Base, TypeToCheck, OtherTypesToCheck...> {
	using unqualified = typename unqualified<TypeToCheck>::Type;
	static constexpr bool Value = is_same<Base, unqualified>::Value || is_derived_from<unqualified, Base>::Value || pack_has_type_or_derived_type<Base, OtherTypesToCheck...>::Value;
};

template <typename T>
static constexpr bool is_always_false = false;
/*
template <bool _First_value, class _First, class... _Rest>
struct _Disjunction { // handle true trait or last trait
	using type = _First;
};

template <class _False, class _Next, class... _Rest>
struct _Disjunction<false, _False, _Next, _Rest...> { // first trait is false, try the next trait
	using type = typename _Disjunction<static_cast<bool>(_Next::Value), _Next, _Rest...>::type;
};

template <class... _Traits>
struct disjunction : false_type {}; // If _Traits is empty, false_type

template <class _First, class... _Rest>
struct disjunction<_First, _Rest...> : _Disjunction<static_cast<bool>(_First::Value), _First, _Rest...>::type {
	// the first true trait in _Traits, or the last trait if none are true
};*/

/*
template <typename T>
struct array_size : std::extent<T> {};
template <typename T, size_t N>
struct array_size<std::array<T, N>> : std::tuple_size<std::array<T, N>> {};*/

template <typename T>
struct is_string
		: public std::disjunction<
				  std::is_same<const char *, std::decay_t<T>>,
				  std::is_same<char *, std::decay_t<T>>,
				  std::is_same<std::string, std::decay_t<T>> >
{
};

template <typename T>
struct is_raw_buffer
		: public std::disjunction<
				  std::is_same<const unsigned char *, std::decay_t<T>>,
				  std::is_same<unsigned char *, std::decay_t<T>>,
				  std::is_same<uint8_t *, std::decay_t<T>>,
				  std::is_same<const uint8_t *, std::decay_t<T>>>
{
};
			


#if !_HAS_CXX20

template <typename UnsignedType>
struct Unsigned {
	static constexpr bool Value = is_unsigned_integral_type<UnsignedType>::Value && !is_same<UnsignedType, uint64_t>::Value;
};
template <typename SignedType>
struct Signed {
	static constexpr bool Value = is_signed_integral_type<SignedType>::Value && !is_same<SignedType, int64_t>::Value;
};
template <typename FloatType>
struct Float {
	static constexpr bool Value = is_float_type<FloatType>::Value;
};
template <typename TArrayType>
struct ArrayType {
	static constexpr bool Value = is_array<TArrayType>::Value;
};
template <typename EnumType>
struct Enumeration {
	static constexpr bool Value = std::is_enum<EnumType>::value && !is_integral_type<EnumType>::Value;
};
template <typename BufferType>
struct Buffer {
	static constexpr bool Value = is_raw_buffer<BufferType>::value && !is_string<BufferType>::value && !Enumeration<BufferType>::Value;
};
template <typename TStringType>
struct StringType {
	static constexpr bool Value = (is_same<std::string, TStringType>::Value || is_string<TStringType>::value ) && !Buffer<TStringType>::Value;
};
template <typename StringBufferType>
struct StringBuffer {
	static constexpr bool Value = StringType<StringBufferType>::Value && !is_same<StringBufferType, std::string>::Value;
};
template <typename PointerType>
struct Pointer {
	static constexpr bool Value = is_pointer_type<PointerType>::Value && !Buffer<PointerType>::Value && !StringType<PointerType>::Value;
};
template <typename BoolType>
struct Boolean {
	static constexpr bool Value = (is_char_type<BoolType>::Value || is_same<BoolType, bool>::Value) && !is_integral_type<BoolType>::Value;
};


#else

#include <concepts>

template <typename UnsignedType>		concept Unsigned = std::unsigned_integral<UnsignedType>::Value && !std::is_same_v<UnsignedType, uint64_t>;
template <typename SignedType>			concept Signed = std::signed_integral<SignedType> && !std::is_same_v<SignedType, int64_t>;
template <typename FloatType>			concept Float = std::floating_point<FloatType>;
template <typename TArrayType>			concept ArrayType = std::is_bounded_array_v<TArrayType>;
template <typename EnumType>			concept Enumeration = std::is_enum_v<EnumType>;
template <typename BufferType>			concept Buffer = is_raw_buffer<BufferType>::value && !is_string<BufferType>::value && !Enumeration<BufferType>;
template <typename TStringType>			concept StringType = (std::is_same<std::string, TStringType>::value || is_string<TStringType>::value) && !Buffer<TStringType>;
template <typename StringBufferType>	concept StringBuffer = StringType<StringBufferType> && !std::is_same_v<StringBufferType, std::string>;
template <typename PointerType>			concept Pointer = !Buffer<PointerType> && !StringType<PointerType> && is_pointer_type<PointerType>::Value;
template <typename BoolType>			concept Boolean = std::_Is_character_or_byte_or_bool<BoolType>::value && !std::integral<BoolType>;

#endif


#if _HAS_CXX20
#define template_with_concept_declare(ConceptType, EnableIfType, ClassName, TemplateTypeName) \
	template <ConceptType TemplateTypeName>                          \
	struct ClassName<TemplateTypeName>

#define template_with_concept_base(ClassName, TemplateTypeName, Requirements) \
	template <typename TemplateTypeName>						\
	requires Requirements									\
	struct ClassName

#define template_with_concept_define(ConceptTypeName, TemplateTypeName) \
	template <ConceptTypeName TemplateTypeName>

#else
#define template_with_concept_declare(ConceptType, EnableIfType, ClassName, TemplateTypeName) \
	template <typename TemplateTypeName>                                 \
	struct ClassName<TemplateTypeName, typename enable_if<EnableIfType TemplateTypeName>::Value>::Type>

#define template_with_concept_base(ClassName, TemplateTypeName, Requirements) \
	template <typename TemplateTypeName, typename = void>       \
	struct ClassName

#define template_with_concept_define(ConceptTypeName, EnableIfType, TemplateTypeName)				\
	template <typename TemplateTypeName, typename enable_if<EnableIfType TemplateTypeName>::Value>::Type>
#endif

// Additional Type Traits
