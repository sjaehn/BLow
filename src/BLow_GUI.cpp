#include <cstdint>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include "../BWidgets/BEvents/ExposeEvent.hpp"
#include "../BWidgets/BUtilities/to_string.hpp"
#include "../BWidgets/BWidgets/Window.hpp"
#include "../BWidgets/BWidgets/ValueDial.hpp"
#include "../BWidgets/BWidgets/ComboBox.hpp"
#include "../BWidgets/BWidgets/HPianoRoll.hpp"
#include "Definitions.hpp"
#include "Ports.hpp"
#include "Urids.hpp"

#define BG_FILE "inc/surface.png"

class BLow_GUI : public BWidgets::Window
{
public:
	BLow_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow);
	void portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer);
	virtual void onConfigureRequest (BEvents::Event* event) override;
	static void valueChangedCallback (BEvents::Event* event);
	static void pianoCallback (BEvents::Event* event);

	LV2UI_Write_Function write_function;
	LV2UI_Controller controller;

protected:
	BLowURIs uris;
	LV2_Atom_Forge forge;
	std::string pluginPath;
	BWidgets::ComboBox sampleMenu;
	BWidgets::ValueDial gainDial;
	BWidgets::ValueDial tuneStDial;
	BWidgets::ValueDial tuneCtDial;
	std::array<BWidgets::Valueable*, BLOW_NR_CONTROLLERS> controllerWidgets;
	BWidgets::HPianoRoll pianoRoll;
	BWidgets::Dial velocityDial;
	std::array<uint8_t, 128> velocities;

	// Style definitions
	BStyles::ColorMap fgColors = {{{0.7, 0.25, 0.1, 1.0}, {0.7, 0.25, 0.1, 1.0}, {0.1, 0.02, 0.01, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap txColors = {{{1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.1, 0.1, 0.1, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::ColorMap bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BStyles::Border menuBorder = {{BStyles::grey, 1.0}, 0.0, 0.0, 0.0};
	BStyles::Border labelborder = {BStyles::noLine, 4.0, 0.0, 0.0};
	BStyles::Fill menuBg = BStyles::Fill (BStyles::Color (0.05, 0.02, 0.01, 1.0));
	BStyles::Font ctLabelFont = BStyles::Font	("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   						 BStyles::Font::TEXT_ALIGN_CENTER, BStyles::Font::TEXT_VALIGN_MIDDLE);
	BStyles::Font lfLabelFont = BStyles::Font	("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   						 BStyles::Font::TEXT_ALIGN_LEFT, BStyles::Font::TEXT_VALIGN_MIDDLE);

	BStyles::Style style = 
	{
		{BUtilities::Urid::urid (BLOW_URI "/dial"), BUtilities::makeAny<BStyles::Style>({
			{BUtilities::Urid::urid (STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(fgColors)},
			{BUtilities::Urid::urid (STYLEPROPERTY_BGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(bgColors)},
			{BUtilities::Urid::urid (BLOW_URI "/dial/label"), BUtilities::makeAny<BStyles::Style>({
				{BUtilities::Urid::urid (STYLEPROPERTY_FONT_URI), BUtilities::makeAny<BStyles::Font>(ctLabelFont)},
				{BUtilities::Urid::urid (STYLEPROPERTY_TXCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(txColors)}
			})}
		})},

		{BUtilities::Urid::urid (BLOW_URI "/piano"), BUtilities::makeAny<BStyles::Style>({
			{BUtilities::Urid::urid (STYLEPROPERTY_FGCOLORS_URI), BUtilities::makeAny<BStyles::ColorMap>(fgColors)}
		})}
	};
};

BLow_GUI::BLow_GUI (const char *bundle_path, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	BWidgets::Window (600, 320, parentWindow, URID_UNKNOWN_URID, "BLow", true, PUGL_MODULE, 0),
	write_function (NULL),
	controller (NULL),
	pluginPath (bundle_path ? std::string (bundle_path) + ((strlen (bundle_path) > 0) && (bundle_path[strlen (bundle_path) - 1] != '/') ? "/" : "") : std::string ("")),
	sampleMenu (100, 90, 140, 20, 0, 20, 140, 180, {"Unfa", "Kuchtaa", "Junkfood2121", "KataVlogsYT", "Peridactyloptrix", "Dleigh", "YYZJJ", "Flash_Shumway", "Shaundoogan", "Breviceps"}, 1, BUtilities::Urid::urid (BLOW_URI "/menu")),
	gainDial	(110, 140, 60, 72, 0.0, -70.0, 30.0, 0.0, 
				 BWidgets::ValueTransferable<double>::noTransfer, BWidgets::ValueTransferable<double>::noTransfer, 
				 [] (const double& x) {return BUtilities::to_string (x, "%1.1f") + " db";}, BWidgets::ValueDial::stringToValue,
				 BUtilities::Urid::urid (BLOW_URI "/dial")),
	tuneStDial 	(350, 140, 60, 72, 0.0, -12.0, 12.0, 1.0, 
				 BWidgets::ValueTransferable<double>::noTransfer, BWidgets::ValueTransferable<double>::noTransfer, 
				 [] (const double& x) {return BUtilities::to_string (x, "%1.0f");}, BWidgets::ValueDial::stringToValue,
				 BUtilities::Urid::urid (BLOW_URI "/dial")),
	tuneCtDial	(430, 140, 60, 72, 0.0, -1.0, 1.0, 0.0, 
				 BWidgets::ValueTransferable<double>::noTransfer, BWidgets::ValueTransferable<double>::noTransfer, 
				 [] (const double& x) {return BUtilities::to_string (x * 100.0, "%1.0f");}, [] (const std::string& s) {return std::stod (s) / 100.0;},
				 BUtilities::Urid::urid (BLOW_URI "/dial")),
	pianoRoll (20, 280, 520, 40, 0, 127, std::vector<uint8_t>{}, BUtilities::Urid::urid (BLOW_URI "/piano")),
	velocityDial(550, 280, 30, 30, 64, 0, 127, 1, 
				 BWidgets::ValueTransferable<double>::noTransfer, BWidgets::ValueTransferable<double>::noTransfer, 
				 BUtilities::Urid::urid (BLOW_URI "/dial"))

{
	// Link controllerWidgets
	controllerWidgets[BLOW_SAMPLE] = (BWidgets::Valueable*) &sampleMenu;
	controllerWidgets[BLOW_GAIN] = (BWidgets::Valueable*) &gainDial;
	controllerWidgets[BLOW_TUNE] = (BWidgets::Valueable*) &tuneStDial;
	controllerWidgets[BLOW_TUNE_CT] = (BWidgets::Valueable*) &tuneCtDial;

	// Set callbacks
	for (BWidgets::Valueable* c : controllerWidgets) c->setCallbackFunction (BEvents::Event::VALUE_CHANGED_EVENT, valueChangedCallback);
	pianoRoll.setCallbackFunction (BEvents::Event::VALUE_CHANGED_EVENT, pianoCallback);

	// Init params
	setStyle (style);
	setBackground (pluginPath + BG_FILE);
	gainDial.setClickable (false);
	tuneStDial.setClickable (false);
	tuneCtDial.setClickable (false);
	velocityDial.setClickable (false);
	velocities.fill (0);
	pianoRoll.activate();

	// Add widgets
	for (BWidgets::Valueable* c : controllerWidgets) 
	{
		if (dynamic_cast<BWidgets::Widget*> (c)) add (dynamic_cast<BWidgets::Widget*> (c));
	}
	add (&pianoRoll);
	add (&velocityDial);

	//Scan host features for URID map
	LV2_URID_Map* map = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			map = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!map) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge, map);
	
}

void BLow_GUI::portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	if (port_index == BLOW_MIDI_IN)
	{
		fprintf (stderr, "MIDI_IN:\n");
	}

	if ((port_index >= BLOW_CONTROLLERS) && (port_index < BLOW_NR_CONTROLLERS) && (format == 0))
	{
		float* pval = (float*) buffer;
		if (port_index == BLOW_CONTROLLERS + BLOW_SAMPLE) sampleMenu.setValue (*pval + 1);
		else 
		{
			BWidgets::ValueableTyped<double>* v = dynamic_cast<BWidgets::ValueableTyped<double>*> (controllerWidgets[port_index - BLOW_CONTROLLERS]);
			if (v) v->setValue (*pval);
		}
	}
}

void BLow_GUI::onConfigureRequest (BEvents::Event* event)
{
	Window::onConfigureRequest (event);

	BEvents::ExposeEvent* ee = dynamic_cast<BEvents::ExposeEvent*>(event);
	if (!ee) return;
	double sz = (ee->getArea().getWidth() / 600.0 > ee->getArea().getHeight() / 320.0 ? ee->getArea().getHeight() / 320.0 : ee->getArea().getWidth() / 600.0);
	setZoom (sz);
}

void BLow_GUI::valueChangedCallback(BEvents::Event* event)
{
	if (!event) return;
	BWidgets::Widget* widget = event->getWidget ();
	if (!widget) return;
	BWidgets::Valueable* valueable = dynamic_cast<BWidgets::Valueable*> (widget);
	if (!valueable) return;
	BLow_GUI* ui = (BLow_GUI*) widget->getMainWindow();
	if (!ui) return;

	// Identify controller
	for (int i = 0; i < BLOW_NR_CONTROLLERS; ++i)
	{
		if (valueable == ui->controllerWidgets[i])
		{
			// Sample ComboBox
			if (i == BLOW_SAMPLE)
			{
				BWidgets::ValueableTyped<size_t>* vt = dynamic_cast<BWidgets::ValueableTyped<size_t>*> (valueable);
				if (vt) 
				{
					float value = std::min (vt->getValue() - 1, 9ul);
					ui->write_function(ui->controller, BLOW_CONTROLLERS + i, sizeof(float), 0, &value);
				}
			}

			// Dials
			else
			{
				BWidgets::ValueableTyped<double>* vt = dynamic_cast<BWidgets::ValueableTyped<double>*> (valueable);
				if (vt) 
				{
					float value = vt->getValue();
					ui->write_function(ui->controller, BLOW_CONTROLLERS + i, sizeof(float), 0, &value);
				}
			}
			
			break;
		}
	}
}

void BLow_GUI::pianoCallback (BEvents::Event* event)
{
	if (!event) return;
	BWidgets::HPianoRoll* widget = dynamic_cast<BWidgets::HPianoRoll*> (event->getWidget ());
	if (!widget) return;
	BLow_GUI* ui = (BLow_GUI*) widget->getMainWindow();
	if (!ui) return;

	for (uint8_t i = 0; i < 128; ++i)
	{
		const uint8_t vel = widget->getKey(i);
		if (vel != ui->velocities[i])
		{
			ui->velocities[i] = vel;
			uint8_t obj_buf[64];
			lv2_atom_forge_set_buffer(&ui->forge, obj_buf, sizeof(obj_buf));
			LV2_URID urid = (vel != 0 ? ui->uris.blow_noteOn : ui->uris.blow_noteOff);
			LV2_Atom_Forge_Frame frame;
			LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ui->forge, &frame, 0, ui->uris.blow_keyboardEvent);
			lv2_atom_forge_key (&ui->forge, urid);
			lv2_atom_forge_int (&ui->forge, i);
			lv2_atom_forge_key (&ui->forge, ui->uris.blow_velocity);
			lv2_atom_forge_int (&ui->forge, vel * ui->velocityDial.getValue() / 64);
			lv2_atom_forge_pop (&ui->forge, &frame);
			ui->write_function (ui->controller, BLOW_MIDI_IN, lv2_atom_total_size (msg), ui->uris.atom_eventTransfer, msg);
		}
	}
}

static LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeView parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BLOW_URI) != 0)
	{
		std::cerr << "BLow_GUI: This GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeView) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BLow_GUI: No parent window.\n";

	BLow_GUI* ui = new BLow_GUI (bundle_path, features, parentWindow);

	if (ui)
	{
		ui->controller = controller;
		ui->write_function = write_function;
		if (resize) resize->ui_resize(resize->handle, 600, 320 );

		PuglNativeView nativeWindow = puglGetNativeWindow (ui->getPuglView ());
		*widget = (LV2UI_Widget) nativeWindow;
	}
	else std::cerr << "BLow_GUI: Couldn't instantiate.\n";
	return (LV2UI_Handle) ui;
}

static void cleanup(LV2UI_Handle ui)
{
	BLow_GUI* pluginGui = (BLow_GUI*) ui;
	if (pluginGui) delete pluginGui;
}

static void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	BLow_GUI* pluginGui = (BLow_GUI*) ui;
	if (pluginGui) pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle(LV2UI_Handle ui)
{
	BLow_GUI* pluginGui = (BLow_GUI*) ui;
	if (pluginGui) pluginGui->handleEvents ();
	return 0;
}

static int callResize (LV2UI_Handle ui, int width, int height)
{
	BLow_GUI* self = (BLow_GUI*) ui;
	if (!self) return 0;
	BEvents::ExposeEvent* ev = new BEvents::ExposeEvent (self, self, BEvents::Event::CONFIGURE_REQUEST_EVENT, self->getPosition().x, self->getPosition().y, width, height);
	self->addEventToQueue (ev);
	return 0;
}

static const LV2UI_Idle_Interface idle = {callIdle};
static const LV2UI_Resize resize = {nullptr, callResize} ;

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else if(!strcmp(uri, LV2_UI__resize)) return &resize;
	else return NULL;
}


static const LV2UI_Descriptor guiDescriptor =
{
		BLOW_GUI_URI,
		instantiate,
		cleanup,
		portEvent,
		extensionData
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index)
	{
		case 0: return &guiDescriptor;
		default:return NULL;
	}
}
