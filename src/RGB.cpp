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
    ZERO_TO_TEN,
    ZERO_TO_FIVE,
    MINUS_FIVE_TO_FIVE,
    NUM_RANGE_MODES
  };

  float rangeModeMin[NUM_RANGE_MODES] = {0, 0, -5};
  float rangeModeMax[NUM_RANGE_MODES] = {10, 5, 5};

  Rgb() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
  {
  }

  void step() override;

  int rangeMode = 0;

  void onReset() override
  {
    rangeMode = ZERO_TO_TEN;
  }

  json_t *toJson() override
  {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "rangeMode", json_integer((int)rangeMode));

    return rootJ;
  }

  void fromJson(json_t *rootJ) override
  {
    json_t *rangeModeJ = json_object_get(rootJ, "rangeMode");
    if (rangeModeJ)
    {
      rangeMode = (Rgb::RangeMode)json_integer_value(rangeModeJ);
    }
  }
};

void Rgb::step()
{
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

struct LedMatrix : TransparentWidget
{
  Rgb *module;
  float MARGIN = 3;
  float COLUMNS = 5;

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
    float radius = (box.size.x - MARGIN * (COLUMNS + 1)) / (COLUMNS * 2); //((box.size.x - MARGIN * (COLUMNS + 1)) / COLUMNS) / 2;
    int rows = (box.size.y - MARGIN) / (MARGIN + radius * 2);

    nvgBeginPath(vg);
    nvgFillColor(vg, COLOR_BLACK);
    nvgRoundedRect(vg, 0, 0, box.size.x, MARGIN + rows * (MARGIN + radius * 2), radius);
    // nvgRect(vg, 0, 0, box.size.x, MARGIN + rows * (MARGIN + radius * 2));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgFillColor(vg, computeColor(module->inputs[Rgb::R_INPUT].value,
                                  module->inputs[Rgb::G_INPUT].value,
                                  module->inputs[Rgb::B_INPUT].value,
                                  module->rangeModeMin[module->rangeMode],
                                  module->rangeModeMax[module->rangeMode]));

    for (int i = 0; i < COLUMNS; i++)
    {
      for (int j = 0; j < rows; j++)
      {
        nvgCircle(vg, MARGIN + radius + i * (MARGIN + radius * 2), MARGIN + j * (MARGIN + radius * 2) + radius, radius);
      }
    }
    nvgFill(vg);

    // nvgFillColor(vg, COLOR_BLACK);
    // char array[20];
    // sprintf(array, "%f", box.size.x);
    // nvgText(vg, 10, 20, array, NULL);
    // sprintf(array, "%f", radius);
    // nvgText(vg, 10, 40, array, NULL);
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
    module->rangeMode = range;
  }
  void step() override
  {
    rightText = (module->rangeMode == range) ? "✔" : "";
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
      LedMatrix *matrix = new LedMatrix();
      matrix->module = module;
      matrix->box.pos = Vec(margin, displayTopMargin);
      matrix->box.size = Vec(box.size.x - margin * 2, box.size.y - (margin + displayTopMargin + (box.size.y - inputY)));
      addChild(matrix);
    }
  }

  void appendContextMenu(Menu *menu) override
  {
    Rgb *module = dynamic_cast<Rgb *>(this->module);
    assert(module);

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Value ranges"));
    menu->addChild(construct<RgbRangeMenuItem>(&MenuItem::text, "0v to 10v", &RgbRangeMenuItem::module, module, &RgbRangeMenuItem::range, Rgb::ZERO_TO_TEN));
    menu->addChild(construct<RgbRangeMenuItem>(&MenuItem::text, "0v to 5v", &RgbRangeMenuItem::module, module, &RgbRangeMenuItem::range, Rgb::ZERO_TO_FIVE));
    menu->addChild(construct<RgbRangeMenuItem>(&MenuItem::text, "-5v to 5v", &RgbRangeMenuItem::module, module, &RgbRangeMenuItem::range, Rgb::MINUS_FIVE_TO_FIVE));
  }
};

Model *rgb = Model::create<Rgb, RgbWidget>("Inja", "RGB", "RGB", VISUAL_TAG);
