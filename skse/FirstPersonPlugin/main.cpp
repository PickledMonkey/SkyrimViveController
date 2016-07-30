#include "Windows.h"
#include "skse/PluginAPI.h"
#include "skse/skse_version.h"

// todo list
// dismounting is weird, this is because actor target changes (try in 3rd person, camera is switching from horse to player)
// up down sensitivity is lower than normal when standing still (771BB0)
// horse z offset is still wrong because wen you start moving it goes back
// hide head but still have shadow

#include "Camera.h"
#include "Config.h"
#include "Papyrus.h"

#define START_ASM1(b) void * jumpAddress = NULL; _asm { mov jumpAddress, offset b##CodeStart
#define START_ASM2(b) jmp b##CodeEnd
#define START_ASM3(b) b##CodeStart:

#define END_ASM(b, addr1, addr2) b##CodeEnd: \
} \
	WriteJump(addr1, addr2, (int)jumpAddress);

int fadeOut1 = 0x83D333;
int fadeOut2 = 0x83D339;
int fadeOut3 = 0x83D3A1;

// Position and rotation update.
// Horse, third person and dragon camera states are handled by this since they share the same function in VTable.
int updateCamA1 = 0x83FE3C;
int updateCamA2 = 0x83FE41;
int updateCamAF = 0x83F690;

// Furniture state.
int updateCamB1 = 0x83AEBC;
int updateCamB2 = 0x83AEC1;
int updateCamBF = 0x83CFB0;

// Bleedout state (paralysis).
int updateCamC1 = 0x838C37;
int updateCamC2 = 0x838C3E;

// VATS camera state.
int updateCamD1 = 0x8415C8;
int updateCamD2 = 0x8415D1;

// Transition state.
int updateCamT1 = 0x83F40A;
int updateCamT2 = 0x83F40F;
int updateCamTF = 0xAAF320;

int headTrack1 = 0x6CF3BB;
int headTrack2 = 0x6CF3C0;
int headTrack3 = 0x6CF3FD;

int headTrackB1 = 0x6CF3DF;
int headTrackB2 = 0x6CF3E5;

int doMU1 = 0x69CC39;
int doMU2 = 0x69CC3F;

int doGU1 = 0xB1798E;
int doGU2 = 0xB17996;
int doGU3 = 0xB179D4;

int horseTurn1 = 0x83B008;
int horseTurn2 = 0x83B012;

int dismount1 = 0x6C3243;
int dismount2 = 0x6C3248;

int mount1 = 0x6C2DEF;
int mount2 = 0x6C2DF5;

int faceCode1 = 0x8403A0;
int faceCode2 = 0x8403A6;
int allowFreeLook = 0;

int faceCode3 = 0x840330;
int faceCode4 = 0x840337;

int zoomIn1 = 0x840E29;
int zoomIn2 = 0x840E2E;
int zoomInF = 0x83CE90;

int zoomOut1 = 0x840E48;
int zoomOut2 = 0x840E4E;

int togglePov1 = 0x840CE7;
int togglePov2 = 0x840CEF;
int togglePov3 = 0x840D08;

int mountedTogglePov1 = 0x83B51A;
int mountedTogglePov2 = 0x83B520;

int mountedZoomIn1 = 0x840E10;
int mountedZoomIn2 = 0x840E15;
int mountedZoomIn3 = 0x6A6EB0;

int updateOSPos1 = 0x83F8B4;
int updateOSPos2 = 0x83F8BE;

int updateOSPos3 = 0x838F6A;
int updateOSPos4 = 0x838F70;

int updateOSPos5 = 0x838F4C;
int updateOSPos6 = 0x838F52;

int updateOSPos7 = 0x83F89C;
int updateOSPos8 = 0x83F8A2;

int updateOSPos9 = 0x83B60E;
int updateOSPos10 = 0x83B614;

int updateOSPos11 = 0x83B6A0;
int updateOSPos12 = 0x83B6A5;

int calcTargetPosA1 = 0x73DF59;
int calcTargetPosA2 = 0x73DF60;

int calcTargetPosB1 = 0x73E06D;
int calcTargetPosB2 = 0x73E072;

int updateMouseB1 = 0x771CEC;
int updateMouseB2 = 0x771CF1;

