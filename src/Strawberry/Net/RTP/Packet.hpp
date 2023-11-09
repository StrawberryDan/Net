#pragma once


/// Strawberry Core
#include "Strawberry/Core/IO/Concepts.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/IO/Endian.hpp"

/// Standard Library
#include <cstdint>
#include <type_traits>
#include <vector>


namespace Strawberry::Net::RTP
{
	class Packet
	{
	public:
#pragma pack(1)
		struct Header {
			uint8_t  csrcCount      : 4  = 0;
			uint8_t  extension      : 1  = 0;
			uint8_t  padding        : 1  = 0;
			uint8_t  version        : 2  = 2;
			uint8_t  payloadType    : 7  = 0;
			uint8_t  marker         : 1  = 0;
			uint16_t sequenceNumber : 16 = 0;
			uint32_t timestamp      : 32 = 0;
			uint32_t ssrc           : 32 = 0;
		};
#pragma pack()


		template <typename DataSource>
			requires IO::Read<DataSource>
		static Core::Result<Packet, Error> Read(DataSource& data)
		{
			auto headerData = data.Read(sizeof(Header));
			if (!headerData) return headerData.Err();

			Header header = headerData->template Into<Header>();
			if (!IsHeaderValid(header)) return Error::ParsingRTPPacket;

			auto ccrcData = data.Read(header.csrcCount * sizeof(uint32_t));
			if (!ccrcData) return ccrcData.Err();
			std::vector<uint32_t> ccrc = ccrcData->template AsVector<uint32_t>();
			for (auto& source : ccrc) source = FromBigEndian(source);

			auto payloadData = data.Read(data.Size() - sizeof(Header) - sizeof(uint32_t) * header.csrcCount);
			if (!payloadData) return payloadData.Err();
			auto payload = payloadData.Unwrap();

			if (header.padding == 1)
			{
				auto lastByte = payload[payload.Size() - 1];
				payload.Resize(payload.Size() - lastByte);
			}

			return {header, ccrc, payload};
		}


		Packet(uint8_t type, uint16_t sequenceNumber, uint32_t timestamp, uint32_t ssrc)
		{
			Core::Assert(type < 128);
			mHeader.payloadType    = type;
			mHeader.sequenceNumber = Core::ToBigEndian(sequenceNumber);
			mHeader.timestamp      = Core::ToBigEndian(timestamp);
			mHeader.ssrc           = Core::ToBigEndian(ssrc);
		}


		Packet(Header header, std::vector<uint32_t> contributingSources, Core::IO::DynamicByteBuffer payload)
			: mHeader(header)
			, mContributingSources(std::move(contributingSources))
			, mPayload(std::move(payload))
		{
			Core::Assert(mContributingSources.size() == mHeader.csrcCount);
		}


		[[nodiscard]] const Header& GetHeader() const { return mHeader; }


		void AddContributingSource(uint32_t source)
		{
			Core::Assert(mHeader.csrcCount < 15);
			mHeader.csrcCount += 1;
			mContributingSources.push_back(source);
		}


		[[nodiscard]] const std::vector<uint32_t>& GetContributingSources() const { return mContributingSources; }


		[[nodiscard]] Core::IO::DynamicByteBuffer GetPayload() const { return mPayload; }


		void SetPayload(Core::IO::DynamicByteBuffer payload) { mPayload = std::move(payload); }


		[[nodiscard]] Core::IO::DynamicByteBuffer AsBytes() const
		{
			Core::IO::DynamicByteBuffer bytes;
			bytes.Push(mHeader);
			bytes.Push(mContributingSources);
			bytes.Push(mPayload);
			return bytes;
		}


	private:
		static bool IsHeaderValid(const Header& header)
		{
			if (header.version == 2) return false;
			return true;
		}


	private:
		Header                mHeader;
		std::vector<uint32_t> mContributingSources;
		Core::IO::DynamicByteBuffer mPayload;
	};


	static_assert(sizeof(Packet::Header) == 12);
	static_assert(std::is_standard_layout_v<Packet::Header>);
} // namespace Strawberry::Net::RTP
