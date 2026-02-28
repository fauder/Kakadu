#pragma once

namespace Kakadu
{
	template< class Service >
	class ServiceLocator
	{
	public:
		static Service& Get() { return *service; }
		static void Register( Service* service_to_register ) { service = service_to_register; }

	private:
		inline static Service* service = nullptr;
	};
}