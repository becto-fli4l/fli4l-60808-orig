; -------------------------------------------------------------------------
; creates fli4l-files and archives                             __FLI4LVER__
;
; Creation: lanspezi 2005-04-04
; $Id$
; -------------------------------------------------------------------------
#include-once

#include "bootloader.au3"

Dim $arch
Dim $kernel_version
Dim $bool_recover="no"

; array holding files/directories to be installed
; each entry consists of three strings:
; - the source file (qualified)
; - the target file (unqualified, as $dir_image\ is prepended automatically)
; - 1 if checksum should be computed, 0 otherwise
Dim $file_list[3][3] ; array is resized if necessary

Func update_version ()
    ; not implement in windows-build
EndFunc

Func get_one_val ()
    ; windows build use "read_var_from_rc_cfg"
EndFunc

Func gen_md5_sums (Const ByRef $entries)
    $msg = build_time () &  $MSG_04_generating_md5sums & " "
    fli4lbuild_msg ($msg,0)
    FileCopy ($dir_base & "\windows\scripts\md5.pif", $glob_dir_tmpbuild, 1) ; needed for W9x
    For $_n = 0 to UBound ($entries) - 1
        If $entries[$_n][2] = 0 Then
            ContinueLoop
        EndIf

        $md5file = $entries[$_n][0]
        If FileExists($md5file) then
            $varname = "FLI4L_MD5_" & StringUpper (StringRegExpReplace ($entries[$_n][1], "[^a-zA-Z0-9_]", "_"))
            $run_call = StringFormat ('"%s\\windows\\md5sum.exe" %s > md5.tmp',$dir_base, $md5file)
            FileWriteLine ($glob_dir_tmpbuild & "\md5.bat", $run_call)
            $val = RunWait ($glob_dir_tmpbuild & "\md5.bat", $glob_dir_tmpbuild , @SW_HIDE)
            $line = FileReadLine ($glob_dir_tmpbuild & "\md5.tmp")
            $md5 = StringLeft ($line, 32)

            FileWriteLine ($glob_dir_tmpbuild & "\rc.cfg", $varname & "='" & $md5 & "'" & @LF)
            FileDelete ($glob_dir_tmpbuild & "\md5.bat")
            FileDelete ($glob_dir_tmpbuild & "\md5.tmp")
            $md5=""
        EndIf
    Next
    FileDelete ($glob_dir_tmpbuild & "\md5.pif")
    fli4lbuild_msg ($MSG_finished & @CRLF,0)
EndFunc

Func get_config_values ()
    Dim $comp_type_rootfs
    Dim $comp_type_opt
    Dim $kernel_version_id

    read_var_from_rc_cfg ("ARCH", $arch)
    read_var_from_rc_cfg ("KERNEL_VERSION", $kernel_version)
    $kernel_version_id = StringUpper(StringReplace(StringReplace($kernel_version, ".", "_"), "-", "_"))

    read_var_from_rc_cfg ("COMP_TYPE_ROOTFS", $comp_type_rootfs)
    read_var_from_rc_cfg ("COMP_TYPE_OPT", $comp_type_opt)

    If ("$remotehostname" = "") then
        read_var_from_rc_cfg ("HOSTNAME", $remotehostname)
    EndIf
    read_var_from_rc_cfg ("OPT_RECOVER", $bool_recover)
EndFunc

