
#include <windows.h>
#include <tlhelp32.h>
#include "resource.h"

/*
This trainer is based on a generic trainer template for external memory overwrite type cheats
I have no idea where I got it from years ago otherwise I would credit the person in this comment

All addresses obtained through hours of reverse code engineering with CheatEngine
*/

BOOL GameRunning;

/** Start of Declarations here **/
BOOL GetProcessList( );
DWORD GetModuleBaseBaseAddress(DWORD iProcId, char* DLLName); 
// Below is the about text that is shown when "About" button is clicked

char *about   =
"This a simple external trainer for Battlefield 2142\n"
"which I wrote sometime in 2011.  It is a simple cheat that\n"
"operates by overwriting code and variables in the BF2142 \n"
"proccess memory space.  No anti-cheat countermeasures are taken\n"
"in this implementation.  For use offline and in anti-cheat\n"
"insecure servers only. \n"
"NOTE: NumLock must be on to activate toggles\n";
/////////////////////////////////////////////////////////////////////

char *gameWindow = "BF2142.exe"; // exe name here
DWORD pid; HWND hwndWindow; DWORD bytes; HANDLE hand = NULL;
DWORD RendDxBase;  //Base address for RendDx9 dll is dynamic and must be assigned from a memory scan

DWORD dwBF2142Base = 0x00400000;  // Base address for BattleField 2142's executable code is always the same (at least as up to patch 1.5)

DWORD bf2142_Minimapjump_offset0 = 0x003816F3;  // the conditional jumps 
DWORD bf2142_Minimapjump_offset1 = 0x00381833;  // at these address offsets
DWORD bf2142_Minimapjump_offset2 = 0x00392CCF;  // control what appears on mimimap and map overview

DWORD bf2142_Nametagsjump_offset0 = 0x000F1068;  // change jump so that game uncondtionally treats enemy name tags the same as friendly

DWORD bf2142_NametagsMaxDistance_offset = 0x001F6768;

DWORD bf2142_3Dmapjump_offset0 = 0x000F2C71;  // the conditinal jumps  at these address offsets control what information is drawn on 3D map
//DWORD bf2142_3Dmapjump_offset1 = 0x000F0952;  // NOPing this jump enables 3d map icons for both teams
DWORD bf2142_3Dmap_maxDistance_offset = 0x001F67AC;

DWORD bf2142_developerconsolejump_offset0 = 0x002733E0; // fliping the conditional jumps at these addresses
DWORD bf2142_developerconsolejump_offset1 = 0x002733F8; // allows one to run console commands reserved for developers only

DWORD bf2142_renderer_object_ptr_base_offset = 0x001F8E58; // address contained at this address (offset from RendDxBase) is the base of the game's renderer object
DWORD renderer_object_base_ptr = 0x00000000;   // defined dynamically

DWORD drawUnderGrowth_offset = 0x000006A6;  // these renderer variables are usually reserved to be modifed only in developer mode
DWORD drawParticles_offset = 0x00000695;  
DWORD drawSunflare_offset = 0x000006A3;
DWORD drawSkydome_offset = 0x000006A2;
DWORD drawPostProduction_offset = 0x000006A5;

DWORD bf2142_level_fog_params_ptr_base_offset = 0x001FBB2C;
DWORD bf2142_level_fog_params_base_ptr = 0x00000000; // defined dynamically


DWORD fogparam0_offset = 0x0000002C;
DWORD fogparam1_offset = 0x00000030;
DWORD fogparam2_offset = 0x00000034;
DWORD fogparam3_offset = 0x00000038;

DWORD bf2142_viewdistance_scalar_ptr = 0x00000000;  // defined dynamically
DWORD bf2142_viewdistance_scalar_ptr_base_offset = 0x0062020C;
DWORD bf2142_viewdistance_scalar_address_offset = 0x000004F3;


HANDLE pFile; //Used for logging address to file (not implimented in this build)

