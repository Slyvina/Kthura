#pragma once
#include <map>
#include <memory>
#include <Slyvina.hpp>
#include <SlyvString.hpp>


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

		enum class KthuraKind { Unknown, TiledArea, StretchedArea, Rect, Zone, Obstacle, Picture, Actor };

		class _Kthura {
		private:
			std::map<std::string, KthuraLayer> _Layers;
			std::vector<std::string> _LayersMap;
			void _LayerRemap();
			bool LayersRemapped{ false };
		public:
			StringMap MetaData = NewStringMap();
			inline bool HasLayer(std::string Lay) { Units::Trans2Upper(Lay); return _Layers.count(Lay); }
			KthuraLayer* Layer(std::string Lay);
			inline std::vector<std::string>* Layers() { if (!LayersRemapped) _LayerRemap(); return &_LayersMap; }
			KthuraLayer* NewLayer(std::string Lay, bool nopanic = false);
			void KillLayer(std::string Lay, bool ignoreifnonexistent = false);
		};

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
			KthuraObject(KthuraObject* _after,KthuraLayer* _ouwe,KthuraKind _k,uint64 giveID);
			inline KthuraObject* Next() { return _next; }
			inline KthuraObject* Prev() { return _prev; }
			inline KthuraLayer* Parent() { return _parent; }
			void __KillMe(); // NEVER use this directly! ALWAYS let the kill commands of the Kthura Layer do this.
			inline int64 ID();
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
		};

		class KthuraLayer {
		private:
			uint64 _objcnt{ 0 };
			_Kthura* _parent{nullptr};
			KthuraObject* _firstObject;
			KthuraObject* _lastObject;
			bool _modified{ false };
			bool _autoRemap{ true };
			KthuraObject* NewObject(KthuraKind k);
			std::map<uint64, KthuraObject*> IDMap{};
			std::map<std::string, KthuraObject*> TagMap{};
		public:
			void PerformAutoRemap();
			inline void __setparent(_Kthura* ouwe) { if (!_parent) _parent = ouwe; } // Only works when not yet initized. Not for direct use			
			void Kill(KthuraObject* obj);
			void KillAllObjects();
			void TotalRemap();
			inline bool HasID(uint64 i) { return IDMap.count(i); }
			inline bool HasTag(std::string T) { Units::Trans2Upper(T); return TagMap.count(T); }
			void RemapID();
			void RemapTags();
			KthuraObject* Obj(uint64 i);
			KthuraObject* NewTiledArea(int32 x = 0, int32 y = 0, int32 w = 0, int32 h = 0,std::string Texture="",std::string Tag="");
			inline ~KthuraLayer() { KillAllObjects(); }
		};
	}
}