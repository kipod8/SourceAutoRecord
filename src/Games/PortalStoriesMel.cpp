#include "PortalStoriesMel.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

PortalStoriesMel::PortalStoriesMel() {
	this->version = SourceGame_PortalStoriesMel;
	Game::mapNames = {
		"sp_a1_tramride",
		"sp_a1_mel_intro",
		"sp_a1_lift",
		"sp_a1_garden",
		"sp_a2_garden_de",
		"sp_a2_underbounce",
		"sp_a2_once_upon",
		"sp_a2_past_power",
		"sp_a2_ramp",
		"sp_a2_firestorm",
		"sp_a3_junkyard",
		"sp_a3_concepts",
		"sp_a3_paint_fling",
		"sp_a3_faith_plate",
		"sp_a3_transition",
		"sp_a4_overgrown",
		"sp_a4_tb_over_goo",
		"sp_a4_two_of_a_kind",
		"sp_a4_destroyed",
		"sp_a4_factory",
		"sp_a4_core_access",
		"sp_a4_finale"
	};

	Game::achievements = {
		{"ACH.A3_DOORS", "Welcome to Aperture"},
		{"ACH.ADDICTED_TO_SPUDS", "Long-Term Relaxation"},
		{"ACH.BLUE_GEL", "Voices from Above"},
		{"ACH.SUMMER_SALE", "Firefighting 101"},
		{"ACH.BREAK_MONITORS", "Back on Track"},
		{"ACH.WHITE_GEL", "Persistent"},
		{"ACH.WHEATLEY_TRIES_TO", "Testing the Waters"},
		{"ACH.WAKE_UP", "Forever Alone"},
		{"ACH.BREAK_OUT", "Deja-Vu"},
		{"ACH.BRIDGE", "Organic Complications"},
		{"ACH.YOU_MONSTER", "Back off track"},
		{"ACH.CATCH_CRAZY_BOX", "Welcome to my Domain"},
		{"ACH.CLEAN_UP", "System Shutdown"},
		{"ACH.TRIVIAL_TEST", "Under the Stairs"},
		{"ACH.TRACTOR_BEAM", "You Shouldn't be Here"},
		{"ACH.SURVIVE_CONTAINER_RIDE", "Single rainbow"},
		{"ACH.STALEMATE_ASSOCIATE", "Burned in Goo"},
		{"ACH.SPEED_RUN_LEVEL", "Crushed"},
		{"ACH.SHOOT_THE_MOON", "Shot"},
		{"ACH.SAVE_REDEMPTION_TURRET", "Electrocution"},
		{"ACH.ORANGE_GEL", "In the Vents"},
		{"ACH.TEAM_BUILDING", "Beyond your Range of Hearing"},
		{"ACH.NO_BOAT", "Into Darkness"},
		{"ACH.PORTRAIT", "Ignorant"},
		{"ACH.HI_FIVE_YOUR_PARTNER", "Curious"},
		{"ACH.REENTER_TEST_CHAMBERS", "Determined"},
		{"ACH.NOT_THE_DROID", "2056"},
		{"ACH.HUG_NAME", "Story Shutdown"}
	};
}
void PortalStoriesMel::LoadOffsets() {
	Portal2::LoadOffsets();

	using namespace Offsets;

#ifdef _WIN32
	// engine.dll
	OnGameOverlayActivated = 144;  // CSteam3Client
#else
	// Transferred from old Portal2 - should be removed if and when game
	// upgrades to new-style PIC engine
	GetClientStateFunction = 11;         // CEngineClient::ClientCmd
	GetLocalClient = 92;                 // CEngineClient::SetViewAngles
	HostState_OnClientConnected = 1523;  // CClientState::SetSignonState
	FireEventIntern = 36;                // CGameEventManager::FireEventClientSide
	ConPrintEvent = 254;                 // CGameEventManager::FireEventIntern
	AutoCompletionFunc = 37;             // listdemo_CompletionFunc
	Key_SetBinding = 60;                 // unbind
	VideoMode_Create = 104;              // CEngineAPI::Init
	AirMove_Offset1 = 14;                // CPortalGameMovement::~CPortalGameMovement
	UTIL_PlayerByIndex = 61;             // CServerGameDLL::Think
	GetClientMode = 11;                  // CHLClient::HudProcessInput
	GetHud = 104;                        // cc_leaderboard_enable
	FindElement = 120;                   // cc_leaderboard_enable
	KeyDown = 295;                       // CInput::JoyStickApplyMovement
	KeyUp = 341;                         // CInput::JoyStickApplyMovement
	StartDrawing = 692;                  // CMatSystemSurface::PaintTraverseEx
	FinishDrawing = 627;                 // CMatSystemSurface::PaintTraverseEx
	OnGameOverlayActivated = 152;        // CSteam3Client
	FontManager = 11;                    // GetFontName
#endif
}
const char *PortalStoriesMel::Version() {
	return "Portal Stories: Mel (8151)";
}
const char *PortalStoriesMel::ModDir() {
	return "portal_stories";
}
