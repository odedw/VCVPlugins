#include "Inja.hpp"

struct RgbModule : Module
{
  enum ParamIds
  {
    NUM_PARAMS
  };
  enum InputIds
  {
    R_INPUT,
    G_INPUT,
    B_INPUT,
    NUM_INPUTS
  };
  enum OutputIds
  {
    NUM_OUTPUTS
  };
  enum LightIds
  {
    NUM_LIGHTS
  };

  RgbModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
  void step() override;

  // For more advanced Module features, read Rack's engine.hpp header file
  // - toJson, fromJson: serialization of internal data
  // - onSampleRateChange: event triggered by a change of sample rate
  // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void RgbModule::step()
{
  // Implement a simple sine oscillator
  // float deltaTime = engineGetSampleTime();

  // Compute the frequency from the pitch parameter and input
  // float pitch = params[PITCH_PARAM].value;
  // pitch += inputs[PITCH_INPUT].value;
  // pitch = clamp(pitch, -4.0f, 4.0f);
  // The default pitch is C4
  // float freq = 261.626f * powf(2.0f, pitch);

  // Accumulate the phase
  // phase += freq * deltaTime;
  // if (phase >= 1.0f)
  // phase -= 1.0f;

  // Compute the sine output
  // float sine = sinf(2.0f * M_PI * phase);
  // outputs[SINE_OUTPUT].value = 5.0f * sine;

  // Blink light at 1Hz
  // blinkPhase += deltaTime;
  // if (blinkPhase >= 1.0f)
  // blinkPhase -= 1.0f;
  // lights[BLINK_LIGHT].value = (blinkPhase < 0.5f) ? 1.0f : 0.0f;
}

struct ColoredInput : PJ301MPort
{
  NVGcolor color;
  ColoredInput(NVGcolor c)
  {
    color = c;
  }

  void draw(NVGcontext *vg)
  {
    PJ301MPort::draw(vg);
    nvgStrokeWidth(vg, 2.2);
    nvgStrokeColor(vg, color);
    nvgBeginPath(vg);
    nvgCircle(vg, box.size.x / 2, box.size.y / 2, box.size.x / 2 - 2.1);
    nvgStroke(vg);
  }
};

struct RedInput : ColoredInput
{
  RedInput() : ColoredInput(COLOR_RED) {}
};

struct GreenInput : ColoredInput
{
  GreenInput() : ColoredInput(COLOR_GREEN) {}
};
struct BlueInput : ColoredInput
{
  BlueInput() : ColoredInput(COLOR_BLUE) {}
};

struct RgbWidget : ModuleWidget
{
  float inputY = 330;
  float inputXMargin = 10;
  float inputSize = 34.6;

  RgbWidget(RgbModule *module) : ModuleWidget(module)
  {
    setPanel(SVG::load(assetPlugin(plugin, "res/rgb.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    RedInput *r = RedInput::create<RedInput>(Vec(inputXMargin, inputY), Port::INPUT, module, RgbModule::R_INPUT);
    rack::Rect inputBox = r->box;
    addInput(r);
    addInput(Port::create<GreenInput>(Vec(box.size.x / 2 - inputBox.size.x / 2, inputY), Port::INPUT, module, RgbModule::G_INPUT));
    addInput(Port::create<BlueInput>(Vec(box.size.x - (inputBox.size.x + inputXMargin), inputY), Port::INPUT, module, RgbModule::B_INPUT));
  }
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *rgb = Model::create<RgbModule, RgbWidget>("Inja", "RGB", "RGB", OSCILLATOR_TAG);