int blockControl1 = 0x772A90;
int blockControl2 = 0x772A96;
int blockControl3 = 0x772AFE;

int blockControlB1 = 0x772AD0;
int blockControlB2 = 0x772AD5;
int blockControlBF = 0x64C170;

int restrictAngleA1 = 0x6AE61F;
int restrictAngleA2 = 0x6AE624;

int restrictAngleB1 = 0x83C9C0;
int restrictAngleB2 = 0x83C9C5;

int restrictAngleC1 = 0x83CC25;
int restrictAngleC2 = 0x83CC2A;

int furniture1 = 0x83AB10;
int furniture2 = 0x83AB16;

int dismountFix1 = 0x83EDA7;
int dismountFix2 = 0x83EDAD;

int updateCross1 = 0x83BB60;
int updateCross2 = 0x83BB66;

int htSpeed1 = 0x5A8BD6;
int htSpeed2 = 0x5A8BDD;

int actorAnimScale1 = 0x64F84A;
int actorAnimScale2 = 0x64F854;

int headShadow1 = 0xB07EFF;
int headShadow2 = 0xB07F04;
int headShadow3 = 0xAAF430;
int headShadow4 = 0xAAF437;
int headShadow5 = 0xB07E07;
int headShadow6 = 0xB07E0C;
int headShadow7 = 0x69AB0C;
int headShadow8 = 0x69AB11;
int headShadow7F = 0xCB9E20;

int arrowCam1 = 0x4AB329;
int arrowCam2 = 0x4AB32F;

int ignoreCollision1 = 0x83D140;
int ignoreCollision2 = 0x83D147;

int geom_cull = 0;
int shad_cull = 0;
int geom_thing = 0;
int geom_thing2 = 0;

int _allowUpRes = 0;

