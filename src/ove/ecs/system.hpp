#pragma once

#include <ove/core/util/event.hpp>
#include <ove/core/util/types.hpp>
#include <ove/ecs/entity.hpp>

namespace ove
{
	namespace ecs
	{
		struct sys_base_t
		{
			virtual void init(entity_mgr_t& es) = 0;
			virtual void update(entity_mgr_t& es, float dt) = 0;
			virtual void render(entity_mgr_t& es) = 0;
			virtual void clean(entity_mgr_t& es) = 0;

		protected:
			static size_t regId()
			{
				static size_t s_sysCounter = -1;
				return ++s_sysCounter;
			}
		};

		template <typename E>
		struct sys_t : public sys_base_t
		{
			static size_t id()
			{
				static const size_t s_id = regId();
				return s_id;
			}

			inline event_bus_t& getEventBus() { return *m_eventBus; }

		private:
			friend struct system_mgr_t;
			event_bus_t* m_eventBus;
		};

		struct sys_mgr_t {
		public:
			sys_mgr_t(event_bus_t& eventBus, entity_mgr_t& entityMgr)
				: m_eventBus(eventBus), m_entityMgr(entityMgr) {}

			template <typename S, typename... Args>
			void addSys(Args... args)
			{
				auto sys = new S(args...);
				sys->m_eventBus = &m_eventBus;
				m_systems.emplace_back(sys);
			}

			template <typename Sys>
			void removeSys()
			{
				// TODO
			}

			inline void init()
			{
				for (auto& sys : m_systems)
				{
					sys->init(m_entityMgr);
				}
			}

			inline void update(f32 dt)
			{
				for (auto& sys : m_systems)
				{
					sys->update(m_entityMgr, dt);
				}
			}

			inline void render()
			{
				for (auto& sys : m_systems)
				{
					sys->render(m_entityMgr);
				}
			}

			inline void clean()
			{
				for (auto& sys : m_systems)
				{
					sys->clean(m_entityMgr);
				}
			}

		private:
			event_bus_t& m_eventBus;
			entity_mgr_t& m_entityMgr;
			std::vector<sys_base_t*> m_systems;
		};
	}
}