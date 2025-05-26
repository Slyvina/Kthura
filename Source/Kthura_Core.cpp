// License:
// 	Kthura/Source/Kthura_Core.cpp
// 	Slyvina - Kthura Core
// 	version: 25.05.26
// 
// 	Copyright (C) 2022, 2023, 2024, 2025 Jeroen P. Broks
// 
// 	This software is provided 'as-is', without any express or implied
// 	warranty.  In no event will the authors be held liable for any damages
// 	arising from the use of this software.
// 
// 	Permission is granted to anyone to use this software for any purpose,
// 	including commercial applications, and to alter it and redistribute it
// 	freely, subject to the following restrictions:
// 
// 	1. The origin of this software must not be misrepresented; you must not
// 	   claim that you wrote the original software. If you use this software
// 	   in a product, an acknowledgment in the product documentation would be
// 	   appreciated but is not required.
// 	2. Altered source versions must be plainly marked as such, and must not be
// 	   misrepresented as being the original software.
// 	3. This notice may not be removed or altered from any source distribution.
// End License

#include <iostream>
#include <math.h>
#include <SlyvString.hpp>
#include <SlyvSTOI.hpp>
#include <Kthura_Core.hpp>

#undef KthuraCoreDebug



#define KthuraObjVal(type,prop) void KthuraObject::prop(type v) {_Obj->prop=v;} type KthuraObject::prop() {return _Obj->prop;}
#define KthuraObjValRP(type,prop) void KthuraObject::prop(type v) {_Obj->prop=v; _parent->PerformAutoRemap();} type KthuraObject::prop() {return _Obj->prop;}
#define KthuraObjValDefFunc(type,prop)  type KthuraObject::prop() { return _Obj->prop;} void KthuraObject::prop(type value)

#define KthuraActVal(type,prop,propstring,eval) \
	void KthuraObject::prop(type v) {\
		if (_Kind!=KthuraKind::Actor) { Paniek(TrSPrintF("Set:'%s' is a propert strictly reserved for actors. Cannot be used for %s",propstring,SKind().c_str())); return; }\
		_Act->prop=v;\
	}\
	type KthuraObject::prop() { \
		if (_Kind!=KthuraKind::Actor) { Paniek(TrSPrintF("Get:'%s' is a propert strictly reserved for actors. Cannot be used for %s",propstring,SKind().c_str())); return eval; }\
		return _Act->prop;\
	}

#ifdef KthuraCoreDebug
#define Chat(msg) cout << "\x1b[33mKthura Debug>\t\x1b[0m" << msg << endl
#else
#define Chat(msg)
#endif
using namespace std;
using namespace Slyvina::Units;
using namespace Slyvina::JCR6;

namespace Slyvina {
	namespace Kthura {

		bool OldAlpha{ false };
		bool AutoRetag{ false };

#pragma region "Loader Data"
		struct RegKL { KthuraLoader Loader{ nullptr }; KthuraRecognizer Recognize{ nullptr }; };
		static Kthura DefaultLoader(JT_Dir D, string pref) { return LoadKthura(D, pref); }
		static bool DefaultRecognizer(JT_Dir D,string prefix) {
			if (prefix.size() && (!Suffixed(prefix, "/"))) prefix += "/";
			return D->EntryExists(prefix + "Objects") && D->EntryExists(prefix + "Data");
		}
		map<string, RegKL> RegKthuraLoader {
			{"default", { DefaultLoader,DefaultRecognizer }}
		};
#pragma endregion

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

		void _Kthura::RenameLayer(std::string OldLay, std::string NewLay, bool nopanic) {
			Trans2Upper(OldLay);
			Trans2Upper(NewLay);
			if (!_Layers.count(OldLay)) {
				if (!nopanic) Paniek("Can't rename a non-existent layer", OldLay + " -> " + NewLay);
				return;
			}
			if (_Layers.count(NewLay)) {
				if (!nopanic) Paniek("Can't rename a when a layer with the new name already exists!", OldLay + " -> " + NewLay);
				return;
			}
			_Layers[NewLay] = _Layers[OldLay];
			_Layers[OldLay].DontKill = true;
			_Layers.erase(OldLay);
		}

		void _Kthura::KillLayer(std::string Lay, bool ignoreifnonexistent) {
			Trans2Upper(Lay);
			if (_Layers.count(Lay)) {
				_Layers.erase(Lay);
			}
		}

		void _Kthura::AutoRemap(bool value) {
			for (auto &l : _Layers) _Layers[l.first].AutoRemap(value);
		}

		void KthuraLayer::PerformAutoRemap() {
			if (_modified && _autoRemap) {
				TotalRemap();
				_modified = false;
			} else if (!_autoRemap) {
				_modified = true;
			}
		}

		void KthuraLayer::VisibilityAll(bool v) {
			for (auto o=FirstObject();o;o=o->Next()) o->visible(v);
		}

