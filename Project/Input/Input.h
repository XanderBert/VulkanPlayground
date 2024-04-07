#pragma once
#include <algorithm>
#include <functional>
#include "vulkanbase/VulkanTypes.h"

namespace Input
{
	enum class KeyType : uint8_t
	{
		Hold = GLFW_REPEAT,
		Press = GLFW_PRESS,
	};

	struct InputProperties
	{
		int key;
		KeyType keyType;

		bool operator==(const InputProperties& other) const
		{
			if (key != other.key) return false;
			if (keyType == KeyType::Press) return true;
			if (keyType != other.keyType) return false;
			return true;
		}
	};

	struct InputPropertiesHash
	{
		std::size_t operator()(const InputProperties& props) const
		{
			std::size_t seed = 0;

			seed ^= std::hash<int>{}(props.key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= std::hash<uint8_t>{}(static_cast<uint8_t>(props.keyType)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	inline std::unordered_map<InputProperties, std::function<void()>, InputPropertiesHash> functionBindings;



	inline void KeyEvent(int key, int scancode, int action, int mods)
	{
		const InputProperties keyProps{ key,static_cast<KeyType>(action)};

		const auto& binding = functionBindings.find(keyProps);

		if (binding != functionBindings.end()) binding->second();

	}

	inline void BindFunction(const InputProperties& inputProps ,std::function<void()>&& function)
	{
		functionBindings.emplace(inputProps, std::move(function));
	}

	inline void SetupInput(GLFWwindow* window)
	{
		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			KeyEvent(key, scancode, action, mods);
		});
	}
}