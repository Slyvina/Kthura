// Lic:
// Kthura/Source/Kthura_Draw.cpp
// Kthura Draw
// version: 23.09.28
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

#undef KTHDRAWDEBUG

#include "../Headers/Kthura_Draw.hpp"


namespace Slyvina {
	namespace Kthura {
		void _KthuraDraw::DrawObject(KthuraObject* o, int insx, int insy) {
			if (!AllowDraw.count(o->Kind())) AllowDraw[o->Kind()]=false;
#ifdef KTHDRAWDEBUG
			printf("Object #%d (%s); AllowKind: %d; Visible: %d\n", o->ID(), o->SKind().c_str(), AllowDraw[o->Kind()], o->visible());
#endif
			if (o->Kind() == KthuraKind::Actor) {
				// Comes later!
				
			} else {
				Animate(o);
			}
			if (o->visible() && AllowDraw[o->Kind()]) {
				if (!DrawFuncs.count(o->Kind())) {
					throw std::runtime_error("No function set to draw Kthura objects of kind: " + o->SKind());
				}
#ifdef KTHDRAWDEBUG
				printf("Draw this object!\n");
#endif
				DrawFuncs[o->Kind()](o, insx, insy);
			}
		}

		_KthuraDraw::_KthuraDraw(std::map<KthuraKind, FKthuraDrawObject> DF, std::map<KthuraKind, bool> AD, FKthuraSize Sz, FKthuraFrames Fr) {
			DrawFuncs = DF;
			AllowDraw = AD;
			_ObstacleSize = Sz;
			_Frames = Fr;
		}

		_KthuraDraw::_KthuraDraw(std::map<KthuraKind, FKthuraDrawObject> DF,FKthuraSize Sz,FKthuraFrames Fr) {
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
			_ObstacleSize = Sz;
			_Frames = Fr;
		}
		KthuraRect _KthuraDraw::ObjectSize(KthuraObject* o) {
			switch (o->Kind()) {
			case KthuraKind::Unknown:
				return { 0,0,0,0 };
			case KthuraKind::TiledArea:
			case KthuraKind::StretchedArea:
			case KthuraKind::Rect:
			case KthuraKind::Zone:
				return { o->x(),o->y(),o->w(),o->h() };
			case KthuraKind::Actor:
			case KthuraKind::Obstacle:
			case KthuraKind::Picture:
				if (_ObstacleSize) return _ObstacleSize(o);
				return { o->x(),o->y(),0,0 };
			case KthuraKind::Exit:
			case KthuraKind::Pivot:
			case KthuraKind::Custom:
				if (CheckPivotExitSelf) return _ObstacleSize(o);
				return { o->x(),o->y(),0,0 };
			}
			return KthuraRect();
		}
		bool _KthuraDraw::InsideObject(KthuraObject* o, int x, int y) {
			auto s{ ObjectSize(o) };
			//*
			if (o->Kind() == KthuraKind::Exit) {
				printf("Obj #%3d; Click (%d,%d) -> (%d,%d) %dx%d\n", (int)o->ID(), x, y, s.x, s.y, s.w, s.h);
			}
			//*/
			return
				x >= s.x &&
				y >= s.y &&
				x <= s.x + s.w &&
				y <= s.y + s.h;
		}

		int _KthuraDraw::Frames(KthuraObject* o) {
			switch (o->Kind()) {
			case KthuraKind::TiledArea:
			case KthuraKind::StretchedArea:
			case KthuraKind::Actor:
			case KthuraKind::Obstacle:
			case KthuraKind::Picture:
				if (_Frames) {
					return std::max(1, _Frames(o));
				}
				// FALLTHROUGH! SO NO BREAK HERE!
			default:
				return 1;
			}			
		}

		void _KthuraDraw::Animate(KthuraObject* o) {
			if (o->animspeed() < 0) return;
			//printf("Obj #%d >> Spd: %d; Skip: %d; Frame: %d/%d\n", (int)o->ID(),o->animspeed(),o->animskip(),o->animframe(),Frames(o)); // DEBUG!!!
			o->animskip((o->animskip() + 1) % (o->animspeed() + 1));
			if (o->animskip()==0) {
				o->animframe((o->animframe() + 1) % Frames(o));
			}
		}

		void _KthuraDraw::DrawLayer(KthuraLayer* L, int insx, int insy) {
#ifdef KTHDRAWDEBUG
			printf("Draw Layer\n");
#endif
			for (auto o = L->DomFirst; o; o= o->DomNext) {
				//std::cout << (int)L->DomFirst << "\t" << (int)o << "\t" << o->DomNext << " \n"; // debug
				DrawObject(o, insx, insy);
			}
		}
	}
}