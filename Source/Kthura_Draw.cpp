// Lic:
// Kthura/Source/Kthura_Draw.cpp
// Kthura Draw
// version: 23.09.27
// Copyright (C) 2015-2022, 2023 Jeroen P. Broks
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// EndLic

#include "../Headers/Kthura_Draw.hpp"


namespace Slyvina {
	namespace Kthura {
		void _KthuraDraw::DrawObject(KthuraObject* o, int insx, int insy) {
			if (!AllowDraw.count(o->Kind())) AllowDraw[o->Kind()]=false;
			if (o->visible() && AllowDraw[o->Kind()]) {
				if (!DrawFuncs.count(o->Kind())) {
					throw std::runtime_error("No function set to draw Kthura objects of kind: " + o->SKind());
				}
				DrawFuncs[o->Kind()](o, insx, insy);
			}
		}

		_KthuraDraw::_KthuraDraw(std::map<KthuraKind, FKthuraDrawObject> DF, std::map<KthuraKind, bool> AD) {
			DrawFuncs = DF;
			AllowDraw = AD;
		}

		_KthuraDraw::_KthuraDraw(std::map<KthuraKind, FKthuraDrawObject> DF) {
			DrawFuncs = DF;
			AllowDraw = {
				{KthuraKind::Actor,true},
				{KthuraKind::Custom,false},
				{KthuraKind::Exit,false},
				{KthuraKind::Obstacle,true},
				{KthuraKind::Picture,true}, // Please note. Pictures should NOT be used anymore. They only exist due to converted TeddyBear maps.
				{KthuraKind::Pivot,false},
				{KthuraKind::Rect,true},
				{KthuraKind::StretchedArea,true},
				{KthuraKind::TiledArea,true},
				{KthuraKind::Unknown,false},
				{KthuraKind::Zone,false}
			};
		}
		void _KthuraDraw::DrawLayer(KthuraLayer* L, int insx, int insy) {
			for (auto o = L->DomFirst; o; o->DomNext) DrawObject(o,insx,insy);
		}
	}
}