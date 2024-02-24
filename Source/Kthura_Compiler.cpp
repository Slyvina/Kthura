// Lic:
// Kthura/Source/Kthura_Compiler.cpp
// Kthura Compiler
// version: 24.02.24
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
#define KC_DEBUG

#include "../Headers/Kthura_Compiler.hpp"

#ifdef KC_DEBUG
#include <iostream>
#define Chat(a) std::cout << "\x1b[32mKthura Compiler Debug> \x1b[0m"<<a<<"\n";
#else
#define Chat(a)
#endif

namespace Slyvina {
	namespace Kthura {

		std::string CompileStorage = "Store";


		class DictCount {
		private:
			std::map<std::string, size_t> _tDict;
			size_t ignore{ 0 };
		public:
			size_t& operator[](std::string k) {
				if (k == "") {
					ignore = 0;
					return ignore;
				}
				if (!_tDict.count(k)) _tDict[k] = 0;
				return _tDict[k];
			}
			void Clean() {
				std::vector<std::string> victims;
				for (auto k : _tDict) if (k.second < 2) victims.push_back(k.first);
				for (auto v : victims) _tDict.erase(v);
			}
#ifdef KC_DEBUG
			void Show() {
				for (auto k : _tDict) Chat("Dictionary: " << k.first << " = " << k.second);
			}
#endif
			void Generate(std::map<std::string, uint64>& D, Byte& S, uint64& idx) {
				for (auto k : _tDict) D[k.first] = idx++;
				if (idx > 0xff) S = 2;
				if (idx > 0xffff) S = 4;
				if (idx > 0xffffffff) S = 8;				
			}
		};

		void WS(JCR6::JT_CreateStream BT, std::string v, Byte& S, std::map<std::string, uint64>& D) {
			if (!D.count(v)) {
				BT->WriteByte(0);
				BT->Write(v);
				return;
			}
			BT->Write(S);
			switch (S) {
			case 1: BT->Write((byte)D[v]); break;
			case 2: BT->WriteUInt16((uint16)D[v]); break;
			case 4: BT->WriteUInt32((uint32)D[v]); break;
			case 8: BT->Write(D[v]); break;
			default: throw std::runtime_error(TrSPrintF("INTERNAL KTHURA COMPILER ERROR! Unknown size(%d) in writing string in objects",S));
			}
		}

		void Compile(Kthura kmap, JCR6::JT_CreateBlock JOutBlock, std::string prefix) {
			// Count for dictionary
			DictCount Cnt{};
			auto lm{ kmap->Layers() };
			for (auto L : *lm) {
				auto lay{ kmap->Layer(L) };
				for (auto o = lay->FirstObject(); o; o = o->Next()) {
					auto data{ o->data() };
					Cnt[o->Tag()]++;
					Cnt[o->texture()]++;
					Cnt[o->labels()]++;
					Cnt[o->SKind()]++;
					Cnt[o->CustomKind]++;
					for (auto di : *data) {
						Cnt[di.first]++;
						Cnt[di.second]++;
					}
				}
			}
			Cnt.Clean();
#ifdef KC_DEBUG
			Cnt.Show();
#endif
			// Generate Dictionary
			Byte size{ 1 };
			uint64 indexes{ 0 };
			std::map<std::string, uint64> Dict{};
			Cnt.Generate(Dict, size, indexes);
			Chat("Dictionary:  size = " << (int)size << "; indexes = " << indexes);

			auto BT{ JOutBlock->nb("Dictionary") };
			BT->Write((byte)1);
			BT->Write(size);
			BT->Write((byte)2);
			BT->Write(indexes);

			for (auto di : Dict) {
				BT->Write((byte)3);
				switch(size){
				case 1: BT->Write((Byte)di.second); break;
				case 2: BT->WriteUInt16((uint16)di.second); break;
				case 4: BT->WriteUInt32((uint32)di.second); break;
				case 8: BT->Write(di.second); break;
				default:
					throw std::runtime_error("INTERNAL KTHURA COMPILER ERROR! Unknown size("+std::to_string(size)+") in writing dictionary");
					break;
				}
				BT->Write(di.first);
			}
			BT->Close();

			// Meta Data
			JOutBlock->NewStringMap(kmap->MetaData, "Data");

			// Objects
			BT = JOutBlock->nb("CObjects");

			for (auto L : *lm) {
				BT->WriteByte(1);
				BT->Write(L);
				auto lay{ kmap->Layer(L) };
				for (auto o = lay->FirstObject(); o; o = o->Next()) {
					auto data{ o->data() };
					// New Object + Kind
					BT->WriteByte(2);
					WS(BT,o->SKind(),size,Dict);
					BT->WriteByte(3);
					BT->Write(o->x());
					BT->Write(o->y());
					BT->WriteByte(4);
					BT->Write(o->w());
					BT->Write(o->h());
					BT->WriteByte(5);
					BT->Write(o->insertx());
					BT->Write(o->inserty());
					BT->WriteByte(6);
					WS(BT, o->texture(), size, Dict);
					BT->WriteByte(7);
					BT->WriteByte(o->a());
					BT->WriteByte(8);
					BT->WriteByte(o->r());
					BT->WriteByte(o->g());
					BT->WriteByte(o->b());
					BT->WriteByte(9);
					BT->Write(o->animframe());
					BT->Write(o->animskip());
					BT->Write(o->animspeed());
					BT->WriteByte(10);
					BT->Write(o->blend());
					BT->WriteByte(11);
					WS(BT, o->CustomKind, size, Dict);
					BT->WriteByte(12);
					BT->Write(o->dominance());
					BT->WriteByte(13);
					BT->Write((byte)o->impassible());
					BT->Write((byte)o->forcepassible());
					BT->WriteByte(14);
					BT->Write(o->rotatedeg());
					BT->WriteByte(15);
					BT->Write(o->scalex());
					BT->Write(o->scaley());
					BT->WriteByte(16);
					WS(BT, o->labels(), size, Dict);
					BT->WriteByte(17);
					WS(BT, o->Tag(), size, Dict);
					BT->WriteByte(17);
					BT->Write((byte)o->visible());
					for (auto di : *data) {
						BT->WriteByte(18);
						WS(BT, di.first, size, Dict);
						WS(BT, di.second, size, Dict);
					}
				}
			}
			BT->WriteByte(0); // The end
			BT->Close();
		}

		void Compile(Kthura kmap, JCR6::JT_Create JOut, std::string prefix) {
			auto JB{ JOut->AddBlock(CompileStorage) };
			Compile(kmap, JB, prefix);
			JB->Close();
		}

		void Slyvina::Kthura::Compile(Kthura kmap, std::string Jout, std::string prefix) {
			auto J{ JCR6::CreateJCR6(Jout,CompileStorage) };
			Compile(kmap, J, prefix);
			J->Close();
		}
	}
}