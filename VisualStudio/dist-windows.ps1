#The purpose of this script is to be able to package and release Alephone and the marathon trilogy easily for windows, executables must already exist before calling this and be sure your git status is clean on data directories

#Our optional parameters
param(
[bool]$x64=$true, #Package 64 or 32 bits version
[bool]$a1=$true, #Package AlephOne
[bool]$m1=$false, #Package Marathon
[bool]$m2=$false, #Package Marathon 2
[bool]$m3=$false, #Package Marathon Infinity
[int]$data=0, #0 or whatever is for building packages with data and without, 1 is build only with data (except for A1), 2 is build only without data
[string]$input_path="./AlephOne.sln", #Path to the AlephOne solution (must include the solution file in the path)
[string]$output_path="./" #Path to the directory where save built packages
)

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
	
	if($package_name -eq "Release") {
		Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Transparent_Sprites.mml") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")
		Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Transparent_Liquids.mml") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Extras")
		Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/default_theme") -Destination (Join-Path -Path $output_package_folder -ChildPath "/Plugins/Default_Theme") -Recurse -Exclude $array_exclude_copy
	}
	
	#now we have our folder with all files except data files
	
	$os_target = if($x64) {""} else {"32"}
	#we can already pack what we have if we wanna pack without data
	if(($data -ne 1) -and ($package_name -ne "AlephOne")) {		
		$zip_name = "${package_fullname}-Exe-Win${os_target}.zip"
		Compress-Archive -Path $output_package_folder -DestinationPath (Join-Path -Path $output_path -ChildPath $zip_name) -Force
	}
	
	if(($data -ne 2) -or ($package_name -eq "AlephOne")) {
		switch($package_name) {
		"Marathon" {
			Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Scenarios/Marathon/*") -Destination $output_package_folder -Recurse -Exclude $array_exclude_copy
			break
		}
		"Marathon2" {
			Copy-Item (Join-Path -Path $root_directory -ChildPath "/data/Scenarios/Marathon 2/*") -Destination $output_package_folder -Recurse -Exclude $array_exclude_copy
			break
		}
		"MarathonInfinity" {
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
	Write-Error "The path to the output directory is uncorrect or the directory can't be created: ${output_path}" -ErrorAction Stop
}

# --- We should be good to go ---
[string]$solution_directory = Split-Path -Path $input_path
[string]$root_directory = Join-Path -Path $solution_directory -ChildPath "../"
[string]$platform = if ($x64) {"x64"} else {"x86"}
[string]$exe_path_only = if ($x64) {"/x64/Release"} else {"/Release"}
[string]$exe_path_only = Join-Path -Path $solution_directory -ChildPath $exe_path_only

$array_exclude_copy = @('Makefile','Makefile.*','*.svn','*.git')
$array_exe_name = @()
$array_package_name = @()
if($a1) {$array_exe_name += "Aleph One.exe"; $array_package_name += "AlephOne"}
if($m1) {$array_exe_name += "Classic Marathon.exe"; $array_package_name += "Marathon"}
if($m2) {$array_exe_name += "Classic Marathon 2.exe"; $array_package_name += "Marathon2"}
if($m3) {$array_exe_name += "Classic Marathon Infinity.exe"; $array_package_name += "MarathonInfinity"}

for (($i = 0); $i -lt $array_exe_name.Count; $i++) {
	$package_name = $array_package_name[$i]
	[string]$exe_path = Join-Path -Path $exe_path_only -ChildPath $array_exe_name[$i]
	
	if(!(Test-Path -Path $exe_path)) {
		Write-Error "Can't find the executable to package in the path ${exe_path}" -ErrorAction Stop
    }
	
	Write-Host "Starting packaging ${package_name}" -ForegroundColor green
	Package -package_name $package_name
	Write-Host "Packaging for ${package_name} done" -ForegroundColor green
}