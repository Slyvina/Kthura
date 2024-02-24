// Lic:
// Kthura/Headers/Kthura_Compiler.hpp
// Kthura Compiler (header)
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