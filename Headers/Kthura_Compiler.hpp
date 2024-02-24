

#pragma once

#include <JCR6_core.hpp>
#include <JCR6_write.hpp>

#include "Kthura_Core.hpp"

namespace Slyvina {
	namespace Kthura {
		extern std::string CompileStorage;
		void Compile(Kthura kmap, JCR6::JT_CreateBlock JOutBlock, std::string prefix = "");
		void Compile(Kthura kmap, JCR6::JT_Create JOut, std::string prefix = "");
		void Compile(Kthura kmap, std::string Jout, std::string prefix = "");
	}
}