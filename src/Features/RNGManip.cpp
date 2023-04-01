#include "RNGManip.hpp"
#include "Utils/json11.hpp"
#include "Event.hpp"
#include "Offsets.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Hook.hpp"
#include <cstring>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include <set>

static std::optional<json11::Json> g_session_state;
static std::optional<json11::Json> g_pending_load;

static std::deque<QAngle> g_queued_view_punches;
static std::vector<QAngle> g_recorded_view_punches;

static json11::Json saveViewPunches() {
	std::vector<json11::Json> punches;

	for (QAngle punch : g_recorded_view_punches) {
		std::vector<json11::Json> ang{ {(double)punch.x, (double)punch.y, (double)punch.z} };
		punches.push_back(json11::Json(ang));
	}

	return json11::Json(punches);
}

static bool restoreViewPunches(const json11::Json &data) {
	if (!data.is_array()) return false;

	for (auto &val : data.array_items()) {
		float x = (float)val[0].number_value();
		float y = (float)val[1].number_value();
		float z = (float)val[2].number_value();
		g_queued_view_punches.push_back({x,y,z});
	}

	return true;
}

static json11::Json savePaintSprayers() {
	std::vector<json11::Json> vals;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		auto classname = server->GetEntityClassName(ent);
		if (!classname || strcmp(classname, "info_paint_sprayer")) continue;

		int seed = SE(ent)->field<int>("m_nBlobRandomSeed");
		vals.push_back({seed});
	}

	return vals;
}

static bool restorePaintSprayers(const json11::Json &data) {
	if (!data.is_array()) return false;

	size_t idx = 0;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		auto classname = server->GetEntityClassName(ent);
		if (!classname || strcmp(classname, "info_paint_sprayer")) continue;

		if (idx == data.array_items().size()) {
			// bad count
			return false;
		}

		SE(ent)->field<int>("m_nBlobRandomSeed") = data[idx].int_value();
		
		++idx;
	}
	
	return idx == data.array_items().size();
}

// clear old rng data
ON_EVENT_P(SESSION_START, 999) {
	g_queued_view_punches.clear();
	g_recorded_view_punches.clear();
}

// load pending rng data
ON_EVENT(SESSION_START) {
	if (!g_pending_load) return;

	json11::Json data = *g_pending_load;
	g_pending_load = std::optional<json11::Json>{};

	if (!engine->isRunning()) return;
	if (!sv_cheats.GetBool()) return;

	if (!data.is_object()) {
		console->Print("Invalid p2rng data!\n");
		return;
	}

	if (data["map"].string_value() != engine->GetCurrentMapName()) {
		console->Print("Invalid map for p2rng data!\n");
		return;
	}

	if (!restorePaintSprayers(data["paint"])) {
		console->Print("Failed to restore p2rng paint sprayer data!\n");
	}

	if (!restoreViewPunches(data["view_punch"])) {
		console->Print("Failed to restore p2rng view punch data!\n");
	}

	console->Print("p2rng restore complete\n");
}

// save rng data (after loading)
ON_EVENT_P(SESSION_START, -999) {
	if (!engine->isRunning()) {
		g_session_state = std::optional<json11::Json>{};
		return;
	}

	g_session_state = json11::Json(json11::Json::object{
		{ "map", { engine->GetCurrentMapName() } },
		{ "paint", savePaintSprayers() },
	});
}

void RngManip::saveData(const char *filename) {
	if (!g_session_state) {
		console->Print("No RNG data to save!\n");
		return;
	}

	auto root = g_session_state->object_items();
	root["view_punch"] = saveViewPunches();

	FILE *f = fopen(filename, "w");
	if (!f) {
		console->Print("Failed to open file %s\n", filename);
		return;
	}

	fputs(json11::Json(root).dump().c_str(), f);
	fclose(f);

	console->Print("Wrote RNG data to %s\n", filename);
}

void RngManip::loadData(const char *filename) {
	std::ifstream st(filename);
	if (!st.good()) {
		console->Print("Failed to open file %s\n", filename);
		return;
	}

	std::stringstream buf;
	buf << st.rdbuf();

	std::string err;
	auto json = json11::Json::parse(buf.str(), err);
	if (err != "") {
		console->Print("Failed to parse p2rng file: %s\n", err.c_str());
		return;
	}

	g_pending_load = json;

	console->Print("Read RNG data from %s\n", filename);
}

