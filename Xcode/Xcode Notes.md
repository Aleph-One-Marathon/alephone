= Xcode Notes

== Changes

1. The old `./PBProjects` directory has been renamed `./Xcode` and its `AlephOne.xcodeproj` and macOS-specific assets reorganized for easier maintenance. Project settings have been updated for Xcode 16.2.


2. To address #464, AO's guts have been moved into a static library, `libalephone`, which is shared across multiple .app Targets for faster build times. This library contains all of the original AO application's `.h` and `.cpp` files, except for `main.cpp` and Win/Lin-specific files.

There is a variation of this library, `libalephonesteam`, used by 3 Steam-enabled M1-3 apps. This is identical to `libalephone` except for the following build settings:


- The Preprocessor Macros for `libalephone` are `SDL`,  `HAVE_BUNDLE_NAME`, `HAVE_CONFIG_H`, same as for the Aleph One Target.

- The Preprocessor Macros for `libalephonesteam` are `SDL`, `HAVE_BUNDLE_NAME`, `HAVE_CONFIG_H`, `HAVE_STEAM`, same as for the M1-3 Steam Targets.


3. Each AO/M1-3 standalone/M1-3 Steam .app Target includes the appropriate library in its TargetDependencies and Link Binary With Libraries Build Phases. New Test app Targets can link to an `libalephone*` library as long as they don't need to substitute existing `.h`/`.cpp` files, set compile-time macros, etc.


4. The Preprocessor Macros for standalone M1-3 Targets previously defined `PREFER_APP_NAME_TO_BUNDLE_ID`, `SCENARIO_IS_BUNDLED` macros in addition to the 3 Aleph One macros. Since all 4 .app Targets now share `libalephone`, these have been removed. A custom `A1_PREFER_APP_NAME_TO_BUNDLE_ID` key has been added to the static `Info.plist` files to replicate the first macro's behavior as this affects search paths for Preferences and other files. The second macro is ignored: in standalone M1-3, the embedded scenario directory (`NAME.app/Contents/Resources/DataFiles/`) is searched for first in `shell.cpp` so the standalone apps appear to behave correctly without it.


== TODO

1. Using static Info.plist files (e.g. “Xcode/App_Resources/Marathon1/Info-Steam.plist”) instead of having Xcode generate each .app's plist is brittle. Make sure values in the Info.plist file match those in Build Settings, e.g. a mismatch between executable names causes codesigning to fail with a misleading error message:

/Users/has/Library/Developer/Xcode/DerivedData/AlephOne-fxnqppngaujozvfsnezkxmdubeav/Build/Products/Debug/Classic Marathon Steam.app: code object is not signed at all
In subcomponent: /Users/has/Library/Developer/Xcode/DerivedData/AlephOne-fxnqppngaujozvfsnezkxmdubeav/Build/Products/Debug/Classic Marathon Steam.app/Contents/MacOS/Classic Marathon Steam
Command CodeSign failed with a nonzero exit code

This problem also occurs after Duplicating an existing Target+Scheme. While the existing Scheme builds successfully, the new Scheme fails.

Getting rid of these static plist files and configuring Xcode to generate each .app's plist automatically at build time should be a lot more robust.


2. #319 and #479 may be a result of these static `Info.plist` files being manually misconfigured. Setting file associations in the Target's Info GUI is fiddly; writing and maintaining `.plist` files by hand is daring.


3. `./Makefile.am` contains hardcoded lists of files to be included in Source code distributions. This is really brittle: packing lists should be generated from a list of known directories and grep rules to find all the files automatically. The list has been updated to match the cleaned up `./Xcode` directory.


4. The Hardened Runtime is unhardened for Steam-enabled M1-3 `.app` Targets (Allow Unsigned Executable Memory, Allow DYLD Environment Variables, Disable Library Validation). A quick web search suggests the Classic Marathon Launcher (aka Steamshim) needs this. Confirming the M1-3 Targets can be re-hardened is left as an exercise for another day.


5. Move the 'Insert Version Into Info.plist' scripts into a single `insert_version_into_info_plist.sh` file for easier maintenance? (Note: 'Run Script' phases are subprocesses so can't set xcbuild's environment vars, so substituting  `A1_DISPLAY_VERSION` and `A1_DATE_VERSION` placeholders in the exported `Info.plist` seems to be best way to do it.)