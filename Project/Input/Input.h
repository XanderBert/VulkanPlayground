#pragma once
#include <algorithm>
#include <functional>
#include <imgui_impl_glfw.h>
#include <glm/vec2.hpp>
#include "vulkanbase/VulkanTypes.h"
#include "Core/Logger.h"

namespace Input
{
	inline GLFWwindow* windowPtr;

	enum class InputType : uint8_t
	{
		Hold = GLFW_REPEAT,
		Press = GLFW_PRESS,
		Release = GLFW_RELEASE,
	};

	struct InputAction
	{
		int key;
		InputType keyType;
	    int mods;

		bool operator==(const InputAction& other) const
		{
			if (key != other.key) return false;
		    if(mods != other.mods) return false;
			return true;
		}
	};

	struct InputPropertiesHash
	{
		std::size_t operator()(const InputAction& props) const
		{
			std::size_t seed = 0;

			seed ^= std::hash<int>{}(props.key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= std::hash<uint8_t>{}(static_cast<uint8_t>(props.keyType)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	inline std::unordered_map<InputAction, std::function<void()>, InputPropertiesHash> functionBindings;
	inline std::vector<std::function<void(glm::vec2)>> mouseMovementListeners;


	inline void KeyEvent(int key, int action, int mods)
	{
		const InputAction keyProps{ key, static_cast<InputType>(action), mods};

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

	inline void BindFunction(const InputAction& inputProps ,std::function<void()>&& function)
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

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scanCode, int action, int mods)
		{
		    ImGui_ImplGlfw_KeyCallback(window, key, action, action, mods);
			KeyEvent(key, action, mods);
		});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
		{
		    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
		    if (ImGui::GetIO().WantCaptureMouse) return;

			MouseMove(xpos, ypos);
		});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
		{
			ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
			if (ImGui::GetIO().WantCaptureMouse) return;
			KeyEvent(button, action, mods);
		});

	    LogInfo("Input Setup");
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