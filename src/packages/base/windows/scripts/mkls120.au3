; -------------------------------------------------------------------------
; creates a LS120-Floppy                                       __FLI4LVER__
;
; Creation: lanspezi 2005-09-11
; $Id$
; -------------------------------------------------------------------------
#include-once

Func mkls120 ()
    msg_lang_read ("mkls120")

    Dim $ls120files[8][3]
    $ls120files[0][0] = "\syslinux.cfg"
    $ls120files[0][1] = "\syslinux.cfg"
    $ls120files[0][2] = $MSG_10_copysyslinux
    $ls120files[1][0] = "\kernel"
    $ls120files[1][1] = "\kernel"
    $ls120files[1][2] = $MSG_10_copykernel
    $ls120files[2][0] = "\rootfs.img"
    $ls120files[2][1] = "\rootfs.img"
    $ls120files[2][2] = $MSG_10_copyrootfstgz
    $ls120files[3][0] = "\rc.cfg"
    $ls120files[3][1] = "\rc.cfg"
    $ls120files[3][2] = $MSG_10_copyrccfg
    $ls120files[4][0] = "\opt.img"
    $ls120files[4][1] = "\opt.img"
    $ls120files[4][2] = $MSG_10_copyopttarbz2
    $ls120files[5][0] = "\boot.msg"
    $ls120files[5][1] = "\boot.msg"
    $ls120files[5][2] = "BOOTMENU_MAIN"
    $ls120files[6][0] = "\boot_s.msg"
    $ls120files[6][1] = "\boot_s.msg"
    $ls120files[6][2] = "BOOTMENU_STD"
    $ls120files[7][0] = "\boot_z.msg"
    $ls120files[7][1] = "\boot_z.msg"
    $ls120files[7][2] = "BOOTMENU_END"
    
    Dim $res1
    Dim $bool_error = "false"
    Dim $bool_usercancel = "false"

    $progressbar = GUICtrlCreateProgress (70,430,520,20)
    GUICtrlSetData ($progressbar,0)
    $icon = GUICtrlCreateIcon ("windows\scripts\ico_build.ico",-1, 20,422,32,32)

    $message = @CRLF & $MSG_LINE & @CRLF & $MSG_10_start & " "
    fli4lbuild_msg ($message ,1)

    $drive_fs = DriveGetFileSystem ($drive)
    If StringLeft ($drive_fs, 3) <> "FAT" then
        $msg = $MSG_10_wrongfs1 & " (" & $drive & ") " & $MSG_10_wrongfs2
        fli4lbuild_errmsg ($msg,1)
        $bool_error = "true"
    EndIf

    If $bool_error = "false" then
        fli4lbuild_msg (@CRLF & $MSG_10_writesystem & " " ,0)
        If @OSType = "WIN32_WINDOWS" Then
            $val = RunWait ("windows\syslinux.com " & $drive, @WorkingDir, @SW_HIDE)
        Else
            $val = RunWait ("windows\syslinux.exe " & $drive, @WorkingDir, @SW_HIDE)
        EndIf

        If Not FileExists ($drive & "\ldlinux.sys") then
            $bool_error = "true"
            ;ERROR on syslinux
        else
            fli4lbuild_msg ($MSG_finished,0)

            If $boot_type = "integrated" then
                $n_max = 3
            Else
                $n_max = 4
            EndIf
            For $_n = 0 to $n_max
                fli4lbuild_msg (@CRLF & $ls120files[$_n][2] & " " ,0)
                $res1 = FileCopy ($glob_dir_tmpbuild & $ls120files[$_n][0], $drive & $ls120files[$_n][1], 1)
                If $res1 = -1 then
                    MsgBox (0, $MSG_msgbox_error, $MSG_10_errorcopy & " " & $ls120files[$_n][0])
                    $bool_error = "true"
                    ExitLoop
                EndIf
                GUICtrlSetData ($progressbar,($_n + 1) * 20 - 10)
                fli4lbuild_msg ($MSG_finished,0)
            Next
        EndIf
    EndIf
    If $bool_error = "false" AND $bool_usercancel = "false" then
        GUICtrlSetData ($progressbar,100)
        If $verbose = "true" then
            $msg = @CRLF & $MSG_10_finish1
        Else
            $msg = $MSG_finished
        EndIf
        $msg =  $msg & @CRLF & $MSG_LINE & @CRLF & $MSG_10_finish2 & @CRLF
        fli4lbuild_msg ($msg,1)
    EndIf

    If $bool_error = "true" then
        $msg = @CRLF & $MSG_10_error & @CRLF
        fli4lbuild_errmsg ($msg,0)
        fli4lbuild_msg ($msg,1)
    EndIf

    If $bool_usercancel = "true" then
        $msg = $MSG_10_cancel & @CRLF
        fli4lbuild_errmsg ($msg,0)
        fli4lbuild_msg ($MSG_cancel & @CRLF, 1)
    EndIf

    sleep (2000)
    GUICtrlDelete ($progressbar)
    GUICtrlDelete ($icon)

EndFunc