Func copy_kernel ()
    Dim $file

    $file = "opt\img\linux-" & $kernel_version & "\kernel"

    If FileExists ($dir_config & "\" & $file) then
        FileCopy ($dir_config & "\" & $file, $glob_dir_tmpbuild & "\kernel",1)
    Else
        If FileExists ($file) then
            FileCopy ($file, $glob_dir_tmpbuild & "\kernel",1)
        Else
            $msg = "Warning: Can't find " & $file & _
                "! Either use a different kernel version or a different compression method."
            fli4lbuild_errmsg ($msg, 1)
            $bool_error = "true"
            exit_fli4l_build ()
        EndIf
    EndIf
EndFunc

Func copy_dtbs ()
    Dim $dtbdir
    Dim $dtb

    Local $prefix = "opt\img\linux-" & $kernel_version
    Local $hSearch = FileFindFirstFile ($prefix & "\*.dtb")
    If $hSearch = -1 Then
        Return
    EndIf

    $dtbdir = $glob_dir_tmpbuild & "\dtbs"
    fli_DirCreate ($dtbdir)

    While 1
        Local $dtb = FileFindNextFile ($hSearch)
        If @error Then ExitLoop

        If FileExists ($dir_config & "\" & $prefix & "\" & $dtb) then
            FileCopy ($dir_config & "\" & $prefix & "\" & $dtb, $dtbdir & "\", 1)
        Else
            FileCopy ($prefix & "\" & $dtb, $dtbdir & "\", 1)
        EndIf
    WEnd

    FileClose ($hSearch)
EndFunc


Func mkopt ()
    Dim $file
    Dim $squeeze_file

    msg_lang_read ("mkopt")

    $msg = @CRLF & $MSG_LINE & @CRLF & build_time () & $MSG_04_start & " "
    fli4lbuild_msg ($msg,1)

    $progressbar = GUICtrlCreateProgress (70,425,520,20)
    GUICtrlSetData ($progressbar,0)
    $icon = GUICtrlCreateIcon ("windows\scripts\ico_build.ico",-1, 20,422,32,32)

    $msg = @CRLF & build_time () & $MSG_04_confdir & $dir_config & "' " & @CRLF & _
    build_time () & $MSG_04_builddir & $dir_build & "'" & @CRLF
    fli4lbuild_msg ($msg,0)
    GUICtrlSetData ($progressbar,25)

    If FileExists ($glob_dir_tmpbuild & "\modules.alias") then
        FileDelete ($glob_dir_tmpbuild & "\modules.alias")
    EndIf
    If FileExists ($glob_dir_tmpbuild & "\modules.dep") then
        FileDelete ($glob_dir_tmpbuild & "\modules.dep")
    EndIF

    ; remove old version of all fli4l-files in build-dir
    cleanup_fli4lfiles ()

    If NOT FileExists ($dir_config & "\etc") then
        fli_DirCreate ($dir_config & "\etc")
    EndIf
    If NOT FileExists ($dir_config & "\etc\rc.cfg") then
        $file = FileOpen ($dir_config & "\etc\rc.cfg",2)
        FileClose($file)
    EndIf

    fli_DirCreate ($dir_image)

    ; if bool_squeeze ist true files are squeezed(compressed)
    If $bool_squeeze <> "true" then
        $squeeze_file = "--no-squeeze"
    Else
        $squeeze_file = ""
    EndIf

    ;parse config-files - check configuration and generate filelist for archives
    $msg = build_time () & $MSG_04_readcheckconf  & @CRLF
    fli4lbuild_msg ($msg,0)
    $run_call = StringFormat ('windows\\mkfli4l.exe %s -c "%s" -t "%s" -b "%s" -l "%s\\mkfli4l.log" %s', _
            $mkfli4l_debug_option, $dir_config, $glob_dir_tmptar, $glob_dir_tmpbuild, $glob_dir_tmplog, $squeeze_file)
    $val = RunWait ($run_call, @WorkingDir, @SW_HIDE)

    GUICtrlSetData ($progressbar,60)
    If $val = 0 then
        ; fli4l-config OK and Archives was build
        $message = read_mkfli4l_log ()
        $wline = "---------------------------------------------------------"
        $wline = $wline & "---------------------------------------------------------------------------"

        $warn = StringInStr ($message, "Warning:")
        If Stringlen($message)>60000 Then
            $message = StringLeft($message, 60000) & @CRLF & @CRLF & "Log too large to show, use log-file: '" & $dir_build & "\mkfli4l.log'" & @CRLF
        EndIf

        If $warn AND $verbose = "false" Then
            fli4lbuild_msg (@CRLF & $wline & @CRLF& $message & $wline & @CRLF,1)
        Else
            If $warn Then
                fli4lbuild_msg (@CRLF & $wline & @CRLF & $message & $wline & @CRLF,0)
            EndIf
        EndIf

        FileCopy ( $dir_config & "\etc\rc.cfg", $glob_dir_tmpbuild & "\rc.cfg", 1)
        fli4lbuild_msg (build_time () & $MSG_finished & @CRLF,0)

        get_config_values ()

        ;copy kernel defined by config to build-dir
        copy_kernel ()
        GUICtrlSetData ($progressbar,75)

        copy_dtbs ()
        GUICtrlSetData ($progressbar,80)

        ;determine entries to copy
        $file_list[0][0] = $glob_dir_tmpbuild & "\rc.cfg"
        If $bool_test_version = "false" then
            $file_list[0][1] = "rc.cfg"
        Else
            $file_list[0][1] = "rc-test.cfg"
        Endif
        $file_list[0][2] = 1

        $file_list[1][0] = $glob_dir_tmpbuild & "\kernel"
        If $bool_test_version = "false" then
            $file_list[1][1] = "kernel"
        Else
            $file_list[1][1] = "kernel3"
        Endif
        $file_list[1][2] = 1

        $file_list[2][0] = $glob_dir_tmpbuild & "\rootfs.img"
        If $bool_test_version = "false" then
            $file_list[2][1] = "rootfs.img"
        Else
            $file_list[2][1] = "rootfs3.img"
        Endif
        $file_list[2][2] = 1

        If $bool_recover <> "no" then
            Local $num = UBound ($file_list)
            ReDim $file_list[$num + 3][3]
            $file_list[$num    ][0] = $dir_base & "\img\boot.msg"
            $file_list[$num    ][1] = "boot.msg"
            $file_list[$num    ][2] = 0
            $file_list[$num + 1][0] = $dir_base & "\img\boot_s.msg"
            $file_list[$num + 1][1] = "boot_s.msg"
            $file_list[$num + 1][2] = 0
            $file_list[$num + 2][0] = $dir_base & "\img\boot_z.msg"
            $file_list[$num + 2][1] = "boot_z.msg"
            $file_list[$num + 2][2] = 0
        EndIf
        If FileExists ($glob_dir_tmpbuild & "\opt.img") Then
            Local $num = UBound ($file_list)
            ReDim $file_list[$num + 1][3]
            $file_list[$num][0] = $glob_dir_tmpbuild & "\opt.img"
            If $bool_test_version = "false" then
                $file_list[$num][1] = "opt.img"
            Else
                $file_list[$num][1] = "opt-test.img"
            Endif
            $file_list[$num][2] = 1
        EndIf
        If FileExists ($glob_dir_tmpbuild & "\dtbs") Then
            Local $num = UBound ($file_list)
            ReDim $file_list[$num + 1][3]
            $file_list[$num][0] = $glob_dir_tmpbuild & "\dtbs"
            $file_list[$num][1] = "dtbs"
            $file_list[$num][2] = 0
        EndIf

        ;copy all files/directories to build dir
        If NOT boot_loader_post_build ($arch, $glob_dir_tmpbuild) Then
            ; TODO: localize!
            fli4lbuild_errmsg ("Error while doing boot loader post-processing!", 1)
            $bool_error = "true"
        Else
            boot_loader_get_entries ($arch, $glob_dir_tmpbuild, $file_list)

            ;generating md5sum of files
            gen_md5_sums ($file_list)

            For $_n = 0 To UBound ($file_list) - 1
                Local $lastslash = StringInStr($file_list[$_n][1], "\", -1)
                If $lastslash > 0 Then
                    Local $dir = StringLeft($file_list[$_n][1], $lastslash - 1)
                    fli_DirCreate($dir_image & "\" & $dir)
                EndIf

                If StringInStr (FileGetAttrib ($file_list[$_n][0]), "D") > 0 Then
                    DirCopy ($file_list[$_n][0], _
                             $dir_image & "\" & $file_list[$_n][1], _
                             1)
                Else
                    FileCopy ($file_list[$_n][0], _
                              $dir_image & "\" & $file_list[$_n][1], _
                              1)
                EndIf
            Next

            If $verbose = "true" then
                $msg = $MSG_04_finished & @CRLF & $MSG_LINE & @CRLF
            Else
                $msg = $MSG_finished & @CRLF & $MSG_LINE & @CRLF
            EndIf
            fli4lbuild_msg (build_time () & $msg,1)
            GUICtrlSetData ($progressbar,100)
            WinActivate("fli4l-Build","")
        EndIf
    else
        ;error in check of fli4l-config or build of fli4l archiv-files
        fli4lbuild_msg ($MSG_cancel & @CRLF , 1)
        $msg = read_mkfli4l_log ()
        fli4l_log_error ($MSG_04_msgboxerror & @CRLF & $MSG_LINE & @CRLF)
        fli4l_log_error ($msg)

        If StringLen ($msg) > 1500 then
            $msg1 = StringLeft ($msg, 1500)
            $msg1 = $msg1 & @CRLF & @CRLF & "<Skip Output> - please read complete output in "
            If $verbose ="true" Then
                $msg1 = $msg1 & "main window or in "
            EndIf
            $msg1 = $msg1 & "log-file!"
        Else
            $msg1=$msg
        EndIf

        MsgBox(48 + 4096, $MSG_04_msgboxerror, $msg1)

        If Stringlen($msg)>60000 Then
            $msg=StringLeft($msg, 60000) & @CRLF & @CRLF & "Log too large to show, use log-file: '" & $dir_build & "\mkfli4l_error.log'" & @CRLF
        EndIf

        fli4lbuild_msg ($MSG_LINE & @CRLF & $msg,0) ; show errormessage in build-window if verbose
        $bool_error = "true"
    EndIf

    sleep (2000)
    GUICtrlDelete ($progressbar)
    GUICtrlDelete ($icon)
EndFunc

; -------------------------------
; Functions only in windows-build
; -------------------------------

Func read_mkfli4l_log ()
    Dim $msg , $_msg , $file
    if FileExists ($glob_dir_tmplog & "\mkfli4l.log") Then
        $file = FileOpen($glob_dir_tmplog & "\mkfli4l.log", 0)
        ; Check if file opened for reading OK
        If $file = -1 Then
            $msg = $MSG_04_nologfile & @CRLF & "'" & $glob_dir_tmpbuild & "\mkfli4l.log'."
            MsgBox(0, $MSG_msgbox_error, $msg)
            FileClose($file)
            Return $msg
        Else
            ; Read in lines of text until the EOF is reached
            While 1
                $_msg = FileReadLine($file)
                ; abort on EOF, strip unnessecary mkfli4l output
                If (@error = -1) or (StringLeft ($_msg,22) = "Error in configuration") or (StringLeft ($_msg,5) = "total") or (StringLeft ($_msg,14) = "         total") Then ExitLoop
                ; strip empty lines
                if $_msg <> "" Then $msg = $msg & $_msg & @CRLF
            Wend
            FileClose($file)
            Return $msg
        EndIf
        FileClose($file)
    Else
        Return ""
    EndIf
EndFunc

Func read_var_from_rc_cfg ($var, ByRef $parameter)
    Dim $result
    Dim $line
    Dim $val
    Dim $file
    Dim $len
    Dim $end
    $len = StringLen($var)
    $file = FileOpen($glob_dir_tmpbuild & "\rc.cfg", 0)
    ; Check if file opened for reading OK
    If $file = -1 Then
        MsgBox(0, "Error", "Unable to open file '"& $glob_dir_tmpbuild & "\rc.cfg")
    else
        ; Read in lines of text until the EOF is reached
        While 1
            $line = FileReadLine($file)
            If @error = -1 Then ExitLoop
            If StringLeft($line, $len) = $var then
                $end = StringInStr($line, "'", 1, 2)
                $result = StringMid ($line, $len + 3, $end - $len - 3)
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
