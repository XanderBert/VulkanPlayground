#pragma once


class Material final
{
public:
	Material() = default;
	~Material() = default;
	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	Material(Material&&) = delete;
	Material& operator=(Material&&) = delete;

};