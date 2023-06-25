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

		/////////////////////////////////////////////
		///
		RenderSortingHash(const MeshRenderObject& p_renderObject)
		{

			std::uint32_t materialHash = std::hash<Resources::MaterialPtr>{}(p_renderObject.material);
			std::uint8_t renderOrder = p_renderObject.material->GetRenderOrder();

			m_hash = (uint64_t)materialHash << 32 | (uint64_t)50 << 24;
		}

		void SetDepth(uint32_t p_depth)
		{
			m_hash |= p_depth >> 8;
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

		bool operator<(const RenderSortingHash& rhs) const
		{
			return this->m_hash < rhs.m_hash;
		}

		bool operator>(const RenderSortingHash& rhs) const
		{
			return this->m_hash > rhs.m_hash;
		}

		bool operator==(const RenderSortingHash& rhs) const
		{
			return this->m_hash == rhs.m_hash;
		}

	private:
		std::uint64_t m_hash;
	};

	template<class Object, class SortingKey = RenderSortingHash, class Compare = std::less<SortingKey>>
	class RenderBuffer {
		using compare_func = Compare;
		using ObjectBuffer = std::vector<Object>;
		using ObjectPair = std::pair<SortingKey, Object*>;
	public:
		using ObjectPriorityVector = std::vector<ObjectPair>;
		using iterator = typename ObjectPriorityVector::iterator;
		using const_iterator = typename ObjectPriorityVector::const_iterator;
		using reverse_iterator = typename ObjectPriorityVector::reverse_iterator;
		using const_reverse_iterator = typename ObjectPriorityVector::const_reverse_iterator;
		using reference = typename ObjectPriorityVector::reference;
		using const_reference = typename ObjectPriorityVector::const_reference;

	public:
		class CompareHelper {
		public:
			CompareHelper(const compare_func& p_compare) :
				m_compare(p_compare)
			{}

			bool operator()(const ObjectPair& a, const ObjectPair& b) const
			{
				return m_compare.operator()(a.first, b.first);
			}

		private:
			const compare_func& m_compare;
		};

		RenderBuffer();
		RenderBuffer(const compare_func& p_compare);
		RenderBuffer(const compare_func& p_compare, size_t p_capcity);

		~RenderBuffer() = default;

		//Vector Fuctions
		//Element access
		inline reference operator[](size_t p_pos) { return m_objectsPriority[p_pos]; }
		inline const_reference operator[](size_t p_pos) const { return m_objectsPriority[p_pos]; }
		inline reference front() { return m_objectsPriority.front(); }
		inline const_reference front() const { return m_objectsPriority.front(); }
		inline reference back() { return m_objectsPriority.back(); }
		inline const_reference back() const { return m_objectsPriority.back(); }
		inline Object* data() { return m_objectsPriority.data(); }
		inline const Object* data() const { return m_objectsPriority.data(); }

		//Iterators
		inline iterator begin() { return m_objectsPriority.begin(); }
		inline const_iterator cbegin() const { return m_objectsPriority.cbegin(); }
		inline iterator end() { return m_objectsPriority.end(); }
		inline const_iterator cend() const { return m_objectsPriority.cend(); }

		inline reverse_iterator rbegin() { return m_objectsPriority.rbegin(); }
		inline const_reverse_iterator crbegin() const { return m_objectsPriority.crbegin(); }
		inline reverse_iterator rend() { return m_objectsPriority.rend(); }
		inline const_reverse_iterator crend() const { return m_objectsPriority.crend(); }

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
		void push_back(const SortingKey& p_hash, Object& p_renderObject);
		void push_back(SortingKey&& p_hash, Object&& p_renderObject);
		void pop_back();

		//Sorting
		void Sort();
		Compare key_compare();

	private:
		ObjectBuffer m_objectsBuffer;
		ObjectPriorityVector m_objectsPriority;
	};

	template<class object, class sortingKey, class compare>
	void RenderBuffer<object, sortingKey, compare>::clear()
	{
		m_objectsPriority.clear();
		m_objectsBuffer.clear();
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderBuffer<Object, SortingKey, Compare>::iterator RenderBuffer<Object, SortingKey, Compare>::erase(iterator p_pos)
	{
		return m_objectsPriority.erase(p_pos);
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderBuffer<Object, SortingKey, Compare>::iterator RenderBuffer<Object, SortingKey, Compare>::erase(iterator p_first, iterator p_last)
	{
		return m_objectsPriority.erase(p_first, p_last);
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderBuffer<Object, SortingKey, Compare>::iterator RenderBuffer<Object, SortingKey, Compare>::erase(const_iterator p_pos)
	{
		return m_objectsPriority.erase(p_pos);
	}

	template<class Object, class SortingKey, class Compare>
	typename RenderBuffer<Object, SortingKey, Compare>::iterator RenderBuffer<Object, SortingKey, Compare>::erase(const_iterator p_first, const_iterator p_last)
	{
		return m_objectsPriority.erase(p_first, p_last);
	}

	template<class Object, class SortingKey, class Compare>
	void RenderBuffer<Object, SortingKey, Compare>::push_back(const SortingKey& p_hash, Object& p_renderObject)
	{
		m_objectsBuffer.push_back(p_renderObject);
		m_objectsPriority.push_back({ p_hash, &m_objectsBuffer.back() });
	}

	template<class Object, class SortingKey, class Compare>
	void RenderBuffer<Object, SortingKey, Compare>::push_back(SortingKey&& p_hash, Object&& p_renderObject)
	{
		m_objectsBuffer.push_back(std::move(p_renderObject));
		m_objectsPriority.push_back({ std::move(p_hash), &m_objectsBuffer.back() });
	}

	template<class Object, class SortingKey, class Compare>
	void RenderBuffer<Object, SortingKey, Compare>::pop_back()
	{
		m_objectsPriority.pop_back();
	}

	template<class Object, class SortingKey, class Compare>
	void RenderBuffer<Object, SortingKey, Compare>::Sort()
	{
		std::sort(m_objectsPriority.begin(), m_objectsPriority.end(), CompareHelper(Compare()));
	}

	template<class Object, class SortingKey, class Compare>
	Compare RenderBuffer<Object, SortingKey, Compare>::key_compare()
	{
		return static_cast<Compare>(*this);
	}

	template<class Object, class SortingKey, class Compare>
	RenderBuffer<Object, SortingKey, Compare>::RenderBuffer()
	{
	}

	template<class Object, class SortingKey, class Compare>
	RenderBuffer<Object, SortingKey, Compare>::RenderBuffer(const compare_func& p_compare) :
		compare_func(p_compare)
	{
	}

	template<class Object, class SortingKey, class Compare>
	RenderBuffer<Object, SortingKey, Compare>::RenderBuffer(const compare_func& p_compare, size_t p_capcity) :
		compare_func(p_compare)
	{
		m_objectsBuffer.reserve(p_capcity);
		m_objectsPriority.reserve(p_capcity);
	}

	template<class Object, class SortingKey, class Compare>
	void RenderBuffer<Object, SortingKey, Compare>::empty()
	{
		m_objectsPriority.empty();
	}

	template<class Object, class SortingKey, class Compare>
	size_t RenderBuffer<Object, SortingKey, Compare>::size()
	{
		return m_objectsPriority.size();
	}

	template<class Object, class SortingKey, class Compare>
	void RenderBuffer<Object, SortingKey, Compare>::reserve(size_t p_newCap)
	{
		m_objectsPriority.reserve(p_newCap);
		m_objectsBuffer.reserve(p_newCap);
	}

	template<class Object, class SortingKey, class Compare>
	size_t RenderBuffer<Object, SortingKey, Compare>::capacity()
	{
		return m_objectsPriority.capacity();
	}
}