bool WriteMemory(int address, char * data, int length)
{
	DWORD oldProtect = 0;

	if (VirtualProtect((void *)address, length, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		memcpy((void*)address, data, length);
		if (VirtualProtect((void *)address, length, oldProtect, &oldProtect))
			return true;
	}

	return false;
}

bool WriteJump(int addressFrom1, int addressFrom2, int addressTo)
{
	DWORD oldProtect = 0;

	int len1 = addressFrom2 - addressFrom1;
	if (VirtualProtect((void *)addressFrom1, len1, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*((unsigned char *)addressFrom1) = (unsigned char)0xE9;
		*((int *)(addressFrom1 + 1)) = (int)addressTo - addressFrom1 - 5;
		for (int i = 5; i < len1; i++)
			*((unsigned char *)(i + addressFrom1)) = (unsigned char)0x90;
		if (VirtualProtect((void *)addressFrom1, len1, oldProtect, &oldProtect))
			return true;
	}

	return false;
}

void _handleSKSEMessage(SKSEMessagingInterface::Message * msg)
{
	if (!msg)
		return;

	switch (msg->type)
	{
	case SKSEMessagingInterface::kMessage_NewGame:
	case SKSEMessagingInterface::kMessage_PostLoadGame:
		Config::ResetPapyrus();
		break;
	}
}

PluginHandle pluginHandle = kPluginHandle_Invalid;

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = IFPV_NAME;
		info->version = IFPV_VERSION_I;
		pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor || skse->runtimeVersion != RUNTIME_VERSION_1_9_32_0)
			return false;
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		// Load configuration files.
		Config::Initialize();

		// Hook headtrack.
		{
			START_ASM1(HU)
				START_ASM2(HU)
				START_ASM3(HU)
			
					call Camera::GetSingleton
						mov ecx, eax
						push esi
						call Camera::_IsHeadTrackOverwritten
						cmp eax, 0
						je HUCodeNormal
						jmp headTrack3
					HUCodeNormal:
					lea ecx, [esp+0x20]
						push ecx
					jmp headTrack2

					END_ASM(HU, headTrack1, headTrack2)
		}
		{
			START_ASM1(HI)
				START_ASM2(HI)
				START_ASM3(HI)

				call Camera::GetSingleton
					mov ecx, eax
					push esi
					call Camera::_IsHeadTrackOverwritten
					cmp eax, 0
					je HICodeNormal
					jmp headTrack3
				HICodeNormal :
				mov edx, [esi+0x130]
					jmp headTrackB2

					END_ASM(HI, headTrackB1, headTrackB2)
		}

		// Hook third person camera position update.
		{
			START_ASM1(TP)
				START_ASM2(TP)
				START_ASM3(TP)

				mov eax, [esp + 0x2C]
					pushad
					pushfd
					push eax
					lea eax, [esi + 0x20]
					push eax
					lea eax, [esi+0xB0]
					push eax
					lea eax, [esi+0xAC]
					push eax
					call Camera::GetSingleton
					mov ecx, eax
					call Camera::_UpdateRestrict
					call Camera::GetSingleton
					mov ecx, eax
					call Camera::_UpdatePosition
					call Camera::GetSingleton
					mov ecx, eax
					call Camera::_UpdateRotation
					popfd
					popad
					call updateCamAF
					jmp updateCamA2

					END_ASM(TP, updateCamA1, updateCamA2)
		}

		// Hook furniture camera position update (start sitting or getting up).
		{
			START_ASM1(FQ)
				START_ASM2(FQ)
				START_ASM3(FQ)

				pop eax
				pop eax

				mov eax, [esp+0x5C]
				pushad
				pushfd
				push eax
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdatePosition
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateRotation
				popfd
				popad
				push 1
				push esi
					call updateCamBF
					jmp updateCamB2

						END_ASM(FQ, updateCamB1, updateCamB2)
		}

		// Hook ragdoll camera update.
		{
			START_ASM1(FR)
				START_ASM2(FR)
				START_ASM3(FR)
				
				pushad
				pushfd
				lea eax, [ebp + 0xBC]
				push eax
				lea eax, [ebp+0x20]
				push eax
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdatePosition
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateRotation
				popfd
				popad

				mov edi, [esp + 0x10]
				add edi, 0x20

				jmp updateCamC2

				END_ASM(FR, updateCamC1, updateCamC2)
		}

		// Hook VATS camera update.
		/*{
			START_ASM1(FV)
				START_ASM2(FV)
				START_ASM3(FV)

				mov ecx, [esp+8]
				lea edx, [esp+0xC]
				push edx

				pushad
				pushfd
				lea eax, [ecx + 0x20]
				push eax
				lea eax, [ecx + 0x44]
				push eax
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdatePosition
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateRotation
				popfd
				popad

				jmp updateCamD2

				END_ASM(FV, updateCamD1, updateCamD2)
		}*/

		// Hook transition camera update.
		{
			START_ASM1(FT)
				START_ASM2(FT)
				START_ASM3(FT)

				pushad
				pushfd

				lea eax, [ecx + 0x20]
				push eax
				lea eax, [ecx +0x44]
				push eax
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdatePosition
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateRotation
				popfd
				popad

				call updateCamTF

				jmp updateCamT2

				END_ASM(FT, updateCamT1, updateCamT2)
		}

		// Hook dismount for checking bDismounting.
		{
			START_ASM1(DM)
				START_ASM2(DM)
				START_ASM3(DM)

				mov byte ptr[esp + 0x16], 1
					pushad
					pushfd
					push esi
					call Config::_dismount
					add esp, 4
					popfd
					popad
					jmp dismount2

					END_ASM(DM, dismount1, dismount2)
		}

		// Hook mount for checking bMounting.
		{
			START_ASM1(DN)
				START_ASM2(DN)
				START_ASM3(DN)

				mov [ebx+0x4AC], edx
					pushad
					pushfd
					push esi
					call Config::_mount
					add esp, 4
					popfd
					popad
					jmp mount2

					END_ASM(DN, mount1, mount2)
		}

		// Hook horse mounting turn angle.
		{
			START_ASM1(HT3)
				START_ASM2(HT3)
				START_ASM3(HT3)

				pushad
				pushfd

				call Camera::GetSingleton
				mov ecx, eax
				call Camera::ShouldTurnHorseCamera
				test eax, eax
				je HT3No
				popfd
				popad
				mov [esi + 0xC0], 0
				jmp horseTurn2

			HT3No:
			popfd
				popad
				jmp horseTurn2

				END_ASM(HT3, horseTurn1, horseTurn2)
		}

		// Hook character fade out when close.
		{
			START_ASM1(FO)
				START_ASM2(FO)
				START_ASM3(FO)

				pushad
					pushfd
					call Camera::GetSingleton
					mov ecx, eax
					call Camera::_CanFadeOut
					test eax, eax
					je FOCodeSkip
					popfd
					popad
					mov eax, [esp + 0x1C]
					test eax, eax
					jmp fadeOut2
				FOCodeSkip :
				popfd
					popad
					jmp fadeOut3

					END_ASM(FO, fadeOut1, fadeOut2)
		}

		// Hook character facing crosshair decision. This will allow to overwrite freelook in any way.
		{
			START_ASM1(FD)
				START_ASM2(FD)
				START_ASM3(FD)

				cmp bl, 0
					jne followWasZ
					mov allowFreeLook, 0
					jmp followWasE
				followWasZ :
				mov allowFreeLook, 1
				followWasE :
						   pushad
						   pushfd
						   call Camera::GetSingleton
						   mov ecx, eax
						   lea eax, allowFreeLook
						   push eax
						   call Camera::_UpdateFollow
						   popfd
						   popad
						   cmp allowFreeLook, CAMERA_LOOK_FORCE_FREE
						   jne followNext1
						   mov bl, 1
						   mov eax, 0
						   jmp followOriginal
					   followNext1 :
				cmp allowFreeLook, CAMERA_LOOK_FORCE_FIXED
					jne followNext2
					mov bl, 0
					jmp followOriginal
				followNext2 :
				cmp allowFreeLook, CAMERA_LOOK_FORCE_FIXED_TURN
					jne followOriginal
					mov bl, 0
					mov eax, 1
				followOriginal :
							   cmp[esi + 0xB4], bl
							   jmp faceCode2

							   END_ASM(FD, faceCode1, faceCode2)
		}

		// Hook character facing crosshair decision. This will allow to freelook while having weapons out but not when moving.
		{
			START_ASM1(FD2)
				START_ASM2(FD2)
				START_ASM3(FD2)

				push ecx
				push esi
				mov esi, ecx
				mov ecx, [esi+8]
				lea eax, [esp+0xC]

				pushad
				pushfd

				push eax
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateFollow2

				popfd
				popad
				
						   jmp faceCode4

						   END_ASM(FD2, faceCode3, faceCode4)
		}

		// Hook zoom in code so we can switch to IFPV instead.
		{
			START_ASM1(ZI)
				START_ASM2(ZI)
				START_ASM3(ZI)

				pushad
					pushfd
					call Camera::GetSingleton
					mov ecx, eax
					call Camera::_UpdateZoomIn
					cmp eax, 0
					je ZICodeNormal
					popfd
					popad
					jmp zoomIn2
				ZICodeNormal :
				popfd
					popad
					call zoomInF
					jmp zoomIn2

					END_ASM(ZI, zoomIn1, zoomIn2)
		}

		// Hook zoom out code so we can switch off IFPV.
		{
			START_ASM1(ZO)
				START_ASM2(ZO)
				START_ASM3(ZO)

				cmp ecx, [ebx+0xF4]
					jne ZOCodeOut
					jmp ZOCodeSkip
				ZOCodeOut:
							 pushad
							 pushfd
							 call Camera::GetSingleton
							 mov ecx, eax
							 call Camera::_UpdateZoomOut
				popfd
					popad
					ZOCodeSkip:
					cmp ecx, [ebx+0xF4]
					jmp zoomOut2

					END_ASM(ZO, zoomOut1, zoomOut2)
		}

		// Hook toggle code so we can switch IFPV.
		{
			START_ASM1(ZP)
				START_ASM2(ZP)
				START_ASM3(ZP)

				pushad
				pushfd
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateTogglePOV
				cmp eax, 0
				je ZPCodeDo
				popfd
				popad
				jmp togglePov3
			ZPCodeDo :
			popfd
				popad
				mov eax, [edi + 0x48]
				mov ecx, 0x12B2350
				jmp togglePov2

				END_ASM(ZP, togglePov1, togglePov2)
		}

		// Hook mounted toggle code so we can switch to IFPV but only if it's overwritten.
		{
			START_ASM1(MP)
				START_ASM2(MP)
				START_ASM3(MP)

				pushad
				pushfd
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateTogglePOV
				popfd
				popad
				mov [ebx + 0xA0], edx
				jmp mountedTogglePov2
			
				END_ASM(MP, mountedTogglePov1, mountedTogglePov2)
		}

		// Hook mounted zoom in code so we can switch to IFPV but only if it's overwritten.
		{
			START_ASM1(MI)
				START_ASM2(MI)
				START_ASM3(MI)

				call mountedZoomIn3
				test al, al
				je MICodeNormal

				pushad
				pushfd
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateZoomIn
			popfd
				popad
				MICodeNormal:
				jmp mountedZoomIn2

				END_ASM(MI, mountedZoomIn1, mountedZoomIn2)
		}

		// Hook update loop so we can do something every frame.
		{
			START_ASM1(MU)
				START_ASM2(MU)
				START_ASM3(MU)

				mov ecx, 0x12E35C8
					mov ecx, [ecx]
					pushad
					pushfd
					call Camera::GetSingleton
					mov ecx, eax
					call Camera::_UpdateFrame
					call Camera::GetSingleton
					mov ecx, eax
					lea eax, geom_thing
					push eax
					lea eax, geom_thing2
					push eax
					call Camera::_updateGeomSkip
					popfd
					popad
					jmp doMU2

					END_ASM(MU, doMU1, doMU2)
		}

		// Hook graphics update so we can change near clipping on the fly.
		{
			START_ASM1(GU)
				START_ASM2(GU)
				START_ASM3(GU)

				jne GUCodeYes
				pushad
				pushfd
				call Config::_ShouldUpdateGraphics
				cmp eax, 0
				jne GUCodePop
				popfd
				popad
				fld dword ptr[ebx + 0xC8]
				jmp GUCodeNormal
			GUCodeYes :
			jmp doGU3
			GUCodeNormal :
			jmp doGU2
			GUCodePop :
			popfd
				popad
				jmp doGU3

				END_ASM(GU, doGU1, doGU2)
		}

		// Hook over shoulder pos update for crosshair accuracy fix.
		{
			START_ASM1(OS1)
				START_ASM2(OS1)
				START_ASM3(OS1)

				mov [ecx+0x40], 0
				mov [ecx+0x44], eax

				pushad
				pushfd
				push ecx
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateOverShoulder
				popfd
				popad
				
				jmp updateOSPos2

				END_ASM(OS1, updateOSPos1, updateOSPos2)
		}
		{
			START_ASM1(OS2)
				START_ASM2(OS2)
				START_ASM3(OS2)

				mov[ecx + 0x40], eax
				mov[ecx + 0x3C], esi

				pushad
				pushfd
				push ecx
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateOverShoulder
				popfd
				popad

				jmp updateOSPos4

				END_ASM(OS2, updateOSPos3, updateOSPos4)
		}
		{
			START_ASM1(OS3)
				START_ASM2(OS3)
				START_ASM3(OS3)

				mov[ecx + 0x40], eax
				mov[ecx + 0x3C], esi

				pushad
				pushfd
				push ecx
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateOverShoulder
				popfd
				popad

				jmp updateOSPos6

				END_ASM(OS3, updateOSPos5, updateOSPos6)
		}
		{
			START_ASM1(OS4)
				START_ASM2(OS4)
				START_ASM3(OS4)

				mov[ecx + 0x40], eax
				mov[ecx + 0x44], edx

				pushad
				pushfd
				push ecx
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateOverShoulder
				popfd
				popad

				jmp updateOSPos8

				END_ASM(OS4, updateOSPos7, updateOSPos8)
		}
		{
			START_ASM1(OS5)
				START_ASM2(OS5)
				START_ASM3(OS5)

				mov[esi + 0x40], eax
				mov[esi + 0x44], ecx

				pushad
				pushfd
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateOverShoulder
				popfd
				popad

				jmp updateOSPos10

				END_ASM(OS5, updateOSPos9, updateOSPos10)
		}
		{
			START_ASM1(OS6)
				START_ASM2(OS6)
				START_ASM3(OS6)

				pushad
				pushfd
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateOverShoulder
				popfd
				popad

				pop esi
				retn 4

				END_ASM(OS6, updateOSPos11, updateOSPos12)
		}

		// Hook crosshair target pos calculation so we can save some info.
		{
			START_ASM1(CT1)
				START_ASM2(CT1)
				START_ASM3(CT1)

				lea eax, [esp+0x3C]

				pushad
				pushfd
				push eax
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_RememberTargetPos
				popfd
				popad

				mov eax, 0x12E2BD8
				test byte ptr [eax], 1

				jmp calcTargetPosA2

				END_ASM(CT1, calcTargetPosA1, calcTargetPosA2)
		}
		{
			START_ASM1(CT2)
				START_ASM2(CT2)
				START_ASM3(CT2)

				pop edi
				pop esi
				mov[eax], edx
				pop ebx

				pushad
				pushfd
				push edx
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_RememberTargetRef
				popfd
				popad

				jmp calcTargetPosB2

				END_ASM(CT2, calcTargetPosB1, calcTargetPosB2)
		}

		// Update mouse so we can know if we wanted to look around manually. Also we can overwrite rotate ourself here.
		{
			{
				START_ASM1(MS1)
				START_ASM2(MS1)
				START_ASM3(MS1)

				mov eax, 0x1B2E8E4
				mov eax, [eax]

				pushad
				pushfd
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_updateOverwriteLook
				popfd
				popad

				jmp updateMouseB2

				END_ASM(MS1, updateMouseB1, updateMouseB2)
			}
		}

		// Hook part where it disables control during VATS killmove.
		{
			START_ASM1(DC1)
				START_ASM2(DC1)
				START_ASM3(DC1)

				cmp [ecx+0x10], 4
				jne DC1NoDisable

				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_shouldAllowDuringVATS
				cmp eax, 0
				jne DC1NoDisable
				jmp blockControl3
				
			DC1NoDisable:
				jmp blockControl2

				END_ASM(DC1, blockControl1, blockControl2)
		}
		{
			START_ASM1(DC2)
				START_ASM2(DC2)
				START_ASM3(DC2)

				call blockControlBF
				test al, al
				je DC2Normal

				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_shouldAllowDuringVATS
				cmp eax, 0
				je DC2No
				mov eax, 0
				jmp DC2Normal

			DC2No :
			mov eax, 1

			DC2Normal :
					  jmp blockControlB2

					  END_ASM(DC2, blockControlB1, blockControlB2)
		}

		// Restrict up down look angle.
		{
			START_ASM1(RA1)
				START_ASM2(RA1)
				START_ASM3(RA1)

				push ecx

				cmp _allowUpRes, 0
				je RA1Skip

				lea eax, [esp + 0xC]
				lea edx, [esp + 0x8]

				pushad
				pushfd
				push edx
				push eax
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_UpdateRestrict2
				popfd
				popad

				RA1Skip:
				fld[esp + 0xC]

				jmp restrictAngleA2

				END_ASM(RA1, restrictAngleA1, restrictAngleA2)
		}

		// Hook if we allow changing up down stuff.
		{
			START_ASM1(RA2)
				START_ASM2(RA2)
				START_ASM3(RA2)

				mov _allowUpRes, 1

				sub esp, 0xC
				push esi
				push edi
				
				jmp restrictAngleB2

				END_ASM(RA2, restrictAngleB1, restrictAngleB2)
		}
		{
			START_ASM1(RA3)
				START_ASM2(RA3)
				START_ASM3(RA3)

				mov _allowUpRes, 0

				pop edi
				pop esi
				add esp, 0xC

				jmp restrictAngleC2

				END_ASM(RA3, restrictAngleC1, restrictAngleC2)
		}
		{
			START_ASM1(CF3)
				START_ASM2(CF3)
				START_ASM3(CF3)

				mov eax, [esp+4]
				pushad
				pushfd
				push eax
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::_updateCrosshair
				test eax, eax
				je CF3Do
				popfd
				popad
				retn 8

			CF3Do:
			popfd
				popad
				mov ecx, [ecx+0x90]
				jmp updateCross2

				END_ASM(CF3, updateCross1, updateCross2)
		}
		static int _hsFunc = 0xAAF430;
		static int _hs2Func = 0xACE100;
		{
			START_ASM1(HS8)
				START_ASM2(HS8)
				START_ASM3(HS8)

				inc geom_cull
				call _hsFunc
				dec geom_cull
				jmp headShadow2

				END_ASM(HS8, headShadow1, headShadow2)
		}
		{
			START_ASM1(HS7)
				START_ASM2(HS7)
				START_ASM3(HS7)

				inc geom_cull
				call _hs2Func
				dec geom_cull
				jmp headShadow6

				END_ASM(HS7, headShadow5, headShadow6)
		}
		{
			START_ASM1(HS10)
				START_ASM2(HS10)
				START_ASM3(HS10)

				inc shad_cull
				call headShadow7F
				dec shad_cull
				jmp headShadow8

				END_ASM(HS10, headShadow7, headShadow8)
		}
		{
			START_ASM1(HS9)
				START_ASM2(HS9)
				START_ASM3(HS9)

				cmp geom_cull, 0
				je HS9NormalBef
				cmp shad_cull, 0
				jne HS9NormalBef
				cmp geom_thing, ecx
				je HS9DoSkip
				cmp geom_thing2, ecx
				je HS9DoSkip
				jmp HS9NormalBef

				HS9DoSkip:
				retn 4

			HS9NormalBef:
			test byte ptr[ecx + 0x98], 1
				jmp headShadow4

				END_ASM(HS9, headShadow3, headShadow4)
		}
		{
			START_ASM1(AC1)
				START_ASM2(AC1)
				START_ASM3(AC1)

				mov ecx, [esi + 0xFC]
				pushad
				pushfd
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::SetLastArrow
				popfd
				popad
				jmp arrowCam2

				END_ASM(AC1, arrowCam1, arrowCam2)
		}
		{
			START_ASM1(IC1)
				START_ASM2(IC1)
				START_ASM3(IC1)

				pushad
				pushfd

				call Camera::GetSingleton
				mov ecx, eax
				call Camera::ShouldIgnoreWorldCollision
				cmp eax, 0
				je IC1Normal
				jg IC1Skip

				popfd
				popad
				xor eax, eax
				cmp eax, 1
				jmp ignoreCollision2

			IC1Skip :
			popfd
				popad
				xor eax, eax
				test eax, eax
				jmp ignoreCollision2

			IC1Normal :
			popfd
				popad
				cmp[esi + 0xA0], 0
				jmp ignoreCollision2

				END_ASM(IC1, ignoreCollision1, ignoreCollision2)
		}

#ifdef CAMERA_INCLUDE_FARCLIP_CONFIG

		// Get far clip.
		{
			static int orig_ja = 0x69F2F0;
			static int temp_v = 0;
			START_ASM1(NF2)
				START_ASM2(NF2)
				START_ASM3(NF2)

				push esi
				mov esi, ecx
				call orig_ja
				fstp temp_v
				lea eax, temp_v
				push eax
				push esi
				call Camera::GetSingleton
				mov ecx, eax
				call Camera::UpdateFarClip
				pop esi
				fld temp_v
				retn

				NF2CodeEnd:
			};

		int ja = (int)jumpAddress;
		WriteMemory(0x10CF13C, (char*)&ja, 4);
		}

#endif

		// Write new camera up down restriction.
		{
			static int _restAddr[] = {
				0x8404F4 + 2, // ThirdPersonState::MoveCameraFromMouse
				//0x83FF56 + 2, // ThirdPersonState::unk_83FE70 <- update over shoulder and zoom ?

				0
			};

			int idxx = 0;
			int addrx = (int)((int*)&NewUpDownLimit);
			while (_restAddr[idxx] != 0)
			{
				WriteMemory(_restAddr[idxx], (char*)&addrx, 4);
				idxx++;
			}
		}

		// Register papyrus commands.
		{
			SKSEPapyrusInterface * papyrusInterface = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);
			if (papyrusInterface)
				papyrusInterface->Register(IFPVPapyrus::_registerFunctions);
		};

		// Register messaging.
		{
			SKSEMessagingInterface * messageInterface = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);
			if (messageInterface)
			{
				void * dispatcher = messageInterface->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ModEvent);
				if (dispatcher != NULL)
					Config::SetModEventDispatcher(dispatcher);
				messageInterface->RegisterListener(pluginHandle, "SKSE", _handleSKSEMessage);
			}
		};

		return true;
	}
};
