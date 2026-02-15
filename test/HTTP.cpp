#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Net/HTTP/Constants.hpp"
#include "Strawberry/Net/HTTP/HTTPClient.hpp"
#include "Strawberry/Net/HTTP/Request.hpp"
#include "Strawberry/Net/HTTP/Response.hpp"
#include "Strawberry/Core/IO/Logging.hpp"


int main()
{
	Strawberry::Net::Endpoint endpoint = Strawberry::Net::Endpoint::Resolve("google.com", 443).Unwrap();
	Strawberry::Net::HTTP::HTTPSClient client(endpoint);

	Strawberry::Net::HTTP::Request request(Strawberry::Net::HTTP::Verb::GET, "/");
	client.SendRequest(request);
	Strawberry::Net::HTTP::Response response = client.Receive();
	Strawberry::Core::Logging::Info("Retrieved google.com! response size = {}", response.GetPayload().Size());

	return 0;
}

