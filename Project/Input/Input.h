#pragma once
#include <algorithm>
#include <functional>
#include <glm/vec2.hpp>
#include "vulkanbase/VulkanTypes.h"
#include "Core/Logger.h"

namespace Input
{
	inline GLFWwindow* windowPtr;

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
			//if (keyType == KeyType::Press) return true;
			//if (keyType != other.keyType) return false;
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
	inline std::vector<std::function<void(glm::vec2)>> mouseMovementListeners;


	inline void KeyEvent(int key, int action)
	{
		const InputProperties keyProps{ key,static_cast<KeyType>(action)};

		const auto& binding = functionBindings.find(keyProps);

		if (binding != functionBindings.end()) binding->second();

	}

	inline void MouseMove(const double xpos, const double ypos)
	{
		const glm::vec2 mousePos{ xpos, ypos };
		for (const auto& listener : mouseMovementListeners)
		{
			listener(mousePos);
		}
	}

	inline void BindFunction(const InputProperties& inputProps ,std::function<void()>&& function)
	{
		functionBindings.emplace(inputProps, std::move(function));
	}

	inline void AddMouseMovementListener(std::function<void(glm::vec2)>&& function)
	{
		mouseMovementListeners.push_back(std::move(function));
	}

	inline void SetupInput(GLFWwindow* window)
	{
		windowPtr = window;

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			KeyEvent(key, action);
		});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) 
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddMousePosEvent(xpos, ypos);

			//if(io.WantCaptureMouse) return;

			MouseMove(xpos, ypos);
		});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) 
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(button, action);

			//if (io.WantCaptureMouse) return;

			KeyEvent(button, action);
		});
	}

	inline glm::vec2 GetMousePosition()
	{
		double xpos, ypos;
		glfwGetCursorPos(windowPtr, &xpos, &ypos);
		return { xpos, ypos };
	}

	inline bool IsKeyPressed(int key)
	{
		const int state = glfwGetKey(windowPtr, key);
		return  state == GLFW_PRESS;
	}

	inline bool IsMouseButtonPressed(int button)
	{
		const int state = glfwGetMouseButton(windowPtr, button);
		return state == GLFW_PRESS;
	}
}