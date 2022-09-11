#include "AddSyn.h"

#include <map>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>

#include "PartialOsc.h"

namespace {
	const char* features[] = { CLAP_PLUGIN_FEATURE_INSTRUMENT, CLAP_PLUGIN_FEATURE_STEREO, nullptr };

	static constexpr clap_plugin_descriptor descriptor = {
		CLAP_VERSION_INIT,
		PLUGIN_ID,
		PLUGIN_NAME,
		PLUGIN_VENDOR,
		PLUGIN_URL,
		PLUGIN_URL,
		PLUGIN_URL,
		ClapAdditiveSynth_VERSION,
		PLUGIN_DESCRIPTION,
		features
	};

	static const std::unordered_map<clap_id, const char*> paramIdToNameMap = {
		// ex. { 1, "Gain" }
		{ addsyn::AddSynPlugin::ParamId::Frequency, "Frequency" }
	};

	static constexpr clap_param_info_flags globalModulationFlags = CLAP_PARAM_IS_MODULATABLE | CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID | CLAP_PARAM_IS_MODULATABLE_PER_KEY;
}

namespace addsyn {
	struct AddSynPlugin::Impl {
		std::unordered_map<clap_id, double*> paramIdToValueMap;
		//std::vector<internal::PartialOsc> sineBank;
		internal::PartialOsc partialOsc; // test
	};

	AddSynPlugin::AddSynPlugin(const clap_host* pHost): clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>(&descriptor, pHost), m{ std::make_unique<Impl>() }
	{
	}

	AddSynPlugin::~AddSynPlugin()
	{
	}

	const clap_plugin_descriptor& AddSynPlugin::getDescriptor() noexcept
	{
		return descriptor;
	}

	bool AddSynPlugin::activate(double sampleRate, uint32_t minFrames, uint32_t maxFrames) noexcept
	{
		m->partialOsc.setSampleRate(static_cast<float>(sampleRate));
		return true;
	}

	clap_process_status AddSynPlugin::process(const clap_process* pProcess) noexcept
	{
		if (pProcess == nullptr) return CLAP_PROCESS_ERROR;
		if (pProcess->audio_outputs_count <= 0) return CLAP_PROCESS_SLEEP; // skip but dont report
		// static Sine at 440hz with a gain mult of 20%		

		
		m->partialOsc.process(
			pProcess->audio_outputs->channel_count, 
			pProcess->frames_count,
			pProcess->audio_outputs[0].data32
		);

		return CLAP_PROCESS_CONTINUE;
	}

	bool AddSynPlugin::startProcessing() noexcept
	{
		return true;
	}

	void AddSynPlugin::stopProcessing() noexcept
	{
	}
	
	bool AddSynPlugin::isValidParamId(clap_id paramId) const noexcept
	{
		return m->paramIdToValueMap.find(paramId) != m->paramIdToValueMap.end();
	}

	uint32_t AddSynPlugin::paramsCount() const noexcept
	{
		return m->paramIdToValueMap.size();
	}
	
	bool AddSynPlugin::paramsInfo(uint32_t paramIndex, clap_param_info* pInfo) const noexcept
	{
		if(paramIndex >= m->paramIdToValueMap.size())
			return false;

		pInfo->flags = CLAP_PARAM_IS_AUTOMATABLE;

		switch (paramIndex) {
		case 0:
			pInfo->id = ParamId::Frequency;
			strncpy(pInfo->name, paramIdToNameMap.at(ParamId::Frequency), CLAP_NAME_SIZE);
			strncpy(pInfo->module, "Partials", CLAP_NAME_SIZE);
			pInfo->min_value = 0.01f;
			pInfo->max_value = 22000.0f;
			pInfo->default_value = 200.0f;
			pInfo->flags |= globalModulationFlags;
			break;
		default:
			break;
		}


		return true;
	}
	
	bool AddSynPlugin::paramsValue(clap_id paramId, double* pValue) noexcept
	{
		auto valueIterator = m->paramIdToValueMap.find(paramId);
		if(valueIterator == m->paramIdToValueMap.end())
			return false;

		pValue = valueIterator->second;
		return true;
	}

	bool AddSynPlugin::paramsValueToText(clap_id paramId, double value, char* displayBuffer, uint32_t size) noexcept
	{
		auto paramIdEnum = (ParamId) paramId;
		std::string text{};

		switch (paramIdEnum) {
		case ParamId::Frequency:
			text = "test";
			break;
		}

		strncpy(displayBuffer, text.c_str(), CLAP_NAME_SIZE);
		return true;
	}

	bool AddSynPlugin::audioPortsInfo(uint32_t portIndex, bool isInput, clap_audio_port_info* pInfo) const noexcept
	{
		if(isInput || portIndex != 0)
			return false; // only set info if we are dealing with the first port

		// we could later add more ports for sidechain inputs, recording, etc...
		pInfo->id = 0;
		pInfo->in_place_pair = CLAP_INVALID_ID;
		strncpy(pInfo->name, "main", sizeof(pInfo->name));
		pInfo->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS;
		pInfo->channel_count = 2;
		pInfo->port_type = CLAP_PORT_STEREO;
		return true;
	}
}