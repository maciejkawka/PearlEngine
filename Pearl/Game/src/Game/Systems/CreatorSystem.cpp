#include "Systems/CreatorSystem.h"

#include "SceneCreators/SceneCreator.h"

namespace ChessGame {

	void CreatorSystem::OnCreate()
	{
		SceneCreator creator{ &m_entityViewer };
		creator.Build();
	}
}
