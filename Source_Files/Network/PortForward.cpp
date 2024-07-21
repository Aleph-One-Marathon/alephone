#include "PortForward.h"

#ifdef HAVE_MINIUPNPC

#include <sstream>

#include <miniupnpc/upnpcommands.h>

PortForward::PortForward(uint16_t port) :
	port_{std::to_string(port)},
	url_freer_{nullptr, FreeUPNPUrls}
{
	int error = 0;

	devlist_freer_t devlist(
		upnpDiscover(2000, nullptr, nullptr,
					 UPNP_LOCAL_PORT_ANY, false, 2, &error),
		freeUPNPDevlist);

	if (!devlist)
	{
		throw PortForwardException("Failed to discover IGD");
	}

	char lanaddr[64];

#if MINIUPNPC_API_VERSION >= 18
	auto igd_found = UPNP_GetValidIGD(devlist.get(), &urls_, &data_, lanaddr, sizeof(lanaddr), nullptr, 0);
#else
	auto igd_found = UPNP_GetValidIGD(devlist.get(), &urls_, &data_, lanaddr, sizeof(lanaddr));
#endif
	if (!igd_found)
	{
		throw PortForwardException("Failed to discover IGD");
	}

	auto url_freer = url_freer_t(&urls_, FreeUPNPUrls);
	if (igd_found != 1)
	{
		throw PortForwardException("Failed to discover IGD");
	}

	auto result = UPNP_AddPortMapping(urls_.controlURL,
									  data_.first.servicetype,
									  port_.c_str(), port_.c_str(), lanaddr,
									  "Aleph One", "TCP", nullptr, nullptr);

	if (result != UPNPCOMMAND_SUCCESS)
	{
		std::ostringstream error;
		error << "Failed to map port " << port << " (TCP)";
		throw PortForwardException(error.str().c_str());
	}

	result = UPNP_AddPortMapping(urls_.controlURL,
								 data_.first.servicetype,
								 port_.c_str(), port_.c_str(), lanaddr,
								 "Aleph One", "UDP", nullptr, nullptr);

	if (result != UPNPCOMMAND_SUCCESS)
	{
		UPNP_DeletePortMapping(urls_.controlURL,
							   data_.first.servicetype,
							   port_.c_str(),
							   "TCP", nullptr);

		std::ostringstream error;
		error << "Failed to map port " << port_ << " (UDP)";
		throw PortForwardException(error.str().c_str());
	}

	url_freer_ = std::move(url_freer);
}

PortForward::~PortForward()
{
	UPNP_DeletePortMapping(urls_.controlURL,
						   data_.first.servicetype,
						   port_.c_str(),
						   "TCP", nullptr);

	UPNP_DeletePortMapping(urls_.controlURL,
						   data_.first.servicetype,
						   port_.c_str(),
						   "UDP", nullptr);
}

#endif
