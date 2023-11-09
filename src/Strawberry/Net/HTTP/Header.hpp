#pragma once


#include <map>
#include <string>
#include <utility>
#include <vector>


namespace Strawberry::Core::Net::HTTP
{
	class Header
	{
	public:
		using Key           = std::string;
		using Value         = std::string;
		using Entries       = std::map<Key, std::vector<Value>>;
		using Iterator      = Entries::iterator;
		using ConstIterator = Entries::const_iterator;

	public:
		void                             Add(const Key& key, const Value& value);
		void                             Set(const Key& key, const Value& value);
		[[nodiscard]] Value              Get(const Key& key) const;
		[[nodiscard]] std::vector<Value> GetAll(const Key& key) const;
		[[nodiscard]] bool               Contains(const Key& key) const;


		[[nodiscard]] inline ConstIterator Begin() const { return mEntries.cbegin(); }


		[[nodiscard]] inline ConstIterator End() const { return mEntries.cend(); }


		inline Iterator begin() { return mEntries.begin(); }


		inline Iterator end() { return mEntries.end(); }


		inline const Entries& operator*() const { return mEntries; }


	private:
		Entries mEntries;
	};
} // namespace Strawberry::Core::Net::HTTP