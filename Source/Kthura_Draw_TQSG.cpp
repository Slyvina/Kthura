// Lic:
// Kthura/Source/Kthura_Draw_TQSG.cpp
// Kthura Draw TQSG driver
// version: 23.09.27
// Copyright (C) 2023 Jeroen P. Broks
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


#include <TQSG.hpp>
#include <TQSE.hpp>
#include <SlyvTime.hpp>

#include "../Headers/Kthura_Draw_TQSG.hpp"

namespace Slyvina {

	using namespace TQSG;
	using namespace TQSE;
	using namespace JCR6;
	using namespace Units;

	namespace Kthura {

#pragma region "External data"
		static TImageFont EdtFont{nullptr};
		static JT_Dir TexDir{nullptr};
#pragma endregion

#pragma region Textures
		struct KillTex { KthuraKind K{ KthuraKind::Unknown }; std::string T{} };

		const int cleanuptime = 3600;
		const int cleanupreset = 1200;

		class MyTexture {
		private:
			static time_t CleanCnt;
			static std::map<KthuraKind, std::map<std::string, MyTexture>> Reg;
			TImage _Tex{ nullptr };
			static void AutoKill();
		public:
			time_t LastCalled;


			MyTexture(KthuraKind K,std::string Entry) {
				auto gt{ TimeStamp() };
				LastCalled = gt;
				Reg[K][Upper(Entry)] = *this;
				_Tex = LoadImage(TexDir, Entry);
				if (K == KthuraKind::Obstacle) {
					// TODO: Take note of .hot files
					_Tex->HotBottomCenter();
				}
			}

			inline TImage Tex() { 
				LastCalled = TimeStamp();  
				if (abs(TimeStamp() - CleanCnt) > cleanupreset) {
					AutoKill();
					CleanCnt = TimeStamp();
				}
				return _Tex; 
			}
			static inline TImage Tex(KthuraKind K, std::string T) {
				Trans2Upper(T);
				if (!Reg[K].count(T)) Reg[K][T] = { K,T };
				return Reg[K][T].Tex();
			}
		};
		time_t MyTexture::CleanCnt{0};
		std::map<KthuraKind, std::map<std::string, MyTexture>> MyTexture::Reg{};

		void MyTexture::AutoKill() {
			auto Nu{ TimeStamp() };
			std::vector<KillTex> Victims;
			for (auto bKind : Reg) {
				for (auto bTex : bKind.second) {
					if (abs(Nu - bTex.second.LastCalled) > cleanuptime) Victims.push_back({ bKind.first,bTex.first });
				}
			}
			for (auto Victim : Victims) {
				Reg[Victim.K].erase(Victim.T);
			}
		}

#pragma endregion


#pragma region "Driver Functions"

		inline void Col(KthuraObject* o) {
			SetColor(
				o->red(),
				o->green(),
				o->blue(),
				o->alpha()
			);
		}

		static void _Obstacle(KthuraObject* o, int ScrollX, int ScrollY) {
			auto scx{ (double)o->scalex() / 1000 };
			auto scy{ (double)o->scaley() / 1000 };
			auto Tex{ MyTexture::Tex(o->Kind(),o->texture()) };
			Col(o);
			SetScale(scx, scy);
			Rotate(o->rotatedeg());
			Tex->XDraw(o->x() - ScrollX, o->y() - ScrollY, o->animframe());
			SetScale(1);
			Rotate(0);
		}

		static void _Rect(KthuraObject* o, int ScrollX, int ScrollY) {
			Col(o);
			ARect(o->x() - ScrollX, o->y() - ScrollY, o->w(), o->h());
			// Please note! ARect and not Rect. This driver is meant to keep alternate screens into account and Rect doesn't do that.
		}

		static void _Zone(KthuraObject* o, int ScrollX, int ScrollY){
			SetColor(o->red(), o->green(), o->blue(), 255);
			EdtFont->Text(o->Tag(), 2 + (o->x() - ScrollX), 2 + (o->y() - ScrollY));
			SetColor(o->red(), o->green(), o->blue(), 120);
			ARect(o->x() - ScrollX, o->y() - ScrollY, o->w(), o->h());
		}

