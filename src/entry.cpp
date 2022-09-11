#include <clap/clap.h>
#include <string>

#include "constants.h"
#include "AddSyn.h"

namespace addsyn::entry {
	uint32_t get_plugin_count(const clap_plugin_factory*) { 
		return 1;
	}

	const clap_plugin_descriptor* get_descriptor(const clap_plugin_factory* pFactory, uint32_t id) {
		return &addsyn::AddSynPlugin::getDescriptor();
	}

	static const clap_plugin* create_plugin(const clap_plugin_factory* pFactory, const clap_host* pHost, const char* pluginId) {
		if (pFactory == nullptr || pHost == nullptr) 
			return nullptr;

		std::string factoryPluginId = pFactory->get_plugin_descriptor(pFactory, 1)->id;
		if(factoryPluginId.compare(pluginId) != 0) 
			return nullptr;

		// memory managed by host
		auto pInstance = new AddSynPlugin(pHost);
		return pInstance->clapPlugin();
	}

	const CLAP_EXPORT struct clap_plugin_factory factory = {
		get_plugin_count,
		get_descriptor,
		create_plugin
	};
	
	static const void* get_factory(const char* factoryId) {
		return &factory;
	}

	bool init(const char*) {
		return true;
	}

	void deinit() {}
}

extern "C" {
	const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
		CLAP_VERSION,
		addsyn::entry::init,
		addsyn::entry::deinit,
		addsyn::entry::get_factory
	};
}