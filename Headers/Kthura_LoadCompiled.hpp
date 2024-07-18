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