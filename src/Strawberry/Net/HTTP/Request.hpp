#pragma once


#include "Constants.hpp"
#include "Header.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include <string>


namespace Strawberry::Core::Net::HTTP
{
	class Request
	{
	public:
		Request(Verb verb, std::string uri, Version version = Version::VERSION_1_1);


		[[nodiscard]] inline const Verb& GetVerb() const { return mVerb; }


		[[nodiscard]] inline const std::string& GetURI() const { return mURI; }


		[[nodiscard]] inline const Version& GetVersion() const { return mVersion; }


		[[nodiscard]] inline const Header& GetHeader() const { return mHeader; }


		inline Header& GetHeader() { return mHeader; }


		[[nodiscard]] inline const IO::DynamicByteBuffer& GetPayload() const { return mPayload; }


		inline void SetPayload(const IO::DynamicByteBuffer& payload) { mPayload = payload; }


	private:
		Verb                  mVerb;
		std::string           mURI;
		Version               mVersion;
		Header                mHeader;
		IO::DynamicByteBuffer mPayload;
	};
} // namespace Strawberry::Core::Net::HTTP
