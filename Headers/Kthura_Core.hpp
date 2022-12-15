// Lic:
// Kthura/Headers/Kthura_Core.hpp
// Slyvina - Kthura Core (header)
// version: 22.12.15
// Copyright (C) 2022 Jeroen P. Broks
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
#include <map>
#include <string>
#include <memory>
#include <Slyvina.hpp>
#include <SlyvString.hpp>
#include <JCR6_Core.hpp>


#define KthuraProp(type,prop) void prop(type); type prop()
#define KthuraPropAlias(type,prop,alias) inline void alias(type v) {prop(v);} inline type alias() { return prop(); }

namespace Slyvina {

	namespace Kthura {

		class _Kthura; // NEVER us this class directly, but always use UKthura or Kthura in stead!!!

		typedef std::unique_ptr<_Kthura> UKthura; // Type used to store a Kthura map at a unique pointer
		typedef std::shared_ptr<_Kthura> Kthura; // Type used to store a Kthura map to a shared pointer. 

		typedef  void (*KthuraPanicFunction)(std::string errormessage, std::string xdata);
		extern KthuraPanicFunction KthuraPanic;


		struct __KthuraObjectData;
		struct __KthuraActorData;

		class KthuraObject;
		class KthuraLayer;

		enum class KthuraKind { Unknown, TiledArea, StretchedArea, Rect, Zone, Obstacle, Picture, Actor, Custom, Exit, Pivot };

		class _Kthura {
		private:
			static uint64 _KthCount;
			uint64 _KthID{ _KthCount++ };
			std::map<std::string, KthuraLayer> _Layers;
			std::vector<std::string> _LayersMap;
			void _LayerRemap();
			bool LayersRemapped{ false };
		public:
			static bool AllowUnknown; // If set to 'true' Kthura will load all entries found in a Kthura resource file (as long as in the same dir as Kthura's data) that it doesn't know in the memory and also save it with the map accordingly when saving. This setting is recommended for editors, but not for games using Kthura maps.
			std::map<std::string, Units::Bank> Unknown{};
			inline uint64 ID() { return _KthID; } // This number will be used by my graphics driver for disposing loaded textures it no longer needs.
			StringMap MetaData = NewStringMap();
			inline bool HasLayer(std::string Lay) { Units::Trans2Upper(Lay); return _Layers.count(Lay); }
			KthuraLayer* Layer(std::string Lay);
			inline std::vector<std::string>* Layers() { if (!LayersRemapped) _LayerRemap(); return &_LayersMap; }
			KthuraLayer* NewLayer(std::string Lay, bool nopanic = false);
			inline void KillLayer(std::string Lay, bool ignoreifnonexistent = false);

			_Kthura(Slyvina::JCR6::JT_Dir Resource, std::string prefix = "");
			inline _Kthura() { NewLayer("__BASE"); }
		};

		std::string KindName(KthuraKind k);
		KthuraKind KindName(std::string s);

		class KthuraObject {
		private:
			std::unique_ptr<__KthuraObjectData> _Obj{ nullptr };
			std::unique_ptr<__KthuraActorData> _Act{ nullptr };
			KthuraObject* _prev{ nullptr };
			KthuraObject* _next{ nullptr };
			KthuraLayer* _parent{ nullptr };
			KthuraKind _Kind;
			uint64 _ID{ 0 };
		public:
			std::string CustomKind{ "" }; // Only needs to be used when this is a custom kind
			KthuraObject* DomNext{ nullptr }; // It would NOT be wise to alter this manually!
			KthuraObject(KthuraObject* _after,KthuraLayer* _ouwe,KthuraKind _k,uint64 giveID);
			inline KthuraObject* Next() { return _next; }
			inline KthuraObject* Prev() { return _prev; }
			inline KthuraLayer* Parent() { return _parent; }
			void __KillMe(); // NEVER use this directly! ALWAYS let the kill commands of the Kthura Layer do this.
			inline int64 ID() { return _ID; }
			inline KthuraKind Kind() { return _Kind; }
			inline void IKind(KthuraKind K) { if (_Kind == KthuraKind::Unknown) _Kind = K; }
			std::string SKind();

