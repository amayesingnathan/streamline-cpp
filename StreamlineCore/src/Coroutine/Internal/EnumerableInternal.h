#pragma once

#include "Common/Base.h"

#include <coroutine>
#include <concepts>

namespace slc {

	template<typename T>
	class IEnumerable;

	template<typename TReturn = void>
	class Enumerator;
}

namespace slc::Internal {
	
	template<typename T>
	class EnumeratorPromise
	{
	public:
		using ValueType = std::remove_reference_t<T>;
		using ReferenceType = std::conditional_t<std::is_reference_v<T>, T, T&>;
		using PointerType = ValueType*;
		using ErrorType = std::exception_ptr;

		EnumeratorPromise() = default;

		auto get_return_object() noexcept -> slc::Enumerator<T>;

		auto initial_suspend() const { return std::suspend_never{}; }
		auto final_suspend() const noexcept { return std::suspend_always{}; }

		template<typename U = T> requires !std::is_rvalue_reference_v<U>
		auto yield_value(ValueType& value) noexcept
		{
			mStorage.emplace<PointerType>(std::addressof(value));
			return std::suspend_always{};
		}

		auto yield_value(ValueType&& value) noexcept
		{
			mStorage.emplace<PointerType>(std::addressof(value));
			return std::suspend_always{};
		}

		void unhandled_exception() { mStorage.emplace<ErrorType>(std::current_exception()); }
		void return_void() {}

		auto extract_value() const noexcept -> decltype(auto) { return static_cast<ReferenceType>(**std::get_if<PointerType>(&mStorage)); }

		template<typename U>
		auto await_transform(U&& value) -> std::suspend_never = delete;

		void try_rethrow()
		{
			if (!std::holds_alternative<ErrorType>(mStorage))
				return;

			std::rethrow_exception(*std::get_if<ErrorType>(&mStorage));
		}

	private:
		using StorageTypes = TypeList<PointerType, ErrorType>;
		StorageTypes::VariantType mStorage;
	};



	struct EnumeratorSentinel {};

	template<typename T>
	class EnumeratorIterator
	{
	public:
		using CoroutineHandle = std::coroutine_handle<EnumeratorPromise<T>>;

	public:
		// Iterator Traits
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::input_iterator_tag;
		using value_type = typename EnumeratorPromise<T>::ValueType;
		using reference  = typename EnumeratorPromise<T>::ReferenceType;
		using pointer	 = typename EnumeratorPromise<T>::PointerType;

	public:
		EnumeratorIterator() noexcept = default;

		explicit EnumeratorIterator(CoroutineHandle coro) noexcept :
			mHandle(coro) {}

		friend auto operator==(const EnumeratorIterator& it, EnumeratorSentinel) noexcept -> bool
		{
			return it.mHandle == nullptr || it.mHandle.done();
		}
		friend auto operator!=(const EnumeratorIterator& it, EnumeratorSentinel s) noexcept -> bool { return !(it == s); }

		friend auto operator==(EnumeratorSentinel s, const EnumeratorIterator& it) noexcept -> bool { return (it == s); }
		friend auto operator!=(EnumeratorSentinel s, const EnumeratorIterator& it) noexcept -> bool { return it != s; }

		EnumeratorIterator& operator++()
		{
			mHandle.resume();
			if (mHandle.done())
			{
				mHandle.promise().try_rethrow();
			}
			
			return *this;
		}
		auto operator++(int) { operator++(); }

		auto operator*() const noexcept -> reference { return mHandle.promise().extract_value(); }
		auto operator->() const noexcept -> pointer { return std::addressof(operator*()); }

	private:
		CoroutineHandle mHandle = nullptr; 
	};
}