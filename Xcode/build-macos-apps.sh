#!/usr/bin/osascript

-- Build all AlephOne macOS apps in Xcode. (The AlephOne.xcodeproj must be opened first.)

-- If the first word of a scheme's name is in this list, it will be built.
-- (This ignores libalephone* schemes which are automatically built when needed.)
property schemePrefixes : {"Aleph", "Marathon", "Tests"}

-- main 

set d to current date
log "Building all AlephOne apps..."
set errorCount to 0

tell application "Xcode"
	tell workspace document "AlephOne.xcodeproj"
		if not (exists) then
			log "Please open the AlephOne.xcodeproj, then run this script again."
			return
		end if
		repeat with schemeRef in its schemes
			set schemeName to name of schemeRef
			if {schemeName's first word} is in schemePrefixes then
				set active scheme to schemeRef
				tell me to log "Building Scheme '" & schemeName & "'"
				set actionResult to build it -- current scheme
				repeat until actionResult's completed
					delay 0.5
				end repeat
				if actionResult's error message is not missing value then
					set errorCount to errorCount + 1
					tell me to log "FAIL: " & actionResult's error message
				else
					tell me to log "OK"
				end if
			end if
		end repeat
	end tell
end tell

set t to (current date) - d
log "Finished in " & (t div 60) & "m" & (t mod 60) & "s with " & errorCount & " errors."