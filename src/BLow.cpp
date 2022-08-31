
#define MINIMP3_IMPLEMENTATION

#include <cmath>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include "Definitions.hpp"
#include "Ports.hpp"
#include "Urids.hpp"
#include "StaticArrayList.hpp"
#include "../BWidgets/BMusic/Sample.hpp"

#define BLOW_VOICES 16
#define BLOW_BLOWSAMPLES 2
#define BLOW_FADINGTIME 0.05
#define BLOW_SAMPLEFLATTENING 50.0
#define BLOW_MODULATIONRATE 1.5

struct Range
{
	uint64_t start;
	uint64_t end;
};

struct BSample
{
	BMusic::Sample sample;
	uint8_t note;
	std::vector<Range> ranges;
	uint64_t sustainEnd;
};

struct Voice
{
	BSample* sample;
	uint8_t note;
	uint8_t velocity;
	uint64_t startFrame;
	uint64_t releaseFrame;
	uint64_t endFrame;
	double position;
	double releasePosition;
	Range range;
	Range nextRange;
};

class BLow 
{
public:
	BLow (const double rate, const char* bundle_path, const LV2_Feature* const* features);
	void run (uint32_t n_samples);
	void connect_port (uint32_t portnr, void* data);

private:
	double rate;
	BLowURIs uris;
	std::array<void*, BLOW_N_PORTS> ports;
	std::array<float, BLOW_NR_CONTROLLERS> controllers;
	StaticArrayList<Voice, BLOW_VOICES> voices;
	uint64_t count;
	float amp;
	float modulation;
	float dm;
	std::array<BSample, 10> samples;
	std::minstd_rand rnd;
	std::uniform_real_distribution<double> unidist;
	std::uniform_real_distribution<float> bidist;

	void noteOn (const uint8_t note, const uint8_t velocity, const uint64_t frame);
	void noteOff (const uint8_t note, const uint64_t frame);
	void allSoundsOff (const uint64_t frame);
	void allNotesOff (const uint64_t frame);
	void process (const uint32_t start, const uint32_t end);
};


BLow::BLow (const double rate, const char* bundle_path, const LV2_Feature* const* features) :
	rate (rate),
	voices(),
	count (0),
	amp (1.0f),
	modulation (0.0f),
	dm (0.0f),
	samples	{BSample {BMusic::Sample((std::string (bundle_path) + "/inc/unfa.wav").c_str()), 51, {{21691, 22000}, {21075, 21690}, {20773, 21383}}, 22000}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/kuchtaa.wav").c_str()), 54, {{20357, 21500}, {18727, 20920}, {19259, 21500}}, 21500}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/junkfood2121.wav").c_str()), 40, {{39615, 42315}, {37316, 45603}, {39542, 47263}}, 47263},
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/katavlogsyt.wav").c_str()), 54, {{2850, 5111}, {1492, 4616}, {2574, 3613}}, 5111}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/peridactyloptrix.wav").c_str()), 34, {{32459, 43958}, {27485, 34226}, {35161, 54495}}, 54495}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/dleigh.wav").c_str()), 44, {{6560, 9495}, {3340, 6242}, {4987, 7131}}, 9495}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/yyzjj.wav").c_str()), 54, {{4491, 10412}, {7272, 15579}, {10863, 16333}}, 16333},  
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/flash-shumway.wav").c_str()), 68, {{4729, 8770}, {3715, 6339}, {5925, 8291}}, 8770}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/shaundoogan.wav").c_str()), 40, {{2970, 16612}, {14899, 24570}, {7564, 11765}}, 24570}, 
			 BSample {BMusic::Sample((std::string (bundle_path) + "/inc/breviceps.wav").c_str()), 40, {{30639, 32202}, {29915, 31307}, {29117, 30638}}, 32202}},
	rnd (time (0)),
	unidist (0.0, 1.0), 
	bidist (-1.0f, 1.0f)
{
	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0) m = (LV2_URID_Map*) features[i]->data;
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");
	getURIs (m, &uris);

	// Init ports
	ports.fill (NULL);
}

void BLow::connect_port (uint32_t portnr, void* data)
{
	if (portnr < BLOW_N_PORTS) ports[portnr] = data;
}

