#include "AddSyn.h"

#include <map>
#include <string>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>

#include "PartialOsc.h"

namespace {
	const char* features[] = { CLAP_PLUGIN_FEATURE_INSTRUMENT, CLAP_PLUGIN_FEATURE_STEREO, nullptr };

	static constexpr clap_plugin_descriptor DESCRIPTOR = {
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

	static const std::unordered_map<clap_id, const char*> PARAM_ID_TO_NAME_MAP = {
		// ex. { 1, "Gain" }
		{ addsyn::AddSynPlugin::ParamId::Frequency, "Frequency" }
	};
}

namespace addsyn {
	struct AddSynPlugin::Impl {
		std::unordered_map<clap_id, double*> paramIdToValueMap;
		//std::vector<internal::PartialOsc> sineBank;
		internal::PartialOsc partialOsc; // test
		bool playing{false};
	};

	AddSynPlugin::AddSynPlugin(const clap_host* pHost): clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>(&DESCRIPTOR, pHost), m{ std::make_unique<Impl>() }
	{
	}

	AddSynPlugin::~AddSynPlugin()
	{
	}

	const clap_plugin_descriptor& AddSynPlugin::getDescriptor() noexcept
	{
		return DESCRIPTOR;
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
		
		auto inputEvents = pProcess->in_events;
		auto inputEventCount = inputEvents->size(inputEvents);
		const clap_event_header* inputEventPtr{nullptr};
		uint32_t inputEventIndex{0};

		if (inputEventCount != 0)
			inputEventPtr = inputEvents->get(inputEvents, inputEventIndex);

		for (uint32_t indexInBuffer = 0; indexInBuffer < pProcess->frames_count; ++indexInBuffer) {
			processEvents(inputEvents, inputEventCount, indexInBuffer, inputEventIndex, inputEventPtr);
			/*
			ok for now but a better approach would be to make the partial return a value and do the
			writing here. also better as it doesnt give the oscillator the responsibility to write
			*/
			auto buffer = pProcess->audio_outputs[0].data32;
			auto out = m->partialOsc.tick();

			for (uint32_t channelIndex = 0; channelIndex < pProcess->audio_outputs->channel_count; channelIndex++)
				buffer[channelIndex][indexInBuffer] = m->playing ? out : 0.0f;
		}

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
		return static_cast<uint32_t>(m->paramIdToValueMap.size());
	}
	
	bool AddSynPlugin::paramsInfo(uint32_t paramIndex, clap_param_info* pInfo) const noexcept
	{
		if(paramIndex >= m->paramIdToValueMap.size())
			return false;

		pInfo->flags = CLAP_PARAM_IS_AUTOMATABLE;

		switch (paramIndex) {
		case 0:
			pInfo->id = ParamId::Frequency;
			strncpy_s(pInfo->name, PARAM_ID_TO_NAME_MAP.at(ParamId::Frequency), CLAP_NAME_SIZE);
			strncpy_s(pInfo->module, "Partials", CLAP_NAME_SIZE);
			pInfo->min_value = 0.01f;
			pInfo->max_value = 22000.0f;
			pInfo->default_value = 200.0f;
			pInfo->flags |= CLAP_PARAM_IS_MODULATABLE | CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID | CLAP_PARAM_IS_MODULATABLE_PER_KEY;
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

		strncpy_s(displayBuffer, CLAP_NAME_SIZE, text.c_str(), CLAP_NAME_SIZE);
		return true;
	}

	bool AddSynPlugin::audioPortsInfo(uint32_t portIndex, bool input, clap_audio_port_info* pInfo) const noexcept
	{
		if(input || portIndex != 0)
			return false; // only set info if we are dealing with the first port

		// we could later add more ports for sidechain inputs, recording, etc...
		pInfo->id = 0;
		pInfo->in_place_pair = CLAP_INVALID_ID;
		strncpy_s(pInfo->name, "main", sizeof(pInfo->name));
		pInfo->flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS;
		pInfo->channel_count = 2;
		pInfo->port_type = CLAP_PORT_STEREO;
		return true;
	}
	
	bool AddSynPlugin::notePortsInfo(uint32_t index, bool input, clap_note_port_info* info) const noexcept
	{
		if(!input) return false;

		info->id = 1;
		info->supported_dialects = CLAP_NOTE_DIALECT_MIDI | CLAP_NOTE_DIALECT_CLAP;
		info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
		strncpy_s(info->name, "NoteInput", CLAP_NAME_SIZE);

		return true;
	}

	void AddSynPlugin::processEvents(const clap_input_events* events, const uint32_t eventCount, const uint32_t bufferPosition, uint32_t& eventIndex, const clap_event_header* pNextEvent)
	{

		// checking if the scheduled event time is equal to our position in the buffer
		while (pNextEvent && pNextEvent->time == bufferPosition) {
			processEvent(pNextEvent);
			eventIndex++;

			if(eventIndex >= eventCount)
				pNextEvent = nullptr;
			else
				pNextEvent = events->get(events, eventIndex);
		}
	}

	void AddSynPlugin::processEvent(const clap_event_header* pEvent)
	{
		auto tryLog = [&](clap_log_severity severity, const char* msg) {
			if(_host.canUseHostLog()) _host.log(severity, msg);
		};

		//pEvent is already valid here
		if(pEvent->space_id != CLAP_CORE_EVENT_SPACE_ID)
			return;

		switch (pEvent->type) {
			case CLAP_EVENT_MIDI: 
			{
				auto pMidiEvent = reinterpret_cast<const clap_event_midi*>(pEvent);
				auto midiMessage = pMidiEvent->data[0] & 0xF0;
				auto midiChannel = pMidiEvent->data[0] & 0x0F;

				// more midi msgs to come... for now we only want to control if we are playing
				switch (midiMessage) {
					case 0x90: // note on
						tryLog(CLAP_LOG_INFO, "received MIDI note on");
						m->playing = true;
						break;
					case 0x80: // note off
						tryLog(CLAP_LOG_INFO, "received MIDI note off");
						m->playing = false;
						break;
					default:
						break;
				}
				break;
			}
			case CLAP_EVENT_NOTE_ON:
				tryLog(CLAP_LOG_INFO, "received CLAP note on");
				m->playing = true;
				break;
			case CLAP_EVENT_NOTE_OFF:
				tryLog(CLAP_LOG_INFO, "received CLAP note off");
				m->playing = false;
				break;
			default:
				break;
		}
	}
}