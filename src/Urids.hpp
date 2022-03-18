/* B.Low
 * Dynamic distorted bandpass filter plugin
 *
 * Copyright (C) 2021 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef URIDS_HPP_
#define URIDS_HPP_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include "Definitions.hpp"

struct BLowURIs
{
	LV2_URID atom_Int;
	LV2_URID atom_Object;
	LV2_URID atom_Blank;
	LV2_URID atom_eventTransfer;
	LV2_URID midi_Event;
	LV2_URID blow_keyboardEvent;
	LV2_URID blow_noteOn;
	LV2_URID blow_noteOff;
	LV2_URID blow_velocity;
};

inline void getURIs (LV2_URID_Map* m, BLowURIs* uris)
{
	uris->atom_Int = m->map(m->handle, LV2_ATOM__Int);
	uris->atom_Object = m->map(m->handle, LV2_ATOM__Object);
	uris->atom_Blank = m->map(m->handle, LV2_ATOM__Blank);
	uris->atom_eventTransfer = m->map(m->handle, LV2_ATOM__eventTransfer);
	uris->midi_Event = m->map(m->handle, LV2_MIDI__MidiEvent);
	uris->blow_keyboardEvent = m->map(m->handle, BLOW_URI "#keyboardEvent");
	uris->blow_noteOn = m->map(m->handle, BLOW_URI "#noteOn");
	uris->blow_noteOff = m->map(m->handle, BLOW_URI "#noteOff");
	uris->blow_velocity = m->map(m->handle, BLOW_URI "#velocity");
}

#endif /* URIDS_HPP_ */