//below you will list the BOOLs for function toggles
BOOL IsHack1On, IsHack2On, IsHack3On, IsHack4On, IsHack5On, IsHack6On, IsHack7On, FirstTime1;


    BYTE MiniMapInf[2] = {0x74, 0x09};	
	BYTE MiniMapVeh[2] = {0x74, 0x09};
	BYTE MiniMapLW[2] = {0x74, 0x06};

	BYTE NameTags[6] = { 0xE9, 0x49, 0x01, 0x00, 0x00, 0x90 };

	BYTE Meter3Dmap[2] = { 0x75, 0x6A };

	BYTE defaultNametagMaxDistance[4] = { 0x00, 0x00, 0x00, 0x00 };
	BYTE defaultMapMaxDistance[4] = { 0x00, 0x00, 0x00, 0x00 };

	BYTE DevConsole0[2] = { 0xEB, 0x36 };
	BYTE DevConsole1[2] = { 0xEB, 0x1E };

	//BYTE NOPS[2] = { 0x90, 0x90 }; // 2 byte no ops

	BYTE renderer_object_base_address[4] = { 0 };
	BYTE bf2142_viewdistance_scalar_address[4] = { 0 };
	BYTE bf2142_level_fog_params_address[4] = { 0 };

	
	BYTE bf2142_viewdistance_scalar_high[1] = { 0x40 };  // higher byte values will work but can cause the game crash with an out of memory error (too much is drawn on the screen with insanely increased viewdistance)
	BYTE bf2142_viewdistance_scalar_off[1] = { 0x3F };   // this is the normal value

	BYTE zero_byte[1] = { 0x00 };
	BYTE one_byte[1] = { 0x01 };

	BYTE zero_float[4] = { 0x00, 0x00, 0x00, 0x00 };  // 0.0f
	BYTE one_float[4] = { 0x00, 0x00, 0x80, 0x3f };   // 1.0f
	BYTE tenK_float[4] = { 0x00, 0x40, 0x1C, 0x46 };  // 10000.0f


	BYTE original_fog_param0[4] = { 0 };
	BYTE original_fog_param1[4] = { 0 };
	BYTE original_fog_param2[4] = { 0 };
	BYTE original_fog_param3[4] = { 0 };

	BYTE original_minimap_code0[2] = {0}; 
	BYTE original_minimap_code1[2] = {0};
	BYTE original_minimap_code2[2] = {0};

	BYTE original_3Dmap_code0[2] = { 0 };
	//BYTE original_3Dmap_code1[2] = { 0 };

	BYTE original_nametag_code0[6] =  {0};

	BYTE original_console_code0[2] =  {0};
	BYTE original_console_code1[2] =  {0};
	

	BYTE unknown_float[4] = { 0x00, 0x00, 0x00, 0x00 };  
	DWORD float_test = 0x00000000;
///////////////////////////////////////////////////////

DWORD buildDwordFromByteArray(byte *a)
	{
		return (a[0]) | (a[1] << 8) | (a[2] << 16) | (a[3] << 24);
	}


void aboutButton(HWND hwnd)
{
	MessageBox(hwnd,about,"About",MB_ICONINFORMATION);
}

void Initialize(HWND hwnd,WPARAM wParam, LPARAM lParam) {
	GetProcessList();

	FirstTime1=TRUE; //This is the true / false flag for "is this the first time the trainers read the game code

	IsHack1On=FALSE; 
	IsHack2On=FALSE;
	IsHack3On=FALSE;
	IsHack4On=FALSE;
	IsHack5On=FALSE;
	IsHack6On=FALSE;
	IsHack7On=FALSE;


	if(GameRunning==TRUE)
	{		 
         GetWindowThreadProcessId(hwndWindow, &pid);
		 hand = OpenProcess(PROCESS_ALL_ACCESS,0,pid);
		 SetTimer(hwnd, 1, 500, NULL);	//Timer speed is 500ms, you can change it here
	} 
	else 
	{ //Error message for when game not found in process list
		MessageBox(NULL, "Warning! BF2142 not detected, please run the game before running the trainer", "Error", MB_OK + MB_ICONWARNING);
	}
}

