// Lic:
// Kthura/Headers/Kthura_LoadCompiled.hpp
// Compiled Kthura Map Leader (header)
// version: 24.09.24
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
#include "Kthura_Core.hpp"

namespace Slyvina {
	namespace Kthura {

		/// <summary>
		/// 
		/// </summary>
		/// <returns>Last error message (empty if last attempt to load a compiled Kthura map was without errors)</returns>
		std::string CompiledKthuraError();

		/// <summary>
		/// Registers the Compiled Loader to XKthuraLoad()
		/// </summary>
		void RegCompiledXLoader();

		/// <summary>
		/// Loads Compiled Kthura Map
		/// </summary>
		/// <param name="D"></param>
		/// <param name="prefix"></param>
		/// <returns>Kthura map as a shared pointer</returns>
		Kthura LoadCompiledKthura(JCR6::JT_Dir D, std::string prefix="");

		/// <summary>
		/// Loads Compiled Kthura Map
		/// </summary>
		/// <param name="D"></param>
		/// <param name="prefix"></param>
		/// <returns>Kthura map as a unique pointer</returns>
		UKthura LoadCompiledUKthura(JCR6::JT_Dir D, std::string prefix="");

		bool IsCompiledKthura(JCR6::JT_Dir D, std::string prefix = "");
	}
}