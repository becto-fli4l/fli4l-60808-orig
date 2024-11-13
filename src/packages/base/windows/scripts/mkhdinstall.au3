; -------------------------------------------------------------------------
; creates a direct HD-Install                                  __FLI4LVER__
;
; Creation: lanspezi 2005-09-11
; $Id$
; -------------------------------------------------------------------------
#include-once

Func mkhdinstall ()
    msg_lang_read ("mkhdinstall")

    Dim $res1
    Dim $bool_error = "false"
    Dim $bool_usercancel = "false"
    Dim $syslinux_option = ""

    $progressbar = GUICtrlCreateProgress (70,430,520,20)
    GUICtrlSetData ($progressbar,0)
    $icon = GUICtrlCreateIcon ("windows\scripts\ico_build.ico",-1, 20,422,32,32)

    $message = @CRLF & $MSG_LINE & @CRLF & $MSG_09_start & " "
    fli4lbuild_msg ($message ,1)

    $drive_fs = DriveGetFileSystem ($drive_hdinstall)
    If StringLeft ($drive_fs, 3) <> "FAT" then
        $msg = $MSG_09_wrongfs1 & " (" & $drive_hdinstall & ") " & $MSG_09_wrongfs2
        fli4lbuild_errmsg ($msg,1)
        $bool_error = "true"
    EndIf

    If $bool_error = "false" then
        fli4lbuild_msg (@CRLF & $MSG_09_writesystem & " " ,0)
        If Not boot_loader_install ($arch, $boot_type, $drive_hdinstall, "") Then
            $bool_error = "true"
            ;ERROR on syslinux
        else
            fli4lbuild_msg ($MSG_finished,0)

            For $_n = 0 To UBound ($file_list) - 1
                fli4lbuild_msg (@CRLF & $file_list[$_n][1] & " " ,0)
                Local $entry = $file_list[$_n][1]

                Local $lastslash = StringInStr ($entry, "\", -1)
                If $lastslash > 0 Then
                    Local $dir = StringLeft ($entry, $lastslash - 1)
                    fli_DirCreate ($drive_hdinstall & "\" & $dir)
                EndIf

                Local $source = $dir_image & "\" & $entry
                If StringInStr (FileGetAttrib ($source), "D") > 0 Then
                    $res1 = DirCopy ($source, $drive_hdinstall & "\" & $entry, 1)
                Else
                    $res1 = FileCopy ($source, $drive_hdinstall & "\" & $entry, 1)
                EndIf

                If $res1 = -1 then
                    MsgBox (0, $MSG_msgbox_error, $MSG_09_errorcopy & " " & $entry)
                    $bool_error = "true"
                    ExitLoop
                EndIf
                GUICtrlSetData ($progressbar,($_n + 1) * 20 - 10)
                fli4lbuild_msg ($MSG_finished,0)
            Next

            If not FileExists ($drive_hdinstall & "\hd.cfg") then
                FileWriteLine ($drive_hdinstall & "\hd.cfg", "hd_boot=sda1")
            EndIf
        EndIf
    EndIf
    If $bool_error = "false" AND $bool_usercancel = "false" then
        GUICtrlSetData ($progressbar,100)
        If $verbose = "true" then
            $msg = @CRLF & $MSG_09_finish1
        Else
            $msg = $MSG_finished
        EndIf
        $msg =  $msg & @CRLF & $MSG_LINE & @CRLF & $MSG_09_finish2 & @CRLF
        fli4lbuild_msg ($msg,1)
    EndIf

    If $bool_error = "true" then
        $msg = @CRLF & $MSG_09_error & @CRLF
        fli4lbuild_errmsg ($msg,0)
        fli4lbuild_msg ($msg,1)
    EndIf

    If $bool_usercancel = "true" then
        $msg = $MSG_09_cancel & @CRLF
        fli4lbuild_errmsg ($msg,0)
        fli4lbuild_msg ($MSG_cancel & @CRLF, 1)
    EndIf

    sleep (2000)
    GUICtrlDelete ($progressbar)
    GUICtrlDelete ($icon)

EndFunc
