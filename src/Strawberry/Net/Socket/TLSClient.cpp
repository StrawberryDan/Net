//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Net/Socket/TLSClient.hpp"
// Core
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/IO/Logging.hpp"
// System
#include <memory>
#include <openssl/tls1.h>


#if defined(__APPLE__) || defined(__linux__)


#include <unistd.h>


#endif


class TLSContext
{
public:
	static SSL_CTX* Get()
	{
		if (!mInstance) { mInstance = std::unique_ptr<TLSContext>(new TLSContext()); }

		return mInstance->mSSL_CONTEXT;
	}


	~TLSContext() { SSL_CTX_free(mSSL_CONTEXT); }


private:
	TLSContext()
	{
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();

		mSSL_CONTEXT = SSL_CTX_new(TLS_client_method());
		Strawberry::Core::Assert(mSSL_CONTEXT != nullptr);
	}


	SSL_CTX* mSSL_CONTEXT;

	static std::unique_ptr<TLSContext> mInstance;
};


std::unique_ptr<TLSContext> TLSContext::mInstance = nullptr;


namespace Strawberry::Net::Socket
{
	Core::Result<TLSClient, Error> TLSClient::Connect(const Endpoint& endpoint)
	{
		auto tcp = TCPClient::Connect(endpoint);
		if (!tcp) { return tcp.Err(); }

		auto ssl = SSL_new(TLSContext::Get());
		if (ssl == nullptr) { return Error::SSLAllocation; }

		if (endpoint.GetHostname())
		{
			auto hostnameResult = SSL_set_tlsext_host_name(ssl, endpoint.GetHostname()->c_str());
			Core::Assert(hostnameResult);
		}

		SSL_set_fd(ssl, tcp->mSocket);
		auto connectResult = SSL_connect(ssl);
		if (connectResult == -1) { return Error::SSLHandshake; }

		TLSClient tls;
		tls.mTCP = tcp.Unwrap();
		tls.mSSL = ssl;
		return tls;
	}


	TLSClient::TLSClient()
		: mSSL(nullptr)
	{}


	TLSClient::TLSClient(TLSClient&& other) noexcept
	{
		mTCP = std::move(other.mTCP);
		mSSL = std::exchange(other.mSSL, nullptr);
	}


	TLSClient& TLSClient::operator=(TLSClient&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return *this;
	}


	TLSClient::~TLSClient()
	{
		if (mSSL)
		{
			SSL_shutdown(mSSL);

#if defined(__APPLE__) || defined(__linux__)
			close(mTCP.mSocket);
#elif defined(_WIN32)
			closesocket(mTCP.mSocket);
#endif

			SSL_free(mSSL);
		}
	}


	bool TLSClient::Poll() const
	{
		return mTCP.Poll();
	}


	Core::Result<Core::IO::DynamicByteBuffer, Core::IO::Error> TLSClient::Read(size_t length)
	{
		auto   buffer    = Core::IO::DynamicByteBuffer::Zeroes(length);
		size_t bytesRead = 0;

		while (bytesRead < length)
		{
			auto thisRead = SSL_read(mSSL, reinterpret_cast<void*>(buffer.Data() + bytesRead), static_cast<int>(length - bytesRead));
			if (thisRead > 0) { bytesRead += thisRead; }
			else
			{
				auto error = SSL_get_error(mSSL, bytesRead);
				switch (error)
				{
					case SSL_ERROR_ZERO_RETURN:
						return Core::IO::Error::Closed;
					case SSL_ERROR_SYSCALL:
						Core::Logging::Error("SSL read error. Error: {}", strerror(errno));
						return Core::IO::Error::Unknown;
					default:
						Core::Logging::Error("Unknown SSL_read error code: {}", error);
						return Core::IO::Error::Unknown;
				}
			}
		}

		Core::Assert(bytesRead == length);
		return buffer;
	}


	Core::Result<size_t, Core::IO::Error> TLSClient::Write(const Core::IO::DynamicByteBuffer& bytes)
	{
		auto bytesSent = SSL_write(mSSL, bytes.Data(), static_cast<int>(bytes.Size()));

		if (bytesSent >= 0)
		{
			Core::Assert(bytesSent == bytes.Size());
			return bytesSent;
		}
		else
		{
			switch (SSL_get_error(mSSL, bytesSent))
			{
				default:
					return Core::IO::Error::Unknown;
			}
		}
	}
} // namespace Strawberry::Net::Socket