		static void _Tiled(KthuraObject* o, int ScrollX, int ScrollY) {
			auto Tex{ MyTexture::Tex(KthuraKind::TiledArea,o->texture()) };
			Col(o);
			Tex->Tile(o->x() - ScrollX, o->y() - ScrollY, o->w(), o->h(), o->animframe(), -o->insertx(), -o->inserty());
		}

		static void _Stretch(KthuraObject* o, int ScrollX, int ScrollY) {
			auto Tex{ MyTexture::Tex(KthuraKind::TiledArea,o->texture()) };
			Col(o);
			Tex->StretchDraw(o->x() - ScrollX, o->y() - ScrollY, o->w(), o->h(), o->animframe());
		}

		static void Mark(int x, int y) {		
			ARect(x - 5, y - 5, 11, 11);
			ALine(x - 5, y, x + 5, y);
			ALine(x, y - 5, x, y + 5);
		}

		static void _Pivot(KthuraObject* o, int ScrollX, int ScrollY) {
			SetColor(255, 255, 255);
			Mark(o->x() - ScrollX, o->y() - ScrollY);
		}

		static void _Exit(KthuraObject* o, int ScrollX, int ScrollY) {
			int
				dx{ o->x() - ScrollX },
				dy{ o->y() - ScrollY };
			bool showt{ ScreenWidth(false) == ScreenWidth(true) && ScreenHeight(false) == ScreenHeight(true) };
			SetColor(100, 255, 0);
			Mark(o->x() - ScrollX, o->y() - ScrollY);
			if (showt && MouseX() > o->x() - 5 && MouseX() < o->x() + 5 && MouseY() > o->y() - 5 && MouseY() < o->y() + 5)
				EdtFont->Text(o->Tag(), o->x(), o->y() + 6, Align::Center, Align::Top);
		}

		static void _Custom(KthuraObject* o, int ScrollX, int ScrollY) {
			int
				dx{ o->x() - ScrollX },
				dy{ o->y() - ScrollY };
			bool showt{ ScreenWidth(false) == ScreenWidth(true) && ScreenHeight(false) == ScreenHeight(true) };
			Col(o);
			Mark(o->x() - ScrollX, o->y() - ScrollY);
			if (showt && MouseX() > o->x() - 5 && MouseX() < o->x() + 5 && MouseY() > o->y() - 5 && MouseY() < o->y() + 5) {
				EdtFont->Text(o->SKind(), o->x(), o->y() + 6, Align::Center, Align::Top);
				EdtFont->Text(o->Tag(), o->x(), o->y() + 6 + EdtFont->Height(o->SKind()), Align::Center, Align::Top);
			}
		}

		static void _Pic(KthuraObject* o, int ScrollX, int ScrollY) {
			auto Tex{ MyTexture::Tex(o->Kind(),o->texture()) };
			Col(o);
			Tex->Draw(o->x() - ScrollX, o->y() - ScrollY, o->animframe());
		}


#pragma endregion

		
		
#pragma region Initization		
		void TQSGKthuraFont(TImageFont F) { EdtFont = F; }


		DrawFuncMap FuncDriver{
			{KthuraKind::Obstacle,_Obstacle},
			{KthuraKind::Rect,_Rect},
			{KthuraKind::Zone,_Zone},
			{KthuraKind::TiledArea,_Tiled},
			{KthuraKind::StretchedArea,_Stretch},
			{KthuraKind::Pivot,_Pivot},
			{KthuraKind::Exit,_Exit},
			{KthuraKind::Custom,_Custom},
			{KthuraKind::Picture,_Pic}
			
		};

		KthuraDraw Init_TQSG_For_Kthura(JT_Dir J) {
			TexDir = J;
			return std::unique_ptr<_KthuraDraw>(new _KthuraDraw(FuncDriver));
		}

		KthuraDrawShared Init_TQSG_For_Kthura_Shared(JT_Dir J) {
			TexDir = J;
			return std::shared_ptr<_KthuraDraw>(new _KthuraDraw(FuncDriver));
		}
#pragma endregion
	}
}