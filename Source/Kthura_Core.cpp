#include <iostream>
#include <SlyvString.hpp>
#include <Kthura_Core.hpp>

#define KthuraObjVal(type,prop) void KthuraObject::prop(type v) {_Obj->prop=v;} type KthuraObject::prop() {return _Obj->prop;}
#define KthuraObjValRP(type,prop) void KthuraObject::prop(type v) {_Obj->prop=v; _parent->PerformAutoRemap();} type KthuraObject::prop() {return _Obj->prop;}

using namespace std;
using namespace Slyvina::Units;

namespace Slyvina {
	namespace Kthura {

#pragma region Panic
		KthuraPanicFunction KthuraPanic{ nullptr };

		static void DefPaniek(std::string msg, std::string xdata) {
			cout << "\x1b[31mKTHURA ERROR!\x1b[0m;\t";
			auto xd{ Split(xdata,';') };
			for (auto d : *xd) {
				auto s{ Split(d,':') };
				if (s->size() >= 2) {
					cout << (*s)[0] << ": ";
					byte b{ (byte)(s->size() + (byte)2) };
					if (b > 35) { cout << endl; b = 0; }
					for (byte i = b; i < 40; i++) {
						cout << " ";
					}
					cout << (*s)[1] << endl;
				}
			}
			exit(100);
		}

		inline void Paniek(std::string msg, std::string xdata="Note:No further data provided") {
			if (!KthuraPanic) KthuraPanic = DefPaniek;
			KthuraPanic(msg, xdata);
		}

#pragma endregion


#pragma region Layers
		void _Kthura::_LayerRemap() { _LayersMap.clear(); for (auto& l : _Layers) _LayersMap.push_back(l.first); }

		KthuraLayer* _Kthura::Layer(std::string Layer) {
			Trans2Upper(Layer);
			if (!_Layers.count(Layer)) { Paniek("Non-existend Layer", "Layer:" + Layer); return nullptr; }
			return &_Layers[Layer];
		}

		KthuraLayer* _Kthura::NewLayer(std::string Lay, bool nopanic) {
			Trans2Upper(Lay);
			if (_Layers.count(Lay)) {
				if (!nopanic) { Paniek("Layer already exists", "Layer:" + Lay); return nullptr; }
			} else {
				_Layers[Lay].__setparent(this);
			}
			LayersRemapped = false;
			return &_Layers[Lay];
		}

		void KthuraLayer::PerformAutoRemap() {
			if (_modified && _autoRemap) {
				TotalRemap();
				_modified = false;
			} else if (!_autoRemap) {
				_modified = true;
			}
		}

		void KthuraLayer::Kill(KthuraObject* k) {
			if (k->Parent() != this) { Paniek("Alien object kill requested!"); return; }
			if (_firstObject == k) _firstObject = _firstObject->Next();
			if (!k->Next()) _lastObject = k->Prev();
			if (!_lastObject) _firstObject = nullptr;
			k->__KillMe();
			delete k;
			PerformAutoRemap();
		}
		void KthuraLayer::Kill(std::string Tag,bool ignorenonexistent) {
			Trans2Upper(Tag);
			if (!TagMap.count(Tag)) {
				if (ignorenonexistent) return;
				Paniek("Trying to kill an object with a non-existent tag","Tag:"+Tag);
				return;
			}
			Kill(TagMap[Tag]);
		}

		void KthuraLayer::KillAllObjects() {
			auto _oa = _autoRemap;
			_autoRemap = false;
			while (_firstObject) Kill(_firstObject);
			_autoRemap = true;
			TotalRemap();
		}

		void KthuraLayer::RemapID() {
			IDMap.clear();
			for (auto o = _firstObject; o; o = o->Next()) {
				IDMap[o->ID()] = o;
			}
		}

		void KthuraLayer::RemapTags() {
			TagMap.clear();
			for (auto o = _firstObject; o; o = o->Next()) {
				if (TagMap.count(o->Tag())) Paniek("RemapTags(): Dupe tag (" + o->Tag() + ")");
				TagMap[o->Tag()] = o;
			}
		}

