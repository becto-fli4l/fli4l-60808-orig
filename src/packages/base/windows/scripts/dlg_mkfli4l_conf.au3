; -------------------------------------------------------------------------
; GUI to change build-settings interactive                     __FLI4LVER__
;
; Creation: lanspezi 2005-04-09
; $Id$
; -------------------------------------------------------------------------
#include-once

Dim $_i_dir_config
Dim $_i_dir_build
Dim $_cb_verbose
Dim $_cb_filesonly
Dim $_cb_remoteupdate
Dim $_i_remotehostname
Dim $_i_remoteusername
Dim $_i_sshkeyfile
Dim $_i_remotepathname
Dim $_i_remoteport
Dim $_sshkeyfile
Dim $_lockfilesonly
Dim $boot_type
Dim $_i_boot_type
Dim $bool_hdinstall_enable
Dim $_cb_hdinstall
Dim $_c_hddrive

Func dlg_mkfli4l_conf ()
    Dim $win_dlg_mkfli4l_conf
    Dim $_dir_config
    Dim $_force_changedirconfig
    $_force_changedirconfig = 0

    msg_lang_read ("dlg_mkfli4l_conf")

    parse_mkfli4l_conf (0)
    if $bool_error = "false" Then
        parse_cmd_option ()

        $_dir_config = $dir_config
        $_dir_build = $dir_build
        $_sshkeyfile = $sshkeyfile

        If (is_dual_screen () = 0 ) then
            $win_dlg_mkfli4l_conf = GUICreate("fli4l-Build - Options",525,390,-1,-1, _
                BitOr ($WS_POPUPWINDOW,$WS_CAPTION), -1 ,$win_fli4l_build)
        Else
            $win_dlg_mkfli4l_conf = GUICreate("fli4l-Build - Options",525,390,15,15, _
                BitOr ($WS_POPUPWINDOW,$WS_CAPTION), -1 ,$win_fli4l_build)
        EndIf
        GUISetIcon("windows\fli4l.ico")
        GUISetState(@SW_SHOW)

        GUICtrlCreateLabel ("Config-Dir:", 10, 20, 50)
        $_i_dir_config = GUICtrlCreateInput ($_dir_config, 80, 15, 360)
        GUICtrlSetState($_i_dir_config,$GUI_DISABLE)
        $_b_dir_config = GUICtrlCreateButton ("...", 450, 15, 20, 20)
        GUICtrlSetTip(-1, $MSG_08TT_btnconfigdir)
        GUICtrlCreateLabel ($MSG_08_dirconfiginfo1 & @CRLF & $MSG_08_dirconfiginfo2, 10,50,480,29)
        GUICtrlSetColor(-1, 0x990000)

        $tab=GUICtrlCreateTab (10,100, 505, 180)

        $tab0=GUICtrlCreateTabitem ("   Build   ")
        GUICtrlCreateLabel ("Build-Dir:", 20, 145, 50)
        $_i_dir_build = GUICtrlCreateInput ($_dir_build, 110, 140, 360)
        GUICtrlSetState($_i_dir_build,$GUI_DISABLE)
        $_b_dir_build = GUICtrlCreateButton ("...", 480, 140, 20, 20)
        GUICtrlSetTip(-1, $MSG_08TT_btnbuilddir)

        If FileExists ($dir_config & "\base.txt") then
            $boot_type = read_var_from_base ("BOOT_TYPE=",$dummy)
        else
            $boot_type = ""
        EndIf
        GUICtrlCreateLabel ("BOOT_TYPE:", 20,175,100)
        $_i_boot_type = GUICtrlCreateInput ($boot_type, 110,170,80)
        GUICtrlSetState($_i_boot_type,$GUI_DISABLE)

        GUICtrlCreateLabel ("Verbose:", 20, 205, 60)
        $_cb_verbose = GUICtrlCreateCheckbox ("", 110, 200, 20, 20)
        GUICtrlSetTip(-1, $MSG_08TT_verbose)

        GUICtrlCreateLabel ("Filesonly:", 280, 175, 60)
        $_cb_filesonly = GUICtrlCreateCheckbox ("", 370, 170, 20, 20)
        GUICtrlSetTip(-1, $MSG_08TT_filesonly)

        GUICtrlCreateLabel ("Remoteupdate:", 280, 205, 80)
        $_cb_remoteupdate = GUICtrlCreateCheckbox ("", 370, 200, 20, 20)
        GUICtrlSetTip(-1, $MSG_08TT_remoteupdate)
        GUICtrlCreateIcon ("windows\fli4l.ico",-1, 470,240,32,32)

        $tab2=GUICtrlCreateTabitem ( "   Remoteupdate   ")
        GUICtrlCreateLabel ("RemoteHostname:", 20, 145, 90)
        $_i_remotehostname = GUICtrlCreateInput ($remotehostname, 120, 140, 120)
        GUICtrlSetTip(-1, $MSG_08TT_remotehostname)
        GUICtrlCreateLabel ("RemoteUsername:", 20, 175, 90)
        $_i_remoteusername = GUICtrlCreateInput ($remoteusername, 120, 170, 120)
        GUICtrlSetTip(-1, $MSG_08TT_remoteusername)
        GUICtrlCreateLabel ("RemotePath:", 280, 145, 90)
        $_i_remotepathname = GUICtrlCreateInput ($remotepathname, 380, 140, 120)
        GUICtrlSetTip(-1, $MSG_08TT_remotepathname)
        GUICtrlCreateLabel ("RemotePort:", 280, 175, 90)
        $_i_remoteport = GUICtrlCreateInput ($remoteport, 380, 170, 120)
        GUICtrlSetTip(-1, $MSG_08TT_remoteport)
        GUICtrlCreateLabel ("SSH-Keyfile:", 20, 205, 90)
        $_i_sshkeyfile = GUICtrlCreateInput ($_sshkeyfile, 120, 200, 320)
        GUICtrlSetState($_i_sshkeyfile,$GUI_DISABLE)
        $_b_sshkeyfile = GUICtrlCreateButton ("...", 455,200,20,20)
        GUICtrlSetTip(-1, $MSG_08TT_btn_sshfile)
        $_b_sshkeyfileclear = GUICtrlCreateButton ("c", 480,200,20,20)
        GUICtrlSetTip(-1, $MSG_08TT_btn_sshfileclear)
        GUICtrlCreateIcon ("windows\scripts\ico_scp.ico",-1, 470,240,32,32)

        ;$tab3=GUICtrlCreateTabitem ( "   Netboot / PXEboot   ")
        ;GUICtrlCreateLabel ("Not available / Nicht verfügbar / Niet beschikbaar",120,170)

        $tab4=GUICtrlCreateTabitem ( "   HD-pre-install   ")
        Global $drive_list = ""
        $drive_list_get = DriveGetDrive( "REMOVABLE" )
        If NOT @error Then
           For $i = 1 to $drive_list_get[0]
               If $drive_list_get[$i] <> "a:" and $drive_list_get[$i] <> "b:" then
                   $drive_list = $drive_list & "|" & $drive_list_get[$i]
               EndIf
           Next
        EndIf
        GUICtrlCreateLabel ("Activate HD-Install:", 20, 145, 120)
        $_cb_hdinstall = GUICtrlCreateCheckbox ("", 150, 140, 20, 20)
        ;GUICtrlSetTip(-1, $MSG_08TT_hdinstall)
        GUICtrlCreateLabel ("Drive for HD-Install:", 20, 175, 120)
        $_c_hddrive = GUICtrlCreateCombo ("", 150,170,60,100)
        GUICtrlSetData($_c_hddrive ,$drive_list,StringRight($drive_list,2))
        ;GUICtrlSetTip(-1, $MSG_08TT_hddrive)
        GUICtrlCreateIcon ("windows\scripts\ico_build.ico",-1, 470,240,32,32)

        If $drive_list = "" then
            $bool_hdinstall_enable = "false"
        Else
            $bool_hdinstall_enable = "true"
        EndIf
        GUICtrlCreateTabitem ("")   ; end tabitem definition
        GUICtrlSetState($tab0,$GUI_SHOW)

        $_b_savemkfli4lconf = GUICtrlCreateButton ($MSG_08_savemkfli4lconf, 100,300,325,20)
        $_b_abort = GUICtrlCreateButton ($MSG_08_btnabort, 25, 345, 200, 25)
        $_b_build = GUICtrlCreateButton ($MSG_08_btnbuild, 300, 345, 200, 25, $BS_DEFPUSHBUTTON)

        check_boottype ()
        parse_mkfli4l_conf (1)
        If @OSType <> "WIN32_NT" Then      ; Workaround mouseproblem W9x
            WinSetState("fli4l-Build - Options","",@SW_HIDE)
            WinSetState("fli4l-Build - Options","",@SW_SHOW)
        EndIf
        parse_cmd_option ()
        set_data_to_controls ()
        $_filesonly_start = $bool_filesonly

        ; check existance of <dir_config>/base.txt and enable or disable Start-button
        If NOT FileExists ($_dir_config & "\base.txt") then
            GUICtrlSetState($_b_build,$GUI_DISABLE)
            $_force_changedirconfig = 1
        EndIf

        If NOT FileExists ($_dir_config) then
            $_force_changedirconfig = 1
        EndIf

        If $bool_force_select_config = "true" then
            $_force_changedirconfig = 1
        EndIf

        Dim $_bool_filesonly = -1
        Dim $_remoteupdate_state_old = -1
        Dim $_filesonly_state_old = -1
        Dim $hdinstall_state_old = -1

        While 1

            If $bool_hdinstall_enable = "true" and ( $boot_type = "hd" or $boot_type = "ls120" ) then
                $hdinstall_state = GUICtrlRead ($_cb_hdinstall)
                If $hdinstall_state <> $hdinstall_state_old then
                   If $hdinstall_state = 1 then
                       GUICtrlSetState($_c_hddrive,$GUI_ENABLE)
                       GUICtrlSetState($_cb_remoteupdate,$GUI_UNCHECKED)
                    Else
                       GUICtrlSetState($_c_hddrive,$GUI_DISABLE)
                    EndIf
                    $hdinstall_state_old = $hdinstall_state
                EndIf
            EndIf

            $_remoteupdate_state = GUICtrlRead ($_cb_remoteupdate)
            if $_remoteupdate_state <> $_remoteupdate_state_old then
                $_remoteupdate_state_old = $_remoteupdate_state
                if $_remoteupdate_state = 1 then
                    ;checked
                    GUICtrlSetState($_i_remotehostname,$GUI_ENABLE)
                    GUICtrlSetState($_i_remoteusername,$GUI_ENABLE)
                    GUICtrlSetState($_b_sshkeyfile,$GUI_ENABLE)
                    GUICtrlSetState($_b_sshkeyfileclear,$GUI_ENABLE)
                    GUICtrlSetState($_i_remotepathname,$GUI_ENABLE)
                    GUICtrlSetState($_i_remoteport,$GUI_ENABLE)
                    If $_lockfilesonly = "false" Then
                        GUICtrlSetState($_cb_filesonly,$GUI_DISABLE)
                        GUICtrlSetState($_cb_filesonly,$GUI_CHECKED)
                        $bool_filesonly = "true"
                    EndIf
                    GUICtrlSetState($tab2,$GUI_SHOW)
                else
                    ;unchecked
                    GUICtrlSetState($_i_remotehostname,$GUI_DISABLE)
                    GUICtrlSetState($_i_remoteusername,$GUI_DISABLE)
                    GUICtrlSetState($_b_sshkeyfile,$GUI_DISABLE)
                    GUICtrlSetState($_b_sshkeyfileclear,$GUI_DISABLE)
                    GUICtrlSetState($_i_remotepathname,$GUI_DISABLE)
                    GUICtrlSetState($_i_remoteport,$GUI_DISABLE)
                    If $_lockfilesonly = "false" Then
                        GUICtrlSetState($_cb_filesonly,$GUI_ENABLE)
                        GUICtrlSetState($_cb_filesonly,$GUI_UNCHECKED)
                        $bool_filesonly = "false"
                    EndIf
                EndIf
            EndIf

            If $_filesonly_start <> "" then
                $bool_filesonly = $_filesonly_start
                $_filesonly_start = ""
                If $bool_filesonly = "true" then
                    GUICtrlSetState($_cb_filesonly,$GUI_CHECKED)
                EndIf
            EndIf

            $_filesonly_state = GUICtrlRead ($_cb_filesonly)
            if $_filesonly_state <> $_filesonly_state_old then
                $_filesonly_state_old = $_filesonly_state
            EndIf

            $msg = GUIGetMsg()
            If $msg = $_b_dir_config OR $_force_changedirconfig = 1 then
                $_force_changedirconfig = 0
                GUISetState ( @SW_DISABLE , $win_dlg_mkfli4l_conf)
                $_dir_config_tmp = change_config_dir ($_dir_config)
                GUISetState ( @SW_ENABLE , $win_dlg_mkfli4l_conf)
                WinActivate("fli4l-Build - Options", "")
                if $_dir_config_tmp <> "" then
                    If FileExists ($_dir_config_tmp & "\base.txt") then
                        GUICtrlSetData ($_i_dir_config,$_dir_config_tmp)
                        $dir_config = $_dir_config_tmp
                        $_dir_config = $dir_config
                        parse_mkfli4l_conf (0)
                        If @OSType <> "WIN32_NT" Then      ; Workaround mouseproblem W9x
                            WinSetState("fli4l-Build - Options","",@SW_HIDE)
                            WinSetState("fli4l-Build - Options","",@SW_SHOW)
                        EndIf
                        WinActivate("fli4l-Build - Options", "")
                        parse_cmd_option ()
                        $_filesonly_start = $bool_filesonly
                        $_remoteupdate_tmp = $remoteupdate
                        $dir_config = $_dir_config_tmp
                        check_boottype ()
                        $remoteupdate = $_remoteupdate_tmp
                        set_data_to_controls ()
                        GUICtrlSetState($_b_build,$GUI_ENABLE)
                    else
                        MsgBox (0,$MSG_msgbox_error, $MSG_08_nobasetxt & @CRLF & $_dir_config)
                    EndIf
                EndIf
            EndIf
            If $msg = $_b_dir_build then
                GUISetState ( @SW_DISABLE , $win_dlg_mkfli4l_conf)
                $_dir_build_tmp = change_build_dir ($_dir_build)
                GUISetState ( @SW_ENABLE , $win_dlg_mkfli4l_conf)
                WinActivate("fli4l-Build - Options", "")
                sleep (1000)
                if $_dir_build_tmp <> "" then
                    If FileExists ($_dir_build_tmp & "\") then
                        GUICtrlSetData ($_i_dir_build,$_dir_build_tmp)
                        $_dir_build = $_dir_build_tmp
                    EndIf
                EndIf
            EndIf
            If $msg = $_b_sshkeyfile then
                $_my_workdir = @WorkingDir
                GUISetState ( @SW_DISABLE , $win_dlg_mkfli4l_conf)
                $_sshkeyfile_tmp = change_sshkeyfile ($_sshkeyfile)
                GUISetState ( @SW_ENABLE , $win_dlg_mkfli4l_conf)
                WinActivate("fli4l-Build - Options", "")
                sleep (1000)
                FileChangeDir($_my_workdir)
                If $_sshkeyfile_tmp <> "-1" then
                    GUICtrlSetData ($_i_sshkeyfile,$_sshkeyfile_tmp)
                    $_sshkeyfile = $_sshkeyfile_tmp
                EndIf
            EndIf
            If $msg = $_b_sshkeyfileclear then
                GUICtrlSetData ($_i_sshkeyfile,"")
                $_sshkeyfile = ""
            EndIf
            If $msg = $_b_savemkfli4lconf then
                use_data_for_build ()
                save_mkfli4lconf ()
            EndIf
            If $msg = $_b_build then
                If check_drive_for_hdinstall () then
                    ;set all data from dlg to vars
                    use_data_for_build ()
                    ;
                    ExitLoop
                EndIf
            EndIf
            If $msg = $GUI_EVENT_CLOSE OR $msg = $_b_abort Then
                $bool_build_abort = "true"
                ExitLoop
            EndIf
        Wend
        GUIDelete()
    EndIF
EndFunc

Func change_config_dir ($dir)
    $result = FileSelectFolder ( $MSG_08_changedirconfig, "" , 0 , $dir )
    If $result <> "" then
        If MsgBox( 4096+4, $MSG_08_changedconfigdir1, $MSG_08_changedconfigdir2) = 6 Then
 		    IniWrite("mkfli4l.ini", "userdefault", "configdir", $result)
 	    EndIf
    EndIf
    Return $result
EndFunc

Func change_build_dir ($dir)
    $result = FileSelectFolder ( $MSG_08_changedirbuild, "" , 2 , $dir )
    Return $result
EndFunc

Func change_sshkeyfile ($file)
    $result = FileOpenDialog ( $MSG_00_changesshkey,$file, "SSH-Keyfile (*.*)", 1+3)
    If @error then
        Return "-1"
    else
        Return $result
    EndIf
EndFunc

Func check_boottype ()
    If FileExists ($dir_config & "\base.txt") then
        $boot_type = read_var_from_base ("BOOT_TYPE=",$dummy)
    Else
        $boot_type = ""
    EndIf

    If $boot_type = "attached" or $boot_type = "integrated" then
        ; enable remoteupdate -> enable remoteupdate*
        GUICtrlSetState($_cb_remoteupdate,$GUI_ENABLE)
        ; unset remoteupdate
        $remoteupdate = "false"
    EndIf
    If $boot_type = "hd" then
        ; enable remoteupdate -> remotupdate*
        GUICtrlSetState($_cb_remoteupdate,$GUI_ENABLE)
        ; unset remoteupdate
        $remoteupdate = "false"
    EndIf
    If $boot_type = "cd" or $boot_type = "netboot" or $boot_type = "pxeboot" then
        ; disable remoteupdate -> disable remoteupdate*
        GUICtrlSetState($_cb_remoteupdate,$GUI_UNCHECKED)
        GUICtrlSetState($_cb_remoteupdate,$GUI_DISABLE)
        ; unset remoteupdate
        $remoteupdate = "false"
    EndIf
EndFunc

Func set_data_to_controls ()
    GUICtrlSetData ($_i_dir_build, $dir_build)
    If $verbose = "true" then
        GUICtrlSetState($_cb_verbose,$GUI_CHECKED)
    else
        GUICtrlSetState($_cb_verbose,$GUI_UNCHECKED)
    EndIf

    If $bool_filesonly = "true" then
        GUICtrlSetState($_cb_filesonly,$GUI_CHECKED)
    else
        GUICtrlSetState($_cb_filesonly,$GUI_UNCHECKED)
    EndIf

    If $remoteupdate = "true" then
        GUICtrlSetState($_cb_remoteupdate,$GUI_CHECKED)
    else
        GUICtrlSetState($_cb_remoteupdate,$GUI_UNCHECKED)
    EndIf
    GUICtrlSetData ($_i_remotehostname,$remotehostname)
    GUICtrlSetData ($_i_sshkeyfile,$sshkeyfile)
    GUICtrlSetData ($_i_remoteusername,$remoteusername)
    GUICtrlSetData ($_i_remotepathname,$remotepathname)
    GUICtrlSetData ($_i_remoteport,$remoteport)
    GUICtrlSetData ($_i_boot_type, $boot_type)

    If $bool_hdinstall_enable = "true" and ($boot_type = "hd" or $boot_type="integrated" or $boot_type="ls120" ) then
        GUICtrlSetState($_cb_hdinstall,$GUI_ENABLE)
        GUICtrlSetState($_c_hddrive,$GUI_ENABLE)
    Else
        GUICtrlSetState($_cb_hdinstall,$GUI_UNCHECKED)
        GUICtrlSetState($_cb_hdinstall,$GUI_DISABLE)
        GUICtrlSetState($_c_hddrive,$GUI_DISABLE)
    EndIf
EndFunc

Func use_data_for_build ()
    $tmp = GUICtrlRead ($_i_dir_build)
    If $tmp <> "" then
        $dir_build = $tmp
    EndIf
    If GUICtrlRead($_cb_verbose) = $GUI_UNCHECKED then
        $verbose = "false"
    else
        $verbose = "true"
    EndIf
    If GUICtrlRead($_cb_filesonly) = $GUI_UNCHECKED then
        $bool_filesonly = "false"
    else
        $bool_filesonly = "true"
    EndIf
    If GUICtrlRead ($_cb_remoteupdate) = $GUI_UNCHECKED then
        $remoteupdate = "false"
    else
        $remoteupdate = "true"
        $tmp = GUICtrlRead ($_i_remotehostname)
        If $tmp <> "" then
            $remotehostname = $tmp
        EndIf
        $tmp = GUICtrlRead ($_i_remoteusername)
        If $tmp <> "" then
            $remoteusername = $tmp
        EndIf
        $sshkeyfile = GUICtrlRead ($_i_sshkeyfile)
        $tmp = GUICtrlRead ($_i_remotepathname)
        If $tmp <> "" then
            $remotepathname = $tmp
        EndIf
        $tmp = GUICtrlRead ($_i_remoteport)
        If $tmp <> "" then
            $remoteport = $tmp
        EndIf
    EndIf
    $bool_do_hdinstall = "false"
    If GUICtrlRead ($_cb_hdinstall) = $GUI_CHECKED then
        $tmp = GUICtrlRead ($_c_hddrive)
        If $tmp <> "" then
            $drive_hdinstall = $tmp
            $bool_do_hdinstall = "true"
        EndIf
    EndIf
EndFunc

Func check_drive_for_hdinstall ()
    Dim $drive_list_system
    Dim $tmp
    Dim $res
    If GUICtrlRead ($_cb_hdinstall) = $GUI_CHECKED then
        $tmp = StringLeft (GUICtrlRead ($_c_hddrive),1)
        $drive_list_system = StringLeft (@AppDataCommonDir, 1) & StringLeft (@DesktopCommonDir, 1) & _
                             StringLeft (@DocumentsCommonDir, 1) & StringLeft (@FavoritesCommonDir, 1) & _
                             StringLeft (@ProgramsCommonDir, 1) & StringLeft (@StartMenuCommonDir, 1) & _
                             StringLeft (@StartupCommonDir, 1) & StringLeft (@AppDataDir, 1) & _
                             StringLeft (@DesktopDir, 1) & StringLeft (@MyDocumentsDir, 1) & _
                             StringLeft (@FavoritesDir, 1) & StringLeft (@ProgramsDir, 1) & _
                             StringLeft (@StartMenuDir, 1) & StringLeft (@StartupDir, 1) & _
                             StringLeft (@UserProfileDir, 1) & StringLeft (@HomeDrive, 1) & _
                             StringLeft (@ProgramFilesDir, 1) & StringLeft (@CommonFilesDir, 1) & _
                             StringLeft (@WindowsDir, 1) & StringLeft (@SystemDir, 1) & _
                             StringLeft (@TempDir, 1)
        If $tmp <> "" then
            If Not StringInStr($drive_list,$tmp,0) then
                If StringInStr($drive_list_system,$tmp,0) then
                     MsgBox (0,"Warning", "You have selected a drive with systemdata. Please select another drive for hd-install!")
                     Return false
                EndIf
                $res = MsgBox (1+64,"Warning", "The selected drive is not removable media! Are you sure, you want to continue hd-install on drive "& $tmp &": ?")
                If $res <> 1 then
                    Return false
                EndIf
            EndIf
        Else
            Return false
        EndIf
    EndIf
    Return true
EndFunc

Func save_mkfli4lconf ()
     $_error = 0
    ; first save old mkfli4l.txt
    If FileExists ($dir_config & "\mkfli4l.txt") then
        $_res = FileCopy ($dir_config & "\mkfli4l.txt", $dir_config & "\mkfli4l.txt.save",1)
        If $_res = 0 then
            $_error = 1
        EndIf
    EndIf
    If $_error = 0 then
        ; remove old mkfli4l.txt
        If FileExists ($dir_config & "\mkfli4l.txt") then
            $_res = FileDelete ($dir_config & "\mkfli4l.txt")
            If $_res = 0 then
                $_error = 1
            EndIf
        EndIf
    EndIf
    If $_error = 0 then
        $_res = FileWriteLine ($dir_config & "\mkfli4l.txt", "##-----------------------------------------------------------------------------")
        If $_res = 0 then
            $_error = 1
        EndIf
    EndIf
    If $_error = 0 then
        FileWriteLine ($dir_config & "\mkfli4l.txt", "## mkfli4l.txt - fli4l build parameters                            __FLI4LVER__")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "##")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "## Creation:  fli4l-windows-build")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "##-----------------------------------------------------------------------------")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "BUILDDIR='" & $dir_build & "'")
        If $verbose = "true" then
            FileWriteLine ($dir_config & "\mkfli4l.txt", "VERBOSE='yes'")
        Else
            FileWriteLine ($dir_config & "\mkfli4l.txt", "VERBOSE='no'")
        EndIf
        If $bool_filesonly = "true" then
            FileWriteLine ($dir_config & "\mkfli4l.txt", "FILESONLY='yes'")
        Else
            FileWriteLine ($dir_config & "\mkfli4l.txt", "FILESONLY='no'")
        EndIf
        If $remoteupdate = "true" then
            FileWriteLine ($dir_config & "\mkfli4l.txt", "REMOTEUPDATE='yes'")
        Else
            FileWriteLine ($dir_config & "\mkfli4l.txt", "REMOTEUPDATE='no'")
        EndIf
        FileWriteLine ($dir_config & "\mkfli4l.txt", "REMOTEHOSTNAME='" & $remotehostname & "'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "REMOTEUSERNAME='" & $remoteusername & "'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "REMOTEPATHNAME='" & $remotepathname & "'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "REMOTEPORT='" & $remoteport & "'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "SSHKEYFILE='" & $sshkeyfile & "'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "#TFTPBOOTPATH='/tftpboot'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "#TFTPBOOTIMAGE=''")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "#PXESUBDIR='fli4l'")
        FileWriteLine ($dir_config & "\mkfli4l.txt", "DISABLE_OPTION_GUI='no'")
        MsgBox (64,$MSG_msgbox_info, $MSG_08_savemkfli4ldone)
    Else
        MsgBox (16, $MSG_msgbox_error, $MSG_08_savemkfli4lerror)
    EndIf
EndFunc