void HookExe() //This function ensures we are attatched to the game at all times
{
	
	CloseHandle(hand);
    GetProcessList( );
    GetWindowThreadProcessId(hwndWindow, &pid);
	hand = OpenProcess(PROCESS_ALL_ACCESS,0,pid);

}



void timerCall() //functions in here run according to timer above
{
		HookExe(); //Call to function above (game always attatched)


/////////////////////////////////////////////////////////////////////////
/////ReadProcMem arrays are used to read and store original code so we 
/////toggle the code on and off


	if(FirstTime1==TRUE) //checks to see if this is the first time its run, if it is continue
	{
		RendDxBase = GetModuleBaseBaseAddress(pid,"RendDX9.dll");
		ReadProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset0) , &original_minimap_code0, 2, &bytes);
		ReadProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset1) , &original_minimap_code1, 2, &bytes);
		ReadProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset2) , &original_minimap_code2, 2, &bytes);

		ReadProcessMemory(hand, (void*)(RendDxBase + bf2142_Nametagsjump_offset0), &original_nametag_code0,6, &bytes);

		ReadProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmapjump_offset0), &original_3Dmap_code0, 2, &bytes);
		//ReadProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmapjump_offset1), &original_3Dmap_code1, 2, &bytes);

		ReadProcessMemory(hand, (void*)(RendDxBase + bf2142_NametagsMaxDistance_offset), &defaultNametagMaxDistance, 4, &bytes);

		ReadProcessMemory(hand, (void*)(dwBF2142Base + bf2142_viewdistance_scalar_ptr_base_offset), &bf2142_viewdistance_scalar_address, 4, &bytes);
		bf2142_viewdistance_scalar_ptr = buildDwordFromByteArray(bf2142_viewdistance_scalar_address);

		ReadProcessMemory(hand, (void*)(RendDxBase + bf2142_renderer_object_ptr_base_offset), &renderer_object_base_address, 4, &bytes);
		renderer_object_base_ptr = buildDwordFromByteArray(renderer_object_base_address);

		ReadProcessMemory(hand, (void*)(RendDxBase + bf2142_level_fog_params_ptr_base_offset), &bf2142_level_fog_params_address, 4, &bytes);
		bf2142_level_fog_params_base_ptr = buildDwordFromByteArray(bf2142_level_fog_params_address);

		ReadProcessMemory(hand, (void*)(dwBF2142Base + bf2142_developerconsolejump_offset0), &original_console_code0, 2, &bytes);
		ReadProcessMemory(hand, (void*)(dwBF2142Base + bf2142_developerconsolejump_offset1), &original_console_code1, 2, &bytes);


		FirstTime1=FALSE;
	}
	

