; -------------------------------------------------------------------------
; creates a fli4l-isoimage                                     __FLI4LVER__
;
; Creation: lanspezi 2005-04-04
; $Id$
; -------------------------------------------------------------------------
#include-once

Func mkiso ()
    msg_lang_read ("mkiso")

    $message = @CRLF & $MSG_LINE & @CRLF & $MSG_06_createisoimage & " "
    fli4lbuild_msg ($message,1)

    $progressbar = GUICtrlCreateProgress (70,430,520,20)
    GUICtrlSetData ($progressbar,0)
    $icon = GUICtrlCreateIcon ("windows\scripts\ico_cd.ico",-1, 20,422,32,32)

    fli_DirCreate ($glob_dir_tmpbuild & "\iso")
    fli_DirCreate ($glob_dir_tmpbuild & "\iso\isolinux")
    FileCopy (                     "opt\img\syslinux\isolinux.bin", $glob_dir_tmpbuild & "\iso\isolinux.bin",1)
    FileCopy (                     "opt\img\syslinux\ldlinux.c32",  $glob_dir_tmpbuild & "\iso\isolinux\ldlinux.c32",1)
    FileCopy ($glob_dir_tmpbuild & "\syslinux.cfg",    $glob_dir_tmpbuild & "\iso\isolinux.cfg",1)
    FileCopy ($glob_dir_tmpbuild & "\kernel",          $glob_dir_tmpbuild & "\iso\kernel",1)
    FileCopy ($glob_dir_tmpbuild & "\rootfs.img",      $glob_dir_tmpbuild & "\iso\rootfs.img",1)
    FileCopy ($glob_dir_tmpbuild & "\rc.cfg",          $glob_dir_tmpbuild & "\iso\rc.cfg",1)
    FileCopy ($glob_dir_tmpbuild & "\opt.img",         $glob_dir_tmpbuild & "\iso\opt.img",1)
    FileCopy ($dir_build         & "\boot.msg",        $glob_dir_tmpbuild & "\iso\boot.msg",1)
    FileCopy ($dir_build         & "\boot_s.msg",        $glob_dir_tmpbuild & "\iso\boot_s.msg",1)
    FileCopy ($dir_build         & "\boot_z.msg",        $glob_dir_tmpbuild & "\iso\boot_z.msg",1)
    If FileExists ($dir_build & "\dtbs") Then
        DirCopy ($dir_build & "\dtbs", $glob_dir_tmpbuild & "\iso\dtbs",1)
    EndIf

    GUICtrlSetData ($progressbar,50)

    $message = @CRLF & $MSG_06_buildisoimage & $dir_build & "\fli4l.iso'"
    fli4lbuild_msg ($message,0)
    $run_call = StringFormat ('windows\\mkisofs -A fli4l -V fli4l -J -r -o "%s\\fli4l.iso" ' & _
               '-b isolinux.bin -c boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table ' & _
               '-input-charset default "%s\\iso"', $dir_build, $glob_dir_tmpbuild)
    $val = RunWait ($run_call, @WorkingDir, @SW_HIDE)
    GUICtrlSetData ($progressbar,90)
    DirRemove ($glob_dir_tmpbuild & "\iso", 1)

    If $verbose = "true" then
        $message = @CRLF & $MSG_06_finish1
    Else
        $message = $MSG_finished
    EndIf
    $message = $message & @CRLF & $MSG_LINE & @CRLF & $MSG_06_finish2 & @CRLF
    fli4lbuild_msg ($message,1)

    GUICtrlSetData ($progressbar,100)
    sleep (2000)
    GUICtrlDelete ($progressbar)
    GUICtrlDelete ($icon)

EndFunc

