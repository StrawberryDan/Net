#pragma once


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/IO/Logging.hpp"
#include "fmt/core.h"
#include <iostream>
#include <regex>


namespace Strawberry::Core::Net::HTTP
{
	template <typename S>
	ClientBase<S>::ClientBase(const Net::Endpoint& endpoint)
		: mSocket(S::Connect(endpoint).Unwrap())
	{}


	template <typename S>
	void ClientBase<S>::SendRequest(const Request& request)
	{
		std::string headerLine = fmt::format(
			"{} {} HTTP/{}\r\n",
			request.GetVerb().ToString(), request.GetURI(), request.GetVersion().ToString());
		mSocket.Write({headerLine.data(), headerLine.length()}).Unwrap();
		for (const auto& [key, values] : *request.GetHeader())
		{
			for (const auto& value : values)
			{
				std::string formatted = fmt::format("{}: {}\r\n", key, value);
				mSocket.Write({formatted.data(), formatted.length()}).Unwrap();
			}
		}

		std::vector<char> blankLine = {'\r', '\n'};
		mSocket.Write({blankLine.data(), blankLine.size()}).Unwrap();

		if (request.GetPayload().Size() > 0)
		{
			mSocket.Write({request.GetPayload().Data(), request.GetPayload().Size()}).Unwrap();
		}
	}


	template <typename S>
	Response ClientBase<S>::Receive()
	{
		static const auto statusLinePattern = std::regex(R"(HTTP\/([^\s]+)\s+(\d{3})\s+([^\r]*)\r\n)");
		static const auto headerLinePattern = std::regex(R"(([^:]+)\s*:\s*([^\r]+)\r\n)");

		std::string currentLine;
		bool        matched;
		std::smatch matchResults;

		do
		{
			currentLine = ReadLine();
			matched     = std::regex_match(currentLine, matchResults, statusLinePattern);
		} while (!matched);


		std::string version    = matchResults[1],
					status     = matchResults[2],
					statusText = matchResults[3];

		Response response(*Version::Parse(version), std::stoi(status), statusText);

		while (true)
		{
			currentLine = ReadLine();

			if (currentLine == "\r\n")
			{
				break;
			}
			else if (std::regex_match(currentLine, matchResults, headerLinePattern))
			{
				response.GetHeader().Add(matchResults[1], matchResults[2]);
			}
		}

		IO::DynamicByteBuffer payload;
		if (response.GetHeader().Contains("Transfer-Encoding"))
		{
			auto transferEncoding = response.GetHeader().Get("Transfer-Encoding");

			if (transferEncoding == "chunked")
			{
				payload = ReadChunkedPayload();
			}
			else
			{
				Logging::Error("Unsupported value for Transfer-Encoding: {}", transferEncoding);
				Unreachable();
			}
		}
		else if (response.GetHeader().Contains("Content-Length"))
		{
			unsigned long contentLength = std::stoul(response.GetHeader().Get("Content-Length"));
			if (contentLength > 0)
			{
				auto data = mSocket.Read(contentLength).Unwrap();
				Core::Assert(data.Size() == contentLength);
				payload.Push(data.Data(), data.Size());
			}
		}
		response.SetPayload(payload);

		return response;
	}


	template <typename S>
	IO::DynamicByteBuffer ClientBase<S>::ReadChunkedPayload()
	{
		std::string       line;
		std::smatch       matchResults;
		static const auto chunkSizeLine     = std::regex(R"(([0123456789abcdefABCDEF]+)\r\n)");
		size_t            sumOfChunkLengths = 0;

		IO::DynamicByteBuffer payload;
		while (true)
		{
			line = this->ReadLine();
			if (line == "\r\n")
			{
				break;
			}
			else if (!std::regex_match(line, matchResults, chunkSizeLine))
			{
				Core::Unreachable();
			}


			auto bytesToRead = std::stoul(matchResults[1], nullptr, 16);
			if (bytesToRead > 0)
			{
				IO::DynamicByteBuffer chunk;
				chunk.Reserve(bytesToRead);

				chunk.Push(this->mSocket.Read(bytesToRead).Unwrap());
				sumOfChunkLengths += chunk.Size();
				Core::Assert(chunk.Size() == bytesToRead);
				payload.Push(chunk);
			}

			auto CRLF = this->ReadLine();
			Core::Assert(CRLF == "\r\n");

			if (bytesToRead == 0) break;
		}


		Core::Assert(sumOfChunkLengths == payload.Size());
		return payload;
	}


	template <typename S>
	std::string ClientBase<S>::ReadLine()
	{
		std::string line;
		while (!line.ends_with("\r\n"))
		{
			line += mSocket.Read(1).Unwrap().template Into<char>();
		}
		return line;
	}
} // namespace Strawberry::Core::Net::HTTP
