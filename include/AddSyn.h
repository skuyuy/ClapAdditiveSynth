#pragma once

#include <clap/helpers/plugin.hh>
#include <memory>
#include "constants.h"

namespace addsyn {
	class AddSynPlugin : public clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal> {
	public:
		enum ParamId : clap_id
		{
			Frequency = 0x000,
			Amplitude = 0x001
		};

		AddSynPlugin(const clap_host* pHost);
		virtual ~AddSynPlugin();
		static const clap_plugin_descriptor& getDescriptor() noexcept;
		/* clap_plugin */
		bool activate(double sampleRate, uint32_t minFrames, uint32_t maxFrames) noexcept override;
		clap_process_status process(const clap_process* pProcess) noexcept override;
		bool startProcessing() noexcept override;
		void stopProcessing() noexcept override;
		/* clap_plugin_params */
		bool implementsParams() const noexcept override { return true; }
		bool isValidParamId(clap_id paramId) const noexcept override;
		uint32_t paramsCount() const noexcept override;
		bool paramsInfo(uint32_t paramIndex, clap_param_info* pInfo) const noexcept override;
		bool paramsValue(clap_id paramId, double* pValue) noexcept override;
		bool paramsValueToText(clap_id paramId, double value, char* displayBuffer, uint32_t size) noexcept override;
		/* clap_plugin_audio_ports */
		bool implementsAudioPorts() const noexcept override { return true; }
		uint32_t audioPortsCount(bool input) const noexcept override { return input ? 0 : 1; }
		bool audioPortsInfo(uint32_t portIndex, bool input, clap_audio_port_info* pInfo) const noexcept override;
		/* clap_plugin_note_ports */
		bool implementsNotePorts() const noexcept override { return true; }
		uint32_t notePortsCount(bool input) const noexcept override { return input ? 1 : 0; }
		bool notePortsInfo(uint32_t index, bool input, clap_note_port_info* info) const noexcept override;
	private:
		void processEvents(const clap_input_events* events, const uint32_t eventCount, const uint32_t bufferPosition, uint32_t& eventIndex, const clap_event_header* pNextEvent);
		void processEvent(const clap_event_header* pEvent);

		struct Impl;
		std::unique_ptr<Impl> m;
	};
}