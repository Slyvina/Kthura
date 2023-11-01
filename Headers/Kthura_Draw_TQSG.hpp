// Lic:
// Kthura/Headers/Kthura_Draw_TQSG.hpp
// Kthura TQSG (header)
// version: 23.11.01
// Copyright (C) 2015-2022, 2023 Jeroen P. Broks
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
#include "Kthura_Draw.hpp"
#include <TQSG.hpp>
#include <JCR6_Core.hpp>

namespace Slyvina {
	namespace Kthura {

		typedef void (*TQSGKTHURAPANIEK)(std::string);
		extern TQSGKTHURAPANIEK TQSG_Kthura_Panic;
		extern bool AllowTexturelessActor;

		/// <summary>
		/// This function has only been implemented for editors which could require zones and exit points to be visible and have their tags shown on screen. That requires a font to be loaded. Other than that you can best leave this be.
		/// </summary>
		/// <param name="F">The Font Pointer</param>
		void TQSGKthuraFont(TQSG::TImageFont F);

		KthuraDraw Init_TQSG_For_Kthura(JCR6::JT_Dir J);
		KthuraDrawShared Init_TQSG_For_Kthura_Shared(JCR6::JT_Dir J);
	}
}