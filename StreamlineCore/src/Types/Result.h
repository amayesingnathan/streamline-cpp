#pragma once

#include "Common/Functional.h"

namespace slc {

	template<typename T>
	class Option;

	template<typename T, IsEnum E>
	class Result
	{
	private:
		using EnumUnderlyingType = int64_t;

		using Type = Result<T, E>;
		using StorageType = std::variant<T, E>;

	public:
		SCONSTEXPR bool NoExceptDefNew = std::is_nothrow_default_constructible_v<T>;
		SCONSTEXPR bool NoExceptNew = std::is_nothrow_constructible_v<T>;
		SCONSTEXPR bool NoExceptMove = std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>;
		SCONSTEXPR bool NoExceptCopy = std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>;

		enum InternalEnum : EnumUnderlyingType
		{
			Ok = Limits<EnumUnderlyingType>::Min
		};

		using EnumUnionType = std::variant<InternalEnum, E>;
		using DataType = T;
		using EnumType = E;

	public:
		explicit constexpr Result() = default;
		explicit constexpr Result(T&& result) noexcept(NoExceptMove)
			: mValue(std::move(result)), mResult(true) {}
		explicit constexpr Result(E error) noexcept
			: mValue(error), mResult(false) {}

		constexpr EnumUnionType as_enum() const noexcept { return mResult ? EnumUnionType(Ok) : GetError(); }

		/// <summary>
		/// Converts from const Result<T, E> to Result<const T&, E>
		/// </summary>
		constexpr Result<const T&, E> as_ref() const noexcept { return mResult ? Result<const T&, E>(GetValRef()) : Result<const T&, E>(GetError()); }

		/// <summary>
		/// Converts from Result<T, E> to Result<T&, E>
		/// </summary>
		constexpr Result<T&, E> as_mut() noexcept { return mResult ? Result<T&, E>(GetValRef()) : Result<T&, E>(GetError()); }

		constexpr bool is_ok() const noexcept { return mResult; }
		constexpr bool is_err() const noexcept { return !mResult; }

		/*
			Extracting contained values

			These methods extract the contained value in a Result<T, E> when it is the Ok variant. Error behaviour depends on the method.
			expect() and unwrap() are strongly discouraged and should only be used for unrecoverable errors.
		*/

		/// <summary>
		/// Throws runtime error with a provided custom message or returns result
		/// </summary>
		constexpr T&& expect(const std::string_view msg)
		{
			if (!mResult)
				throw std::runtime_error(msg.data());
			else
				return MoveVal();
		}

		/// <summary>
		/// Throws runtime error with a generic message or returns result
		/// </summary>
		constexpr T&& unwrap() { return expect("emergency failure"); }

		/// <summary>
		/// Returns result or the provided default value
		/// </summary>
		constexpr T&& unwrap_or(const T& defaultValue) noexcept(NoExceptMove) requires std::copyable<T>
		{
			if (mResult)
				return MoveVal();
			else
				return T(defaultValue);
		}
		/// <summary>
		/// Returns result or the default value of the underlying type
		/// </summary>
		constexpr T&& unwrap_or_default() noexcept(NoExceptDefNew) requires std::is_default_constructible_v<T>
		{
			return unwrap_or(T());
		}
		/// <summary>
		/// Returns result or the result of evaluating the provided function
		/// </summary>
		template<typename Func> requires IsFunc<Func, T>
		constexpr T&& unwrap_or_else(Func&& op) noexcept(noexcept(op()) && NoExceptMove)
		{
			return unwrap_or(op());
		}


		/*
			Transforming contained values

			These methods transform the contained value in a Result<T, E> while maintaining the result.
		*/

		/// <summary>
		/// Transforms Result&lt;T, E&gt; into Result&lt;R, E&gt; by applying the provided function to the contained value of Ok and leaving Err values unchanged
		/// </summary>
		template<typename Func, typename U> requires IsFunc<Func, Result<U, E>, T&&>
		constexpr Result<U, E> map(Func&& op) noexcept(noexcept(op()) && Result<U, E>::NoExceptMove&& NoExceptMove)
		{
			if (mResult)
				return op(MoveVal());

			return Result<U, E>(GetError());
		}
		/// <summary>
		/// Transforms Result&lt;T, E&gt; into Result&lt;T, F&gt; by applying the provided function to the contained value of Err and leaving Ok values unchanged
		/// </summary>
		template<typename Func, IsEnum O> requires IsFunc<Func, O, E>
		constexpr Result<T, E>&& map_err(Func&& op) noexcept(noexcept(op()) && NoExceptMove)
		{
			if (mResult)
				return Result<T, O>(MoveVal());

			return Result<T, O>(op(GetError()));
		}

