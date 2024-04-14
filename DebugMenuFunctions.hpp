#pragma once

#include <Windows.h>

#include "MissionScripts.hpp"

void ChangeHero(const char* hero);
void SpawnToPoint(size_t idx);
void FullHealth();
void UnlockAllUpgrades();
void KillHero();
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
void KillAllEntities();
void TeleportAllEntitiesToMe();
void TeleportToNearestEntity();
void TeleportAllPedestriansToMe();
void LoadCutscene(const char* instance);
void SpawnToNearestSpawnPoint();
 void LoadMissionScript(RHCheckpointScript* mission);