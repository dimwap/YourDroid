; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "YourDroid"
#define MyAppVersion "2.0"
#define MyAppPublisher "Profi_GMan"
#define MyAppURL "https://yourdroid.github.io/"
#define MyAppExeName "yourdroid.exe"

[Setup]
;DisableDirPage=yes
DefaultDirName=c:\{#MyAppName}
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{C441E1AC-42FC-4058-A531-4B4C3EB76D0E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
;DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=C:\programming
OutputBaseFilename=yourdroid-setup
SetupIconFile=C:\programming\YourDroid\yourdroid.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\programming\YourDroid\bin\yourdroid.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\D3Dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\install.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\Qt5WinExtras.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\programming\YourDroid\bin\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\programming\YourDroid\bin\config\*"; DestDir: "{app}\config"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\programming\YourDroid\bin\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\programming\YourDroid\bin\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\programming\YourDroid\bin\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\programming\YourDroid\bin\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\programming\YourDroid\bin\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\data\ext2fsd-driver\Setup.bat"; Parameters: "install"; Flags: runhidden
;Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: skipifsilent

