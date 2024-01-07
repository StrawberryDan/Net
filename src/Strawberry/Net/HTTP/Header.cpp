#include "Strawberry/Net/HTTP/Header.hpp"


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Util/Strings.hpp"


namespace Strawberry::Net::HTTP
{
	void Header::Add(const Header::Key& key, const Header::Value& value)
	{
		auto lc = Core::ToLowercase(key);
		if (mEntries.contains(lc)) { mEntries.at(lc).push_back(value); }
		else { mEntries.insert({lc, {value}}); }
	}


	void Header::Set(const Header::Key& key, const Header::Value& value)
	{
		auto lc = Core::ToLowercase(key);
		mEntries.insert_or_assign(lc, std::vector<Value>{value});
	}


	Header::Value Header::Get(const Header::Key& key) const
	{
		auto lc = Core::ToLowercase(key);
		Core::Assert(mEntries.contains(lc));
		return mEntries.at(lc)[0];
	}


	std::vector<Header::Value> Header::GetAll(const Header::Key& key) const
	{
		auto lc = Core::ToLowercase(key);
		Core::Assert(mEntries.contains(lc));
		return mEntries.at(lc);
	}


	bool Header::Contains(const Header::Key& key) const
	{
		auto lc = Core::ToLowercase(key);
		return mEntries.contains(lc);
	}
} // namespace Strawberry::Net::HTTP
