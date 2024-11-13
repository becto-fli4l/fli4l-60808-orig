; -------------------------------------------------------------------------
; functions for the build-process (used by all other files)    __FLI4LVER__
;
; Creation: lanspezi 2005-04-15
; $Id$
; -------------------------------------------------------------------------
#include-once

Global $lang_msg_file
Func msg_lang_init ()
    Select
        Case StringInStr("0409,0809,0c09,1009,1409,1809,1c09,2009," & _
                                   "2409,2809,2c09,3009,3409", @OSLang)
            $lang = "english"
        Case StringInStr("0407,0807,0c07,1007,1407", @OSLang)
            $lang = "deutsch"
        Case StringInStr("040c,080c,0c0c,100c,140c,180c", @OSLang)
            $lang = "french"
        Case Else
            $lang = "english"
    EndSelect

    For $i = 1 to $CmdLine[0]
        If $CmdLine[$i] = "--lang" and $CmdLine[0] >= ($i + 1) then
            $lang = $CmdLine[$i + 1]
            ExitLoop
        EndIf
    Next
    Select
        Case $lang = "english"
            ;
        Case $lang = "deutsch"
            ;
        Case $lang = "francais"     ; compatibility
            $lang = "french"
        Case $lang = "french"
            ;
        Case Else
            $lang = "english"
    EndSelect
    $lang_msg_file = "windows\scripts\_msg_" & $lang & ".msg"
    If not FileExists ($lang_msg_file) then
        $lang_msg_file = "windows\scripts\_msg_english.msg"
    EndIf
    $var = IniReadSection($lang_msg_file, "Global")
    If @error Then
        MsgBox(4096, "Error occurred", "can not read message-file: " & @CRLF & $lang_msg_file)
        $bool_error = "true"
        exit_fli4l_build ()
    EndIf
    msg_lang_read ("Global")
EndFunc

Func msg_lang_read ($section)
    $_var = IniReadSection ($lang_msg_file, $section)
    If @error Then
        MsgBox (4096, "Error", "can not read from message-file: " & @CRLF & "File: " & _
        $lang_msg_file & @CRLF & "Section: " & $section)
    Else
        For $i = 1 to $_var[0][0]
            Assign($_var[$i][0], $_var[$i][1],2)
        Next
    EndIf
EndFunc

Func read_var_from_config_file_or_fli4ltxt ($filename, $var, ByRef $parameter)
    Dim $res
    $res = read_var_from_config_file ($filename, $var, $dummy)
    If FileExists ($dir_config & "\_fli4l.txt") then
        read_var_from_config_file ("_fli4l.txt", $var, $res)
    EndIf
    If $res <> "" then
        $parameter = $res
    EndIf
    Return $res
EndFunc


Func read_var_from_base ($var, ByRef $parameter)
    Dim $res
    $res = read_var_from_config_file ("base.txt", $var, $dummy)
    If FileExists ($dir_config & "\_fli4l.txt") then
        read_var_from_config_file ("_fli4l.txt", $var, $res)
    EndIf
    If $res <> "" then
        $parameter = $res
    EndIf
    Return $res
EndFunc