		void KthuraLayer::RemapLabels() {
			_LabelMap.clear();
			for (auto o = _firstObject; o; o = o->Next()) {
				if (o->labels().size()) {
					auto l{ Split(Upper(o->labels()),';') };
					for (auto& lb : *l) _LabelMap[lb].push_back(o);
				}
			}
		}

		void KthuraLayer::RemapDominance() {
			DomFirst = nullptr;
			//for (auto o = _firstObject; o; o = o->Next()) o->DomNext = nullptr;
			map<string, KthuraObject*> tempmap;
			for (auto o = _firstObject; o; o = o->Next()) {
				o->DomNext = nullptr;
				tempmap[TrSPrintF("DOM::%09d::%09d::%09d:%09d", o->dominance(), o->y(), o->x(), o->ID())] = o;
			}
			KthuraObject* Last{ nullptr };
			for (auto& dom : tempmap) {
				if (!Last) 
					DomFirst = dom.second;
				else {
					Last->DomNext = dom.second;
					Last = dom.second;
				}
			}
		}

		KthuraObject* KthuraLayer::NewTiledArea(int32 x, int32 y, int32 w, int32 h, std::string Texture, std::string Tag) {
			auto o = NewObject(KthuraKind::TiledArea);
			o->x(x);
			o->y(y);
			o->w(w);
			o->h(h);
			o->texture(Texture);
			o->Tag(Tag);
			return o;
		}

		void KthuraLayer::TotalRemap() {
			RemapTags();
#pragma message ("Still gotta do remapping by dominance")
			RemapID();
#pragma message ("Still gotta do remapping by labels")
			_modified = false;
		}

		KthuraObject* KthuraLayer::NewObject(KthuraKind k) {
			while (IDMap.count(++_objcnt)) {}
			auto o = new KthuraObject(_lastObject, this, k, _objcnt);
			_modified = true;
			return o;
		}

		KthuraObject* KthuraLayer::Obj(uint64 i) {
			if (HasID(i)) return IDMap[i];
			Paniek("Object doesn't exist", "Object ID:#" + to_string(i));
		}
#pragma endregion

#pragma Objects
		struct __KthuraObjectData {
			KthuraObject* parent{ nullptr };
			int32
				x{ 0 },
				y{ 0 },
				w{ 0 },
				h{ 0 },
				animspeed{-1};
			uint32
				dominance{ 20 },
				animskip{ 0 },
				animframe{ 0 };
			std::string 
				Tag{""},
				texture{""},
				labels{""};
			byte
				r{ 255 },
				g{ 255 },
				b{ 255 },
				a{ 255 };
			bool
				impassible{ false },
				forcepassible{ false },
				visible{ true };
		};

		struct __KthuraActorData {
			KthuraObject* parent{ nullptr };
		};

		KthuraObject::KthuraObject(KthuraObject* _after,KthuraLayer* _ouwe,KthuraKind k,uint64 _giveID) {
			_parent = _ouwe;
			if (_after) {
				_next = _after->_next;
				_prev = _after;
				_after->_next = this;
				if (_next) _next->_prev = this;
			}
			_Obj = std::make_unique<__KthuraObjectData>();
			if (k == KthuraKind::Actor) _Act = std::make_unique<__KthuraActorData>();
			_ID = _giveID;
		}

		void KthuraObject::__KillMe() {
			if (_prev) _prev->_next = _next;
			if (_next) _next->_prev = _prev;
			//delete this;
		}


		// Properties
		void KthuraObject::Tag(std::string t) {
			Trans2Upper(t);
			if (_Obj->Tag == t) return; // No changes = no actions needed
			_Obj->Tag = t;
			_parent->PerformAutoRemap();
		}
		std::string KthuraObject::Tag() { return _Obj->Tag; }
		
		KthuraObjVal(int32, x);
		KthuraObjVal(int32, y);
		KthuraObjVal(int32, w);
		KthuraObjVal(int32, h);
		KthuraObjVal(bool, visible);
		KthuraObjValRP(bool, impassible);
		KthuraObjValRP(bool, forcepassible);
		KthuraObjVal(std::string, texture);
		KthuraObjValRP(std::string, labels);
		KthuraObjValRP(uint32, dominance);
		KthuraObjVal(uint32, animframe);
		KthuraObjVal(int32, animspeed);
	}
}