		void KthuraLayer::Kill(KthuraObject* k) {
			if (k->Parent() != this) {
				//throw runtime_error("Alien object kill: " + to_string(k->ID()) + "::" + k->Tag() + "::" + k->SKind());
				Paniek("Alien object kill requested!");
				return;
			}
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
					if (TagMap.count(Upper(o->Tag()))) {
						if (AutoRetag) {
							cout << "\x1b[31mERROR!\x1b[37m Dupe Tag! (" + o->Tag() + ") autoretagging to: ";
							static size_t rt{0};
							string newtag;
							do { newtag = o->Tag() + TrSPrintF("__retag_%x", ++rt); } while (HasTag(newtag));
							o->Tag(newtag);
							cout << newtag << "\n";
						} else {
							Paniek("RemapTags(): Dupe tag (" + o->Tag() + ")");
						}
					}
					TagMap[Upper(o->Tag())] = o;
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
					auto l{ Split(Upper(o->labels()),',') };
					for (auto& lb : *l) {
						//std::cout<< "Label: " << l->size() << ": "<<lb<<"\n";
						_LabelMap[lb].push_back(o);
					}
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
				if (!Last) {
					DomFirst = dom.second;
					Last = dom.second;

				} else {
					Last->DomNext = dom.second;
					Last = dom.second;
				}
			}
		}

		bool KthuraLayer::Block(int x, int y) {
			return
				x < 0 ||
				y < 0 ||
				x >= _BlockW ||
				y >= _BlockH ||
				BlockMap(x, y);
		}

