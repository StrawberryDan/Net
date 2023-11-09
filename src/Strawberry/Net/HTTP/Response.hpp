#pragma once


#include "Constants.hpp"
#include "Header.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"


namespace Strawberry::Net::HTTP
{
	class Response
	{
	public:
		Response(Version mVersion, unsigned int mStatus, std::string mStatusText);


		[[nodiscard]] inline const Version& GetVersion() const { return mVersion; }


		[[nodiscard]] inline const unsigned int& GetStatus() const { return mStatus; }


		[[nodiscard]] inline const std::string& GetStatusText() const { return mStatusText; }


		[[nodiscard]] inline const Header& GetHeader() const { return mHeader; }


		inline Header& GetHeader() { return mHeader; }


		[[nodiscard]] inline const Core::IO::DynamicByteBuffer& GetPayload() const { return mPayload; }


		inline void SetPayload(const Core::IO::DynamicByteBuffer& payload) { mPayload = payload; }


	private:
		Version               mVersion;
		unsigned int          mStatus;
		std::string           mStatusText;
		Header                mHeader;
		Core::IO::DynamicByteBuffer mPayload;
	};
} // namespace Strawberry::Net::HTTP
