#pragma once 
#include <cassert>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "Core/Logger.h"
#include "vulkanbase/VulkanTypes.h"




class ServiceLocator
{
public:
	ServiceLocator() = default;
	~ServiceLocator() = default;
	ServiceLocator(const ServiceLocator&) = delete;
	ServiceLocator& operator=(const ServiceLocator&) = delete;
	ServiceLocator(ServiceLocator&&) = delete;
	ServiceLocator& operator=(ServiceLocator&&) = delete;

	template <typename T, typename... Args>
	static void RegisterService(Args&&... args)
	{
		const std::type_index typeIndex = std::type_index(typeid(T));
		if (m_ServiceInstances.find(typeIndex) == m_ServiceInstances.end())
		{
			std::unique_ptr<void, decltype(&MakeDeleter<T>)> p(new T(std::forward<Args>(args)...), &MakeDeleter<T>);
			m_ServiceInstances.emplace(std::type_index(typeid(T)), std::move(p));
		}
	}

	template <typename T>
	static T* GetService()
	{
		const auto it = m_ServiceInstances.find(std::type_index(typeid(T)));

		if (it == m_ServiceInstances.end())
		{
			assert(false && "Service Not Found");
			return nullptr;
		}


		return static_cast<T*>(it->second.get());
	}

private:
	template <typename T>
	static void MakeDeleter(void const* data)
	{
		T const* p = static_cast<T const*>(data);

#ifdef _DEBUG
		std::stringstream ss;
		ss << "Service Located at: ";
		ss << p;
		ss << " is being deleted";
		LogInfo(ss.str());
#endif

		delete p;
	}


	inline static std::unordered_map<std::type_index, std::unique_ptr<void, void (*)(void const*)>> m_ServiceInstances{};
};


class ServiceConfigurator 
{
public:
	ServiceConfigurator() = default;
	~ServiceConfigurator() = default;
	ServiceConfigurator(const ServiceConfigurator&) = delete;
	ServiceConfigurator& operator=(const ServiceConfigurator&) = delete;
	ServiceConfigurator(ServiceConfigurator&&) = delete;
	ServiceConfigurator& operator=(ServiceConfigurator&&) = delete;

	static void Configure()
	{
		ServiceLocator::RegisterService<VulkanContext>("Vulkan Shader Editor");
	    LogInfo("Services Configured");
	}
};