void BLow::run (uint32_t n_samples)
{
	// Check if all ports connected
	for (int i = 0; i < BLOW_N_PORTS; ++i)
	{
		if (!ports[i]) return;
	}

	// Update controllers
	for (int i = 0; i < BLOW_NR_CONTROLLERS; ++i) 
	{
		float nc = *(float*)ports[BLOW_CONTROLLERS + i];
		// TODO Validate nc

		if (nc != controllers[i])
		{
			switch (i)
			{
				case BLOW_SAMPLE:	// TODO Switch sample
									break;

				case BLOW_GAIN:		amp = powf (10.0, 0.05f * nc);
									break;

				default:			break;
			}

			controllers[i] = nc;
		}
	}

	const LV2_Atom_Sequence* const midiIn  = (const LV2_Atom_Sequence*) ports[BLOW_MIDI_IN];
	uint32_t last_t = 0;
	LV2_ATOM_SEQUENCE_FOREACH(midiIn, ev)
	{
		// Atom messages
		if ((ev->body.type == uris.atom_Object) || (ev->body.type == uris.atom_Blank))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
			
			// GUI Keyboard
			if (obj->body.otype ==uris.blow_keyboardEvent)
			{
				LV2_Atom *okOn = NULL, *okOff = NULL, *oVel = NULL;
				lv2_atom_object_get (obj,
						     uris.blow_noteOn, &okOn,
						     uris.blow_noteOff, &okOff,
							 uris.blow_velocity, &oVel,
						     NULL);

				if (oVel && (oVel->type == uris.atom_Int))
				{
					const int vel = ((const LV2_Atom_Int*)oVel)->body;

					if (okOn && (okOn->type == uris.atom_Int))
					{
						const int note = ((const LV2_Atom_Int*)okOn)->body;
						noteOn (note, vel, count + ev->time.frames);
					}

					if (okOff && (okOff->type == uris.atom_Int))
					{
						const int note = ((const LV2_Atom_Int*)okOff)->body;
						noteOff (note, count + ev->time.frames);
					}
				}
			}
		}


		// Read incoming MIDI events
		if (ev->body.type == uris.midi_Event)
		{
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			const uint8_t status = (msg[0] & 0xF0);
			//const uint8_t channel = msg[0] & 0x0F;
			const uint8_t note = msg[1];
			const uint8_t value = msg[2];

			switch (status)
			{
				case LV2_MIDI_MSG_NOTE_ON:		if (value != 0)
												{
													noteOn (note, value, count + ev->time.frames);
													break;
												}
												// Otherwise continue with note off
												// No break here!								

				case LV2_MIDI_MSG_NOTE_OFF:		noteOff (note, count + ev->time.frames);
												break;

				case LV2_MIDI_MSG_CONTROLLER:	switch (note)	// note == cc
												{
													case LV2_MIDI_CTL_SUSTAIN:			break;

													case LV2_MIDI_CTL_ALL_SOUNDS_OFF:	allSoundsOff (count + ev->time.frames);
																						break;

													case LV2_MIDI_CTL_ALL_NOTES_OFF:	allNotesOff (count + ev->time.frames);
																						break;

													// All other MIDI signals
													default: break;
												}
				
				default: break;
			}
		}

		// Update for this iteration
		uint32_t next_t = (ev->time.frames < n_samples ? ev->time.frames : n_samples);
		process (last_t, next_t);
		last_t = next_t;
	}

	// Process all samples
	if (last_t < n_samples) process (last_t, n_samples);
}

