#pragma once

#include"Renderer/Core/MeshRenderObject.h"

#include<vector>
#include<utility>

namespace PrRenderer::Core {

	class RenderSortingHash {
	public:
		//Sorting Structs
		struct TransparenctySort
		{
			bool operator()(const RenderSortingHash& a, const RenderSortingHash& b) const
			{
				return a.GetDepth() > b.GetDepth();
			}
		};

		RenderSortingHash(const MeshRenderObject& p_renderObject)
		{

			size_t materialHash = std::hash<Resources::Materialv2Ptr>{}(p_renderObject.material);
			size_t renderOrder = p_renderObject.material->GetRenderOrder();

			m_hash = materialHash << 32 | renderOrder << 24;
		}

		inline void SetDepth(size_t p_depth)
		{
			m_hash |= p_depth & 0xFFFFFF;
		}

		inline size_t GetRenderOrder() const
		{
			return (m_hash & 0xFF000000) >> 24;
		}
		inline size_t GetMaterialHash() const
		{
			return m_hash >> 32;
		}
		inline size_t GetDepth() const
		{
			return m_hash & 0xFFFFFF;
		}

		inline bool operator<(const RenderSortingHash& rhs) const
		{
			return this->m_hash < rhs.m_hash;
		}

		inline bool operator>(const RenderSortingHash& rhs) const
		{
			return this->m_hash > rhs.m_hash;
		}

		inline bool operator==(const RenderSortingHash& rhs) const
		{
			return this->m_hash == rhs.m_hash;
		}

	private:
		std::uint64_t m_hash;
	};

	template<class RenderObject, class SortingHash = RenderSortingHash, class Compare = std::less<SortingHash>>
	class RenderObjectsBuffer {
		using compare_func = Compare;
		using ObjectStorage = std::vector<RenderObject>;
		using BufferPair = std::pair<SortingHash, RenderObject*>;
	public:
		using ObjectBuffer = std::vector<BufferPair>;
		using key_type = SortingHash;
		using value_type = RenderObject;
		using iterator = typename ObjectBuffer::iterator;
		using const_iterator = typename ObjectBuffer::const_iterator;
		using reverse_iterator = typename ObjectBuffer::reverse_iterator;
		using const_reverse_iterator = typename ObjectBuffer::const_reverse_iterator;
		using reference = typename ObjectBuffer::reference;
		using const_reference = typename ObjectBuffer::const_reference;

	private:
		class CompareHelper {
		public:
			CompareHelper(const compare_func& p_compare) :
				m_compare(p_compare)
			{}

			bool operator()(const BufferPair& a, const BufferPair& b) const
			{
				return m_compare.operator()(a.first, b.first);
			}

		private:
			const compare_func& m_compare;
		};

	public:
		RenderObjectsBuffer();
		RenderObjectsBuffer(const compare_func& p_compare);
		RenderObjectsBuffer(const compare_func& p_compare, size_t p_capcity);

		~RenderObjectsBuffer() = default;

		//Vector Fuctions
		//Element access
		inline reference operator[](size_t p_pos) { return m_objectBuffer[p_pos]; }
		inline const_reference operator[](size_t p_pos) const { return m_objectBuffer[p_pos]; }
		inline reference front() { return m_objectBuffer.front(); }
		inline const_reference front() const { return m_objectBuffer.front(); }
		inline reference back() { return m_objectBuffer.back(); }
		inline const_reference back() const { return m_objectBuffer.back(); }
		inline RenderObject* data() { return m_objectBuffer.data(); }
		inline const RenderObject* data() const { return m_objectBuffer.data(); }

		//Iterators
		inline iterator begin() { return m_objectBuffer.begin(); }
		inline const_iterator cbegin() const { return m_objectBuffer.cbegin(); }
		inline iterator end() { return m_objectBuffer.end(); }
		inline const_iterator cend() const { return m_objectBuffer.cend(); }

		inline reverse_iterator rbegin() { return m_objectBuffer.rbegin(); }
		inline const_reverse_iterator crbegin() const { return m_objectBuffer.crbegin(); }
		inline reverse_iterator rend() { return m_objectBuffer.rend(); }
		inline const_reverse_iterator crend() const { return m_objectBuffer.crend(); }

		//Capacity
		void empty();
		size_t size();
		void reserve(size_t p_newCap);
		size_t capacity();