Func read_var_from_config_file ($filename, $var, ByRef $parameter)
    Dim $result
    Dim $line
    Dim $val
    Dim $file
    Dim $len
    Dim $end
    $len = StringLen($var)
    $file = FileOpen($dir_config & "\" & $filename, 0)
    ; Check if file opened for reading OK
    If $file = -1 Then
        MsgBox(0, "Error", "Unable to open file '"& $dir_config & "\" & $filename & "'")
    else
        ; Read in lines of text until the EOF is reached
        While 1
            $line = FileReadLine($file)
            If @error = -1 Then ExitLoop
            $splitline = StringSplit($line, "=", 0)
            If $splitline[0] > 1 Then
                $line = StringStripWS($splitline[1], 3)
                $splitline[2] = StringStripWS($splitline[2], 1)
                $delim = StringLeft($splitline[2], 1)
                For $i = 2 To $splitline[0]
                    $line = $line & "=" & $splitline[$i]
                Next
            Endif
            If StringLeft($line, $len) = $var then
                $end = StringInStr($line, $delim, 1, 2)
                $result = StringMid ($line, $len + 2, $end - $len - 2)
                ExitLoop
            EndIf
        Wend
        FileClose($file)
        If $result <> "" then
            $parameter = $result
        EndIf
        Return $result
    EndIf
EndFunc

Dim $log_output_line
Func fli4lbuild_msg ($buildmsg, $alwaysverbose)
    fli4l_log_build ($buildmsg)
    If $verbose = "true" OR $alwaysverbose = 1 then
        $log_output_line = $log_output_line & $buildmsg
        GUICtrlSetData($log_output, $log_output_line)

        ;WinActivate("fli4l-Build","")
        ;GUICtrlSetState($log_output,$GUI_FOCUS)
        ;Send ("^{END}",0)
        ;GUICtrlSetData($log_output,$buildmsg ,1)
    EndIf
EndFunc

Func fli4lbuild_errmsg ($builderrmsg, $msgbox)
    fli4l_log_error ($builderrmsg)
    If $msgbox = 1 then
        MsgBox(48 + 4096, $MSG_msgbox_error, $builderrmsg)
    EndIf
EndFunc

Func fli_DirCreate ($path)
    If NOT FileExists ($path) then
        $val = DirCreate ($path)
        If $val = 0 then
            MsgBox (0,"Fatal-Error", "CreateDir failed for -> " & $path)
            $bool_error = "true"
            exit_fli4l_build ()
        EndIf
    EndIf
EndFunc

Func DriveSizeCluster ( $drive )

    #cs
    Returns the size of the clusters for a drive
    ----------------------------
    DriveSizeCluster ( "drive" )
    ----------------------------
    drive | the drive to receive information from (for example "A:")
    Success: Returns Size of clusters in bytes
    Failure: Returns 0
    #ce

    $result=DLLCall("kernel32.dll", "int", "GetDiskFreeSpace", "str", $drive & "\", _
            "long*", 0, "long*", 0, "long*", 0, "long*", 0)
    If @error = 1 Then
        Return 0
    Else
        Return $result[2] * $result[3]
    EndIf
EndFunc

Func fli4l_logs_init ()
    FileDelete ($glob_dir_tmplog & "\mkfli4l_build.log")
    FileDelete ($glob_dir_tmplog & "\mkfli4l_error.log")
EndFunc

Func fli4l_log_build ($msg)
    $file = FileOpen($glob_dir_tmplog & "\mkfli4l_build.log", 1)
    ; Check if file opened for writing OK
    If $file = -1 Then
        MsgBox(0, $MSG_msgbox_error, $MSG_errorfilelog)
    else
        FileWriteLine($file, $msg)
    EndIf
    FileClose($file)

EndFunc

Func fli4l_log_error ($msg)
    $file = FileOpen($glob_dir_tmplog & "\mkfli4l_error.log", 1)
    ; Check if file opened for writing OK
    If $file = -1 Then
        MsgBox(0, $MSG_msgbox_error, $MSG_errorfilelogerror)
    Else
        FileWriteLine($file, $msg)
    EndIf
    FileClose($file)
EndFunc

Func build_time ()
    Return ("[" & @HOUR & ":" & @MIN & "." & @SEC & "] ")
EndFunc

Func scp ($scpfiles)
    dim $_flag_abort = 0
    dim $_transport_error = 0
    dim $_i
    dim $_repeat = 0

    $msg = @CRLF & $MSG_LINE & @CRLF & $MSG_00_scpstart & " "
    fli4lbuild_msg ($msg, 1)

    If $sshkeyfile <> "" then
        $_scpoption = '-scp -P ' & $remoteport & ' -i ' & '"' & $sshkeyfile & '"'
        $_plinkoption = '-ssh -batch -P ' & $remoteport & ' -i ' & '"' & $sshkeyfile & '"'
    else
        $res = MsgBox(4+32,$MSG_00_pass,$MSG_00_usebasepass)
        if $res = 7 then
            $msg = $MSG_00_scpenterpass & " " & $remotehostname
            $sshpasswd = InputBox($MSG_00_pass, $msg, "", "*M")
            If @error = 1 then
                $_flag_abort = 1
            EndIf
        EndIf
        $_scpoption = "-scp -P " & $remoteport & " -pw """ & $sshpasswd & """ "
        $_plinkoption = "-ssh -batch -P " & $remoteport & " -pw """ & $sshpasswd & """ "
    EndIf

    If $_flag_abort = 1 then
        ; Cancel
        fli4lbuild_errmsg ($MSG_00_scpcancel & @CRLF, 0)
        fli4lbuild_msg ($MSG_cancel & @CRLF, 1)
    else
        do
            $icon = GUICtrlCreateIcon ("windows\scripts\ico_scp.ico",-1, 20,422,32,32)
            If FileExists ($dir_build & "\scp.bat") then
                FileDelete ($dir_build & "\scp.bat")
            EndIf
            FileWriteLine ($dir_build & "\scp.bat", "@echo off")
            FileWriteLine ($dir_build & "\scp.bat", "echo.")
            FileWriteLine ($dir_build & "\scp.bat", "echo SCP-Transfer")
            FileWriteLine ($dir_build & "\scp.bat", "echo ============")
            FileWriteLine ($dir_build & "\scp.bat", "echo.")
            FileWriteLine ($dir_build & "\scp.bat", "echo to: " & $remotehostname)
            FileWriteLine ($dir_build & "\scp.bat", "echo.")
            $run_call = StringFormat ('"%s\\windows\\pscp.exe" %s -r %s %s@%s:%s', _
                  $dir_base, $_scpoption, $scpfiles, $remoteusername, $remotehostname, _
                  $remotepathname)
            FileWriteLine ($dir_build & "\scp.bat", $run_call)
            FileWriteLine ($dir_build & "\scp.bat", "if NOT errorlevel 1 goto END")
            FileWriteLine ($dir_build & "\scp.bat", "echo ERROR on SCP-Transfer > scperror.log")
            FileWriteLine ($dir_build & "\scp.bat", "echo.")
            FileWriteLine ($dir_build & "\scp.bat", "pause")
            FileWriteLine ($dir_build & "\scp.bat", ":END")
            FileWriteLine ($dir_build & "\scp.bat", "ping -n 3 127.0.0.1 > NUL")
            $_wdirsave = @WorkingDir
            FileChangeDir ($dir_build)
            $val = RunWait ("scp.bat", $dir_build)
            FileChangeDir ($_wdirsave)
            FileDelete ($dir_build & "\scp.bat")
            $scperror=readerrorfile("scperror.log")
            if StringLen ($scperror) <> 0 then
                $_transport_error = 1
            EndIf
            GUICtrlDelete ($icon)
            If $_transport_error = 1 then
                $_val = MsgBox ( 5 + 32 + 256,$MSG_msgbox_error,$MSG_00_scperror1 & @CRLF & $MSG_00_scperror2)
                if $_val = 4 then
                    $_repeat = 1
                    $_transport_error = 0
                else
                    $_repeat = 0
                endif
            else
                $_repeat = 0
            endif
        until $_repeat <> 1
        if $_transport_error <> 0 then
            $msg = @CRLF & $MSG_00_scperror1 & @CRLF & $MSG_00_scperror2 & @CRLF
            fli4lbuild_errmsg ($msg,0)
        Else
            $msg = $MSG_finished & @CRLF
            fli4lbuild_msg ($msg, 1)
            $res = Msgbox (4 + 32 + 256, $MSG_00_askreboot, $MSG_00_plinkaskreboot)
            If $res = 6 then
                If FileExists ($dir_build & "\fli4l_reboot.bat") then
                    FileDelete ($dir_build & "\fli4l_reboot.bat")
                EndIf
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "@echo off")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "echo.")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "echo fli4l Reboot")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "echo ============")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "echo.")
                $run_call = StringFormat ('"%s\\windows\\plink.exe" %s %s@%s %s', _
                $dir_base, $_plinkoption, $remoteusername, $remotehostname, "if [ -f /usr/local/bin/check-bootfiles.sh ];then /usr/local/bin/check-bootfiles.sh --log-version --reboot; else /sbin/reboot; fi")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", $run_call)
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "if NOT errorlevel 1 goto END")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "echo ERROR on fli4l Reboot > fli4l_reboot.log")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "echo.")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "pause")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", ":END")
                FileWriteLine ($dir_build & "\fli4l_reboot.bat", "ping -n 3 127.0.0.1 > NUL")
                $val = RunWait ($dir_build & "\fli4l_reboot.bat", $dir_build)
                FileDelete ($dir_build & "\fli4l_reboot.bat")
                $rebooterror=readerrorfile("fli4l_reboot.log")
                if StringLen ($rebooterror) <> 0 then
                    $msg = $MSG_LINE & @CRLF & $MSG_00_scpend2 & @CRLF & @CRLF
                else
                    $msg = $MSG_LINE & @CRLF & $MSG_00_plinksentreboot & @CRLF & @CRLF
                EndIf
            else
                If FileExists ($dir_build & "\fli4l_chkboot.bat") then
                    FileDelete ($dir_build & "\fli4l_chkboot.bat")
                EndIf
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "@echo off")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "echo.")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "echo fli4l bootfilecheck")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "echo ===================")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "echo.")
                $run_call = StringFormat ('"%s\\windows\\plink.exe" %s %s@%s %s', _
                $dir_base, $_plinkoption, $remoteusername, $remotehostname, "if [ -f /usr/local/bin/check-bootfiles.sh ];then /usr/local/bin/check-bootfiles.sh --log-version; fi")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", $run_call)
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "if NOT errorlevel 1 goto END")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "echo ERROR on fli4l bootfilecheck")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "echo.")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "pause")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", ":END")
                FileWriteLine ($dir_build & "\fli4l_chkboot.bat", "ping -n 3 127.0.0.1 > NUL")
                $val = RunWait ($dir_build & "\fli4l_chkboot.bat", $dir_build)
                FileDelete ($dir_build & "\fli4l_chkboot.bat")
                $msg = $MSG_LINE & @CRLF & $MSG_00_scpend2 & @CRLF & @CRLF
            EndIf
        EndIf
        fli4lbuild_msg ($msg, 1)
    EndIf

    WinActivate("fli4l-Build","")
EndFunc

Func readerrorfile ($filename)
    Dim $file_read="", $file
    $file_path=$dir_build & "\" & $filename

    If FileExists ($file_path) then
        $file = FileOpen ($file_path, 0)
        If $file = -1 Then
            MsgBox(0, "Error", "Unable to open file." & @CRLF & $file_path)
        else
            $size = FileGetSize($file_path)
            $file_read = FileRead ($file, $size)
        EndIf
        FileClose ($file)
        FileDelete ($file_path)
    EndIf
    Return $file_read
EndFunc

Func cleanup ()
    ;Msgbox (0,"Debug","cleanup()")
    If FileExists ($glob_dir_tmp) then
        DirRemove ( $glob_dir_tmp, 1)
    EndIf
EndFunc

Func cleanup_fli4lfiles ()
    Dim $_remove_file[5]
    $_remove_file [0] = "fli4l.iso"
    $_remove_file [1] = "mkfli4l.log"
    $_remove_file [2] = "mkfli4l_build.log"
    $_remove_file [3] = "mkfli4l_error.log"
    $_remove_file [4] = "mkfli4l_error.flg"
    If FileExists ($dir_image) Then
        DirRemove ($dir_image, 1)
    EndIf
    For $_n = 0 to UBound($_remove_file) - 1
        If FileExists ($dir_build & "\" & $_remove_file[$_n]) then
            FileDelete ( $dir_build & "\" & $_remove_file[$_n])
        EndIf
    Next
    If FileExists ($dir_config & "\etc\rc.cfg") then
        FileDelete ($dir_config & "\etc\rc.cfg")
    EndIf
EndFunc

Func read_regkey ($key,$var)
    Dim $value
    $value = RegRead("HKEY_CURRENT_USER\" & $key, $var)
    if $value = "" AND @error <> 0 then
        $value = RegRead("HKEY_LOCAL_MACHINE\" & $key, $var)
        if $value = "" AND @error <> 0 then
           $value = "error"
        endif
    endif
    return ($value)
EndFunc

Func exists_regsubkey ($key)
    Dim $value
    $value = RegEnumVal("HKEY_CURRENT_USER\" & $key, 1)
    if $value = "" AND @error <> 0 then
        $value = RegEnumVal("HKEY_LOCAL_MACHINE\" & $key, 1)
        if $value = "" AND @error <> 0 then
           $value = "error"
        endif
    endif
    return ($value)
EndFunc

func set_glob_dir ()
    Global $glob_dir_tmp
    Global $glob_dir_tmpbuild
    Global $glob_dir_tmplog
    Global $glob_dir_tmptar
    Global $glob_dir_tmptemp

    $rand = @YEAR & @MON &@MDAY & @HOUR & @MIN & "." & random(1,999,1)
    If FileExists (@TempDir) then
        $glob_dir_tmp = @TempDir & "\fli4l_tmp_" & $rand
    else
        $glob_dir_tmp = "c:\fli4l_tmp_" & $rand
    EndIf
    $glob_dir_tmpbuild = $glob_dir_tmp & "\build"
    $glob_dir_tmplog = $glob_dir_tmp & "\log"
    $glob_dir_tmptar = $glob_dir_tmp & "\tar"
    $glob_dir_tmptemp = $glob_dir_tmp & "\temp"

    fli_DirCreate ($glob_dir_tmp)
    fli_DirCreate ($glob_dir_tmpbuild)
    fli_DirCreate ($glob_dir_tmplog)
    fli_DirCreate ($glob_dir_tmptar)
    fli_DirCreate ($glob_dir_tmptemp)
EndFunc

func is_dual_screen ()
    If ( @DesktopWidth / @DesktopHeight > 2) then
        return (1)
    else
        return (0)
    EndIf
EndFunc
