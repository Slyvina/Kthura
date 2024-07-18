// Lic:
// Kthura/Source/Kthura_LoadCompiled.cpp
// Compiled Kthura Map Leader
// version: 24.07.18
// Copyright (C) 2024 Jeroen P. Broks
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
#undef KTHURA_LOADCOMPILED_DEBUG

#include <SlyvString.hpp>
#include "../Headers/Kthura_LoadCompiled.hpp"

#ifdef KTHURA_LOADCOMPILED_DEBUG
#define Chat(AAA) std::cout << "\x1b[37m\x1b[41mKCK Loader Debug>\x1b[40m\x1b[37m " << AAA << std::endl;
#else
#define Chat(AAA)
#endif

namespace Slyvina {
	namespace Kthura {

		static std::string _ErrorLog{};

#pragma region "The REAL stuff"

#define WR() _WR(BT,Dict); if (_ErrorLog.size()) return
#define WRF(F) F(_WR(BT,Dict)); if (_ErrorLog.size()) return
		static std::string _WR(Bank BT, std::map < uint64, std::string>& Dict) {
			//uint64 idx{ 0 };
			Byte Siz{ BT->ReadByte() }; Chat("GetDictString: Size: " << (int)Siz << TrSPrintF(" (Position %d::%x)",BT->Position(), BT->Position()));
			switch (Siz) {
			case 0: return BT->ReadString();
			case 1: 
#ifdef KTHURA_LOADCOMPILED_DEBUG
			{
				int idx = BT->ReadByte();
				Chat("Dict size1; Index:" << idx << " -> " << Dict[idx]);
				for (auto DI : Dict) Chat(" Dict #" << DI.first << ": " << DI.second);
				return Dict[idx];
			}
#else
				return Dict[BT->ReadByte()];
#endif
			case 2: return Dict[BT->ReadUInt16()];
			case 4: return Dict[BT->ReadUInt32()];
			case 8: return Dict[BT->ReadUInt64()];
			default: _ErrorLog = TrSPrintF("Unknown string dictionary index size (%d)",Siz); return "???";
			}
		}
		static void _TrueLoadCompiledKthura(_Kthura* K,JCR6::JT_Dir D, std::string prefix) {
			_ErrorLog = "";
			if (prefix.size() && (!Suffixed(prefix, "/"))) prefix += "/";

			// Load Dictionary
			Byte size{ 1 };
			uint64 indexes{ 0 };
			auto BT{ D->B(prefix + "Dictionary") };
			if (!BT) {
				_ErrorLog = "Failed to read dictionary! (" + prefix + "Dictionary)\n" + JCR6::Last()->ErrorMessage;
				Chat(_ErrorLog);
				return;
			}
			std::map < uint64, std::string> Dict{};
			while (!BT->AtEnd()) {
				Chat("Dictionary " << BT->Position() << "/" << BT->Size());
				auto Tag{ BT->ReadByte() }; 
				if (!Tag) break;
				switch (Tag) {
				case 1:	size = BT->ReadByte(); break;
				case 2: indexes = BT->ReadUInt64(); break;
				case 3: {
					uint64 i; std::string s;
					switch (size) {
					case 1: i = BT->ReadByte(); break;
					case 2: i = BT->ReadUInt16(); break;
					case 4: i = BT->ReadUInt32(); break;
					case 8: i = BT->ReadUInt64(); break;
					default: _ErrorLog = TrSPrintF("Unknown Dictionary Index Size (%d)", size); return;
					}
					Dict[i] = BT->ReadString();
					Chat("Dictionary entry #" << i << " has been set to \"" << Dict[i] << "\"");
				}
				}
			}

			// Load Compiled Objects
			BT = D->B(prefix + "CObjects");
			KthuraLayer* L{ nullptr };
			KthuraObject* O{ nullptr };
			auto LTag{ BT->ReadByte() };
			std::string LayName{};
			while (LTag) {
				Chat("Compiled Object Command Tag: " << (int)LTag << "\tPos:" << BT->Position() - 1);
				if ((!L) && LTag != 1) { _ErrorLog = "No layer"; return; }
				if ((!O) && LTag != 1 && LTag!=2 && LTag!=255) { _ErrorLog = "No objext"; return; }
				switch (LTag) {
				case 1:
					LayName = BT->ReadString();
					if (!K->HasLayer(LayName)) {
						Chat("Creating Layer " << LayName);
						L = K->NewLayer(LayName);
					} else {
						Chat("Appending to Layer " << LayName);
						L = K->Layer(LayName);
					}
					if (!L) { _ErrorLog = "Creating or getting Layer '" + LayName + "' failed"; return; }
					L->gridx = 32;
					L->gridy = 32;
					break;
				case 2: {
					std::string _kind; _kind = WR();
					Chat("Creating object: " << _kind << " (" << (int)KindName(_kind) << ")");
					O = L->NewObject(KindName(_kind));
					if (!O) { _ErrorLog = "Creating an object failed"; return; }
				} break;
				case 3:
					O->x(BT->ReadInt32());
					O->y(BT->ReadInt32());
					break;
				case 4:
					O->w(BT->ReadInt32());
					O->h(BT->ReadInt32());
					break;
				case 5:
					O->insertx(BT->ReadInt32());
					O->inserty(BT->ReadInt32());
					break;
				case 6: {
					auto tex = WR();
					O->texture(tex);
				} break;
				case 7:
					O->a(BT->ReadByte());
					break;
				case 8:
					O->r(BT->ReadByte());
					O->g(BT->ReadByte());
					O->b(BT->ReadByte());
					break;
				case 9:
					O->animframe(BT->ReadInt32());
					O->animskip(BT->ReadUInt32());
					O->animspeed(BT->ReadInt32());
					break;
				case 10:
					O->blend(BT->ReadInt32());
					break;
				case 11:
					O->CustomKind = WR();
					break;
				case 12:
					O->dominance(BT->ReadInt32());
					break;
				case 13:
					O->impassible(BT->ReadByte() > 0);
					O->forcepassible(BT->ReadByte() > 0);
					break;
				case 14:
					O->rotatedeg(BT->ReadInt32());
					break;
				case 15:
					O->scalex(BT->ReadInt32());
					O->scaley(BT->ReadInt32());
					break;
				case 16:
					WRF(O->labels);
					break;
				case 17:
					WRF(O->Tag);
					break;
				case 18:
					O->visible(BT->ReadByte() > 0);
					break;
				case 19: {
					auto k = WR();
					auto v = WR();
					O->data(k, v);
					break;
				} break;
				case 255:
					L->gridx = BT->ReadInt32();
					L->gridy = BT->ReadInt32();
					break;
				default:
					_ErrorLog = TrSPrintF("Unknown Compiled Objects Command Tag (%d)", LTag);
					return;
				}
				LTag = BT->ReadByte();
#ifdef KTHURA_LOADCOMPILED_DEBUG
				if (!LTag) Chat("Ending tag found! "<< TrSPrintF("%d/%x",BT->Position() - 1, BT->Position() - 1));
#endif
			}
			K->MetaData = D->GetStringMap(prefix + "Data");
		}

#pragma endregion

#pragma region "Public Stuff"
		static bool _RCXLRecognize(JCR6::JT_Dir D, std::string prefix) {
			if (prefix.size() && (!Suffixed(prefix, "/"))) prefix += "/";
			Chat("Recognize check:\t" << D->EntryExists(prefix + "CObjects") << D->EntryExists(prefix + "Data") << D->EntryExists(prefix + "Dictionary"));
			return D->EntryExists(prefix + "CObjects") && D->EntryExists(prefix + "Data") && D->EntryExists(prefix + "Dictionary");
		}

		std::string CompiledKthuraError() { return _ErrorLog; }

		void RegCompiledXLoader() {
			RegisterKthuraLoader("Compiled", LoadCompiledKthura, _RCXLRecognize);
		}

		Kthura LoadCompiledKthura(JCR6::JT_Dir D, std::string prefix) {
			auto NK{ new _Kthura(false) };
			_TrueLoadCompiledKthura(NK, D, prefix);
			if (_ErrorLog.size()) { delete NK; return nullptr; }
			return std::shared_ptr<_Kthura>(NK);
		}

		UKthura LoadCompiledUKthura(JCR6::JT_Dir D, std::string prefix) {
			auto NK{ new _Kthura(false) };
			_TrueLoadCompiledKthura(NK, D, prefix);
			if (_ErrorLog.size()) { delete NK; return nullptr; }
			return std::unique_ptr<_Kthura>(NK);
		}

		bool IsCompiledKthura(JCR6::JT_Dir D,std::string prefix) {			return _RCXLRecognize(D,prefix);		}

#pragma endregion 
	}
}