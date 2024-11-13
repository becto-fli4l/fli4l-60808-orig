; -------------------------------------------------------------------------
; parse/read config-dir/mkfli4l.txt                            __FLI4LVER__
;
; Creation: lanspezi 2005-04-04
; $Id$
; -------------------------------------------------------------------------
#include-once

Func parse_mkfli4l_conf ($silent)
    msg_lang_read ("parse_mkfli4l_conf")
    ; get parameters from config-dir/mkfli4l.txt
    If FileExists ($dir_config & "\mkfli4l.txt") then
        $message3 = $MSG_03_readcheck_buildenv & $dir_config & "\mkfli4l.txt' ... "
        If $silent = 0 then
            GUICtrlSetData($log_output,$message3 ,1)
        EndIf
        $_dir_config_tmp = $dir_config & "\_tmp_conf"
        if NOT FileExists($_dir_config_tmp) then
            fli_DirCreate ($_dir_config_tmp)
        EndIf
        FileCopy("check\mkfli4l.exp.win", "check\mkfli4l.exp", 1)

        $run_call = StringFormat ('windows\\mkfli4l.exe -c "%s" -b "%s" -t "%s" -l "%s\\mkfli4l_conf.log" -p mkfli4l', _
                    $dir_config, $_dir_config_tmp, $_dir_config_tmp, $_dir_config_tmp)
        $val = RunWait ($run_call, @WorkingDir, @SW_HIDE)

        If $val = 0 then
            If $silent = 0 then
                GUICtrlSetData($log_output,$MSG_finished & @CRLF ,1)
            EndIf
            Dim $_res
            $_res = read_var_from_config_file  ("mkfli4l.txt", "BUILDDIR=", $dir_build)
            $_res = read_var_from_config_file  ("mkfli4l.txt", "VERBOSE=", $verbose)
            $_res = read_var_from_config_file  ("mkfli4l.txt", "FILESONLY=", $bool_filesonly)
            $_res = read_var_from_config_file  ("mkfli4l.txt", "REMOTEUPDATE=", $remoteupdate)
            $_res = read_var_from_config_file  ("mkfli4l.txt", "REMOTEHOSTNAME=", $remotehostname)
            if $_res = "" then
                ; set remotehostname as default to hostname defined in config-dir/base.txt
                $remotehostname = read_var_from_base ("HOSTNAME=", $dummy)
            EndIf
            $_res = read_var_from_config_file  ("mkfli4l.txt", "REMOTEUSERNAME=", $remoteusername)
            $_res = read_var_from_config_file  ("mkfli4l.txt", "REMOTEPATHNAME=", $remotepathname)
            $_res = read_var_from_config_file  ("mkfli4l.txt", "REMOTEPORT=", $remoteport)
            $sshkeyfile = read_var_from_config_file  ("mkfli4l.txt", "SSHKEYFILE=", $dummy)
            $_res = read_var_from_config_file ("mkfli4l.txt", "TFTPBOOTPATH=", $tftpboot_path)
            $_res = read_var_from_config_file ("mkfli4l.txt", "TFTPBOOTIMAGE=", $tftpboot_image)
            $_res = read_var_from_config_file ("mkfli4l.txt", "PXESUBDIR=", $pxe_subdir)
            $_res = read_var_from_config_file ("mkfli4l.txt", "MKFLI4L_DEBUG_OPTION=", $mkfli4l_debug_option)
            $_res = read_var_from_config_file ("mkfli4l.txt", "SQUEEZE_SCRIPTS=", $bool_squeeze)

            ;replace the / from unix path style to \ like windows path style
            $dir_build = StringReplace($dir_build, "/","\")
            
            If $verbose = "yes" then
                $verbose = "true"
            else
                $verbose = ""
            EndIf
            If $bool_filesonly = "yes" then
                $bool_filesonly = "true"
            else
                $bool_filesonly = "false"
            EndIf
            If $remoteupdate = "yes" then
                $remoteupdate = "true"
            else
                $remoteupdate = "false"
            EndIf
            If $bool_squeeze = "yes" then
                $bool_squeeze = "true"
            EndIf
            If $bool_squeeze = "no" then
                $bool_squeeze = "false"
            EndIf
        Else
            If $silent = 0 then
                GUICtrlSetData($log_output,$MSG_finished & @CRLF ,1)
            EndIf
            Dim $msg
            Dim $file
            Dim $line
            $file = FileOpen($dir_config & "\_tmp_conf\mkfli4l_conf.log", 0)
            ; Check if file opened for reading OK
            If $file = -1 Then
                MsgBox(0, $MSG_msgbox_error, $MSG_03_nologfile & @CRLF & $dir_config & "\_tmp_conf\mkfli4l_conf.log'.")
            Else
                ; Read in lines of text until the EOF is reached
                While 1
                    $line = FileReadLine($file)
                    If @error = -1 Then ExitLoop
                    if $line <> "" Then
                        $msg = $msg & $line & @CRLF
                    EndIf
                Wend
                MsgBox(48 + 4096, $MSG_msgbox_error & " in " & $dir_config & "\mkfli4l.txt", $msg)
                $wline = "------------------------------------------------------------------------------------------------------------------"
                GUICtrlSetData($log_output, $wline & @CRLF & $msg & $wline & @CRLF,1)
            EndIf
            FileClose($file)
            $bool_error = "true"
        EndIf
    ; make path absolute
    $dir_config=make_abs($dir_config)
    $dir_build=make_abs($dir_build)

    DirRemove($dir_config & "\_tmp_conf", 1)

    EndIf
EndFunc
