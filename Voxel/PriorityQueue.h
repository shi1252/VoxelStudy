#pragma once
#include <vector>

template <typename T>
class PriorityQueue
{
public:
	PriorityQueue(uint32_t size = 32)
		: size(size)
	{
		data = new T[size];
	}

	~PriorityQueue()
	{
		delete[] data;
	}

	void Enqueue(T item)
	{
		if (count >= size)
		{
			size = size * 2;
			T* temp = new T[size];
			std::copy(data, data + count, temp);
			delete[] data;
			data = temp;
		}

		data[count++] = item;
		int i = count - 1;

		while (i > 0)
		{
			int j = (i - 1) / 2;
			int compare = data[i].Compare(data[j]);
			if (compare > 0) break;
			Swap(i, j);
			i = j;
		}
	}

	T Dequeue()
	{
		if (count <= 0)
			return T();

		T ret = data[0];
		int last = count - 1;
		data[0] = data[last];
		count--;

		last--;
		int i = 0;
		if (count > 0)
		{
			while (true)
			{
				int lc = i * 2 + 1;
				if (lc > last) break;
				int rc = lc + 1;
				if (rc <= last && data[rc].Compare(data[lc]) < 1)
					lc = rc;
				int compare = data[lc].Compare(data[i]);
				if (compare > 0) break;
				Swap(i, lc);
				i = lc;
			}
		}
		return ret;
	}

	void Swap(int n1, int n2)
	{
		T temp = data[n1];
		data[n1] = data[n2];
		data[n2] = temp;
	}

	int Count() const { return count; }

	bool Contains(T item)
	{
		for (int i = 0; i < count; ++i)
		{
			if (data[i] == item)
				return true;
		}
		return false;
	}

private:
	T* data;
	unsigned int count = 0;
	unsigned int size = 0;
};