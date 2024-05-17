//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Net/Socket/TLSSocket.hpp"
// Core
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/IO/Logging.hpp"
// System
#include <memory>
#include <openssl/tls1.h>


#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX


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
	Core::Result<TLSSocket, Error> TLSSocket::Connect(const Endpoint& endpoint)
	{
		auto tcp = TCPSocket::Connect(endpoint);
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

		TLSSocket tls;
		tls.mTCP = tcp.Unwrap();
		tls.mSSL = ssl;
		return tls;
	}


	TLSSocket::TLSSocket()
		: mSSL(nullptr)
	{}


	TLSSocket::TLSSocket(TLSSocket&& other) noexcept
	{
		mTCP = std::move(other.mTCP);
		mSSL = std::exchange(other.mSSL, nullptr);
	}


	TLSSocket& TLSSocket::operator=(TLSSocket&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return *this;
	}


	TLSSocket::~TLSSocket()
	{
		if (mSSL)
		{
			SSL_shutdown(mSSL);

#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
			close(mTCP.mSocket);
#elif STRAWBERRY_TARGET_WINDOWS
			closesocket(mTCP.mSocket);
#endif

			SSL_free(mSSL);
		}
	}


	bool TLSSocket::Poll() const
	{
		return mTCP.Poll();
	}


	StreamReadResult TLSSocket::Read(size_t length)
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
						return Error::ConnectionReset;
					case SSL_ERROR_SYSCALL:
						Core::Logging::Error("SSL read error. Error: {}", strerror(errno));
						Core::Unreachable();
					default:
						Core::Logging::Error("Unknown SSL_read error code: {}", error);
						Core::Unreachable();
				}
			}
		}

		Core::Assert(bytesRead == length);
		return buffer;
	}


	StreamWriteResult TLSSocket::Write(const Core::IO::DynamicByteBuffer& bytes)
	{
		int bytesSent = 0;

		while (bytesSent < bytes.Size())
		{
			auto writeResult = SSL_write(mSSL, bytes.Data() + bytesSent, bytes.Size() - bytesSent);

			if (writeResult <= 0)
			{
				int error = SSL_get_error(mSSL, writeResult);
				switch (error)
				{
					default:
						Core::Unreachable();
				}
			}
			else
			{
				bytesSent += writeResult;
			}
		}

		return bytesSent;
	}
} // namespace Strawberry::Net::Socket
