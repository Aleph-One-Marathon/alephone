#The purpose of this script is to be able to build and release Alephone and the marathon trilogy easily for windows with msbuild#

#Our optional parameters
param(
[bool]$x64=$true, #Build 64 or 32 bits
[bool]$a1=$true, #Build AlephOne
[bool]$m1=$false, #Build Marathon
[bool]$m2=$false, #Build Marathon 2
[bool]$m3=$false, #Build Marathon Infinity
[int]$data=0, #0 or whatever is for building packages with data and without, 1 is build only with data (except for A1), 2 is build only without data
[string]$input_path="./AlephOne.sln", #Path to the solution to build (must include the solution file in the path)
[string]$output_path="./" #Path to the directory where save build packages
) 

function MsBuild {
	Param (
        [string]$configuration
    )
	
	if(Test-Path -Path $exe_path) {
		Remove-Item -Path $exe_path
	}
	&$msbuild_path $input_path /t:"Build" /p:Configuration=$configuration /p:Platform=$platform | Out-Host
	([bool]$success = Test-Path -Path $exe_path) | Out-Null
	return $success
}

function GetCommonFiles() {
	Copy-Item $exe_path -Destination $output_package_folder
	Copy-Item (Join-Path -Path $root_directory -ChildPath "THANKS") -Destination (Join-Path -Path $output_package_folder -ChildPath "THANKS.txt")
	Copy-Item (Join-Path -Path $root_directory -ChildPath "COPYING") -Destination (Join-Path -Path $output_package_folder -ChildPath "COPYING.txt")
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/docs/README.txt") -Destination (Join-Path -Path $output_package_folder -ChildPath "README.txt")
	New-Item -Path (Join-Path -Path $output_package_folder -ChildPath "/docs") -ItemType Directory -ErrorAction Stop | Out-Null
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/docs/Lua.html") -Destination (Join-Path -Path $output_package_folder -ChildPath "/docs")
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/docs/Lua_HUD.html") -Destination (Join-Path -Path $output_package_folder -ChildPath "/docs")
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/docs/MML.html") -Destination (Join-Path -Path $output_package_folder -ChildPath "/docs")
	New-Item -Path (Join-Path -Path $output_package_folder -ChildPath "/Extras") -ItemType Directory -ErrorAction Stop | Out-Null
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Software_Transparent_Liquids.mml") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Carnage_Messages.mml") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")	
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/examples/lua/Cheats.lua") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")
	Copy-Item (Join-Path -Path $root_directory -ChildPath "/Resources/Library Licenses") -Destination (Join-Path -Path $output_package_folder -ChildPath "/docs") -Recurse -Exclude $array_exclude_copy
}

