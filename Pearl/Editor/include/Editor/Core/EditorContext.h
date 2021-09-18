#pragma once
#include"Core/Entry/AppContext.h"

namespace PrEditor::Core {

	class EditorContext : public PrCore::Entry::AppContext {
	public:
		EditorContext();
		~EditorContext();
	};

}