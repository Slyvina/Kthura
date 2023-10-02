
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
			for (auto node = Result->First(); node; node = node->Next()) Ret.push_back({ node->x(),node->y() });
			LastSuccess = Ret.size();
			return Ret;
		}

		static bool GetLastSuccess(KthuraWalk* s) { return LastSuccess; }

		void Slyvina::Kthura::InitDijkstraForKthura() {
			_Kthura::Walk.Route = DRVRoute;
			_Kthura::Walk.Succes = GetLastSuccess;
		}
	}
}