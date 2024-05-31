#include <utility>

#include "Strawberry/Net/HTTP/Response.hpp"


namespace Strawberry::Net::HTTP
{
    Response::Response(Version mVersion, unsigned int mStatus, std::string mStatusText)
        : mVersion(mVersion)
        , mStatus(mStatus)
        , mStatusText(std::move(mStatusText)) {}
} // namespace Strawberry::Net::HTTP
