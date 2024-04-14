#pragma once

#include <vector>

#include "game/mission_manager.hpp"

enum class E_RH_MISSION_SCRIPT_TYPE
{
	E_NONE,
	E_SPAWN_POINT, // teleport player to a spawn point
	E_LOAD_REGION, // load and teleport player to a region / interrior
	E_POSITION, // teleport player to a specific position
	E_PHOTO, // teleport to robbie in daily bugle
};

typedef struct RHCheckpointScript
{
	const char* instance_name = "";
	E_RH_MISSION_SCRIPT_TYPE script_type = E_RH_MISSION_SCRIPT_TYPE::E_NONE;
	union ScriptPositionData
	{
		spawn_point_index_t spawn_point_index;
		const char* region_name;
		vector3d absolute_position = { };
	} script_position_data;
	bool has_delay = false;
	mission_checkpoint_t selected_checkpoint = static_cast<mission_checkpoint_t>(0);
	union
	{
		region* region = nullptr;
	} cache;
	const char* display_name = nullptr;

	RHCheckpointScript(const mission_checkpoint_t& checkpoint)
	{
		this->selected_checkpoint = checkpoint;
	}

	inline RHCheckpointScript type(const E_RH_MISSION_SCRIPT_TYPE& script_type)
	{
		this->script_type = script_type;
		return *this;
	}

	inline RHCheckpointScript spawn_point_index(const spawn_point_index_t& spawn_point_index)
	{
		this->script_position_data.spawn_point_index = spawn_point_index;
		return *this;
	}

	inline RHCheckpointScript spawm_region(const char* const& region_name)
	{
		this->script_position_data.region_name = region_name;
		return *this;
	}

	inline RHCheckpointScript spawn_position(const vector3d& absolute_position)
	{
		this->script_position_data.absolute_position = absolute_position;
		return *this;
	}

	inline RHCheckpointScript delay_load(const bool& value = true)
	{
		this->has_delay = value;
		return *this;
	}

	inline RHCheckpointScript custom_display_name(const char* const& name)
	{
		this->display_name = name;
		return *this;
	}

} RHCheckpointScript;

typedef struct RHMissionScript : public RHCheckpointScript
{
	std::vector<RHCheckpointScript> checkpoints_scripts;

	RHMissionScript(const char* const& instance_name) : RHCheckpointScript(0)
	{
		this->instance_name = instance_name;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.instance_name = instance_name;
		}
	}

	inline RHMissionScript type(const E_RH_MISSION_SCRIPT_TYPE& script_type)
	{
		this->script_type = script_type;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_type = script_type;
		}

		return *this;
	}

	inline RHMissionScript spawn_point_index(const spawn_point_index_t& spawn_point_index)
	{
		this->script_position_data.spawn_point_index = spawn_point_index;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_position_data.spawn_point_index = spawn_point_index;
		}

		return *this;
	}

	inline RHMissionScript spawm_region(const char* const& region_name)
	{
		this->script_position_data.region_name = region_name;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_position_data.region_name = region_name;
		}

		return *this;
	}

	inline RHMissionScript spawn_position(const vector3d& absolute_position)
	{
		this->script_position_data.absolute_position = absolute_position;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.script_position_data.absolute_position = absolute_position;
		}

		return *this;
	}


	inline RHMissionScript specific_checkpoints_scripts(const std::vector<RHCheckpointScript>& scrips)
	{
		this->checkpoints_scripts = scrips;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.instance_name = this->instance_name;
		}

		return *this;
	}

	inline RHMissionScript checkpoints(const mission_checkpoint_t& start, const mission_checkpoint_t& end)
	{
		for (mission_checkpoint_t i = start; i <= end; i++)
		{
			RHCheckpointScript checkpoint(i);
			checkpoint.instance_name = this->instance_name;
			this->checkpoints_scripts.push_back(checkpoint);
		}

		return *this;
	}

	inline RHMissionScript checkpoints(const std::vector<mission_checkpoint_t>& list)
	{
		for (mission_checkpoint_t i : list)
		{
			RHCheckpointScript checkpoint(i);
			checkpoint.instance_name = this->instance_name;
			this->checkpoints_scripts.push_back(checkpoint);
		}

		return *this;
	}

	inline RHMissionScript delay_load(const bool& value = true)
	{
		this->has_delay = value;
		for (RHCheckpointScript& checkpoint : this->checkpoints_scripts)
		{
			checkpoint.has_delay = value;
		}
		return *this;
	}

} RHMissionScript;

