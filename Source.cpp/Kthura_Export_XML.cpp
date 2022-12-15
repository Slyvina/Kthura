// Lic:
// Kthura/Source.cpp/Kthura_Export_XML.cpp
// Slyvina - Kthura XML Exporter
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
#include <Kthura_Export_XML.hpp>
#include <SlyvStream.hpp>

using namespace Slyvina::Units;

namespace Slyvina {
	namespace Kthura {

		std::string Kthura_ToXML(_Kthura* Map) {
			std::string ret{ "<Kthura>\n" };
			auto MD{ Map->MetaData };
			for (auto& md : *MD) {
				ret += "\t<metadata key=\"" + md.first + "\" value=\"" + md.second + "\" />\n";
			}
			ret += "\t<LayerList>\n";
			for (auto l : *Map->Layers()) {
				ret += "\t\t<LayerListItem>" + l + "</LayerListItem>\n";
			}
			ret += "\t</LayerList>\n";
			for (auto l : *Map->Layers()) {
				auto Lay{ Map->Layer(l) };
				ret += TrSPrintF("\t<Layer name=\"%s\" gridx=\"%d\" gridy=\"%d\">\n",l.c_str(),Lay->gridx,Lay->gridy);
				for (auto o = Map->Layer(l)->FirstObject(); o; o=o->Next()) {
					auto kind{ o->SKind() };
					std::cout <<o->ID() << "> Kind: " << kind << std::endl;
					ret += "\t\t<object kind=\"" + kind + "\">\n";
					ret += TrSPrintF("\t\t\t<x>%d</x>\n", o->x());
					ret += TrSPrintF("\t\t\t<y>%d</y>\n", o->y());
					ret += TrSPrintF("\t\t\t<w>%d</w>\n", o->w());
					ret += TrSPrintF("\t\t\t<h>%d</h>\n", o->h());
					ret += TrSPrintF("\t\t\t<y>%d</y>\n", o->y());
					ret += TrSPrintF("\t\t\t<red>%d</red>\n", o->r());
					ret += TrSPrintF("\t\t\t<green>%d</green>\n", o->g());
					ret += TrSPrintF("\t\t\t<blue>%d</blue>\n", o->b());
					ret += TrSPrintF("\t\t\t<alpha>%d</alpha>\n", o->a());
					ret += "\t\t\t<impassible>" + boolstring(o->impassible()) + "</impassible>\n";
					ret += "\t\t\t<forcepassible>" + boolstring(o->impassible()) + "</forcepassible>\n";
					ret += "\t\t\t<visible>" + boolstring(o->impassible()) + "</visible>\n";
					ret += "\t\t\t<texture>" + o->texture() + "</texture>\n";
					ret += "\t\t\t<labels>" + o->texture() + "</labels>\n";
					ret += "\t\t\t<tag>" + o->Tag() + "</tag>\n";
					ret += TrSPrintF("\t\t\t<dominance>%d</dominance>\n", o->dominance());
					ret += TrSPrintF("\t\t\t<animspeed>%d</animspeed>\n", o->animspeed());
					ret += TrSPrintF("\t\t\t<animframe>%d</animframe>\n", o->animframe());
					ret += TrSPrintF("\t\t\t<insert x=\"%d\" y=\"%d\" />\n", o->insertx(),o->inserty());
					ret += TrSPrintF("\t\t\t<scale x=\"%d\" y=\"%d\" />\n", o->scalex(), o->scaley());
					ret += TrSPrintF("\t\t\t<rotate>%d</rotate>\n", o->rotate());
					for (auto& d : *o->data()) {
						if (d.second.size()) ret += "\t\t\t<data key=\"" + d.first + "\" value=\"" + d.second + "\" />\n";
					}
					ret += "\t\t</object>\n";
				}
				ret += "\t</Layer>\n";
			}
			ret += "</Kthura>";
			return ret;
		}

		void Kthura_Export_XML(_Kthura* Map, std::string outputfile) {
			SaveString(outputfile, Kthura_ToXML(Map));
		}
	}
}