#pragma once

#include <Windows.h>

#include "MissionScripts.hpp"

void ChangeHero(const char* hero);
void SpawnToPoint(size_t idx);
void FullHealth();
void UnlockAllUpgrades();
void KillHero();
void TeleportToCamera();
void FailCurrentMission();
void CompleteCurrentMission();
void AbortCurrentMission();
void SetWorldTime(DWORD hours);
void SetCameraFovDefault();
void ShowTimer();
void HideTimer();
void SetTimerTime();
void SetTimerColor();
void SetCameraMode();
void SetHeroColliderFlags(E_ENTITY_COLLIDER_FLAGS flags);
void SetScreenResolution(const std::pair<DWORD, DWORD>* resolution);
void KillAllEntities();
void TeleportAllEntitiesToMe();
void TeleportToNearestEntity();
void TeleportAllPedestriansToMe();
void LoadCutscene(const char* instance);
void SpawnToNearestSpawnPoint();
void LoadMissionScript(RHCheckpointScript* mission);