function Package {
	Param (
        [string]$build,
		[string]$package_name
    )
	
	#path to the executable
	$exe_path = Resolve-Path -Path $exe_path
	
	#find out what's the version of the executable (for package naming)
	$package_version = (Get-Item "${exe_path}").VersionInfo.FileVersionRaw
	$package_version_year = $package_version.Minor
	$package_version_month = '{0:d2}' -f $package_version.Build
	$package_version_day = '{0:d2}' -f $package_version.Revision
	$package_version_string = "${package_version_year}${package_version_month}${package_version_day}"
	
	
	$package_fullname = "${package_name}-${package_version_string}"
	
	$output_package_folder = Join-Path -Path $output_path -ChildPath $package_fullname
	
	if(!(Test-Path -Path $output_package_folder -PathType Container)) {
		New-Item -Path $output_package_folder -ItemType Directory -ErrorAction Stop | Out-Null
	}
	
	GetCommonFiles -ErrorAction Stop
	
	if($build -eq "Release") {
		Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Transparent_Sprites.mml") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")
		Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Transparent_Liquids.mml") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")
		Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/default_theme") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Plugins/Default_Theme") -Recurse -Exclude $array_exclude_copy
	}
	
	#now we have our folder with all files except data files
	
	$os_target = if($x64) {""} else {"32"}
	#we can already pack what we have if we wanna pack without data
	if(($data -ne 1) -and ($build -ne "Release")) {		
		$zip_name = "${package_fullname}-Exe-Win${os_target}.zip"
		Compress-Archive -Path $output_package_folder -DestinationPath (Join-Path -Path $output_path -ChildPath $zip_name) -Force
	}
	
	if(($data -ne 2) -or ($build -eq "Release")) {
		switch($build) {
		"Marathon" {
			Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Scenarios/Marathon/*") -Destination $output_package_folder -Recurse -Exclude $array_exclude_copy
			break
		}
		"Marathon 2" {
			Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Scenarios/Marathon 2/*") -Destination $output_package_folder -Recurse -Exclude $array_exclude_copy
			break
		}
		"Marathon Infinity" {
			Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Scenarios/Marathon Infinity/*") -Destination $output_package_folder -Recurse -Exclude $array_exclude_copy
			break
		}
	  }
	  
	  $zip_name = "${package_fullname}-Win${os_target}.zip"
	  Compress-Archive -Path $output_package_folder -DestinationPath (Join-Path -Path $output_path -ChildPath $zip_name) -Force -ErrorAction Stop
	}
	
	Remove-Item -Path $output_package_folder -Recurse
}

# --- Check parameters are correct, paths are valid and we have everything we need on the device ---
if([int]$PSVersionTable.PSVersion.Major -lt 5) {
	[string]$psVersion = $PSVersionTable.PSVersion
	Write-Error "This script need PowerShell version 5+ to work, your current version is: ${psVersion}" -ErrorAction Stop
}

if(!(Test-Path -Path $input_path)) {
	Write-Error "The path to the solution is uncorrect: ${input_path}" -ErrorAction Stop
}

if(!(Test-Path -Path $output_path)) {
    New-Item -Path $output_path -ItemType Directory -ErrorAction Stop | Out-Null
}

if(!(Test-Path -Path $output_path -PathType Container)) {
	Write-Error "The path to the output directory is uncorrect or can't be created: ${output_path}" -ErrorAction Stop
}

[string]$vswhere_path = if ([Environment]::Is64BitOperatingSystem) {"${Env:Programfiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"} else {"${Env:Programfiles}\Microsoft Visual Studio\Installer\vswhere.exe"}

if(!(Test-Path -Path $vswhere_path)) {
	Write-Error "Can't find vswhere.exe that should be located to ${vswhere_path}" -ErrorAction Stop
}

[string]$msbuild_path = &$vswhere_path -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe"

if(!(Test-Path -Path $msbuild_path)) {	
	Write-Error "Can't find msbuild.exe that should be located to ${msbuild_path}" -ErrorAction Stop
}

# --- We should be good to go ---
[string]$solution_directory = Split-Path -Path $input_path
[string]$root_directory = Join-Path -Path $solution_directory -ChildPath "../"
[string]$platform = if ($x64) {"x64"} else {"x86"}
[string]$exe_path_only = if ($x64) {"/x64/Release"} else {"/Release"}
[string]$exe_path_only = Join-Path -Path $solution_directory -ChildPath $exe_path_only

$array_exclude_copy = @('Makefile','Makefile.*','*.svn','*.git')
$array_build = @()
$array_exe_name = @()
$array_package_name = @()
if($a1) {$array_build += "Release"; $array_exe_name += "Aleph One.exe"; $array_package_name += "AlephOne"}
if($m1) {$array_build += "Marathon"; $array_exe_name += "Classic Marathon.exe"; $array_package_name += "Marathon"}
if($m2) {$array_build += "Marathon 2"; $array_exe_name += "Classic Marathon 2.exe"; $array_package_name += "Marathon2"}
if($m3) {$array_build += "Marathon Infinity"; $array_exe_name += "Classic Marathon Infinity.exe"; $array_package_name += "MarathonInfinity"}

for (($i = 0); $i -lt $array_build.Count; $i++) {
	$build = $array_build[$i]
	$package_name = $array_package_name[$i]
	Write-Host "Building ${build} ${platform}" -ForegroundColor green
	[string]$exe_path = Join-Path -Path $exe_path_only -ChildPath $array_exe_name[$i]
	[bool]$success = &MsBuild -configuration $build
	if(!($success)) {
		Write-Error "${build} build failed" -ErrorAction Stop
	} 
	else {
		Write-Host "${build} build succeeded" -ForegroundColor green
		Write-Host "Starting packaging ${build}" -ForegroundColor green
		Package -build $build -package_name $package_name
		Write-Host "Packaging for ${build} done" -ForegroundColor green
	}
}