///////////////////////////////////////////////////////////////////////////
/////Start Hotkey Functions Below

	if(GetAsyncKeyState(VK_NUMPAD1)) // User Pressed the NumPad1 to switch on code
	{			
		   
		if(IsHack1On==FALSE) //if this hack is not on do this........
			{  
				WriteProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset0) , &MiniMapInf, 2, &bytes);
				WriteProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset1) , &MiniMapVeh, 2, &bytes);
				WriteProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset2) , &MiniMapLW, 2, &bytes);			
				IsHack1On=TRUE; //Sets our "Is On" flag to "on"
			}
			else // .... do this
			{
				
				WriteProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset0), &original_minimap_code0,2, &bytes); // Write the original code into memory
				WriteProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset1), &original_minimap_code1,2, &bytes);
				WriteProcessMemory(hand, (void*) (dwBF2142Base + bf2142_Minimapjump_offset2), &original_minimap_code2,2, &bytes);
				IsHack1On=FALSE; //Sets our "Is On" flag to "off"
			}
		}
	//The function above will toggle between hack on and hack off status. For a list of virtual keys please visit:
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/WindowsUserInterface/UserInput/VirtualKeyCodes.asp
     		
	if(GetAsyncKeyState(VK_NUMPAD2)) 
	{			
		   
		if(IsHack2On==FALSE) 
			{  
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_Nametagsjump_offset0), &NameTags, 6, &bytes);
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_NametagsMaxDistance_offset), &tenK_float, 4, &bytes);
				IsHack2On=TRUE; 
			}
			else 
			{
				
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_Nametagsjump_offset0), &original_nametag_code0,5, &bytes); // Write the original code into memory
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_NametagsMaxDistance_offset), &defaultNametagMaxDistance, 4, &bytes);
				IsHack2On=FALSE; 
			}
	}

	if(GetAsyncKeyState(VK_NUMPAD3))
	{			
		   
		if(IsHack3On==FALSE) 
			{  
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmapjump_offset0), &Meter3Dmap, 2, &bytes);
				//WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmapjump_offset1), &NOPS, 2, &bytes);
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmap_maxDistance_offset), &tenK_float, 4, &bytes);
				IsHack3On=TRUE; 
			}
			else
			{
				
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmapjump_offset0), &original_3Dmap_code0,2, &bytes); // Write the original code into memory
				//WriteProcessMemory(hand, (void*)(RendDxBase + bf2_3Dmapjump_offset1), &original_3Dmap_code1,2, &bytes);
				WriteProcessMemory(hand, (void*)(RendDxBase + bf2142_3Dmap_maxDistance_offset), &defaultMapMaxDistance, 4, &bytes);
				IsHack3On=FALSE; 
			}
	}
    
    if(GetAsyncKeyState(VK_NUMPAD4)) 
	{
		if (IsHack4On == FALSE)
		{
			WriteProcessMemory(hand, (void*)(bf2142_viewdistance_scalar_ptr + bf2142_viewdistance_scalar_address_offset), &bf2142_viewdistance_scalar_high, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawUnderGrowth_offset), &zero_byte, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawParticles_offset), &zero_byte, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawSunflare_offset), &zero_byte, 1, &bytes);
			//WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawSkydome_offset), &zero_byte, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawPostProduction_offset), &zero_byte, 1, &bytes);

			ReadProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam3_offset), &unknown_float, 4, &bytes);
			float_test = buildDwordFromByteArray(unknown_float);
			if (*(float*)&float_test < 1.0f)  // last fog parameter is never naturally 1 or greater so this catches the edge case where the cheat was left toggled on then map transitioned to new level
			{
				ReadProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam0_offset), &original_fog_param0, 4, &bytes); // since fog vaires on level this must be backed up each time
				ReadProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam1_offset), &original_fog_param1, 4, &bytes); // otherwise the wrong ammount maybe reapplied
				ReadProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam2_offset), &original_fog_param2, 4, &bytes); // when a new level is loaded
				ReadProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam3_offset), &original_fog_param3, 4, &bytes);
			}
			WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam0_offset), &zero_float, 4, &bytes);  // these also are the values these parameters take on temporarily
			WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam1_offset), &tenK_float, 4, &bytes);  // when a player enters the commander overview screen
			WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam2_offset), &zero_float, 4, &bytes);  // and zooms in
			WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam3_offset), &one_float, 4, &bytes);
			IsHack4On = TRUE;
		}
		else
		{
			WriteProcessMemory(hand, (void*)(bf2142_viewdistance_scalar_ptr + bf2142_viewdistance_scalar_address_offset), &bf2142_viewdistance_scalar_off, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawUnderGrowth_offset), &one_byte, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawParticles_offset), &one_byte, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawSunflare_offset), &one_byte, 1, &bytes);
			//WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawSkydome_offset), &one_byte, 1, &bytes);
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawPostProduction_offset), &one_byte, 1, &bytes);

			ReadProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam3_offset), &unknown_float, 4, &bytes);
			float_test = buildDwordFromByteArray(unknown_float);
			if (*(float*)&float_test >= 1.0f)  // last fog parameter is never naturally 1 or greater so this catches the edge case where the cheat was left toggled on then map transitioned to new level
			{
				WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam0_offset), &original_fog_param0, 4, &bytes);
				WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam1_offset), &original_fog_param1, 4, &bytes);
				WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam2_offset), &original_fog_param2, 4, &bytes);
				WriteProcessMemory(hand, (void*)(bf2142_level_fog_params_base_ptr + fogparam3_offset), &original_fog_param3, 4, &bytes);
			}
			IsHack4On = FALSE;
		}

    }
	if (GetAsyncKeyState(VK_NUMPAD5)) 
	{
		if (IsHack5On == FALSE) 
		{
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawSkydome_offset), &zero_byte, 1, &bytes);
			IsHack5On = TRUE; 
		}
		else
		{
			WriteProcessMemory(hand, (void*)(renderer_object_base_ptr + drawSkydome_offset), &one_byte, 1, &bytes);
			IsHack5On = FALSE; 
		}

	}
	if (GetAsyncKeyState(VK_NUMPAD6)) 
	{
		if (IsHack6On == FALSE)
		{
			WriteProcessMemory(hand, (void*)(dwBF2142Base + bf2142_developerconsolejump_offset0), &DevConsole0, 2, &bytes);
			WriteProcessMemory(hand, (void*)(dwBF2142Base + bf2142_developerconsolejump_offset1), &DevConsole1, 2, &bytes);
			IsHack7On = TRUE;
		}
		else 
		{
			WriteProcessMemory(hand, (void*)(dwBF2142Base + bf2142_developerconsolejump_offset0), &original_console_code0, 2, &bytes);
			WriteProcessMemory(hand, (void*)(dwBF2142Base + bf2142_developerconsolejump_offset1), &original_console_code1, 2, &bytes);
			IsHack6On = FALSE; 
		}

	}

	/** End **/
}
DWORD GetModuleBaseBaseAddress(DWORD iProcId, char* DLLName)
{
	HANDLE hSnap;
	MODULEENTRY32 xModule;
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, iProcId);
	xModule.dwSize = sizeof(MODULEENTRY32);
	if (Module32Next(hSnap,&xModule))
	{
		while(Module32Next(hSnap,&xModule))
		{
			if(strcmp(xModule.szModule, DLLName) == 0)
			{
				CloseHandle(hSnap);
				return (DWORD)xModule.modBaseAddr;
			}
		}			
	}
	CloseHandle(hSnap);
	return (DWORD)0;

}

