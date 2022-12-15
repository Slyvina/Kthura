// Lic:
// Kthura/Source/Kthura_Core.cpp
// Slyvina - Kthura Core
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
#include <iostream>
#include <SlyvString.hpp>
#include <SlyvSTOI.hpp>
#include <Kthura_Core.hpp>

#define KthuraCoreDebug



#define KthuraObjVal(type,prop) void KthuraObject::prop(type v) {_Obj->prop=v;} type KthuraObject::prop() {return _Obj->prop;}
#define KthuraObjValRP(type,prop) void KthuraObject::prop(type v) {_Obj->prop=v; _parent->PerformAutoRemap();} type KthuraObject::prop() {return _Obj->prop;}
#define KthuraObjValDefFunc(type,prop)  type KthuraObject::prop() { return _Obj->prop;} void KthuraObject::prop(type value) 

#ifdef KthuraCoreDebug
#define Chat(msg) cout << "\x1b[33mKthura Debug>\t\x1b[0m" << msg << endl
#else
define Chat(msg)
#endif
using namespace std;
using namespace Slyvina::Units;
using namespace Slyvina::JCR6;

namespace Slyvina {
	namespace Kthura {

#pragma region Panic
		KthuraPanicFunction KthuraPanic{ nullptr };

		static void DefPaniek(std::string msg, std::string xdata) {
			cout << "\x1b[31mKTHURA ERROR!\x1b[0m\t"<<msg<<endl;
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

		inline void Paniek(std::string msg, std::string xdata = "Note:No further data provided") {
			if (!KthuraPanic) KthuraPanic = DefPaniek;
			KthuraPanic(msg, xdata);
		}

#pragma endregion

#pragma region EntireMap
		uint64 _Kthura::_KthCount{0};
		bool _Kthura::AllowUnknown{ false };
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
		void KthuraLayer::Kill(std::string Tag, bool ignorenonexistent) {
			Trans2Upper(Tag);
			if (!TagMap.count(Tag)) {
				if (ignorenonexistent) return;
				Paniek("Trying to kill an object with a non-existent tag", "Tag:" + Tag);
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
				if (o->Tag().size()) {
					if (TagMap.count(o->Tag())) Paniek("RemapTags(): Dupe tag (" + o->Tag() + ")");
					TagMap[o->Tag()] = o;
				}
			}
		}

		uint64 KthuraLayer::CountObjects() {
			uint64 ret{ 0 };
			for (auto o = _firstObject; o; o = o->Next()) ret++;
			return ret;
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

		KthuraObject* KthuraLayer::NewObstacle(int32 x, int32 y, std::string Texture, std::string Tag) {
			auto o = NewObject(KthuraKind::Obstacle);
			o->x(x);
			o->y(y);
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
			if (!_firstObject) _firstObject = o;
			_lastObject = o;
			_modified = true;
			return o;
		}

		KthuraObject* KthuraLayer::Obj(uint64 i) {
			if (HasID(i)) return IDMap[i];
			Paniek("Object doesn't exist", "Object ID:#" + to_string(i));
		}
#pragma endregion

#pragma region Objects
		struct __KthuraObjectData {
			KthuraObject* parent{ nullptr };
			int32
				x{ 0 },
				y{ 0 },
				w{ 0 },
				h{ 0 },
				animspeed{ -1 },
				insertx{ 0 },
				inserty{ 0 },
				scalex{ 1000 },
				scaley{ 1000 },
				rotatedeg{ 0 };
			double
				truescalex{ 1 },
				truescaley{ 1 },
				rotaterad{ 0 };
			uint32
				dominance{ 20 },
				animskip{ 0 },
				animframe{ 0 },
				blend{ 0 };
			std::string
				Tag{ "" },
				texture{ "" },
				labels{ "" };
			byte
				r{ 255 },
				g{ 255 },
				b{ 255 },
				a{ 255 };
			bool
				impassible{ false },
				forcepassible{ false },
				visible{ true };
			map<string,string>
				data{ };
		};

		struct __KthuraActorData {
			KthuraObject* parent{ nullptr };
		};

		KthuraObject::KthuraObject(KthuraObject* _after, KthuraLayer* _ouwe, KthuraKind k, uint64 _giveID) {
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
		KthuraObjVal(byte, r);
		KthuraObjVal(byte, g);
		KthuraObjVal(byte, b);
		KthuraObjVal(byte, a);
		KthuraObjVal(bool, visible);
		KthuraObjValRP(bool, impassible);
		KthuraObjValRP(bool, forcepassible);
		KthuraObjVal(std::string, texture);
		KthuraObjValRP(std::string, labels);
		KthuraObjValRP(uint32, dominance);
		KthuraObjVal(uint32, animframe);
		KthuraObjVal(int32, animspeed);
		KthuraObjVal(int32, insertx);
		KthuraObjVal(int32, inserty);
		KthuraObjValDefFunc(int32, scalex) { _Obj->scalex = value; _Obj->truescalex = (double)value / (double)1000; }
		KthuraObjValDefFunc(int32, scaley) { _Obj->scaley = value; _Obj->truescaley = (double)value / (double)1000; }
		KthuraObjValDefFunc(int, rotatedeg) { _Obj->rotatedeg = value; _Obj->rotaterad = (double)(((double)value) * (180.0 / PI)); }
		KthuraObjValDefFunc(double, rotaterad) { _Obj->rotaterad = value; _Obj->rotatedeg = (int)floor(value * (180.0 / PI)); }
		KthuraObjVal(int32, blend);
		void KthuraObject::data(std::string k, std::string v) { _Obj->data[Upper(k)] = v; }
		std::string KthuraObject::data(std::string k) { return _Obj->data[Upper(k)]; }
		std::map<std::string, std::string>* KthuraObject::data() { return &(_Obj->data); }


		static map<KthuraKind, string> _KindName{
			{KthuraKind::Unknown,"Unknown"},
			{KthuraKind::Actor,"Actor"},
			{KthuraKind::Obstacle,"Obstacle"},
			{KthuraKind::Picture,"Picture"},
			{KthuraKind::Rect,"Rect"},
			{KthuraKind::StretchedArea,"StretchedArea"},
			{KthuraKind::TiledArea,"TiledArea"},
			{KthuraKind::Zone,"Zone"},
			{KthuraKind::Custom,"Custom"},
			{KthuraKind::Pivot,"Pivor"},
			{KthuraKind::Exit,"Exit"}
		};

		std::string KindName(KthuraKind k) {
			Chat("Kind requestsed (" << (int)k << ")   Found: " << boolstring(_KindName.count(k)));
			if (_KindName.count(k)) {
				Chat("Returning: " << _KindName[k]);
				return _KindName[k];
			}
			return "Unknown";
		}

		KthuraKind KindName(std::string s) {
			if (!s.size()) return KthuraKind::Unknown;
			if (s[0] == '$') return KthuraKind::Custom;
			for (auto& kn : _KindName) {
				if (Upper(s) == Upper(kn.second)) return kn.first;
			}
			return KthuraKind::Unknown;
		}
		std::string KthuraObject::SKind() { 
			if (_Kind == KthuraKind::Custom) return CustomKind; else return KindName(_Kind); 
		}
#pragma endregion

#pragma region Loader
#define I_Want_An_Object if (!co) { Paniek("I cannot define field '"+fld+"' without an object",TrSPrintF("Line:%d;Instruction:%s",line,instruction.c_str())); return; }
#define SyntaxError { Paniek("Syntax error", TrSPrintF("Line:%d;Instruction:%s", line, instruction.c_str())); return; }

		_Kthura::_Kthura(Slyvina::JCR6::JT_Dir Resource, std::string prefix) {
			if (prefix.size() && (!Suffixed(prefix, "/"))) prefix += "/";
			if (!Resource->EntryExists(prefix + "Data")) { Paniek("JCR6 resource doesn't appear to have the '" + prefix + "Data' entry that Kthura needs"); return; }
			if (!Resource->EntryExists(prefix + "Objects")) { Paniek("JCR6 resource doesn't appear to have the '" + prefix + "Objects' entry that Kthura needs"); return; }
			auto entries{ Resource->Entries() };
			for (auto ent : *entries) {
				auto n = ent->Name().substr(prefix.size());
				auto nu = Upper(n);
				Chat("Processing entry: " << n);
				if (nu == "DATA") {
					Chat("Reading Data");
					MetaData = Resource->GetStringMap(prefix + "Data");
					if ((!MetaData) || Last()->Error) {
						Paniek("Something went wrong reading data", "JCR6:" + Last()->ErrorMessage); return;
					}
				} else if (nu == "OBJECTS") {
					Chat("Reading objects");
					auto Source{ Resource->GetLines(prefix + "Objects") };
					auto LayerScope{ false };
					KthuraObject* co{ nullptr };
					KthuraLayer* cl{ nullptr };
					for (size_t _line = 0; _line < Source->size(); _line++) {
						auto line{ _line + 1 }; // Chat("Parsing line: " << line << " (Vector position: " << _line << ") out of " << Source->size());
						auto instruction{ Trim((*Source)[_line]) };
						if (instruction == "" || Prefixed(instruction, "--")) {
							// Do nothing at all. These lines have no values!
						} else if (LayerScope) {
							if (instruction == "__END") {
								Chat("End Layer Scope");
								LayerScope = false;
							} else {
								Chat("Creating new layer: " << instruction);
								NewLayer(instruction)->AutoRemap(false);
							}
							Chat("Layer scope " << boolstring(LayerScope) << "; '" << instruction << "'" << "; HMMMM: " << boolstring(instruction == "__END"));
						} else if (instruction == "LAYERS") {
							LayerScope = true;
						} else if (instruction == "NEW") {
							if (!cl) { Paniek("Object without a layer", TrSPrintF("Line:%d", line)); return; }
							co = cl->NewObject(KthuraKind::Unknown);
							Chat("Objects in layer: " << cl->CountObjects());
						} else {
							auto isteken{ FindFirst(instruction,'=') };
							if (isteken < 0) SyntaxError;
							auto fld = Trim(instruction.substr(0, isteken));
							auto val = Trim(instruction.substr(isteken + 1));
							if (fld == "KIND") {
								I_Want_An_Object;
								co->IKind(KindName(val));
								if (co->Kind() == KthuraKind::Unknown) { Paniek("Unknown kind", TrSPrintF("Line:%d;Instruction:%s", line, instruction.c_str())); }
								if (co->Kind() == KthuraKind::Actor) { Paniek("Actors cannot be defined through loading", TrSPrintF("Line:%d;Instruction:%s", line, instruction.c_str())); }
								if (co->Kind() == KthuraKind::Custom) co->CustomKind = val;
							} else if (fld == "COORD") {
								I_Want_An_Object;
								auto cs = Split(val, ',');
								if (cs->size() < 2) SyntaxError;
								int32
									x{ ToInt((*cs)[0]) },
									y{ ToInt((*cs)[1]) };
								co->x(x);
								co->y(y);

							} else if (fld == "INSERT") {
								I_Want_An_Object;
								auto cs = Split(val, ',');
								if (cs->size() < 2) SyntaxError;
								int32
									x{ ToInt((*cs)[0]) },
									y{ ToInt((*cs)[1]) };
								co->insertx(x);
								co->inserty(y);
							} else if (fld == "ROTATION") {
								I_Want_An_Object;
								co->rotatedeg(ToInt(val)); // Please note! Rotations are only stored in degrees. That was first of all the BlitzMax standard what Kthura was originally in, but radians also force me to work in decimals and that's a recipe for disaster.
							} else if (fld == "SIZE") {
								I_Want_An_Object;
								auto cs = Split(val, 'x');
								if (cs->size() < 2) SyntaxError;
								int32
									w{ ToInt((*cs)[0]) },
									h{ ToInt((*cs)[1]) };
								co->w(w);
								co->h(h);
							} else if (fld == "TAG") {
								I_Want_An_Object;
								co->Tag(val);
							} else if (fld == "LABELS") {
								I_Want_An_Object;
								co->labels(val);
							} else if (fld == "DOMINANCE") {
								I_Want_An_Object;
								co->dominance(ToInt(val));
							} else if (fld == "TEXTURE") {
								I_Want_An_Object;
								co->texture(val);
							} else if (fld == "ANIMSPEED" || fld == "FRAMESPEED") {
								I_Want_An_Object;
								co->animspeed(ToInt(val));
							} else if (fld == "ANIMFRAME" || fld == "CURRENTFRAME") {
								I_Want_An_Object;
								co->animframe(ToInt(val));
							} else if (fld == "ALPHA") {
								Paniek("The ALPHA field was deprecated ages ago and has by now officially been removed", TrSPrintF("Line:%d\nNote ALPHA255 should be used in stead!", line));
								return;
							} else if (fld == "ALPHA255") {
								I_Want_An_Object;
								co->alpha((byte)ToInt(val));
							} else if (fld == "VISIBLE") {
								I_Want_An_Object;
								co->visible(ToInt(val) > 0);
							} else if (fld == "COLOR") {
								I_Want_An_Object;
								auto cs = Split(val, ',');
								if (cs->size() < 3) SyntaxError;
								byte
									r{ (byte)ToInt((*cs)[0]) },
									g{ (byte)ToInt((*cs)[1]) },
									b{ (byte)ToInt((*cs)[2]) };
								co->red(r);
								co->green(g);
								co->blue(b);
							} else if (fld == "IMPASSIBLE") {
								I_Want_An_Object;
								co->impassible(ToInt(val) > 0);
							} else if (fld == "FORCEPASSIBLE") {
								I_Want_An_Object;
								co->forcepassible(ToInt(val) > 0);
							} else if (fld == "SCALE") {
								I_Want_An_Object;
								auto cs = Split(val, ',');
								if (cs->size() < 2) SyntaxError;
								int32
									x{ ToInt((*cs)[0]) },
									y{ ToInt((*cs)[1]) };
								co->scalex(x);
								co->scaley(y);
							} else if (fld == "BLEND") {
								I_Want_An_Object;
								co->blend(ToInt(val));
							} else if (Prefixed(fld, "DATA.")) {
								I_Want_An_Object;
								auto dfld = fld.substr(5);
								Chat("Defining data field: " << dfld << " = " << val);
								co->data(dfld, val);
							} else if (fld == "LAYER") {
								if (cl) cl->AutoRemap(true);
								if (!HasLayer(val)) { Paniek("Trying to switch to non-existent layer", TrSPrintF("Line:%d;Layer:%s", line, val.c_str())); return; }
								cl = Layer(val);
							} else if (fld == "BLOCKMAPGRID") {
								if (!cl) { Paniek("Grid definition without a layer", TrSPrintF("Line:%d", line)); return; }
								auto cs = Split(val, 'x');
								if (cs->size() < 2) SyntaxError;
								int32
									x{ ToInt((*cs)[0]) },
									y{ ToInt((*cs)[1]) };
								cl->gridx = x;
								cl->gridy = y;

							} else { Paniek("Unknown field!", TrSPrintF("Line:%d;Instruction:%s", line, instruction.c_str())); }
						}
					}
					if (cl) cl->AutoRemap(true);
				} else if (nu == "OPTIONS") {
					//cout << "OPTIONS NOT YET IMPLEMENTED! DATA WILL BE LOST!\n";
					Options = ParseUGINIE(Resource->GetString(prefix + "Options"));
				} else if (AllowUnknown) {
					Chat("Loaded Uknown: " << n);
					Unknown[n] = Resource->B(ent->Name());
				}
			}
		}

		Kthura LoadKthura(std::string resfile, std::string prefix) {
			auto J{ JCR6_Dir(resfile) };
			if (Last()->Error) {
				Paniek("JCR6 error", "Kthura Map file:" + resfile + ";Prefix:" + prefix); return nullptr;
			}
			return LoadKthura(J, prefix);
		}

		UKthura LoadUKthura(std::string resfile, std::string prefix) {
			auto J{ JCR6_Dir(resfile) };
			if (Last()->Error) {
				Paniek("JCR6 error", "Kthura Map file:" + resfile + ";Prefix:" + prefix); return nullptr;
			}
			return std::make_unique<_Kthura>(J, prefix);
		}
#pragma endregion
	}
}