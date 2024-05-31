#include <utility>

#include "Strawberry/Net/HTTP/Request.hpp"


namespace Strawberry::Net::HTTP
{
    Request::Request(Verb verb, std::string uri, Version version)
        : mVerb(verb)
        , mURI(std::move(uri))
        , mVersion(version) {}
} // namespace Strawberry::Net::HTTP
