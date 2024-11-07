#pragma once

struct IUnknown;

// This struct acts as a smart pointer for IUnknown pointers making sure to call AddRef() and Release() as needed.
template <typename T>
struct ComPtr
{
	ComPtr(T* lComPtr = nullptr) : m_comPtr(lComPtr)
	{
		static_assert(std::is_base_of<IUnknown, T>::value, "T needs to be IUnknown based");
		if (m_comPtr) m_comPtr->AddRef();
	}

	ComPtr(const ComPtr<T>& lComPtrObj)
	{
		static_assert(std::is_base_of<IUnknown, T>::value, "T needs to be IUnknown based");
		m_comPtr = lComPtrObj.m_comPtr;
		if (m_comPtr) m_comPtr->AddRef();
	}

	ComPtr(ComPtr<T>&& lComPtrObj)
	{
		m_comPtr = lComPtrObj.m_comPtr;
		lComPtrObj.m_comPtr = nullptr;
	}

	T* operator=(T* lComPtr)
	{
		if (m_comPtr) m_comPtr->Release();
		m_comPtr = lComPtr;
		if (m_comPtr) m_comPtr->AddRef();
		return m_comPtr;
	}

	T* operator=(const ComPtr<T>& lComPtrObj)
	{
		if (m_comPtr) m_comPtr->Release();
		m_comPtr = lComPtrObj.m_comPtr;
		if (m_comPtr) m_comPtr->AddRef();

		return m_comPtr;
	}

	~ComPtr()
	{
		Reset();
	}

	T** operator&()
	{
		// The assert on operator& usually indicates a bug. Could be a potential memory leak.
		// If this really what is needed, however, use GetInterface() explicitly.
		assert(m_comPtr == nullptr);
		return &m_comPtr;
	}

	operator T*() const { return m_comPtr;}

	T* GetInterface() const { return m_comPtr; }

	T& operator*() const { return *m_comPtr; }
	T* operator->() const { return m_comPtr; }

	bool operator!() const { return (nullptr == m_comPtr); }
	bool operator<(T* lComPtr) const { return m_comPtr < lComPtr; }
	bool operator!=(T* lComPtr) const { return !operator==(lComPtr); }
	bool operator==(T* lComPtr) const { return m_comPtr == lComPtr; }

	void Reset()
	{
		if (m_comPtr)
		{
			m_comPtr->Release();
			m_comPtr = nullptr;
		}
	}

private:
	T* m_comPtr;
};