			// Yeah, these are quick defintions done by macros.
			// I do NOT need you to tell me that this is bad practise, because I already know and I don't care!
			KthuraProp(std::string, Tag);
			KthuraProp(int32, x);
			KthuraProp(int32, y);
			KthuraProp(int32, w); KthuraPropAlias(int32, w, width);
			KthuraProp(int32, h); KthuraPropAlias(int32, h, height);
			KthuraProp(byte, r); KthuraPropAlias(byte, r, red);
			KthuraProp(byte, g); KthuraPropAlias(byte, g, green);
			KthuraProp(byte, b); KthuraPropAlias(byte, b, blue);
			KthuraProp(byte, a); KthuraPropAlias(byte, b, alpha);
			KthuraProp(bool, visible);
			KthuraProp(bool, impassible);
			KthuraProp(bool, forcepassible);
			KthuraProp(std::string, texture);
			KthuraProp(std::string, labels);
			KthuraProp(uint32, dominance);
			KthuraProp(int32, animspeed);
			KthuraProp(uint32, animframe);
			KthuraProp(int32, insertx);
			KthuraProp(int32, inserty);
			KthuraProp(int32, scalex);
			KthuraProp(int32, scaley);
			KthuraProp(int32, rotatedeg); KthuraPropAlias(int32, rotatedeg, rotate);
			KthuraProp(double, rotaterad);
			KthuraProp(int32, blend);
			void data(std::string,std::string);
			std::string data(std::string);
			std::map<std::string, std::string>* data();

		};

		class KthuraLayer {
		private:
			uint64 _objcnt{ 0 };
			_Kthura* _parent{nullptr};
			KthuraObject* _firstObject;
			KthuraObject* _lastObject;
			bool _modified{ false };
			bool _autoRemap{ true };
			std::map<uint64, KthuraObject*> IDMap{};
			std::map<std::string, KthuraObject*> TagMap{};
			std::map<std::string, std::vector<KthuraObject*>> _LabelMap;
		public:
			KthuraObject* DomFirst{ nullptr };
			uint32
				gridx{ 32 },
				gridy{ 32 };
			KthuraObject* NewObject(KthuraKind k);
			void PerformAutoRemap();
			inline void __setparent(_Kthura* ouwe) { if (!_parent) _parent = ouwe; } // Only works when not yet initized. Not for direct use			
			void Kill(KthuraObject* obj);
			void Kill(std::string Tag,bool ignorenonexistent=true);
			void KillAllObjects();
			uint64 CountObjects();
			void TotalRemap();
			inline bool HasID(uint64 i) { return IDMap.count(i); }
			inline bool HasTag(std::string T) { Units::Trans2Upper(T); return TagMap.count(T); }
			void RemapID();
			void RemapTags();
			void RemapLabels();
			void RemapDominance();
			inline KthuraObject* FirstObject() { return _firstObject; }
			KthuraObject* Obj(uint64 i);
			KthuraObject* NewTiledArea(int32 x = 0, int32 y = 0, int32 w = 0, int32 h = 0,std::string Texture="",std::string Tag="");
			KthuraObject* NewObstacle(int32 x = 0, int32 y = 0, std::string Texture = "", std::string Tag = "");
			inline ~KthuraLayer() { KillAllObjects(); }
			inline void AutoRemap(bool onoff) { _autoRemap = onoff; PerformAutoRemap(); }
			inline bool AutoRemap() { return _autoRemap; }			
		};



		inline Kthura LoadKthura(Slyvina::JCR6::JT_Dir J, std::string prefix="") { return std::make_shared<_Kthura>(J, prefix); }
		Kthura LoadKthura(std::string resfile, std::string prefix = "");
		inline UKthura LoadUKthura(Slyvina::JCR6::JT_Dir J, std::string prefix = "") { return std::make_unique<_Kthura>(J, prefix); }
		UKthura LoadUKthura(std::string resfile,  std::string prefix = "");
		inline Kthura CreateKthura() { return std::make_shared<_Kthura>(); }
		inline UKthura CreateUniqueKthura() { return std::make_unique<_Kthura>(); }
	}
}