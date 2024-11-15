#include "Strawberry/Net/HTTP/Header.hpp"


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Util/Strings.hpp"


namespace Strawberry::Net::HTTP
{
    void Header::Add(const Header::Key& key, const Header::Value& value)
    {
        if (mEntries.contains(key))
        {
            mEntries.at(key).push_back(value);
        }
        else
        {
            mEntries.insert({key, {value}});
        }
    }


    void Header::Set(const Header::Key& key, const Header::Value& value)
    {
        mEntries.insert_or_assign(key, std::vector<Value>{value});
    }


    Header::Value Header::Get(const Header::Key& key) const
    {
        Core::Assert(mEntries.contains(key));
        return mEntries.at(key)[0];
    }


    std::vector<Header::Value> Header::GetAll(const Header::Key& key) const
    {
        Core::Assert(mEntries.contains(key));
        return mEntries.at(key);
    }


    bool Header::Contains(const Header::Key& key) const
    {
        return mEntries.contains(key);
    }
} // namespace Strawberry::Net::HTTP
