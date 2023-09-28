#include "includes.h"
#include "pch.h"
#include "SDK/DonkehFramework_Classes.h"
#include <chrono>
#include <math.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
CG::UWorld* World;
CG::UGameInstance* OwningGameInstance;

bool ShowMenu = false;
bool bNoRecoil = false;
bool bEnableSpeed = false;
float fSpeedMultiplier = 1.0f;
bool bRapidfire = false;
bool bAimbot = false;
int FovSize = false;
bool bCrosshair = false;
bool bFov = false;
static ImColor FovColor = {0, 0 , 0 , 255};
static ImColor CrosshairColor = { 0, 0 , 0 , 255 };
static ImColor EnemyColor = { 255, 0 , 0 , 255 };
static ImColor TeamColor = { 0, 0 , 255 , 255 };
int MiddleY;
int MiddleX;
float fRapidFire = 1.0f;
bool bPlayerFov = false;
int PlayerFov = 100;
bool bInfiniteAmmo = false;
bool bPlayerFlight = false;
bool bFreecam = false;
bool bEnableEsp = false;
bool bEnemyPlayerEsp = false;
bool bTeamPlayerEsp = false;
bool bPlayerWeaponEsp = false;
bool bDistanceEsp = false; 
bool bLineToPlayer = false;
bool bEnableWarnings = false;
bool bFeatureList = false;

void GetNearestFovClient(CG::FVector* AimbotTargetLocation, CG::FVector2D* AimbotLine);

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
	int MiddleY = ImGui::GetIO().DisplaySize.y / 2;
	int MiddleX = ImGui::GetIO().DisplaySize.x / 2;
}