		//Modifiers
		void clear();
		iterator erase(iterator p_pos);
		iterator erase(iterator p_first, iterator p_last);
		iterator erase(const_iterator p_pos);
		iterator erase(const_iterator p_first, const_iterator p_last);
		void push_back(const SortingHash& p_hash, RenderObject& p_renderObject);
		void push_back(SortingHash&& p_hash, RenderObject&& p_renderObject);
		void pop_back();

		//Sorting
		void Sort();
		Compare key_compare();

	private:
		ObjectStorage m_objectStorage;
		ObjectBuffer m_objectBuffer;
	};

	template<class object, class sortingKey, class compare>
	void RenderObjectsBuffer<object, sortingKey, compare>::clear()
	{
		m_objectBuffer.clear();
		m_objectStorage.clear();
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderObjectsBuffer<Object, SortingKey, Compare>::iterator RenderObjectsBuffer<Object, SortingKey, Compare>::erase(iterator p_pos)
	{
		return m_objectBuffer.erase(p_pos);
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderObjectsBuffer<Object, SortingKey, Compare>::iterator RenderObjectsBuffer<Object, SortingKey, Compare>::erase(iterator p_first, iterator p_last)
	{
		return m_objectBuffer.erase(p_first, p_last);
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderObjectsBuffer<Object, SortingKey, Compare>::iterator RenderObjectsBuffer<Object, SortingKey, Compare>::erase(const_iterator p_pos)
	{
		return m_objectBuffer.erase(p_pos);
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderObjectsBuffer<Object, SortingKey, Compare>::iterator RenderObjectsBuffer<Object, SortingKey, Compare>::erase(const_iterator p_first, const_iterator p_last)
	{
		return m_objectBuffer.erase(p_first, p_last);
	}

	template<class Object, class SortingKey, class Compare>
	void RenderObjectsBuffer<Object, SortingKey, Compare>::push_back(const SortingKey& p_hash, Object& p_renderObject)
	{
		m_objectStorage.push_back(p_renderObject);
		m_objectBuffer.push_back({ p_hash, &m_objectStorage.back() });
	}

	template<class Object, class SortingKey, class Compare>
	void RenderObjectsBuffer<Object, SortingKey, Compare>::push_back(SortingKey&& p_hash, Object&& p_renderObject)
	{
		m_objectStorage.push_back(std::move(p_renderObject));
		m_objectBuffer.push_back({ std::move(p_hash), &m_objectStorage.back() });
	}

	template<class Object, class SortingKey, class Compare>
	void RenderObjectsBuffer<Object, SortingKey, Compare>::pop_back()
	{
		m_objectBuffer.pop_back();
	}

	template<class Object, class SortingKey, class Compare>
	void RenderObjectsBuffer<Object, SortingKey, Compare>::Sort()
	{
		std::sort(m_objectBuffer.begin(), m_objectBuffer.end(), CompareHelper(Compare()));
	}

	template<class Object, class SortingKey, class Compare>
	Compare RenderObjectsBuffer<Object, SortingKey, Compare>::key_compare()
	{
		return static_cast<Compare>(*this);
	}

	template<class Object, class SortingKey, class Compare>
	RenderObjectsBuffer<Object, SortingKey, Compare>::RenderObjectsBuffer()
	{
	}

	template<class Object, class SortingKey, class Compare>
	RenderObjectsBuffer<Object, SortingKey, Compare>::RenderObjectsBuffer(const compare_func& p_compare) :
		compare_func(p_compare)
	{
	}

	template<class Object, class SortingKey, class Compare>
	RenderObjectsBuffer<Object, SortingKey, Compare>::RenderObjectsBuffer(const compare_func& p_compare, size_t p_capcity) :
		compare_func(p_compare)
	{
		m_objectStorage.reserve(p_capcity);
		m_objectBuffer.reserve(p_capcity);
	}

	template<class Object, class SortingKey, class Compare>
	void RenderObjectsBuffer<Object, SortingKey, Compare>::empty()
	{
		m_objectBuffer.empty();
	}

	template<class Object, class SortingKey, class Compare>
	size_t RenderObjectsBuffer<Object, SortingKey, Compare>::size()
	{
		return m_objectBuffer.size();
	}

	template<class Object, class SortingKey, class Compare>
	void RenderObjectsBuffer<Object, SortingKey, Compare>::reserve(size_t p_newCap)
	{
		m_objectBuffer.reserve(p_newCap);
		m_objectStorage.reserve(p_newCap);
	}

	template<class Object, class SortingKey, class Compare>
	size_t RenderObjectsBuffer<Object, SortingKey, Compare>::capacity()
	{
		return m_objectBuffer.capacity();
	}
}