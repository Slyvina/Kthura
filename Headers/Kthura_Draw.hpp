// Lic:
// Kthura/Headers/Kthura_Draw.hpp
// Kthura Draw Base (header)
// version: 23.10.08
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
#pragma once
#include "Kthura_Core.hpp"

namespace Slyvina {
	namespace Kthura {

		class _KthuraDraw;

		struct KthuraRect { int x, y, w, h; };

		typedef std::unique_ptr<_KthuraDraw> KthuraDraw;
		typedef std::shared_ptr<_KthuraDraw> KthuraDrawShared;

		typedef void (*FKthuraDrawObject)(KthuraObject*,int,int);
		typedef KthuraRect (*FKthuraSize)(KthuraObject*);
		typedef int (*FKthuraFrames)(KthuraObject*);

		typedef std::map<KthuraKind, FKthuraDrawObject> DrawFuncMap;
		typedef std::map<KthuraKind, bool> DrawAllowMap;


		class _KthuraDraw{
		private:
		public:
			bool CheckPivotExitSelf{false};
			std::map<KthuraKind, FKthuraDrawObject> DrawFuncs{};
			std::map<KthuraKind, bool> AllowDraw{};
			void DrawObject(KthuraObject* o, int insx, int insy);			
			_KthuraDraw(std::map<KthuraKind, FKthuraDrawObject> DF, std::map<KthuraKind, bool> AD, FKthuraSize FKS = nullptr, FKthuraFrames Fr = nullptr);
			_KthuraDraw(std::map<KthuraKind, FKthuraDrawObject> DF, FKthuraSize FKS = nullptr, FKthuraFrames Fr = nullptr);
			FKthuraSize _ObstacleSize{ nullptr };
			FKthuraFrames _Frames{ nullptr };

			KthuraRect ObjectSize(KthuraObject* o);
			bool InsideObject(KthuraObject* o, int x, int y);
			int Frames(KthuraObject* o);
			void Animate(KthuraObject* o);

			void DrawLayer(KthuraLayer* L, int insx, int insy);
		};

		inline KthuraDraw MakeDrawDriver(std::map<KthuraKind, FKthuraDrawObject> DF, std::map<KthuraKind, bool> AD) { return std::unique_ptr<_KthuraDraw>(new _KthuraDraw(DF, AD)); }
		inline KthuraDraw MakeDrawDriver(std::map<KthuraKind, FKthuraDrawObject> DF) { return std::unique_ptr<_KthuraDraw>(new _KthuraDraw(DF)); }
		inline KthuraDrawShared MakeDrawDriverShared(std::map<KthuraKind, FKthuraDrawObject> DF, std::map<KthuraKind, bool> AD) { return std::shared_ptr<_KthuraDraw>(new _KthuraDraw(DF, AD)); }
		inline KthuraDrawShared MakeDrawDriverShared(std::map<KthuraKind, FKthuraDrawObject> DF) { return std::shared_ptr<_KthuraDraw>(new _KthuraDraw(DF)); }


	}
}