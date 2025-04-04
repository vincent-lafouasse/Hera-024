#pragma once

#include "JuceHeader.h"
#include "Processor.hpp"

//==============================================================================

class VolumeKnob final : public juce::Slider {
   public:
    juce::String getTextFromValue(double value) override;
};

//==============================================================================
/**
 */
class HeraEditor final : public juce::AudioProcessorEditor, public juce::Timer {
   public:
    explicit HeraEditor(HeraProcessor&);
    ~HeraEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

   private:
    void setupGainKnob();
    void setupKeyboard();

   private:
    HeraProcessor& audioProcessor;

    VolumeKnob volume_knob;
    juce::Label volume_label;

    juce::MidiKeyboardComponent keyboardComponent;

   public:
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        volume_attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeraEditor)
};