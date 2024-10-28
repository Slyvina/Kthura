// License:
// 
// Kthura
// Dijkstra driver
// 
// 
// 
// 	(c) Jeroen P. Broks, 2015-2021, 2023, 2024
// 
// 		This program is free software: you can redistribute it and/or modify
// 		it under the terms of the GNU General Public License as published by
// 		the Free Software Foundation, either version 3 of the License, or
// 		(at your option) any later version.
// 
// 		This program is distributed in the hope that it will be useful,
// 		but WITHOUT ANY WARRANTY; without even the implied warranty of
// 		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// 		GNU General Public License for more details.
// 		You should have received a copy of the GNU General Public License
// 		along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// 	Please note that some references to data like pictures or audio, do not automatically
// 	fall under this licenses. Mostly this is noted in the respective files.
// 
// Version: 24.10.28
// End License

#include <SlyvDijkstra.hpp>

#include "../Headers/Kthura_Walk_Dijkstra.hpp"
#include "../Headers/Kthura_Core.hpp"



namespace Slyvina {
	using namespace Units;

	
	namespace Kthura {
		static std::vector<KthuraSpot> DRVRoute(KthuraWalk* s, KthuraLayer* L, int startx, int starty, int endx, int endy);
		static bool DRVBlock(int x, int y);

		static Dijkstra DrvDijkstra{DRVBlock, DijkstraDirectionAllow::DiagonalIfStraightIsPossible};
		static KthuraLayer* ChkLayer{ nullptr };

		static bool DRVBlock(int x, int y) {
			return ChkLayer->Block(x, y);
		}

		static bool LastSuccess{ false };
		
		static std::vector<KthuraSpot> DRVRoute(KthuraWalk* s, KthuraLayer* L, int startx, int starty, int endx, int endy) {
			std::vector<KthuraSpot> Ret{};
			ChkLayer = L; // Not the cleanest method, but the only way to allow Dijkstra and Kthura to work together.
			auto Result{ DrvDijkstra.Route(startx,starty,endx,endy) };
			Ret.clear();
			if (Result)
				for (auto node = Result->First(); node; node = node->Next()) Ret.push_back({ node->x(),node->y() });
			LastSuccess = Ret.size();
			return Ret;
		}

		static bool GetLastSuccess(KthuraWalk* s) { return LastSuccess; }

		void InitDijkstraForKthura() {
			_Kthura::Walk.Route = DRVRoute;
			_Kthura::Walk.Succes = GetLastSuccess;
		}
	}
}
