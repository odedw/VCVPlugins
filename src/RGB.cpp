#include "Inja.hpp"

struct Rgb : Module
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

  enum RangeMode
  {
    MINUS_FIVE_TO_FIVE,
    ZERO_TO_FIVE,
    ZERO_TO_TEN,
    NUM_RANGE_MODES
  };

  float rangeModeMin[NUM_RANGE_MODES] = {-5, 0, 0};
  float rangeModeMax[NUM_RANGE_MODES] = {5, 5, 10};

  Rgb() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
  {
  }

  void step() override;

  int range = 0;

  void onReset() override
  {
    range = MINUS_FIVE_TO_FIVE;
  }

  // For more advanced Module features, read Rack's engine.hpp header file
  // - toJson, fromJson: serialization of internal data
  // - onSampleRateChange: event triggered by a change of sample rate
  // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void Rgb::step()
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
  Rgb *module;

  float scale(float value, float minA, float maxA, float minB, float maxB)
  {
    float relativePart = ((value - minA) / (maxA - minA));
    return relativePart * (maxB - minB) + minB;
  }
  NVGcolor computeColor(float r, float g, float b, float min, float max)
  {
    return nvgRGB(scale(r, min, max, 0, 255), scale(g, min, max, 0, 255), scale(b, min, max, 0, 255));
  }

  void draw(NVGcontext *vg) override
  {
    nvgBeginPath(vg);
    nvgFillColor(vg, computeColor(module->inputs[Rgb::R_INPUT].value,
                                  module->inputs[Rgb::G_INPUT].value,
                                  module->inputs[Rgb::B_INPUT].value,
                                  module->rangeModeMin[module->range],
                                  module->rangeModeMax[module->range]));
    nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 10);
    nvgFill(vg);

    // nvgFillColor(vg, COLOR_BLACK);
    // char array[20];
    // sprintf(array, "%f",
    //         scale(module->inputs[Rgb::R_INPUT].value,
    //               module->rangeModeMin[module->range],
    //               module->rangeModeMax[module->range], 0, 255));
    // nvgText(vg, 10, 20, array, NULL);
    // nvgStroke(vg);
    // nvgFill(vg);
  }
};

struct RgbRangeMenuItem : MenuItem
{
  Rgb *module;
  int range;
  void onAction(EventAction &e) override
  {
    module->range = range;
  }
  void step() override
  {
    rightText = (module->range == range) ? "✔" : "";
    MenuItem::step();
  }
};

struct RgbWidget : ModuleWidget
{
  float inputY = 330;
  float margin = 10;
  float displayTopMargin = 20;

  RgbWidget(Rgb *module) : ModuleWidget(module)
  {
    setPanel(SVG::load(assetPlugin(plugin, "res/rgb.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    RedInput *r = RedInput::create<RedInput>(Vec(margin, inputY), Port::INPUT, module, Rgb::R_INPUT);
    rack::Rect inputBox = r->box;
    addInput(r);
    addInput(Port::create<GreenInput>(Vec(box.size.x / 2 - inputBox.size.x / 2, inputY), Port::INPUT, module, Rgb::G_INPUT));
    addInput(Port::create<BlueInput>(Vec(box.size.x - (inputBox.size.x + margin), inputY), Port::INPUT, module, Rgb::B_INPUT));

    {
      ColorDisplay *display = new ColorDisplay();
      display->module = module;
      display->box.pos = Vec(margin, displayTopMargin);
      display->box.size = Vec(box.size.x - margin * 2, box.size.y - (margin + displayTopMargin + (box.size.y - inputY)));
      addChild(display);
    }
  }

  void appendContextMenu(Menu *menu) override
  {
    Rgb *module = dynamic_cast<Rgb *>(this->module);
    assert(module);

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Value ranges"));
    menu->addChild(construct<RgbRangeMenuItem>(&MenuItem::text, "-5v to 5v", &RgbRangeMenuItem::module, module, &RgbRangeMenuItem::range, Rgb::MINUS_FIVE_TO_FIVE));
    menu->addChild(construct<RgbRangeMenuItem>(&MenuItem::text, "0v to 5v", &RgbRangeMenuItem::module, module, &RgbRangeMenuItem::range, Rgb::ZERO_TO_FIVE));
    menu->addChild(construct<RgbRangeMenuItem>(&MenuItem::text, "0v to 10v", &RgbRangeMenuItem::module, module, &RgbRangeMenuItem::range, Rgb::ZERO_TO_TEN));
  }
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *rgb = Model::create<Rgb, RgbWidget>("Inja", "RGB", "RGB", VISUAL_TAG);
