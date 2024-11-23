#pragma once
#include <array>

template <std::size_t BufferSize>
class CircularBuffer
{
public:
	CircularBuffer() = default;
	~CircularBuffer() = default;

	CircularBuffer(const CircularBuffer&) = delete;
	CircularBuffer& operator=(const CircularBuffer&) = delete;
	CircularBuffer(CircularBuffer&&) = delete;
	CircularBuffer& operator=(CircularBuffer&&) = delete;

	void Push(float value)
	{
		data[head] = value;
		head = (head + 1) % BufferSize; // Circular increment

		if (size < BufferSize)
		{
			size++;
		}
	}

	const float* Data() const
	{
		return data.data();
	}

	[[nodiscard]] std::size_t Size() const
	{
		return size;
	}

private:
	std::array<float, BufferSize> data;
	std::size_t head; // Index for the next element to overwrite
	std::size_t size; // Current size of valid data in the buffer
};