void RngManip::viewPunch(QAngle *offset) {
	if (g_queued_view_punches.size() > 0) {
		*offset = g_queued_view_punches.front();
		g_queued_view_punches.pop_front();
	}

	g_recorded_view_punches.push_back(*offset);
}

CON_COMMAND(sar_rng_save, "sar_rng_save <filename> - save RNG seed data to the specified file\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_rng_save.ThisPtr()->m_pszHelpString);
		return;
	}

	std::string filename = std::string(args[1]) + ".p2rng";
	RngManip::saveData(filename.c_str());
}

CON_COMMAND(sar_rng_load, "sar_rng_load <filename> - load RNG seed data on next session start\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_rng_load.ThisPtr()->m_pszHelpString);
		return;
	}

	std::string filename = std::string(args[1]) + ".p2rng";
	RngManip::loadData(filename.c_str());
}

static float *g_PhysicsHook_impactSoundTime;
static std::set<unsigned short> g_reset_sound_files;

struct SoundFile {
	unsigned short symbol;
	uint8_t gender;
	uint8_t available;
};

extern Hook g_EnsureAvailableSlotsForGender_Hook;
DECL_DETOUR_T(void, EnsureAvailableSlotsForGender, SoundFile *pSounds, int count, int gender) {
	for (int i = 0; i < count; ++i) {
		if (g_reset_sound_files.insert(pSounds[i].symbol).second) {
			//console->Print("sound availability reset\n");
			pSounds[i].available = 1;
		}
	}

	g_EnsureAvailableSlotsForGender_Hook.Disable();
	EnsureAvailableSlotsForGender(thisptr, pSounds, count, gender);
	g_EnsureAvailableSlotsForGender_Hook.Enable();
}
Hook g_EnsureAvailableSlotsForGender_Hook(EnsureAvailableSlotsForGender_Hook);

void RngManip::init() {
#ifdef _WIN32
	uintptr_t PhysFrame = Memory::Scan(server->Name(), "55 8B EC 8B 0D ? ? ? ? 83 EC 14 53 56 57 85 C9 0F 84 ? ? ? ? 80 3D ? ? ? ? 00 0F 85 ? ? ? ? F3 0F 10 4D 08 0F 2F 0D ? ? ? ? F3 0F 10 15 ? ? ? ? 0F 57 C0");
	uintptr_t m_bPaused = *(uint32_t *)(PhysFrame + 25);
	g_PhysicsHook_impactSoundTime = (float *)(m_bPaused - 4);

	EnsureAvailableSlotsForGender = (decltype(EnsureAvailableSlotsForGender))Memory::Scan(MODULE("soundemittersystem"), "55 8B EC 8B 4D 0C 33 C0 83 EC 20 3B C8 0F 8E ? ? ? ? 53 56 33 DB 33 F6 89 5D E0 89 45 E4 89 45 E8 89 75 EC 89 45 F0");
	g_EnsureAvailableSlotsForGender_Hook.SetFunc(EnsureAvailableSlotsForGender);
#else
	// TODO: mod support
	uintptr_t PhysFrame = Memory::Scan(server->Name(), "A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 55 89 E5 57 56 53 83 EC 3C 0F 2F 05 ? ? ? ?");
	uintptr_t m_bPaused = *(uint32_t *)(PhysFrame + 15);
	g_PhysicsHook_impactSoundTime = (float *)(m_bPaused - 4);

	EnsureAvailableSlotsForGender = (decltype(EnsureAvailableSlotsForGender))Memory::Scan(MODULE("soundemittersystem"), "55 57 56 53 83 EC 2C 8B 74 24 48 8B 5C 24 44 8B 7C 24 4C 85 F6 0F 8E ? ? ? ? C7 44 24 0C 00 00 00 00 31 D2 31 C9 31 C0");
	g_EnsureAvailableSlotsForGender_Hook.SetFunc(EnsureAvailableSlotsForGender);
#endif
}

ON_EVENT(SESSION_END) {
	engine->ExecuteCommand("phys_timescale 1", true);
	//console->Print("physics rng state reset\n");
	*g_PhysicsHook_impactSoundTime = 0.0f;
	//console->Print("impact sound time reset\n");
	g_reset_sound_files.clear();
}
