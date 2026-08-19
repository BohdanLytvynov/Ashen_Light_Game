#pragma once

#include "CoreMinimal.h"

template<typename T>
class FRectMatrix
{
private:
	int32 Size = 0;
	TArray<T> MatrixData;

public:
	FRectMatrix() = default;

	explicit FRectMatrix(int32 InSize, const T& DefaultValue = T())
		: Size(InSize)
	{
		if (Size > 0)
		{
			MatrixData.Init(DefaultValue, Size * Size);
		}
	}

	int32 GetSize() const { return Size; }

	FORCEINLINE bool IsValidIndex(int32 i, int32 j) const
	{
		return i >= 0 && i < Size && j >= 0 && j < Size;
	}

	T* Get(int32 i, int32 j)
	{
		const int32 Index = i * Size + j;
		return MatrixData.IsValidIndex(Index) ? &MatrixData[Index] : nullptr;
	}

	const T* Get(int32 i, int32 j) const
	{
		const int32 Index = i * Size + j;
		return MatrixData.IsValidIndex(Index) ? &MatrixData[Index] : nullptr;
	}

	void Set(const T& Data, int32 i, int32 j)
	{
		if (IsValidIndex(i, j))
		{
			MatrixData[i * Size + j] = Data;
		}
	}
	template<class Enum>
	const T* Get(Enum i, Enum j) const
	{
		const int32 Index = static_cast<int32>(i) * Size + static_cast<int32>(j);
		return MatrixData.IsValidIndex(Index) ? &MatrixData[Index] : nullptr;
	}

	template<class Enum>
	void Set(const T& Data, Enum i, Enum j)
	{
		const int32 it = static_cast<int32>(i);
		const int32 jt = static_cast<int32>(j);
		if (IsValidIndex(it, jt))
		{
			MatrixData[it * Size + jt] = Data;
		}
	}
};