void BLow::process (uint32_t start, uint32_t end)
{
	float* const audioOut = (float*) ports[BLOW_AUDIO_OUT];

	for (uint32_t i = start; i < end; ++i)
	{
		float out = 0.0f;

		for (Voice** it = voices.begin(); it < voices.end(); )
		{
			BSample* s = (**it).sample;
			if (count >= (**it).startFrame)
			{
				const double dp = pow (2.0, (double (int((**it).note) - int(s->note)) + controllers[BLOW_TUNE] + controllers[BLOW_TUNE_CT]) / 12.0);
				float adsOut = 0.0f;
				float rOut = 0.0f;
				float f = 0.0f;

				
				if (s && (count < (**it).endFrame))
				{
					// ADS
					if ((**it).position + BLOW_SAMPLEFLATTENING < double ((**it).range.end)) adsOut = s->sample.get((**it).position, 0, rate) * amp * (1.0 + 0.5 * modulation);

					// Crossfading sustain loop end -> sustain loop begin
					else
					{
						const float out1 = s->sample.get((**it).position, 0, rate) * amp * (1.0f + 0.5f * modulation);
						const float out2 = s->sample.get(double ((**it).nextRange.start) + (**it).position + BLOW_SAMPLEFLATTENING - double ((**it).range.end), 0, rate) * amp * (1.0f + 0.5f * modulation);
						const float f1 = ((**it).position + BLOW_SAMPLEFLATTENING - double ((**it).range.end)) / BLOW_SAMPLEFLATTENING;
						adsOut = (1.0f - f1) * out1  + f1 * out2;
					}

					(**it).position += dp * (1.0f + 0.01f * modulation);
					if ((**it).position >= (**it).range.end) 
					{
						(**it).position = (**it).position - double ((**it).range.end) + double ((**it).nextRange.start);
						(**it).range = (**it).nextRange;
						const size_t idx = double (s->ranges.size()) * unidist (rnd);
						(**it).nextRange = s->ranges[idx];
					}

					// R
					if (count >= (**it).releaseFrame)
					{
						rOut = s->sample.get((**it).releasePosition, 0, rate) * amp * (1.0 + 0.5 * modulation);
						f = std::min (((**it).releasePosition - s->sustainEnd) / (BLOW_FADINGTIME * (s->sample.info.frames - s->sustainEnd)), 1.0);

						(**it).releasePosition += dp * (1.0 + 0.01 * modulation);
					}

					float vel = float ((**it).velocity) / 127.0f;

					out += vel * ((1.0 - f) * adsOut + f * rOut);
					++it;
				}

				// Ended
				else it = voices.erase (it);
			}

			else ++it;
		}

		audioOut[i] = out;
		++count;

		dm += bidist(rnd) * 0.1 / rate;
		dm = std::max (std::min (dm, 1.0f - modulation), -1.0f - modulation);
		modulation += dm;
	}
}

void BLow::noteOn (const uint8_t note, const uint8_t velocity, const uint64_t frame)
{
	// Scan if this is an additional midi message
	// (e.g., double note on, velocity changed)
	for (Voice** it = voices.begin(); it < voices.end(); ++it)
	{
		if (((**it).note == note) && (frame < (**it).releaseFrame))
		{
			(**it).velocity = velocity;
			return;
		}
	}

	// New voice
	if (voices.size < BLOW_VOICES)
	{
		// Add new voice
		BSample* s = &samples[int (controllers[BLOW_SAMPLE])];
		const Voice v = Voice {s, note, velocity, frame, UINT64_MAX, UINT64_MAX, 0.0, 0.0, s->ranges[0], s->ranges[0]};
		voices.push_back (v);
	}
}

void BLow::noteOff (const uint8_t note, const uint64_t frame)
{
	for (Voice** it = voices.begin(); it < voices.end(); ++it)
	{
		if (((**it).note == note) && (frame < (**it).releaseFrame))
		{
			// Calulate release time
			BSample* s = (**it).sample;
			if (s)
			{
				const uint64_t rt = (s->sample.info.frames - s->sustainEnd) / pow (2.0, double (int((**it).note) - int(s->note)) / 12.0);

				(**it).releaseFrame = frame;
				(**it).releasePosition = s->sustainEnd;
				(**it).endFrame = frame + rt;
				break;
			}
		}
	}
}

void BLow::allSoundsOff (const uint64_t frame)
{
	// TODO
	voices.clear();
}

void BLow::allNotesOff (const uint64_t frame)
{
	for (Voice** it = voices.begin(); it < voices.end(); ++it) noteOff ((**it).note, frame);
}

static LV2_Handle
instantiate	(const LV2_Descriptor* descriptor, double rate, const char* bundle_path, const LV2_Feature* const* features)
{
	BLow* instance = new BLow (rate, bundle_path, features);
	return (LV2_Handle) instance;
}

static void
connect_port(LV2_Handle instance, uint32_t port, void* data)
{
	BLow* bLow = (BLow*)instance;
	if (bLow) bLow->connect_port (port, data);
}

static void
run(LV2_Handle instance, uint32_t n_samples)
{
	BLow* bLow = (BLow*) instance;
	if (bLow) bLow->run (n_samples);
}

static void
cleanup(LV2_Handle instance)
{
	BLow* bLow = (BLow*) instance;
	if (bLow) delete bLow;
}

static const LV2_Descriptor descriptor =
{
	BLOW_URI,
	instantiate,
	connect_port,
	NULL,
	run,
	NULL,
	cleanup,
	NULL
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index)
	{
		case 0:  return &descriptor;
		default: return NULL;
	}
}