		/// <summary>
		/// Applies the provided function to the contained value of Ok, or returns the provided default value if the Result is Err.
		/// Function returns U&& where U is a possibly new type. 
		/// </summary>
		template<typename Func, typename U> requires IsFunc<Func, U&&, T&&>
		constexpr U&& map_or(U defaultVal, Func&& op) noexcept(noexcept(op()) && Result<U, E>::NoExceptMove&& NoExceptMove)
		{
			if (mResult)
				return MoveVal();

			return std::move(defaultVal);
		}

		/// <summary>
		/// Applies the provided function to the contained value of Ok, or applies the provided default fallback function to the contained value of Err.
		/// Both functions return U&& where U is a possibly new type. 
		/// </summary>
		template<typename Func, typename ErrFunc, typename U> requires IsFunc<Func, U&&, T&&> and IsFunc<ErrFunc, U&&, E>
		constexpr U&& map_or_else(Func&& op, ErrFunc&& errOp) noexcept(
			noexcept(op()) &&
			noexcept(errOp()) &&
			Result<U, E>::NoExceptMove&&
			NoExceptMove
			)
		{
			if (mResult)
				return op(MoveVal());

			return errOp(GetError());
		}

		/// <summary>
		/// Returns the result of the provided function, or the error result
		/// </summary>
		template<typename Func> requires IsFunc<Func, Result<T, E>>
		constexpr Result<T, E> operator |(Func&& next) noexcept(noexcept(next()) && NoExceptMove)
		{
			if (mResult)
				return next();

			return Result<T, E>(GetError());
		}

		/// <summary>
		/// For use with macros only.
		/// </summary>
		constexpr const E* as_err() const noexcept { return std::get_if<E>(&mValue); }
		/// <summary>
		/// For use with macros only.
		/// </summary>
		constexpr T& as_valref() noexcept { return *std::get_if<T>(&mValue); }

	protected:
		/// <summary>
		/// Only for internal use. Should never be used without checking the internal state prior first. Marked noexcept given this assumption as std::get_if should never return null.
		/// </summary>
		/// <returns></returns>
		constexpr T&& MoveVal() noexcept(NoExceptMove) { return std::move(*std::get_if<T>(&mValue)); }

		/// <summary>
		/// Only for internal use. Should never be used without checking the internal state prior first. Marked noexcept given this assumption as std::get_if should never return null.
		/// </summary>
		/// <returns></returns>
		constexpr T& GetValRef() noexcept { return *std::get_if<T>(&mValue); }

		/// <summary>
		/// Only for internal use. Should never be used without checking the internal state prior first. Marked noexcept given this assumption as std::get_if should never return null.
		/// </summary>
		/// <returns></returns>
		constexpr E GetError() const noexcept { return *std::get_if<E>(&mValue); }

	protected:
		bool mResult = false;

	private:
		StorageType mValue;
	};


	template<typename T>
	struct OkFunctor;

	template<typename T>
	struct ErrorFunctor;

	template<typename T, IsEnum E>
	struct OkFunctor<Result<T, E>>
	{
		constexpr Result<T, E> operator()(T&& result) const noexcept(Result<T, E>::NoExceptMove)
		{
			return Result<T, E>(std::move(result));
		}

		template<typename... Args>
		constexpr Result<T, E> operator()(Args&&... args) const noexcept(Option<T>::NoExceptNew)
		{
			return Result<T, E>(std::move(T(std::forward<Args>(args)...)));
		}
	};

	template<typename T, IsEnum E>
	struct ErrorFunctor<Result<T, E>>
	{
		constexpr Result<T, E> operator()(E error) const noexcept
		{
			return Result<T, E>(error);
		}
	};

	template<typename T>
	SCONSTEXPR OkFunctor<T> Ok;

	template<typename T>
	SCONSTEXPR ErrorFunctor<T> Err;
}