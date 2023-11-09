#pragma once


#include "Strawberry/Core/Types/Optional.hpp"
#include <string>


namespace Strawberry::Core::Net::HTTP
{
	class Verb
	{
	public:
		enum _Enum
		{
			POST,
			GET,
			PUT,
			PATCH,
			DEL,
		};

	public:
		inline Verb(_Enum value)
			: mValue(value)
		{}


		operator _Enum() const { return mValue; }

		static Optional<Verb>     Parse(const std::string& string);
		[[nodiscard]] std::string ToString() const;

	private:
		_Enum mValue;
	};


	class Version
	{
	public:
		enum _Enum
		{
			VERSION_1_0,
			VERSION_1_1,
			VERSION_2,
			VERSION_3
		};

	public:
		inline Version(_Enum value)
			: mValue(value)
		{}


		inline operator _Enum() const { return mValue; }

		static Optional<Version>  Parse(const std::string& string);
		[[nodiscard]] std::string ToString() const;

	private:
		_Enum mValue;
	};
} // namespace Strawberry::Core::Net::HTTP