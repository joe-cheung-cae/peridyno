namespace dyno 
{
	template<typename T>
	void Array<T, DeviceType::GPU>::resize(const size_t n)
	{
		//		assert(n >= 1);
		if (NULL != m_data) clear();

		m_totalNum = n;
		cuSafeCall(cudaMalloc(&m_data, n * sizeof(T)));
	}

	template<typename T>
	void Array<T, DeviceType::GPU>::clear()
	{
		if (m_data != NULL)
		{
			cuSafeCall(cudaFree((void*)m_data));
		}

		m_data = NULL;
		m_totalNum = 0;
	}

	template<typename T>
	void Array<T, DeviceType::GPU>::reset()
	{
		cuSafeCall(cudaMemset((void*)m_data, 0, m_totalNum * sizeof(T)));
	}

	template<typename T>
	void Array<T, DeviceType::GPU>::assign(const Array<T, DeviceType::GPU>& src)
	{
		if (m_totalNum != src.size())
			this->resize(src.size());

		cuSafeCall(cudaMemcpy(m_data, src.begin(), src.size() * sizeof(T), cudaMemcpyDeviceToDevice));
	}

	template<typename T>
	void Array<T, DeviceType::GPU>::assign(const Array<T, DeviceType::CPU>& src)
	{
		if (m_totalNum != src.size())
			this->resize(src.size());

		cuSafeCall(cudaMemcpy(m_data, src.begin(), src.size() * sizeof(T), cudaMemcpyHostToDevice));
	}


	template<typename T>
	void Array<T, DeviceType::GPU>::assign(const std::vector<T>& src)
	{
		if (m_totalNum != src.size())
			this->resize(src.size());

		cuSafeCall(cudaMemcpy(m_data, src.data(), src.size() * sizeof(T), cudaMemcpyHostToDevice));
	}

	template<typename T>
	void Array<T, DeviceType::CPU>::resize(const size_t n)
	{
		m_data.resize(n);
	}

	template<typename T>
	void Array<T, DeviceType::CPU>::clear()
	{
		m_data.clear();
	}

	template<typename T>
	void Array<T, DeviceType::CPU>::reset()
	{
		memset((void*)m_data.data(), 0, m_data.size()*sizeof(T));
	}

	template<typename T>
	void Array<T, DeviceType::CPU>::assign(const Array<T, DeviceType::GPU>& src)
	{
		if (m_data.size() != src.size())
			this->resize(src.size());

		cuSafeCall(cudaMemcpy(this->begin(), src.begin(), src.size() * sizeof(T), cudaMemcpyDeviceToHost));
	}

	template<typename T>
	void Array<T, DeviceType::CPU>::assign(const Array<T, DeviceType::CPU>& src)
	{
		if (m_data.size() != src.size())
			this->resize(src.size());

		memcpy(this->begin(), src.begin(), src.size() * sizeof(T));
	}

}