const char* va(const char* fmt, ...)
{
	static char buffer[1024]{ 0 };

	va_list args;
	va_start(args, fmt);

	std::vsnprintf(buffer, sizeof(buffer), fmt, args);

	va_end(args);

	return buffer;
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_HOME) & 1)
	{
		ShowMenu = !ShowMenu;
	}


	if (CG::UWorld::GWorld)
	{
		World = *CG::UWorld::GWorld;
		OwningGameInstance = World->OwningGameInstance;
	}


	if (ShowMenu)
	{
		if (ImGui::Begin("Powered Client v1.0 | Poweredxd | Home - Open/Close | End - Unhook"))
		{


			if (ImGui::CollapsingHeader("Aimbot"))
			{
				ImGui::Text("Aimbot");
				ImGui::Checkbox("Enable Aimbot", &bAimbot); ImGui::SameLine(); ImGui::Checkbox("Enable Fov", &bFov);
				ImGui::Checkbox("Enable Crosshair", &bCrosshair); 
				ImGui::SliderInt("Fov Size", &FovSize, 0, 300);
				ImGui::ColorEdit4("Fov Color", (float*)&FovColor);
				ImGui::ColorEdit4("Crosshair Color", (float*)&CrosshairColor);

			}

			if (ImGui::CollapsingHeader("Esp"))
			{
				ImGui::Text("Esp");
				ImGui::Checkbox("Enable Esp", &bEnableEsp); ImGui::SameLine(); ImGui::Checkbox("Enemey Player Esp", &bEnemyPlayerEsp);
				ImGui::Checkbox("Team Player Esp", &bTeamPlayerEsp); ImGui::SameLine();ImGui::Checkbox("Player Weapon Esp", &bPlayerWeaponEsp);
				ImGui::Checkbox("Distance Esp", &bDistanceEsp); ImGui::SameLine(); ImGui::Checkbox("Line to Player", &bLineToPlayer);
			}

			if (ImGui::CollapsingHeader("Esp Colors"))
			{
				ImGui::Text("Esp Colors");
				ImGui::ColorEdit4("Enemy Color", (float*)&EnemyColor);
				ImGui::ColorEdit4("Team Color", (float*)&TeamColor);

			}

			if (ImGui::CollapsingHeader("Misc Features"))
			{
				ImGui::Text("Misc Features");

				ImGui::Checkbox("No Recoil", &bNoRecoil); ImGui::SameLine(); ImGui::Checkbox("Rapidfire", &bRapidfire); ImGui::SameLine(); ImGui::Checkbox("Infinite Ammo", &bInfiniteAmmo);
				ImGui::Checkbox("Enable Flight", &bPlayerFlight); ImGui::SameLine(); ImGui::Checkbox("Freecam", &bFreecam); ImGui::SameLine(); ImGui::Checkbox("Enable Feature List", &bFeatureList);
				ImGui::Checkbox("Enable Warnings", &bEnableWarnings); ImGui::SameLine(); ImGui::Checkbox("Player Fov", &bPlayerFov); ImGui::SameLine(); ImGui::Checkbox("Speed", &bEnableSpeed);
				ImGui::SliderFloat("Speed Multiplier", &fSpeedMultiplier, 1.0f, 10.0f);
				ImGui::SliderFloat("RapidFire Multiplier", &fRapidFire, 0.0f, 1.0f);
				ImGui::SliderInt("Player Fov", &PlayerFov, 0, 150);
			}
		}
		ImGui::End();
	}

	if (bFov)
	{
		ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), FovSize, FovColor, FovSize / 4 , 2.0f);
	}

	if (bCrosshair)
	{
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(ImGui::GetIO().DisplaySize.x/2 + 10, ImGui::GetIO().DisplaySize.y/2 + 1), ImVec2(ImGui::GetIO().DisplaySize.x/2 - 10, ImGui::GetIO().DisplaySize.y/2 - 1), CrosshairColor);
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(ImGui::GetIO().DisplaySize.x/2 + 1, ImGui::GetIO().DisplaySize.y/2 + 10), ImVec2(ImGui::GetIO().DisplaySize.x/2 - 1, ImGui::GetIO().DisplaySize.y/2 - 10), CrosshairColor);

	}

	if (bNoRecoil)
	{
		if (!World) return 0;
		if (!OwningGameInstance) return 0;

		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);
		auto PlayerWeapon = reinterpret_cast<CG::AHDBaseWeapon*>(SelfPlayer->EquippedItem);
		if (PlayerWeapon)
		{
			PlayerWeapon->bNoRecoil = true;
		}
	}

	if (bEnableSpeed)
	{
		if (!World) return 0;
		if (!OwningGameInstance) return 0;

		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);

		SelfPlayer->CustomTimeDilation = fSpeedMultiplier;
	}
	
	if (bRapidfire)
	{
		if (!World) return 0;
		if (!OwningGameInstance) return 0;

		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);
		auto PlayerWeapon = reinterpret_cast<CG::AHDBaseWeapon*>(SelfPlayer->EquippedItem);

		PlayerWeapon->FireRate = fRapidFire;
	}

	if (bInfiniteAmmo)
	{
		if (!World) return oPresent(pSwapChain, SyncInterval, Flags);
		if (!OwningGameInstance) return oPresent(pSwapChain, SyncInterval, Flags);

		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);
		auto PlayerWeapon = reinterpret_cast<CG::AHDBaseWeapon*>(SelfPlayer->EquippedItem);

		PlayerWeapon->bUsesAmmo = false;
	}

	if (bPlayerFov)
	{
		if (!World) return oPresent(pSwapChain, SyncInterval, Flags);
		if (!OwningGameInstance) return  oPresent(pSwapChain, SyncInterval, Flags);

		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);
	
		LocalPlayer->PlayerController->FOV(PlayerFov);
	}

	if (bPlayerFlight)
	{

		if (!World) return oPresent(pSwapChain, SyncInterval, Flags);
		if (!OwningGameInstance) return oPresent(pSwapChain, SyncInterval, Flags);

		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);

		SelfPlayer->SetReplicateMovement(false);
		SelfPlayer->CharacterMovement->MovementMode = CG::EMovementMode::MOVE_Flying;
		SelfPlayer->bActorEnableCollision = false;
	}
	
	if (bEnableEsp)
	{
		if (!World) return oPresent(pSwapChain, SyncInterval, Flags);
		if (!World->PersistentLevel) return oPresent(pSwapChain, SyncInterval, Flags);
		CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
		CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);

		for (int i = 0; i < World->PersistentLevel->Actors.Count(); i++)
		{
			auto Actor = World->PersistentLevel->Actors[i];
			auto Player = static_cast<CG::AHDPlayerCharacter*>(Actor);

			if (!Actor) continue;
			if (!Actor->RootComponent) continue;
			if (!Actor->IsA(CG::AHDPlayerCharacter::StaticClass())) continue;
			if (!Player->PlayerState) continue;

			auto Name = Player->PlayerState->GetPlayerName().ToString();
			auto Health = Player->Health;
			auto Weapon = Player->EquippedItem->GetName();
			CG::FVector2D PlayerLocation;

			if (!OwningGameInstance->LocalPlayers[0]->PlayerController->ProjectWorldLocationToScreen(Actor->K2_GetActorLocation(), &PlayerLocation, true)) continue;

			if (bEnemyPlayerEsp && Player->TeamNum != SelfPlayer->TeamNum)
			{
				ImGui::GetBackgroundDrawList()->AddText(ImVec2(PlayerLocation.X, PlayerLocation.Y), EnemyColor, Name.data());
				ImGui::GetBackgroundDrawList()->AddText(ImVec2(PlayerLocation.X, PlayerLocation.Y + 20), EnemyColor, va("Health: %0.f", Player->Health));
			}
		
			if (bTeamPlayerEsp && Player->TeamNum == SelfPlayer->TeamNum)
			{
				ImGui::GetBackgroundDrawList()->AddText(ImVec2(PlayerLocation.X, PlayerLocation.Y), TeamColor, Name.data());
				ImGui::GetBackgroundDrawList()->AddText(ImVec2(PlayerLocation.X, PlayerLocation.Y + 20), TeamColor, va("Health: %0.f", Player->Health));
			}
			
			if (bLineToPlayer) 
			{
				if (bEnemyPlayerEsp && Player->TeamNum != SelfPlayer->TeamNum)
				{
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(PlayerLocation.X, PlayerLocation.Y), ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y), EnemyColor);
				}

				//if (bTeamPlayerEsp && Player->TeamNum == SelfPlayer->TeamNum)
				//{
				//	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(PlayerLocation.X, PlayerLocation.Y), ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y), TeamColor);
				//}
			}

			if (bPlayerWeaponEsp)
			{
				if (bEnemyPlayerEsp && Player->TeamNum != SelfPlayer->TeamNum)
				{
					ImGui::GetBackgroundDrawList()->AddText(ImVec2(PlayerLocation.X, PlayerLocation.Y + 40), EnemyColor, Weapon.data());
				}

				if (bTeamPlayerEsp && Player->TeamNum == SelfPlayer->TeamNum)
				{
					ImGui::GetBackgroundDrawList()->AddText(ImVec2(PlayerLocation.X, PlayerLocation.Y + 40), TeamColor, Weapon.data());
				}
			}
		}

		
	}

	if (bAimbot)
	{
		OwningGameInstance = World->OwningGameInstance;
	
		CG::FVector AimbotTargetLocation;
		CG::FVector2D AimbotLine;
		//if (!bFov) return oPresent(pSwapChain, SyncInterval, Flags);
		GetNearestFovClient(&AimbotTargetLocation, &AimbotLine);

		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(AimbotLine.X, AimbotLine.Y), ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), EnemyColor);

		if (GetAsyncKeyState(VK_NUMPAD7))
		{
			
			auto math = (CG::UKismetMathLibrary*)CG::UKismetMathLibrary::StaticClass();
		
			auto rotation = math->STATIC_FindLookAtRotation(OwningGameInstance->LocalPlayers[0]->PlayerController->PlayerCameraManager->CameraCachePrivate.POV.Location, AimbotTargetLocation);
				
			//rotation.Yaw = 0;

			OwningGameInstance->LocalPlayers[0]->PlayerController->SetControlRotation(rotation);
		}
	}

	if (bFeatureList) 
	{

	}

	if (bEnableWarnings) 
	{
		auto ActorCount = World->PersistentLevel->Actors.Count();
		std::string ActorCountt = std::to_string(ActorCount);

		//ImGui::GetBackgroundDrawList()->AddText(ImVec2(ImGui::GetIO().DisplaySize.x / 2, 0), TeamColor, ActorCountt);


	}

	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		CG::InitSdk();
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}


