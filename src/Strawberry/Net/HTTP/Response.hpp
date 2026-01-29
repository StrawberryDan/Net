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


            [[nodiscard]] const Version& GetVersion() const
            {
                return mVersion;
            }


            [[nodiscard]] const unsigned int& GetStatus() const
            {
                return mStatus;
            }


            [[nodiscard]] const std::string& GetStatusText() const
            {
                return mStatusText;
            }


            [[nodiscard]] const Header& GetHeader() const
            {
                return mHeader;
            }


			Header& GetHeader()
            {
                return mHeader;
            }


            [[nodiscard]] const Core::IO::DynamicByteBuffer& GetPayload() const
            {
                return mPayload;
            }


			void SetPayload(const Core::IO::DynamicByteBuffer& payload)
            {
                mPayload = payload;
            }

        private:
            Version                     mVersion;
            unsigned int                mStatus;
            std::string                 mStatusText;
            Header                      mHeader;
            Core::IO::DynamicByteBuffer mPayload;
    };
} // namespace Strawberry::Net::HTTP
