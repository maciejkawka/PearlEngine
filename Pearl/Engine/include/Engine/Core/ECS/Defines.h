#pragma once
#include<cstdint>
#include<bitset>

namespace PrCore::ECS {

	constexpr unsigned int MAX_ENTITIES = 10000;
	constexpr unsigned int MAX_COMPONENTS = 64;

	using ComponentSignature = std::bitset<MAX_COMPONENTS>;

	//ID wrapps version and Index
	//       ________________________
	//		|		ID STRUCTURE	 |
	//      |---------64 bit---------|
	//      |  32 bit   |   32 bit   |
	//		|  INDEX    |   VERSION  |
	//      |________________________|
	struct ID
	{
		ID();
		explicit ID(uint64_t p_ID);
		explicit ID(uint32_t p_index, uint32_t p_version);

		ID(const ID& p_ID) = default;
		ID& operator = (const ID& p_ID) = default;

		inline uint32_t GetIndex() const { return m_ID >> 32; }
		inline uint32_t GetVersion() const { return uint32_t(m_ID); }
		inline uint32_t GetID() const { return m_ID; }

		inline bool IsValid() const { return m_ID >> 32 != 0; }

		inline bool operator ==(const ID& p_ID) const { return p_ID.m_ID == m_ID; }
		inline bool operator !=(const ID& p_ID) const { return p_ID.m_ID != m_ID; }

	private:
		uint64_t m_ID;

#ifdef _DEBUG
		uint32_t DEBUG_INDEX;
		uint32_t DEBUG_VERSION;
#endif
	};

	const ID INVALID_ID;
}

namespace std {

	template<>
	struct hash<PrCore::ECS::ID>
	{
		inline size_t operator()(const PrCore::ECS::ID& p_ID) const {
			return p_ID.GetID();
		}
	};
}