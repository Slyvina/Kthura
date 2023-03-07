// Lic:
// Kthura/Source/Kthura_Save.cpp
// Slyvina - Kthura - Save
// version: 23.03.06
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

#include <SlyvString.hpp>
#include <SlyvTime.hpp>
#include "Kthura_Save.hpp"

using namespace Slyvina::Units;

namespace Slyvina {

	namespace Kthura {
		void Kthura_Save(Kthura M, JCR6::JT_Create J, std::string Dir, std::string storage, std::string Author, std::string Notes) {
			Dir = ChReplace(Dir, '\\', '/');
			if (Dir.size() && (!Suffixed(Dir, "/"))) Dir += "/";
			J->NewStringMap(M->MetaData, Dir + "Data", "Store", Author, Notes);
			std::string ObjectsStr{ "-- Kthura Object Data\n-- Saved: " + CurrentDate() + "; " + CurrentTime() + "\n\n\nLAYERS\n" };
			auto Lays{ M->Layers() };
			for (auto& IT : *Lays) ObjectsStr += "\t" + IT + "\n";
			ObjectsStr += "__END\n\n";
			for (auto& IT : *Lays) {
				auto Lay{ M->Layer(IT) };
				ObjectsStr += "\n\n\n-- Layer: " + IT + "\nLAYER = " + IT + "\n";
				ObjectsStr += TrSPrintF("BLOCKMAPGRID = %dx%d\n\n", Lay->gridx, Lay->gridy);
				for (auto O = Lay->FirstObject(); O; O = O->Next()) {
					ObjectsStr += TrSPrintF("\n\n-- Object #%d\nNEW\n", O->ID());
					ObjectsStr += "\tKIND = " + O->SKind() + "\n";
					ObjectsStr += TrSPrintF("\tCOORD = %d,%d\n", O->x(), O->y());
					ObjectsStr += TrSPrintF("\tINSERT = %d,%d\n", O->insertx(), O->inserty());
					ObjectsStr += TrSPrintF("\tROTATION = %d\n", O->rotatedeg());
					ObjectsStr += TrSPrintF("\tSIZE = %dx%d\n", O->w(), O->h());
					ObjectsStr += "\tTAG = " + O->Tag() + "\n";
					ObjectsStr += "\tLABELS = " + O->labels() + "\n";
					ObjectsStr += TrSPrintF("\tDOMINANCE = %d\n", O->dominance());
					ObjectsStr += "\tTEXTURE = " + O->texture() + "\n";
					ObjectsStr += TrSPrintF("\tCURRENTFRAME = %d\n", O->animframe());
					ObjectsStr += TrSPrintF("\tFRAMESPEED = %d\n", O->animspeed());
					ObjectsStr += TrSPrintF("\tALPHA255 = %d\n", O->alpha());
					ObjectsStr += TrSPrintF("\tVISIBLE = %d\n", (int)O->visible());
					ObjectsStr += TrSPrintF("\tCOLOR = %d,%d,%d\n", O->red(), O->green(), O->blue());
					ObjectsStr += TrSPrintF("\tIMPASSIBLE = %d\n", (int)O->impassible());
					ObjectsStr += TrSPrintF("\tFORCEPASSIBLE = %d\n", (int)O->forcepassible());
					ObjectsStr += TrSPrintF("\tSCALE = %d,%d\n", O->scalex(), O->scaley());
					ObjectsStr += TrSPrintF("\tBLEND = %d\n", O->blend());
				}
			}
			std::string S{ "Store" }; if (ObjectsStr.size() > 2048) S = storage;
			J->AddString(ObjectsStr, Dir + "Objects", S, Author, Notes);
			std::string OptionsStr{ M->UnParseOptions() };
			if (OptionsStr.size() > 2048) S = storage; else S = "Store";
			J->AddString(OptionsStr, Dir + "Options", S, Author, Notes);
			if (_Kthura::AllowUnknown) {
				for (auto& UNK : M->Unknown) {
					if (UNK.second->Size() > 2048) S = storage; else S = "Store";
					J->AddBank(UNK.second, Dir + UNK.first, S, Author, Notes);
				}
			}
		}

		void Kthura_Save(Kthura M, std::string File, std::string Dir, std::string storage, std::string Author, std::string Notes) {
			auto J{ JCR6::CreateJCR6(File) };
			Kthura_Save(M, J, Dir, storage);
			J->Close();
		}
	}
}