static RHMissionScript s_MissionsScripts[] = /* MEGACITY.PCPACK */
{
	{
		RHMissionScript("SWINGING_TUTORIAL_GO")
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_1")
			.checkpoints(0, 5)
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_2")
			.checkpoints(0, 6)
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_3")
			.specific_checkpoints_scripts({
				RHCheckpointScript(1),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 2674.779f, 20, 375 })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 2596, 88, 857.203f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2),

				RHCheckpointScript(5)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2),

				RHCheckpointScript(6)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2),

				RHCheckpointScript(7)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(2)
			})

	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_4")
			.specific_checkpoints_scripts({
				RHCheckpointScript(1),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2479.906f, 8, -1018.858f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2515, 8, -1018.858f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MA4I06"),

				RHCheckpointScript(5)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MA4I03")
			})
	},
	{
		RHMissionScript("STORY_INSTANCE_MAD_BOMBER_5")
			.checkpoints(0, 3)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
			.spawn_position(vector3d({ 1362.808f, 131.012f, 182.475f }))
			.delay_load()
	},
	{
		RHMissionScript("STORY_INSTANCE_LIZARD_1")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 1245.894f, 1, 441.800f })),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 1427.700f, 1, 367.942f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 783.314f, 1, 233.503f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 452.307f, 1.5f, 39.993f }))
			})
	},
	{
		RHMissionScript("STORY_INSTANCE_LIZARD_2")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(1),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -1538.952f, -177.150f, -352.311f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -1702.201f, -170, -785.925f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2384.687f, -160.800f, -489.823f })),

				RHCheckpointScript(5)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2384.390f, -251, -396.396f })),

				RHCheckpointScript(6)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2200.871f, -238, -275.614f })),

				RHCheckpointScript(7)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB2I03"),

				RHCheckpointScript(8)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB2I03"),
			})
	},
	{
		RHMissionScript("STORY_INSTANCE_LIZARD_3")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -1051.248f, -215.723f, 47.251f })),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB3I02"),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB3I02"),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("MB3I03"),
			})
	},
	{
		RHMissionScript("GANG_INSTANCE_ATOMIC_PUNK_01")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H02")
	},
	{
		RHMissionScript("GANG_INSTANCE_ATOMIC_PUNK_05")
			.checkpoints(1, 4)
	},
	{
		RHMissionScript("GANG_INSTANCE_ATOMIC_PUNK_07")
			.checkpoints(0, 4)
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_01")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("N08I01")
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_02")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("M07I01"),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("M07I01"),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position({ -320.349f, 5, 1432.874f })
			})
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_04")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
					.spawn_point_index(0),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
					.spawm_region("N05I01")
			})
	},
	{
		RHMissionScript("GANG_INSTANCE_GOTHIC_LOLITA_05")
			.checkpoints(0, 1)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_01")
			.checkpoints({0, 2})
			.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
			.spawn_point_index(4)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_05")
			.checkpoints(1, 4)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_06")
			.checkpoints(1, 5)
	},
	{
		RHMissionScript("GANG_INSTANCE_PAN_ASIAN_07")
			.checkpoints(0, 4)
	},
	{
		RHMissionScript("LOCATION_INSTANCE_DEWOLFE_1")
			.checkpoints(1, 3)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("G19I01")
	},
	{
		RHMissionScript("LOCATION_INSTANCE_DEWOLFE_3")
			.checkpoints(0, 1)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_SPAWN_POINT)
			.spawn_point_index(12)
	},
	{
		RHMissionScript("LOCATION_INSTANCE_DEWOLFE_4")
			.checkpoints(0, 2)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("J02")
	},
	{
		RHMissionScript("MJ_THRILLRIDE_H17_00")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H17")
			.delay_load()
	},
	{
		RHMissionScript("MJ_THRILLRIDE_H17_01")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H17")
			.delay_load()
	},
	{
		RHMissionScript("MJ_THRILLRIDE_H17_02")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("H17")
			.delay_load()
	},
	{
		RHMissionScript("STORY_INSTANCE_SCORPION_2")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
			.spawn_position(vector3d({ 3287.11f, 116.0f, 531.651f }))
	},
	{
		RHMissionScript("STORY_INSTANCE_SCORPION_3")
			.checkpoints(0, 5)
	},
	{
		RHMissionScript("STORY_INSTANCE_SCORPION_5")
			.checkpoints({0, 2, 3})
	},
	{
		RHMissionScript("STORY_INSTANCE_KINGPIN_1")
			.checkpoints(0, 1)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("MD1I01")
	},
	{
		RHMissionScript("STORY_INSTANCE_KINGPIN_2")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("MD2I01")
	},
	{
		RHMissionScript("BROCK_BEATDOWN")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("DBGI01")
	},
	{
		RHMissionScript("MJ_SCARERIDE_Q05_00")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("Q05")
			.delay_load()
	},
	{
		RHMissionScript("LOCATION_INSTANCE_CONNORS_1")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 273.580f, -48, -492.695f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 273.641f, -190, -606.589f })),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -494.323f, -168.920f, -363.920f }))
			})
	},
	{
		RHMissionScript("LOCATION_INSTANCE_CONNORS_4")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(1)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2005.964f, -204, -465.470f })),

				RHCheckpointScript(2)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -2006.510f, -196.804f, -460.531f })),

				RHCheckpointScript(3)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ -1532.684f, -176.212f, -374.370f }))
			})
	},
	{
		RHMissionScript("STORY_INSTANCE_MOVIE_1")
			.checkpoints({0, 2, 3})
	},
	{
		RHMissionScript("STORY_INSTANCE_MOVIE_3")
			.checkpoints(0, 4)
			.type(E_RH_MISSION_SCRIPT_TYPE::E_LOAD_REGION)
			.spawm_region("ME3I01")
	},
	{
		RHMissionScript("STORY_INSTANCE_MOVIE_4")
			.specific_checkpoints_scripts({
				RHCheckpointScript(0),

				RHCheckpointScript(2),

				RHCheckpointScript(3),

				RHCheckpointScript(4)
					.type(E_RH_MISSION_SCRIPT_TYPE::E_POSITION)
					.spawn_position(vector3d({ 2835.128f, 106, -99.330f })),

				RHCheckpointScript(4)
					.custom_display_name("STORY_INSTANCE_MOVIE_4_04 (NO SCRIPT)")
			})
	},
	{
		RHMissionScript("PHOTO_CITY_TOUR")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_BEAUTY_CONTEST_1")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_GANG_EXO_1")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_EXOBITION_1")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_STUNTMAN")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	},
	{
		RHMissionScript("PHOTO_UFO")
			.type(E_RH_MISSION_SCRIPT_TYPE::E_PHOTO)
	}
};

void LoadMissionScript(RHCheckpointScript* mission);