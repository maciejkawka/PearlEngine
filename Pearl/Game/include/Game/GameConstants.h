#pragma once

#include "Engine/Core/Input/PrKey.h"

#include <string_view>

namespace ChessGame {

	// Scene entity names
	constexpr std::string_view MainCameraName      = "MainCamera";
	constexpr std::string_view SquareOverlayPrefix = "Square_Overlay_";
	constexpr std::string_view ScenePiecePrefix    = "Piece_";
	constexpr std::string_view OverlayName         = "Overlay";
	constexpr std::string_view OverlayTag          = "Overlay";

	// Asset paths
	constexpr std::string_view MeshPath            = "Chess/mesh/";
	constexpr std::string_view MaterialsPath       = "Chess/materials/";
	constexpr std::string_view PhysicsPath         = "Chess/phys/";
	constexpr std::string_view OverlayMaterialPath = "Chess/materials/overlay.mat";

	// Save file
	constexpr std::string_view SaveFilePath        = "save.chess";

	// Board
	constexpr int   BoardSquaresCount = 64;
	constexpr int   BoardFilesCount   = 8;
	constexpr float PieceHeight       = 0.064655f;
	constexpr float PieceLiftHeight   = 0.1f;

	// Input bindings
	constexpr PrCore::PrKey ResetGameKey     = PrCore::PrKey::R;
	constexpr PrCore::PrKey SaveGameKey      = PrCore::PrKey::P;
	constexpr PrCore::PrKey LoadGameKey      = PrCore::PrKey::L;
	constexpr PrCore::PrKey DeselectKey      = PrCore::PrKey::G;
	constexpr PrCore::PrKey CameraUpKey      = PrCore::PrKey::W;
	constexpr PrCore::PrKey CameraDownKey    = PrCore::PrKey::S;
	constexpr PrCore::PrKey CameraLeftKey    = PrCore::PrKey::A;
	constexpr PrCore::PrKey CameraRightKey   = PrCore::PrKey::D;
	constexpr PrCore::PrKey CameraPreset1Key = PrCore::PrKey::KEY_1;
	constexpr PrCore::PrKey CameraPreset2Key = PrCore::PrKey::KEY_2;
	constexpr PrCore::PrKey CameraPreset3Key = PrCore::PrKey::KEY_3;
	constexpr PrCore::PrKey CameraPreset4Key = PrCore::PrKey::KEY_4;
}
