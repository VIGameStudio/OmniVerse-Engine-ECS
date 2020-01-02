#pragma once

#include <ove/core/container/pool.hpp>
#include <ove/core/util/guard.hpp>
#include <ove/core/util/event.hpp>
#include <ove/core/util/assert.hpp>

namespace ove
{
	namespace core
	{
		struct cmp_base_t
		{
		protected:
			static size_t regId()
			{
				static size_t s_cmpCounter = -1;
				return ++s_cmpCounter;
			}
		};

		template <typename E>
		struct cmp_t : public cmp_base_t
		{
			static size_t id()
			{
				static const size_t s_id = regId();
				return s_id;
			}
		};

		struct entity_mgr_t;

		struct entity_t
		{
		private:
			friend struct entity_mgr_t;
			entity_t(size_t id, entity_mgr_t& entityMgr) : m_id(id), m_entityMgr(entityMgr) {}

		public:
			template <typename C> inline void addComponent(const C& cmp) const;

			template <typename C, typename... Args>
			inline void addComponent(Args... args) const;

			template <typename C> inline bool hasComponent() const;

			template <typename C> inline C& getComponent() const;

			template <typename C> inline void removeComponent() const;

			inline size_t id() const { return m_id; }

		private:
			size_t m_id;
			entity_mgr_t& m_entityMgr;
			std::vector<size_t> m_cmps;
		};

		struct entity_mgr_t
		{
		private:
			template <typename C> using cmp_pool_t = pool_t<std::pair<size_t, C>>;
			using cmp_map_t = std::unordered_map<size_t, pool_base_t*>;

			template <typename C>
			inline cmp_pool_t<C>& getPool()
			{
				auto handle = static_cast<cmp_pool_t<C>*>(m_cmpMap[cmp_t<C>::id()]);
				if (handle == nullptr)
				{
					handle = new cmp_pool_t<C>();
					m_cmpMap[cmp_t<C>::id()] = handle;
				}
				return *handle;
			}

		public:
			entity_t createEntity()
			{
				static size_t s_entityCounter = -1;
				return entity_t(++s_entityCounter, *this);
			}

			void addEntity(entity_t e) { m_entities.emplace_back(e); }

			void removeEntity(entity_t e) {}

			template <typename C>
			void addComponent(entity_t e, const C& cmp)
			{
				auto& poolHandle = getPool<C>();
				poolHandle.add(std::make_pair(e.id(), cmp));
			}

			template <typename C, typename... Args>
			void addComponent(entity_t e, Args... args)
			{
				auto& poolHandle = getPool<C>();
				poolHandle.add(std::make_pair(e.id(), C(args...)));
			}

			template <typename C>
			bool hasComponent(entity_t e)
			{
				auto& poolHandle = getPool<C>();
				for (auto data : poolHandle.data())
				{
					if (data.first && data.second.first == e.id())
					{
						return true;
					}
				}
				return false;
			}

			template <typename C>
			C& getComponent(entity_t e)
			{
				auto& poolHandle = getPool<C>();
				for (auto& data : poolHandle.data())
				{
					if (data.first && data.second.first == e.id())
					{
						return data.second.second;
					}
				}
				ASSERT(false && "Entity doesn't have component!");
			}

			template <typename C>
			void removeComponent(entity_t e)
			{
				auto poolHandle = getPool<C>();
				for (auto data : poolHandle.data())
				{
					if (data.first == e.id())
					{
						poolHandle.remove(data);
					}
				}
			}

			inline std::vector<entity_t>& entities() { return m_entities; }

		private:
			cmp_map_t m_cmpMap;
			std::vector<entity_t> m_entities;
		};

		template <typename C>
		inline void entity_t::addComponent(const C& cmp) const
		{
			m_entityMgr.addComponent(id(), cmp);
		}

		template <typename C, typename... Args>
		inline void entity_t::addComponent(Args... args) const
		{
			m_entityMgr.addComponent<C>(*this, std::forward<Args>(args)...);
		}

		template <typename C>
		inline bool entity_t::hasComponent() const
		{
			return m_entityMgr.hasComponent<C>(*this);
		}

		template <typename C>
		inline C& entity_t::getComponent() const
		{
			return m_entityMgr.getComponent<C>(*this);
		}

		template <typename C>
		inline void entity_t::removeComponent() const
		{
			m_entityMgr.removeComponent<C>();
		}
	}
}