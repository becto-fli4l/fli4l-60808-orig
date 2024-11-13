; -------------------------------------------------------------------------
; parse command line                                           __FLI4LVER__
;
; Creation: lanspezi 2005-04-04
; $Id$
; -------------------------------------------------------------------------
#include-once

Global $verbose

Func parse_cmd()
    msg_lang_read ("parse_cmd")
    Global $dummy    ; needed for func read_var_from_config_file

    $message = @CRLF & $MSG_02_start & @CRLF & $MSG_LINE & @CRLF & @CRLF
    fli4lbuild_msg ($message,1)

    ; save active drive and dir
    Global $dir_base = @WorkingDir
    Global $drive_base = StringLeft( $dir_base, 2)

    ; set default value of variables
    parse_cmd_default()

    ; migrate config-dir/mkfli4l.conf to config-dir/mkfli4l.txt if necessary
    If FileExists($dir_config & "\mkfli4l.conf") AND NOT FileExists($dir_config & "\mkfli4l.txt") then
        fli4lbuild_msg ($MSG_02_migrateconfig & @CRLF & @CRLF, 1)
        FileCopy($dir_config & "\mkfli4l.conf", $dir_config & "\mkfli4l.txt", 1)
    EndIf

    ; get saved config-dir from ini-file
    If FileExists ("mkfli4l.ini") then
        $dir_config = IniRead("mkfli4l.ini", "userdefault", "configdir", $dir_config)
  	EndIf

    ; first check of the command-line-options - only need "dir_config"
    parse_cmd_option ()

    if ($bool_interactive = "true") AND ($bool_error <> "true") then
        If @OSType = "WIN32_NT" then
            GUISetState ( @SW_DISABLE , $win_fli4l_build)
        EndIf
        dlg_mkfli4l_conf ()
        GUISetState ( @SW_ENABLE , $win_fli4l_build)
        WinActivate("fli4l-Build","")
    EndIf

    ;check for exist of base.txt in config-dir
    If NOT FileExists($dir_config & "\base.txt") Then
        fli4lbuild_msg ($MSG_02_noconfdir & @CRLF & $dir_config, 1)
        fli4lbuild_errmsg ($MSG_02_noconfdir & @CRLF & $dir_config, 1)
        $bool_error = "true"
        exit_fli4l_build ()
    EndIf

    If ($bool_build_abort = "false") AND ($bool_error = "false") then
        $message = $MSG_02_useconfig & $dir_config & "' ..." & @CRLF
        fli4lbuild_msg ($message, 1)
    EndIf

    If $bool_interactive = "false" then
        ; get parameters from config-dir/mkfli4l.txt
        If FileExists($dir_config & "\mkfli4l.txt") then
            parse_mkfli4l_conf(0)
        EndIf
        ; check options/parameters from command-line again - overwrite parameters set in mkfli4l.txt
        parse_cmd_option ()
    EndIf

    $dir_image = $dir_build & "\image"

    ;check exist of build-dir create it
    if NOT FileExists($dir_build) then
        fli_DirCreate ($dir_build)
    EndIf

    ;check boot_type from base.txt BOOT_TYPE=
    Global $boot_type = read_var_from_base ("BOOT_TYPE=", $dummy)
    If $boot_type = "" then
        fli4lbuild_errmsg ($MSG_02_undefboottype,1)
        $bool_error = "true"
        exit_fli4l_build ()
    EndIf

    ;read passwd from <config>/base.txt
    read_var_from_base ("PASSWORD=", $sshpasswd)

    ;replace all "\" to "/" in build_dir - tar has a Problem when in string "\r" or "\R"
    Global $dir_build_r = StringReplace($dir_build, "\","/")
EndFunc

; ---
Func parse_cmd_default()
    ; Variabel with defaults
    ; logic - boolean
    Global $bool_error = "false"
    Global $bool_filesonly = "false"
    Global $bool_cleanup = "false"
    Global $bool_imonc = "false"
    Global $bool_interactive = "true"
    Global $bool_squeeze = "true"
    Global $bool_do_hdinstall = "false"
    Global $bool_force_select_config = "false"
    Global $bool_test_version = "false"
    ; pathnames
    Global $dir_config = "config"
    Global $dir_build = "build"
    Global $dir_image = ""
    ; type of opt [integrated/attached/bzip2/tar]
    Global $opt_type
    ; others
    Global $drive_hdinstall = ""
    ;scp - remoteupdate
    Global $remoteusername = "fli4l"
    Global $remotehostname
    Global $remotepathname = "/boot"
    Global $remoteupdate = "false"
    Global $remoteport = 22
    Global $sshpasswd = ""
    Global $sshkeyfile = ""
    Global $mkfli4l_debug_option = ""
    ; tftpboot
    Global $tftpboot_path = "/tftpboot"
    Global $tftpboot_image = "fli4l.tftp"
    Global $pxe_subdir = "fli4l"
EndFunc

Func parse_cmd_option ()
    $bool_interactive = "true"
    For $i = 1 to $CmdLine[0]
        Select
            Case $CmdLine[$i] = "-t"
                $bool_test_version = "true"
            Case $CmdLine[$i] = "-v" OR $CmdLine[$i] = "--verbose"
                $verbose = "true"
            Case $CmdLine[$i] = "-b" OR $CmdLine[$i] = "--build"
                If ($i + 1) <= $CmdLine[0] Then
                    $dir_build = $CmdLine[$i + 1]
                    If StringRight($dir_build, 1) = "\" Then
                        $dir_build = StringTrimRight ($dir_build, 1)
                    EndIf
                    $i = $i + 1
                Else
                    MsgBox(0,"Debug","Error --build at end")
                    ; todo - error-msg
                EndIf
            Case $CmdLine[$i] = "-c" OR $CmdLine[$i] = "--clean"
                $bool_cleanup = "true"
            Case $CmdLine[$i] = "-h" OR $CmdLine[$i] = "--help"
                usage()
            Case $CmdLine[$i] = "--filesonly"
                $bool_filesonly = "true"
            Case $CmdLine[$i] = "--remotehost"
                If ($i + 1) <= $CmdLine[0] Then
                    $remotehostname = $CmdLine[$i + 1]
                    $i = $i + 1
                Else
                    MsgBox(0,"Debug","Error --remotehost at end")
                    ; todo - error-msg
                EndIf
            Case $CmdLine[$i] = "--remoteuser"
                If ($i + 1) <= $CmdLine[0] Then
                    $remoteusername = $CmdLine[$i + 1]
                    $i = $i + 1
                Else
                    MsgBox(0,"Debug","Error --remoteuser at end")
                    ; todo - error-msg
                EndIf
            Case $CmdLine[$i] = "--remotepath"
                If ($i + 1) <= $CmdLine[0] Then
                    $remotepathname = $CmdLine[$i + 1]
                    $i = $i + 1
                Else
                    MsgBox(0,"Debug","Error --remotepath at end")
                    ; todo - error-msg
                EndIf
            Case $CmdLine[$i] = "--remoteport"
                If ($i + 1) <= $CmdLine[0] Then
                    $remoteport = $CmdLine[$i + 1]
                    $i = $i + 1
                Else
                    MsgBox(0,"Debug","Error --remoteport at end")
                    ; todo - error-msg
                EndIf
            Case $CmdLine[$i] = "--remoteupdate"
                $remoteupdate = "true"
            Case $CmdLine[$i] = "--imonc"
                $bool_imonc = "true"
            Case $Cmdline[$i] = "--nogui"
                $bool_interactive = "false"
            Case $Cmdline[$i] = "--no-squeeze"
                $bool_squeeze = "false"
            Case $Cmdline[$i] = "--force-select-config"
                $bool_force_select_config = "true"
            Case $Cmdline[$i] = "--lang"
                If ($i + 1) <= $CmdLine[0] Then
                    $i = $i + 1
                Else
                    MsgBox(0,"Debug","Error --lang at end")
                    ; todo - error-msg
                EndIf
            Case Else
                $dir_config = $CmdLine[$i]
                If StringRight($dir_config, 1) = "\" then
                    $dir_config = StringTrimRight ($dir_config, 1)
                EndIf
        EndSelect
    Next
    If $bool_imonc = "true" then
        $bool_interactive = "false"
    EndIf

    ; make path absolute
    $dir_config=make_abs($dir_config)
    $dir_build=make_abs($dir_build)

    ;check for the hostname of the fli4l-router if none is parsed from the command-line
    If FileExists ($dir_config) then
        If FileExists ($dir_config & "\base.txt") then
            If $remotehostname = "" then
                $remotehostname = read_var_from_base ("HOSTNAME=", $dummy)
            EndIf
        EndIf
        If FileExists ($dir_config & "\mkfli4l.txt") then
            $tmp = read_var_from_config_file ("mkfli4l.txt", "DISABLE_OPTION_GUI=", $dummy)
            If $tmp = "yes" then
                $bool_interactive = "false"
            EndIf
        EndIf
    EndIf
EndFunc

Func make_abs ($path)
    ; make $path absolute
    If StringLeft ($path,1) = "\" then
        $path = $drive_base & $path
    Else
        If StringMid ($path, 2, 1) <> ":" then
            $path = $dir_base & "\" & $path
        EndIf
    EndIf
    Return $path
EndFunc

Func usage ()
    $win_dlg_mkfli4l_conf = GUICreate("fli4l-Build - Usage",500,380,-1,-1, _
    BitOr ($WS_POPUPWINDOW,$WS_CAPTION), -1 ,$win_fli4l_build)
    GUISetIcon("windows\fli4l.ico")
    GUISetState(@SW_SHOW)

    $l1 = 30
    $l2 = 45
    $l3 = 134
    GUICtrlCreateLabel ("mkfli4l.bat [options] [config-dir] ", 10, 20, 480)
    GUICtrlSetFont (-1, 10, 400)

    GUICtrlCreateGroup ("[options]", 10, 50, 480, 230)

    GUICtrlCreateLabel ("-c,", $l1, 70)
    GUICtrlCreateLabel ("--clean", $l2, 70)
    GUICtrlCreateLabel ("cleanup the build-directory", $l3, 70)

    GUICtrlCreateLabel ("-b,", $l1, 90)
    GUICtrlCreateLabel ("--build <dir>", $l2, 90)
    GUICtrlCreateLabel ("set build-directory to <dir> for the fli4l-files", $l3, 90)

    GUICtrlCreateLabel ("-v,", $l1, 110)
    GUICtrlCreateLabel ("--verbose", $l2, 110)
    GUICtrlCreateLabel ("verbose - some debug-output", $l3, 110)

    GUICtrlCreateLabel ("", $l1, 130)
    GUICtrlCreateLabel ("--filesonly", $l2, 130)
    GUICtrlCreateLabel ("create only fli4l-files - do not create a disk or do an update", $l3, 130)

    GUICtrlCreateLabel ("", $l1, 150)
    GUICtrlCreateLabel ("--no-squeeze", $l2, 150)
    GUICtrlCreateLabel ("don't compress shell scripts", $l3, 150)

    GUICtrlCreateLabel ("", $l1, 170)
    GUICtrlCreateLabel ("--nogui", $l2, 170)
    GUICtrlCreateLabel ("disable the dialog to change options", $l3, 170)

    GUICtrlCreateLabel ("", $l1, 190)
    GUICtrlCreateLabel ("--lang", $l2, 190)
    GUICtrlCreateLabel ("change language [deutsch|english|french]", $l3, 190)

    GUICtrlCreateLabel ("-h,", $l1, 220)
    GUICtrlCreateLabel ("--help", $l2, 220)
    GUICtrlCreateLabel ("display this usage", $l3, 220)

    GUICtrlCreateLabel ("[config-dir]", $l1, 270)
    GUICtrlCreateLabel ("set other config-directory - default is 'config'", $l3, 270)

    $_b_OK = GUICtrlCreateButton ("OK", 120, 300, 140, 25 , $BS_DEFPUSHBUTTON)
    While 1
        $msg = GUIGetMsg()
        If $msg = $GUI_EVENT_CLOSE OR $msg = $_b_OK Then
            ExitLoop
        EndIf
    Wend
    GUIDelete()
    $bool_error = "false"
    exit_fli4l_build ()
EndFunc