		void KthuraLayer::BuildBlockmap() {

			// No need to reinvent the wheel.
			// This has been copied from my original C# class and been adapted to the current Slyvina version.

			// KthuraObject O;
			auto GW = gridx;
			auto GH = gridy;
			int X, Y, W, H; //BX, BY,
			int TX, TY, AX, AY, TW, TH;
			int BoundX = 0, BoundY = 0;
			int iw, tiw, ih, tih;
			// Let's first get the bounderies
			for (KthuraObject* O = FirstObject(); O; O = O->Next()) { // foreach(KthuraObject O in Objects) {
				X = std::max(0, O->x()); //if (X < 0) X = 0;
				Y = std::max(0, O->y()); // O.y; if (Y < 0) Y = 0;
				W = std::max(0, O->w() - 1); //if (W < 0) W = 0;
				H = std::max(0, O->h() - 1); //if (H < 0) H = 0;
				switch (O->Kind()) {
				case KthuraKind::TiledArea:
				case KthuraKind::Zone:
				case KthuraKind::StretchedArea:
				case KthuraKind::Rect:
					TX = (int)ceil((double)((X + W) / GW));
					TY = (int)ceil((double)((Y + H) / GH));
					if (TX > BoundX) BoundX = TX;
					if (TY > BoundY) BoundY = TY;
					break;
					/* Obstacle and Pics no longer supported for Blockmaps!
					case KthuraKind::Obstacle:
						TX = (int)Math.Floor((decimal)(X / GW));
						TY = (int)Math.Floor((decimal)(Y / GH));
						if (TX > BoundX) BoundX = TX;
						if (TY > BoundY) BoundY = TY;
						break;
					case KthuraKind::Pic:
						TX = (int)Math.Floor((decimal)X / GW);
						TY = (int)Math.Floor((decimal)Y / GW);
						if (TX > BoundX) BoundX = TX;
						if (TY > BoundY) BoundY = TY;
						break;
					}
					*/
				}
			}
			_BlockW = BoundX; //BlockMapBoundW = BoundX;
			_BlockH = BoundY; //BlockMapBoundH = BoundY;
			delete[] _BlockMap;
			_BlockMap = new bool[(BoundY + 1) * (BoundX + 1)]; //BlockMap = new bool[BoundX + 1, BoundY + 1];
			for (int i = 0; i < (BoundY + 1) * (BoundX + 1); ++i) _BlockMap[i] = false; // Must be sure all is false before starting

			// And now for the REAL work.
			for (KthuraObject* O = FirstObject(); O; O = O->Next()) { //foreach(KthuraObject O in Objects) {
				if (O->impassible()) {
					//Debug.WriteLine($"Checking object {O.kind}; {O.Texture}; {O.Labels}");
					X = std::max(0, O->x()); //if (X < 0) X = 0;
					Y = std::max(0, O->y()); // O.y; if (Y < 0) Y = 0;
					W = std::max(0, O->w() - 1); //if (W < 0) W = 0;
					H = std::max(0, O->h() - 1); //if (H < 0) H = 0;
					switch (O->Kind()) {
					case KthuraKind::TiledArea:
					case KthuraKind::Zone:
					case KthuraKind::StretchedArea:
					case KthuraKind::Rect:
						//Kthura.EDITTORLOG($"Working on Impassible {O.kind} {O.Tag}");
						TX = (int)floor((double)X / GW);
						TY = (int)floor((double)Y / GH);
						TW = (int)ceil((double)((X + W) / GW));
						TH = (int)ceil((double)((Y + H) / GH));
						//Print "DEBUG: Blockmapping area ("+TX+","+TY+") to ("+TW+","+TH+")"
						for (AX = TX; AX <= TW; AX++) {
							for (AY = TY; AY <= TH; AY++) {
								//for (AX = TX; AX < TW; AX++) {
								//    for (AY = TY; AY < TH; AY++) {
								//try {
									//Kthura.EDITTORLOG($"Blocking {AX},{AY}");
									//BlockMap[AX, AY] = true;
								BlockMap(AX, AY, true);
								//} catch {
									//throw new Exception($"BlockMap[{AX},{AY}]: Out of bounds ({BlockMap.GetLength(0)}x{BlockMap.GetLength(1)})");
								//}
							}
						}
						break;
						/*
					case "Obstacle":
						TX = (int)Math.Floor((decimal)(X / GW));
						TY = (int)Math.Floor((decimal)((Y - 1) / GH));
						BlockMap[TX, TY] = true;
						if (KthuraDraw.DrawDriver == null) throw new Exception("Draw Driver is null!");
						if (KthuraDraw.DrawDriver.HasTexture(O))
							iw = KthuraDraw.DrawDriver.ObjectWidth(O);
						else
							iw = 0;
						tiw = (int)Math.Ceiling((decimal)iw / GW) - 1;

						for (AX = TX - (tiw); AX <= TX + (tiw); AX++) {
							if (AX >= 0 && AX <= BoundX && TY <= BoundY && TY >= 0) BlockMap[AX, TY] = true;
						}
						break;
					case "Pic":
						TX = (int)Math.Floor((decimal)X / GW);
						TY = (int)Math.Floor((decimal)Y / GH);
						BlockMap[TX, TY] = true;
						if (KthuraDraw.DrawDriver.HasTexture(O)) {
							iw = KthuraDraw.DrawDriver.ObjectWidth(O); //ImageWidth(o.textureimage)
							tiw = (int)Math.Ceiling((decimal)iw / GW);
							ih = KthuraDraw.DrawDriver.ObjectHeight(O); //ImageHeight(o.textureimage)
							tih = (int)Math.Ceiling((decimal)ih / GH);
							for (AX = TX; AX < TX + (tiw); AX++) for (AY = TY; AY < TY + tih; AY++) {
								if (AX >= 0 && AX <= BoundX && AY <= BoundY && AY >= 0) BlockMap[AX, AY] = true;
							}
						}
						break;
						*/
					}
				}
			}
			// And this will force a way open if applicable
			for (KthuraObject* O = FirstObject(); O; O = O->Next()) { //foreach(KthuraObject O in Objects) {
				if (O->forcepassible()) {
					//Debug.WriteLine($"Checking object {O.kind}; {O.Texture}; {O.Labels}");
					X = std::max(0, O->x()); //if (X < 0) X = 0;
					Y = std::max(0, O->y()); // O.y; if (Y < 0) Y = 0;
					W = std::max(0, O->w() - 1); //if (W < 0) W = 0;
					H = std::max(0, O->h() - 1); //if (H < 0) H = 0;
					switch (O->Kind()) {
					case KthuraKind::TiledArea:
					case KthuraKind::Zone:
					case KthuraKind::StretchedArea:
					case KthuraKind::Rect:
						//Kthura.EDITTORLOG($"Working on Impassible {O.kind} {O.Tag}");
						TX = (int)floor((double)X / GW);
						TY = (int)floor((double)Y / GH);
						TW = (int)ceil((double)((X + W) / GW));
						TH = (int)ceil((double)((Y + H) / GH));
						//Print "DEBUG: Blockmapping area ("+TX+","+TY+") to ("+TW+","+TH+")"
						for (AX = TX; AX <= TW; AX++) {
							for (AY = TY; AY <= TH; AY++) {
								//for (AX = TX; AX < TW; AX++) {
								//    for (AY = TY; AY < TH; AY++) {
								//try {
									//Kthura.EDITTORLOG($"Blocking {AX},{AY}");
									//BlockMap[AX, AY] = true;
								BlockMap(AX, AY, false);
								//} catch {
									//throw new Exception($"BlockMap[{AX},{AY}]: Out of bounds ({BlockMap.GetLength(0)}x{BlockMap.GetLength(1)})");
								//}
							}
						}
						break;
						/*
					case "Obstacle":
						TX = (int)Math.Floor((decimal)(X / GW));
						TY = (int)Math.Floor((decimal)((Y - 1) / GH));
						BlockMap[TX, TY] = true;
						if (KthuraDraw.DrawDriver == null) throw new Exception("Draw Driver is null!");
						if (KthuraDraw.DrawDriver.HasTexture(O))
							iw = KthuraDraw.DrawDriver.ObjectWidth(O);
						else
							iw = 0;
						tiw = (int)Math.Ceiling((decimal)iw / GW) - 1;

						for (AX = TX - (tiw); AX <= TX + (tiw); AX++) {
							if (AX >= 0 && AX <= BoundX && TY <= BoundY && TY >= 0) BlockMap[AX, TY] = true;
						}
						break;
					case "Pic":
						TX = (int)Math.Floor((decimal)X / GW);
						TY = (int)Math.Floor((decimal)Y / GH);
						BlockMap[TX, TY] = true;
						if (KthuraDraw.DrawDriver.HasTexture(O)) {
							iw = KthuraDraw.DrawDriver.ObjectWidth(O); //ImageWidth(o.textureimage)
							tiw = (int)Math.Ceiling((decimal)iw / GW);
							ih = KthuraDraw.DrawDriver.ObjectHeight(O); //ImageHeight(o.textureimage)
							tih = (int)Math.Ceiling((decimal)ih / GH);
							for (AX = TX; AX < TX + (tiw); AX++) for (AY = TY; AY < TY + tih; AY++) {
								if (AX >= 0 && AX <= BoundX && AY <= BoundY && AY >= 0) BlockMap[AX, AY] = true;
							}
						}
						break;
						*/
					}
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

		void KthuraLayer::Tags(std::vector<String>* vec) {
			vec->clear();
			for (auto t : TagMap) vec->push_back(t.first);
		}

		bool KthuraLayer::_autoRemap{ true };

		void KthuraLayer::TotalRemap() {
			RemapTags();
			RemapDominance();
			RemapID();
			RemapLabels();
			BuildBlockmap();
			_modified = false;
		}

		void KthuraLayer::BlockMap(int x, int y, bool v) {
			if (x >= 0 && y >= 0 && x <= _BlockW && y <= _BlockH) _BlockMap[(y * _BlockW) + x] = v; else Paniek("BlockMap out of Range", TrSPrintF("Set: (%d,%d) %dx%d", x, y, _BlockW, _BlockH));
		}

		bool KthuraLayer::BlockMap(int x, int y) {
			if (x >= 0 && y >= 0 && x <= _BlockW && y <= _BlockH)
				return _BlockMap[(y * _BlockW) + x];
			Paniek("BlockMap out of Range", TrSPrintF("Get: (%d,%d) %dx%d", x, y, _BlockW, _BlockH));
			return false;
		}

		KthuraObject* KthuraLayer::NewObject(KthuraKind k) {
			while (IDMap.count(++_objcnt)) {}
			auto o = new KthuraObject(_lastObject, this, k, _objcnt);
			if (!_firstObject) _firstObject = o;
			_lastObject = o;
			_modified = true;
			return o;
		}

		KthuraObject* KthuraLayer::NewObject(std::string knd) {
			auto o{ NewObject(KindName(knd)) };
			o->SKind(knd);
			return o;
		}

		void KthuraLayer::VisibilityByLabel(std::string Label, bool value) {
			Trans2Upper(Label);
			if (!_LabelMap.count(Label)) return;
			auto LO{ &_LabelMap[Label] };
			for (auto o : *LO) o->visible(value);
		}

		void KthuraLayer::VisibilityButLabel(std::string Label, bool value) {
			Trans2Upper(Label);
			for (auto o = FirstObject(); o; o = o->Next()) {
				o->visible(!value ? o->HasLabel(Label) : !o->HasLabel(Label));
			}
		}

		void KthuraLayer::ViewLabelMap() {
			//for(auto& lm:_LabelMap) {
			//	std::cout <<
			//		"\x1b[96mLabel \x1b[x93m" <<
			//		lm <<
			//		"\x1b[96 has \x1b[94m" <<
			//		lm.size() <<
			//		"\x1b[96 objects!x1b[97m\n";
			//}
		}


		KthuraObject* KthuraLayer::Obj(uint64 i) {
			if (HasID(i)) return IDMap[i];
			Paniek("Object doesn't exist", "Object ID:#" + to_string(i));
			return nullptr;
		}

		KthuraObject* KthuraLayer::Obj(std::string _tag) {
			if (Prefixed(_tag, "%")) return Obj(ToUInt(_tag.substr(1)));
			if (HasTag(_tag)) return TagMap[Upper(_tag)];
			Paniek("Object doesn't exist", "Object Tag " + _tag);
			return nullptr;
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
			std::string ChosenPic{""};
			bool NotInMotionThen0{ true };
			bool InMotion{ false };
			bool Walking{ false };
			bool Moving{ false };
			bool WalkingIsInMotion{ true };
			bool MoveIgnoreBlock{ false };
			bool AutoWind{ true };
			int UnMoveTimer{ 4 };
			int MoveX{ 0 }, MoveY{ 0 };
			int MoveSkip{ 4 };
			int FrameSpeed{ 4 };
			int FrameSpeedCount{ 0 };
			int WalkSpot{ 0 };
			std::string Wind { "North" };
			int WalkingToX{ 0 }, WalkingToY{ 0 };
			int PathIndex{ 0 };
			std::vector<KthuraSpot> FoundPath{};
			bool WalkAutoDom{ true }; // Will remap based on dominance when y coordinate changes
			int WalkAutoDomOldY{ 0 };
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
			_Kind = k;
		}

		void KthuraObject::__KillMe(bool DisposeMe) {
			if (_prev) _prev->_next = _next;
			if (_next) _next->_prev = _prev;
			if (DisposeMe) delete this;
			if (KthuraLayer::AutoRemap()) _parent->TotalRemap();
		}


		// Properties
		void KthuraObject::Tag(std::string t) {
			//Trans2Upper(t);
			if (_Obj->Tag == t) return; // No changes = no actions needed
			_Obj->Tag = t;
			_parent->PerformAutoRemap();
		}
		std::string KthuraObject::Tag() { return _Obj->Tag; }

		// All Objects
		KthuraObjVal(int32, x);
		//KthuraObjVal(int32, y);
		int KthuraObject::y() { return _Obj->y; }
		void KthuraObject::y(int v) {
			_Obj->y = v;
			if (_Act) {
				//cout << "DEBUG: AutoDom:" << _Act->WalkAutoDom << " OldY: " << _Act->WalkAutoDomOldY << "Y: "<<_Obj->y << endl; // DEBUG
				if (_Act->WalkAutoDom && abs(_Act->WalkAutoDomOldY - _Obj->y) > 50) {
					_parent->RemapDominance();
					_Act->WalkAutoDomOldY = v;
				}
			}
		}
		KthuraObjVal(int32, w);
		KthuraObjVal(int32, h);
		KthuraObjVal(byte, r);
		KthuraObjVal(byte, g);
		KthuraObjVal(byte, b);
		KthuraObjVal(byte, a);
		KthuraObjVal(bool, visible);
		KthuraObjValRP(bool, impassible);
		KthuraObjValRP(bool, forcepassible);
		//KthuraObjVal(std::string, texture);
		void KthuraObject::texture(std::string t) { if (t != texture()) { _Obj->texture = t; _Obj->animframe = 0; } }
		std::string KthuraObject::texture() { return _Obj->texture; }
		KthuraObjValRP(std::string, labels);
		KthuraObjValRP(uint32, dominance);
		KthuraObjVal(uint32, animframe);
		KthuraObjVal(uint32, animskip);
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

		// Actors Only
		KthuraActVal(bool, NotInMotionThen0, "NotInMotionThen0", false);
		KthuraActVal(std::string, ChosenPic, "ChosenPic", "");
		void KthuraObject::InMotion(bool value) {
			if (_Kind != KthuraKind::Actor) { Paniek("You cannot set InMotion into an non-actor!"); return; }
			_Act->InMotion = value;
		}
		bool KthuraObject::InMotion() {
			if (_Kind != KthuraKind::Actor) { Paniek("You cannot get InMotion from an non-actor!"); return false; }
			if (_Act->WalkingIsInMotion)
				return _Act->Walking || _Act->Moving;
			else
				return _Act->InMotion;
		}
		KthuraActVal(bool, Walking, "Walking", false);
		KthuraActVal(bool, Moving, "Moving", false);
		KthuraActVal(bool, WalkingIsInMotion, "WalkingIsInMotion", true);
		KthuraActVal(bool, MoveIgnoreBlock, "MoveIgnoreBlock", false);
		KthuraActVal(bool, AutoWind, "AutoWind", true);
		KthuraActVal(int, UnMoveTimer, "UnMoveTime", 0);
		KthuraActVal(int, MoveX, "MoveX", 0);
		KthuraActVal(int, MoveY, "MoveY", 0);
		KthuraActVal(int, MoveSkip, "MoveSkip", 0);
		KthuraActVal(int, FrameSpeed, "FrameSpeed", 0);
		KthuraActVal(int, FrameSpeedCount, "FrameSpeedCount", 0);
		KthuraActVal(int, WalkSpot, "WalkSpot", 0);
		KthuraActVal(std::string, Wind, "Wind", "North");
		KthuraActVal(int, WalkingToX, "WalkingToX", 0);
		KthuraActVal(int, WalkingToY, "WalkingToY", 0);
		void KthuraObject::MoveTo(int x, int y) {
			MoveX(x);
			MoveY(y);
			Moving(true);
		}

		void KthuraObject::MoveTo(KthuraObject* o) { MoveTo(o->x(), o->y()); }

		void KthuraObject::MoveTo(std::string T) {
			if (!Parent()->HasTag(T)) { Paniek(TrSPrintF("Object tagged `%s` not found", T.c_str())); return; }
			MoveTo(Parent()->Obj(T));
		}

		KthuraObject* KthuraObject::Spawn(KthuraLayer* parent, std::string spot) {
			auto ret{ parent->NewObject(KthuraKind::Actor) }; //var ret = new KthuraActor(parent);
			if (!parent->HasTag(spot)) {
				std::cout << "\nWARNING! I cannot spawn an actor on not existent spot " << spot << "\n";
				return nullptr;
			}
			auto obj = parent->Obj(spot);
			ret->x(obj->x());
			ret->y(obj->y());
			ret->dominance(obj->dominance());
			ret->alpha(255);
			ret->r(255);
			ret->g(255);
			ret->b(255);
			ret->visible(true);
			ret->impassible(false);
			ret->forcepassible(false);
			if (obj->data()->count("WIND")) ret->Wind(obj->data("Wind")); else ret->Wind("North");
			parent->TotalRemap();
			return ret;
		}

		bool KthuraObject::InMe(KthuraObject*O) {
			switch(O->Kind()) {
				case KthuraKind::Zone:
				case KthuraKind::Rect:
				case KthuraKind::TiledArea:
				case KthuraKind::StretchedArea:
					return InMe(O->x(),O->y()) && InMe(O->x()+O->w(),O->y()+O->h());
					break;
				default:
					return InMe(O->x(),O->y());
			}

		}

		void KthuraLayer::HideByZone(KthuraObject* Zone, bool outside) {
			switch(Zone->Kind()) {
				case KthuraKind::Zone:
				case KthuraKind::Rect:
				case KthuraKind::TiledArea:
				case KthuraKind::StretchedArea:
					for(auto o=FirstObject();o;o=o->Next()) {
						auto iz{Zone->InMe(o)};
						if (iz)	o->visible(outside?iz:!iz);
					} break;
				default:
					std::cout << "\x1b[31mERROR!>\x1b[37m Object kind cannot be used for HideByZone()!\n";
			}
		}

		void KthuraLayer::ShowByZone(KthuraObject* Zone, bool outside) {
			switch(Zone->Kind()) {
				case KthuraKind::Zone:
				case KthuraKind::Rect:
				case KthuraKind::TiledArea:
				case KthuraKind::StretchedArea:
					for(auto o=FirstObject();o;o=o->Next()) {
						auto iz{Zone->InMe(o)};
						if (iz) o->visible(outside?!iz:iz);
					} break;
				default:
					std::cout << "\x1b[31mERROR!>\x1b[37m Object kind cannot be used for HideByZone()!\n";
			}
		}

		KthuraObject* KthuraObject::Spawn(KthuraLayer* parent, int x, int y, std::string wind, byte R, byte G, byte B, byte alpha, int Dominance) {
			auto ret{ parent->NewObject(KthuraKind::Actor) };
			ret->x(x);
			ret->y(y);
			ret->Wind(wind);
			ret->r(R);
			ret->g(G);
			ret->b(B);
			ret->alpha(alpha);
			ret->dominance(Dominance);
			ret->visible(true);
			ret->impassible(false);
			ret->forcepassible(false);
			parent->TotalRemap();
			return ret;
		}

		int KthuraObject::PathLength() {
			return (int)_Act->FoundPath.size();
		}

		int KthuraObject::CWalkX() {
			if (!_Act) return 0;
			if (_Act->Walking) return _Act->FoundPath[_Act->PathIndex].x;
			return 0;
		}

		int KthuraObject::CWalkY() {
			if (!_Act) return 0;
			if (_Act->Walking) return _Act->FoundPath[_Act->PathIndex].y;
			return 0;
		}

		void KthuraObject::Walk2Move() {
			if (!_Act) return;
			_Act->MoveX = (CWalkX() * _parent->gridx) + (_parent->gridx / 2);
			_Act->MoveY = ((CWalkY() * _parent->gridy) + (_parent->gridy)) - 1;
			_Act->Moving = true;
		}

		void KthuraObject::WalkTo(int to_x, int to_y, bool real) {
			//printf("_Act = %d\n", _Act!=nullptr);
			if (!_Act) return;
			auto gridx = _parent->gridx;
			auto gridy = _parent->gridy;
			int tox = to_x, toy = to_y;
			int fromx = x(), fromy = y();
			if (real) {
				tox = to_x / gridx;
				toy = to_y / gridy;
				fromx = x() / gridx;
				fromy = y() / gridy;
			}
			//FoundPath = Dijkstra.QuickPath(Parent.PureBlockRev, Parent.BlockMapWidth, Parent.BlockMapHeight, fromx, fromy, tox, toy);
			_Act->FoundPath = _Kthura::Walk.Route(&_Kthura::Walk, _parent, fromx, fromy, tox, toy);
			//printf("Actor.WalkTo(%d,%d,%d): Going from(%d,%d) to (%d,%d)  (Success: %d)\n", to_x, to_y, real, fromx, fromy, tox, toy, _Kthura::Walk.Succes(&_Kthura::Walk));
			if (_Kthura::Walk.Succes(&_Kthura::Walk)) {
				_Act->PathIndex = 0;
				_Act->Walking = true;
				_Act->WalkingToX = to_x; //FoundPath.Nodes[0].x;
				_Act->WalkingToY = to_y; //FoundPath.Nodes[1].y;
				_Act->MoveX = x();
				_Act->MoveY = y();
				Walk2Move();
			} else {
				_Act->Walking = false;
				//FoundPath = null;
				_Act->FoundPath.clear();
			}
		}

		void KthuraObject::WalkTo(KthuraObject* o) { WalkTo(o->x(), o->y(), true); }

		void KthuraObject::WalkTo(std::string oTag) { WalkTo(_parent->Obj(oTag)); }

		void KthuraObject::StopWalking() {
			if (!_Act) return;
			_Act->Moving = false;
			_Act->Walking = false;
		}

		void KthuraObject::StopMoving() {
			if (!_Act) return;
			_Act->Moving = false;
		}

		bool KthuraObject::UpdateMoves() {
			bool ret{false};
			if (_Act->Moving || _Act->Walking) {
				if (_Act->MoveY < _Obj->y) { _Obj->y -= _Act->MoveSkip; if (_Obj->y < _Act->MoveY) _Obj->y = _Act->MoveY; if (_Act->AutoWind) _Act->Wind = "North"; }
				if (_Act->MoveY > _Obj->y) { _Obj->y += _Act->MoveSkip; if (_Obj->y > _Act->MoveY) _Obj->y = _Act->MoveY; if (_Act->AutoWind) _Act->Wind = "South"; }
				if (_Act->MoveX < _Obj->x) { _Obj->x -= _Act->MoveSkip; if (_Obj->x < _Act->MoveX) _Obj->x = _Act->MoveX; if (_Act->AutoWind) _Act->Wind = "West"; }
				if (_Act->MoveX > _Obj->x) { _Obj->x += _Act->MoveSkip; if (_Obj->x > _Act->MoveX) _Obj->x = _Act->MoveX; if (_Act->AutoWind) _Act->Wind = "East"; }
				if (_Act->MoveX == _Obj->x && _Act->MoveY == _Obj->y) {
					if (!_Act->Walking)
						_Act->Moving = false;
					else {
						//* Pathfinder driver required an not yet present!
						_Act->PathIndex++;
						if (_Act->PathIndex >= PathLength()) {
							_Act->Walking = false;
							_Act->Moving = false;
						} else {
							Walk2Move();
						}
						//*/
					}
				}
			} else {
				_Act->MoveX = _Obj->x;
				_Act->MoveY = _Obj->y;
			}
			if ((_Act->WalkingIsInMotion && _Act->Walking) || InMotion()) {
				_Act->FrameSpeedCount++;
				if (_Act->FrameSpeedCount >= _Act->FrameSpeed) {
					_Act->FrameSpeedCount = 0;
					_Obj->animframe++;
				}
			} else if (_Act->WalkingIsInMotion && (!_Act->Walking)) _Obj->animframe = 0;
			if (_Act) {
				//cout << "DEBUG: AutoDom:" << _Act->WalkAutoDom << " OldY: " << _Act->WalkAutoDomOldY << "Y: "<<_Obj->y << endl; // DEBUG
				if (_Act->WalkAutoDom && abs(_Act->WalkAutoDomOldY - _Obj->y) > 50) {
					//_parent->RemapDominance();
					ret=true;
					_Act->WalkAutoDomOldY = _Obj->y;
				}
			}
			return ret;
		}


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
			{KthuraKind::Pivot,"Pivot"},
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
			if (s=="Pic") return KthuraKind::Picture;
			for (auto& kn : _KindName) {
				if (Upper(s) == Upper(kn.second)) return kn.first;
			}
			return KthuraKind::Unknown;
		}
		std::string KthuraObject::SKind() {
			if (_Kind == KthuraKind::Custom) return CustomKind; else return KindName(_Kind);
		}
		void KthuraObject::SKind(std::string k) {
			_Kind = KindName(k);
			if (k[0] =='$') CustomKind = k;
			// cout << "DEBUG! Kind of Object #" << ID() << " is " << SKind() << "!\n"; // DEBUG ONLY!
		}
		bool KthuraObject::HasLabel(std::string L) {
			Trans2Upper(L);
			if (labels() == "") return false;
			auto l{ Split(labels(),',') };
			for (auto cl : *l) if (Upper(cl) == L) return true;
			return false;
		}
#pragma endregion


		KthuraWalk _Kthura::Walk{};
#pragma region Loader
#define I_Want_An_Object if (!co) { Paniek("I cannot define field '"+fld+"' without an object",TrSPrintF("Line:%d;Instruction:%s",line,instruction.c_str())); return; }
#define SyntaxError { Paniek("Syntax error", TrSPrintF("Line:%d;Instruction:%s", line, instruction.c_str())); return; }

		_Kthura::_Kthura(Slyvina::JCR6::JT_Dir Resource, std::string prefix) {
			if (prefix.size() && (!Suffixed(prefix, "/"))) prefix += "/";
			if (!Resource->EntryExists(prefix + "Data")) { Paniek("JCR6 resource doesn't appear to have the '" + prefix + "Data' entry that Kthura needs"); return; }
			if (!Resource->EntryExists(prefix + "Objects")) { Paniek("JCR6 resource doesn't appear to have the '" + prefix + "Objects' entry that Kthura needs"); return; }
			auto entries{ Resource->Entries() };
			for (auto ent : *entries) {
				auto
					uename{ Upper(ent->Name()) },
					uprefix{ Upper(prefix) };
				if (Prefixed(uename,uprefix)) {
					auto n = ent->Name().substr(prefix.size());
					auto nu = Upper(n);
					Chat("Processing entry: " << n);
					if (nu == "DATA") {
						Chat("Reading Data");
						MetaData = Resource->GetStringMap(prefix + "Data");
						if ((!MetaData) || Last()->Error) {
							Paniek("Something went wrong reading data", "JCR6:" + Last()->ErrorMessage+" ("+Last()->MainFile+" -> "+Last()->Entry+")"); return;
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
								if (OldAlpha && fld == "ALPHA") {
									fld = "ALPHA255";
									double a1 = stod(val);
									double a255 = 255 * a1;
									int a255i = (int)floor(a255);
									val = to_string(a255i);
									cout << "\x1b[31mWARNING!!!\7\x1b[0m ALPHA deprecated! Will convert to ALPHA255 (Line #" << line << ")\n";
									cout << "\t=>" << a1 << " -> " << a255 << " -> " << a255i << "\n";
								}
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
									//if (cl) cl->AutoRemap(true);
									if (!HasLayer(val)) { Paniek("Trying to switch to non-existent layer", TrSPrintF("Line:%d;Layer:%s", line, val.c_str())); return; }
									cl = Layer(val);
								} else if (fld == "BLOCKMAPGRID" || fld == "GRID") {
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
						//if (cl) cl->AutoRemap(true);
						auto L{ Layers() };
						for (auto arl : *L) Layer(arl)->AutoRemap(true);

					} else if (nu == "OPTIONS") {
						//cout << "OPTIONS NOT YET IMPLEMENTED! DATA WILL BE LOST!\n";
						Options = ParseUGINIE(Resource->GetString(prefix + "Options"));
					} else if (AllowUnknown) {
						Chat("Loaded Uknown: " << n);
						Unknown[n] = Resource->B(ent->Name());
					}
				}
			}
		}

		KthuraLayer* _Kthura::operator[](std::string laytag) {
			return Layer(laytag);
		}


		Kthura LoadKthura(std::string resfile, std::string prefix) {
			auto J{ JCR6_Dir(resfile) };
			if (Last()->Error) {
				Paniek("JCR6 error", "Kthura Map file:" + ChReplace(resfile,':','.') + ";Prefix:" + prefix + ";JCR6:" + Last()->ErrorMessage); return nullptr;
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

#pragma region XLoader
		void RegisterKthuraLoader(std::string Name, KthuraLoader KL, KthuraRecognizer KR) {
			Trans2Upper(Name);
			auto NR{ &RegKthuraLoader[Name] };
			NR->Loader = KL;
			NR->Recognize = KR;
		}

		std::string XRecognizeKthura(Slyvina::JCR6::JT_Dir J, std::string prefix) {
			for (auto XRK : RegKthuraLoader) {
				if (XRK.second.Recognize(J, prefix)) return XRK.first;
			}
			return "unknown";
		}

		Kthura XLoadKthura(Slyvina::JCR6::JT_Dir J, std::string prefix ) {
			auto drv{ XRecognizeKthura(J,prefix) };
			if (drv == "unknown") {
				Paniek("Tried to load a Kthura Map through XLoadKthura without a known driver"); return nullptr;
			}
			if (!RegKthuraLoader.count(drv)) {
				Paniek("Internal error! Recognized as driver '"+drv+"' which doesn't exist! Please report!"); return nullptr;
			}
			return RegKthuraLoader[drv].Loader(J, prefix);
		}
	}
#pragma endregion
}