void GetNearestFovClient(CG::FVector* AimbotTargetLocation, CG::FVector2D* AimbotLine)
{
	World = *CG::UWorld::GWorld;
	OwningGameInstance = World->OwningGameInstance;
	CG::ULocalPlayer* LocalPlayer = OwningGameInstance->LocalPlayers[0];
	CG::AHDPlayerCharacter* SelfPlayer = static_cast<CG::AHDPlayerCharacter*>(LocalPlayer->PlayerController->AcknowledgedPawn);

	float currentDistance = FLT_MAX;

	


	for (int x = 0; x < World->PersistentLevel->Actors.Count(); x++)
	{
		auto Actor = World->PersistentLevel->Actors[x];
		auto Player = static_cast<CG::AHDPlayerCharacter*>(Actor);
		if (!Actor) continue;

		if (!Actor->IsA(CG::AHDPlayerCharacter::StaticClass())) continue;
	 
		if (!Player->IsAlive()) continue;

		if (Player->TeamNum == SelfPlayer->TeamNum) continue;

		CG::FVector2D ActorLocation;
		if (!OwningGameInstance->LocalPlayers[0]->PlayerController->ProjectWorldLocationToScreen(Actor->K2_GetActorLocation(), &ActorLocation, true)) continue;

		auto DistanceSquared = (ActorLocation.X - (ImGui::GetIO().DisplaySize.x / 2)) * (ActorLocation.X - (ImGui::GetIO().DisplaySize.x / 2)) + (ActorLocation.Y - (ImGui::GetIO().DisplaySize.y / 2)) * (ActorLocation.Y - (ImGui::GetIO().DisplaySize.y / 2));
		auto Distance = std::sqrt(DistanceSquared);

		if (Distance < currentDistance)
		{
			currentDistance = Distance;
			
			auto Location = Actor->K2_GetActorLocation();
			auto ActorVelocity = Actor->GetVelocity();
			auto SelfVelocity = SelfPlayer->GetVelocity();


			AimbotTargetLocation->X = Location.X + ActorVelocity.X / 4 - SelfVelocity.X / 4;
			AimbotTargetLocation->Y = Location.Y + ActorVelocity.Y / 4 - SelfVelocity.Y / 4;
			AimbotTargetLocation->Z = Location.Z + ActorVelocity.Z / 4 - SelfVelocity.Z / 4;

			AimbotLine->X = ActorLocation.X;
			AimbotLine->Y = ActorLocation.Y;

		}
	}

}
