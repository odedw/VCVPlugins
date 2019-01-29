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
    nvgStrokeWidth(vg, 3);
    nvgStrokeColor(vg, color);
    nvgBeginPath(vg);
    nvgCircle(vg, box.size.x / 2, box.size.y / 2, box.size.x / 2 - 2);
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

struct ColorDisplay : TransparentWidget
{
  RgbModule *module;

  NVGcolor computeColor(float r, float g, float b)
  {
    return nvgRGB(255 * r / 10, 255 * g / 10, 255 * b / 10);
  }

  void draw(NVGcontext *vg) override
  {
    nvgBeginPath(vg);
    // nvgStrokeColor(vg, COLOR_WHITE);
    nvgFillColor(vg, computeColor(module->inputs[RgbModule::R_INPUT].value, module->inputs[RgbModule::G_INPUT].value, module->inputs[RgbModule::B_INPUT].value));
    nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 10);
    nvgFill(vg);
    // nvgFillColor(vg, COLOR_WHITE);
    // char array[10];
    // sprintf(array, "%f", module->inputs[RgbModule::R_INPUT].value);
    // nvgText(vg, 10, 20, array, NULL);
    // nvgStroke(vg);
  }
};

struct RgbWidget : ModuleWidget
{
  float inputY = 330;
  float margin = 10;
  float displayTopMargin = 20;

  RgbWidget(RgbModule *module) : ModuleWidget(module)
  {
    setPanel(SVG::load(assetPlugin(plugin, "res/rgb.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    RedInput *r = RedInput::create<RedInput>(Vec(margin, inputY), Port::INPUT, module, RgbModule::R_INPUT);
    rack::Rect inputBox = r->box;
    addInput(r);
    addInput(Port::create<GreenInput>(Vec(box.size.x / 2 - inputBox.size.x / 2, inputY), Port::INPUT, module, RgbModule::G_INPUT));
    addInput(Port::create<BlueInput>(Vec(box.size.x - (inputBox.size.x + margin), inputY), Port::INPUT, module, RgbModule::B_INPUT));

    {
      ColorDisplay *display = new ColorDisplay();
      display->module = module;
      display->box.pos = Vec(margin, displayTopMargin);
      display->box.size = Vec(box.size.x - margin * 2, box.size.y - (margin + displayTopMargin + (box.size.y - inputY)));
      addChild(display);
    }
  }
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *rgb = Model::create<RgbModule, RgbWidget>("Inja", "RGB", "RGB", OSCILLATOR_TAG);
