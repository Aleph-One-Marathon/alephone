#!/usr/bin/osascript

(*
Build all AlephOne macOS apps in Xcode. 

If the first word of a Scheme's name is in `schemePrefixes` below, build it.

(This ignores `libalephone*`, which is automatically built when needed.)

The `AlephOne.xcodeproj` must already be open in Xcode.app.
*)

property schemePrefixes : {"Aleph", "Marathon", "Steam"}

--------------------------------------------------------------------------------
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
				if actionResult's status is not succeeded then
					set errorCount to errorCount + 1
					set errorMessage to actionResult's error message
					if errorMessage is missing value then set errorMessage to ""
					tell me to log "FAIL: " & actionResult's status as string & ". " & errorMessage
				else
					tell me to log "OK"
				end if
			end if
		end repeat
	end tell
end tell

set t to (current date) - d
log "Finished in " & (t div 60) & ":" & (text -2 thru -1 of ("0" & (t mod 60))) & " with " & errorCount & " errors."