BOOL GetProcessList( )
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  DWORD dwPriorityClass;
  int PidTest;
  GameRunning=FALSE;
 
  
  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if( hProcessSnap == INVALID_HANDLE_VALUE ) return( FALSE );
  

  // Set the size of the structure before using it.
  pe32.dwSize = sizeof( PROCESSENTRY32 );

  // Retrieve information about the first process,
  // and exit if unsuccessful
  if( !Process32First( hProcessSnap, &pe32 ) )
  {
    CloseHandle( hProcessSnap );     // Must clean up the snapshot object!
    return( FALSE );
  }

  // Now walk the snapshot of processes, and
  // display information about each process in turn
  
  do
  {
    // Retrieve the priority class.
    dwPriorityClass = 0;
    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
    if( hProcess != NULL )
    {
      dwPriorityClass = GetPriorityClass( hProcess );
      if( !dwPriorityClass )
        
      CloseHandle( hProcess );
    }

    PidTest=strcmp(gameWindow, pe32.szExeFile);
	if(PidTest==0){ pid=pe32.th32ProcessID; GameRunning=TRUE;}

  } while( Process32Next( hProcessSnap, &pe32 ) );

  // Don't forget to clean up the snapshot object!
  CloseHandle( hProcessSnap );
  return( TRUE );
}

BOOL CALLBACK DialogProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
	{
		case WM_INITDIALOG:
			Initialize(hwnd,wParam,lParam);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_ABOUT:
					aboutButton(hwnd);
					return TRUE;

				case IDC_EXIT:
					EndDialog (hwnd, 0);
					return TRUE;
			}
		return TRUE;

		case WM_DESTROY:
			CloseHandle(pFile);
			PostQuitMessage(0);
			return TRUE;

		case WM_CLOSE:
			PostQuitMessage(0);
			return TRUE;
		case WM_TIMER:
			timerCall();
			return TRUE;
    }
    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAINDLG), NULL,DialogProc);
	return 0;
}


