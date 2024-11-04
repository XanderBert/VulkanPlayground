#pragma once
#include <functional>
#include <vector>
#include <algorithm>

template<typename... Args>
class Delegate
{
public:
    using FunctionType = std::function<void(Args...)>;

    void Add(const FunctionType& function)
    {
        m_Listeners.emplace_back(function);
    }

    void AddLambda(const std::function<void(Args...)>& function)
    {
        m_Listeners.emplace_back(function);
    }

    void Remove(const FunctionType& function)
    {
        m_Listeners.erase(std::remove(m_Listeners.begin(), m_Listeners.end(), function), m_Listeners.end());
    }

    void Broadcast(Args... args)
    {
        for (const auto& listener : m_Listeners)
        {
            listener(args...);
        }
    }

private:
    std::vector<FunctionType> m_Listeners;
};