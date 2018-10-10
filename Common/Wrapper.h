#pragma once

#include <unordered_map>
#include <algorithm>
#include "Common\Logging.h"

constexpr UINT MaxIndex = 8;

template <typename D>
class AddressLookupTableDinput
{
public:
	explicit AddressLookupTableDinput() {}
	~AddressLookupTableDinput()
	{
		ConstructorFlag = true;
		for (const auto& cache : g_map)
		{
			for (const auto& entry : cache)
			{
				entry.second->DeleteMe();
			}
		}
	}

	template <typename T>
	struct AddressCacheIndex { static constexpr UINT CacheIndex = 0; };
	template <>
	struct AddressCacheIndex<m_IDirectInputA> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectInputW> {	static constexpr UINT CacheIndex = 2;
		using Type1A = m_IDirectInputA;
		using Type2A = m_IDirectInput2A;
		using Type7A = m_IDirectInput7A;
		using Type1W = m_IDirectInputW;
		using Type2W = m_IDirectInput2W;
		using Type7W = m_IDirectInput7W;
	};
	template <>
	struct AddressCacheIndex<m_IDirectInput2A> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirectInput2W> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectInput7A> { static constexpr UINT CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirectInput7W> { static constexpr UINT CacheIndex = 6; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDeviceA> { static constexpr UINT CacheIndex = 1; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDeviceW> { static constexpr UINT CacheIndex = 2;
		using Type1A = m_IDirectInputDeviceA;
		using Type2A = m_IDirectInputDevice2A;
		using Type7A = m_IDirectInputDevice7A;
		using Type1W = m_IDirectInputDeviceW;
		using Type2W = m_IDirectInputDevice2W;
		using Type7W = m_IDirectInputDevice7W;
	};
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice2A> { static constexpr UINT CacheIndex = 3; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice2W> { static constexpr UINT CacheIndex = 4; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice7A> { static constexpr UINT CacheIndex = 5; };
	template <>
	struct AddressCacheIndex<m_IDirectInputDevice7W> { static constexpr UINT CacheIndex = 6; };

	template <typename T>
	T *FindAddress(void *Proxy, DWORD Version, DWORD Type)
	{
		switch (Type)
		{
		case ANSI_CHARSET:
		{
			switch (Version)
			{
			case 1:
				return (T*)FindAddress<AddressCacheIndex<T>::Type1A>(Proxy);
			case 2:
				return (T*)FindAddress<AddressCacheIndex<T>::Type2A>(Proxy);
			case 7:
				return (T*)FindAddress<AddressCacheIndex<T>::Type7A>(Proxy);
			default:
				return nullptr;
			}
		}
		case UNICODE:
		{
			switch (Version)
			{
			case 1:
				return (T*)FindAddress<AddressCacheIndex<T>::Type1W>(Proxy);
			case 2:
				return (T*)FindAddress<AddressCacheIndex<T>::Type2W>(Proxy);
			case 7:
				return (T*)FindAddress<AddressCacheIndex<T>::Type7W>(Proxy);
			default:
				return nullptr;
			}
		}
		default:
			return nullptr;
		}
	}

	template <typename T>
	T * FindAddress(void *Proxy)
	{
		if (Proxy == nullptr)
		{
			return nullptr;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		auto it = g_map[CacheIndex].find(Proxy);

		if (it != std::end(g_map[CacheIndex]))
		{
			return static_cast<T *>(it->second);
		}

		return new T(static_cast<T *>(Proxy));
	}

	template <typename T>
	void SaveAddress(T *Wrapper, void *Proxy)
	{
		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
		if (Wrapper != nullptr && Proxy != nullptr)
		{
			g_map[CacheIndex][Proxy] = Wrapper;
		}
	}

	template <typename T>
	void DeleteAddress(T *Wrapper)
	{
		if (Wrapper != nullptr && !ConstructorFlag)
		{
			constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;
			auto it = std::find_if(g_map[CacheIndex].begin(), g_map[CacheIndex].end(),
				[=](auto Map) -> bool { return Map.second == Wrapper; });

			if (it != std::end(g_map[CacheIndex]))
			{
				it = g_map[CacheIndex].erase(it);
			}
		}
	}

private:
	bool ConstructorFlag = false;
	std::unordered_map<void*, class AddressLookupTableObject*> g_map[MaxIndex];
};

class AddressLookupTableObject
{
public:
	virtual ~AddressLookupTableObject() { }

	void DeleteMe()
	{
		delete this;
	}
};