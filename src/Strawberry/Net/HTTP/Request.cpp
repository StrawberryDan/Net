#include <utility>

#include "Strawberry/Net/HTTP/Request.hpp"


namespace Strawberry::Core::Net::HTTP
{
	Request::Request(Verb verb, std::string uri, Version version)
		: mVerb(verb)
		, mURI(std::move(uri))
		, mVersion(version)
	{}
} // namespace Strawberry::Core::Net::HTTP
