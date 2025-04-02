/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "Processor.hpp"
#include "Editor.hpp"

#include <cassert>

namespace {
template <typename NumericType>
void wrapping_add(NumericType& value, NumericType addend, NumericType limit) {
    value += addend;
    while (value >= limit) {
        value -= limit;
    }
}
}  // namespace

const std::string HeraProcessor::gain_id = "gain";
const std::string HeraProcessor::gain_name = "GAIN";

//==============================================================================
HeraProcessor::HeraProcessor()
    : AudioProcessor(
          BusesProperties().withOutput("Output",
                                       juce::AudioChannelSet::stereo(),
                                       true))

      ,
      params(*this, nullptr, "PARAMETERS", {}),
      nominal_volume(0) {
    assert(std::atomic<float>::is_always_lock_free);
}

HeraProcessor::~HeraProcessor() = default;

void HeraProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                 juce::MidiBuffer&) {
    auto* left_channel = buffer.getWritePointer(0);
    auto* right_channel = buffer.getWritePointer(1);
    const auto bufferSize = buffer.getNumSamples();

    float target_volume = nominal_volume.load(std::memory_order_relaxed);

    for (auto i = 0; i < bufferSize; ++i) {
        constexpr float sine_volume = 0.5f;
        const float sine = sine_volume * std::sin(this->phase);
        wrapping_add(this->phase, this->phase_increment,
                     juce::MathConstants<float>::twoPi);

        if (!juce::approximatelyEqual(this->volume, target_volume)) {
            this->volume = 0.5f * (target_volume + this->volume);
        }

        left_channel[i] = this->volume * sine;
        right_channel[i] = this->volume * sine;
    }
}

// period T = 1/f  <-> sample len delta_t = 1/sample_rate
// 2pi             <-> phase_increment = 2pi f / sample_rate

void HeraProcessor::prepareToPlay(const double sampleRate,
                                  const int samplesPerBlock) {
    this->sample_rate = static_cast<float>(sampleRate);
    this->set_frequency(220.0f);

    juce::String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlockExpected = " << samplesPerBlock << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::writeToLog(message);
}

void HeraProcessor::releaseResources() {
    juce::Logger::writeToLog("Releasing audio resources");
}

void HeraProcessor::set_volume(const float vol) {
    this->nominal_volume.store(vol, std::memory_order_relaxed);
}

void HeraProcessor::set_frequency(const float freq) {
    this->frequency = freq;
    this->phase_increment =
        juce::MathConstants<float>::twoPi * this->frequency / this->sample_rate;
}

//================== boiler plate =============================================

const juce::String HeraProcessor::getName() const {
    return JucePlugin_Name;
}

bool HeraProcessor::acceptsMidi() const {
    return true;
}

bool HeraProcessor::producesMidi() const {
    return false;
}

bool HeraProcessor::isMidiEffect() const {
    return false;
}

double HeraProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int HeraProcessor::getNumPrograms() {
    return 1;  // NB: some hosts don't cope very well if you tell them there are
               // 0 programs, so this should be at least 1, even if you're not
               // really implementing programs.
}

int HeraProcessor::getCurrentProgram() {
    return 0;
}

void HeraProcessor::setCurrentProgram(const int index) {
    (void)index;
}

const juce::String HeraProcessor::getProgramName(const int index) {
    (void)index;
    return "Hera";
}

void HeraProcessor::changeProgramName(const int index,
                                      const juce::String& newName) {
    (void)index;
    (void)newName;
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool HeraProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

//==============================================================================

bool HeraProcessor::hasEditor() const {
    return true;  // (change this to false if you choose to not supply an
                  // editor)
}

juce::AudioProcessorEditor* HeraProcessor::createEditor() {
    return new HeraEditor(*this);
}

//==============================================================================
void HeraProcessor::getStateInformation(juce::MemoryBlock& destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    (void)destData;
}

void HeraProcessor::setStateInformation(const void* data,
                                        const int sizeInBytes) {
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the getStateInformation()
    // call.
    (void)data;
    (void)sizeInBytes;
}

//==============================================================================
// This creates new instances of the plugin...
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new HeraProcessor();
}
