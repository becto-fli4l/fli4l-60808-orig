; -------------------------------------------------------------------------
; creates a fli4l-Floppy or Image                              __FLI4LVER__
;
; Creation: 2005-04-03 lanspezi
; $Id$
; -------------------------------------------------------------------------
#include "..\autoit\GUIConstantsEx.au3"
#include "..\autoit\WindowsConstants.au3"
#include "..\autoit\ListboxConstants.au3"
#include "..\autoit\EditConstants.au3"
#include "..\autoit\ButtonConstants.au3"

#include "_lib_functions.au3"
#include "parse_cmd.au3"
#include "parse_mkfli4l_conf.au3"
#include "mkopt.au3"
#include "mkiso.au3"
#include "mknetboot.au3"
#include "mkpxeboot.au3"
#include "mkhdinstall.au3"
#include "mkls120.au3"
#include "dlg_mkfli4l_conf.au3"

Opt("TrayIconHide", 1)

set_glob_dir ()
fli4l_logs_init ()

msg_lang_init ()
msg_lang_read ("mkfli4l")

Global $log_output
Global $win_fli4l_build
Global $bool_build_abort = "false"
Global $bool_build_restart ="false"

SplashImageOn("fli4l Windows-build", "logo\fli4l.jpg",477,193)
Sleep(1000)
SplashOff()

If (is_dual_screen () = 0 ) then
    $win_fli4l_build = GUICreate("fli4l-Build",600,480,-1,-1,BitOr($WS_POPUP, $WS_CAPTION, $DS_SETFOREGROUND, $WS_SYSMENU))
else
    $win_fli4l_build = GUICreate("fli4l-Build",600,480,10,10,BitOr($WS_POPUP, $WS_CAPTION, $DS_SETFOREGROUND, $WS_SYSMENU))
EndIf

GUISetIcon("windows\fli4l.ico")
$log_output=GUICtrlCreateEdit ("", 10,10,580,400,BitOr($WS_BORDER, $WS_VSCROLL, $LBS_NOSEL, $ES_READONLY))
GuiSetState (@SW_SHOW)

;check parameters from commandline and current env
parse_cmd()

GUISwitch($win_fli4l_build)

If $bool_cleanup = "true" then
    cleanup ()
    cleanup_fli4lfiles ()
    exit_fli4l_build ()
EndIf

If $bool_error = "false" AND $bool_build_abort = "false" then
    mkopt ()
    If $remoteupdate = "false" AND $bool_error = "false" AND $bool_filesonly = "false" then
        Select
            Case $boot_type = "ls120"
                if $bool_do_hdinstall = "true" then
                    mkhdinstall ()
                else
                    mkls120 ()
                endif
            Case $boot_type = "netboot"
                mknetboot()
            Case $boot_type = "pxeboot"
                mkpxeboot()
            Case $boot_type = "hd" OR $boot_type = "attached" OR $boot_type = "integrated"
                ; all finished by mkopt ()
                If $bool_do_hdinstall = "true" then
                    mkhdinstall ()
                EndIf
            Case $boot_type = "cd"
                mkiso ()
            Case Else
                $msg_error = $MSG_01_no_boot_type & @CRLF & "BOOT_TYPE='" & $boot_type & "'"
                fli4lbuild_errmsg ($msg_error, 1)
                Cleanup ()
                $bool_error = "true"
        EndSelect
    EndIf

    If $remoteupdate = "true" AND $bool_error = "false" then
        Local $files = ""

        ; $dir_image is set by mkopt.au3
        Local $hSearch = FileFindFirstFile ($dir_image & "\*.*")
        If $hSearch <> -1 Then
            While 1
                Local $file = FileFindNextFile ($hSearch)
                If @error Then ExitLoop
                $files = $files & " " & $dir_image & "\" & $file
            WEnd
            FileClose ($hSearch)
        EndIf

        scp ($files)
    EndIf
EndIf

If $bool_build_abort = "true" then
    $message = $MSG_LINE & @CRLF & $MSG_01_build_abort & @CRLF
    fli4lbuild_errmsg ($message & $MSG_LINE  & @CRLF, 0)
    fli4lbuild_msg (@CRLF & $message, 1)
EndIf

If $bool_error = "true" then
    $message = $MSG_LINE & @CRLF & $MSG_01_build_error & @CRLF
    fli4lbuild_errmsg ($message, 0)
    fli4lbuild_msg ($message, 1)
EndIf


if $bool_imonc = "true" then
    ;wait for some time
    sleep (1000)
else
    $close = GUICtrlCreateButton ($MSG_BTN_close, 390, 430, 100, 25, $BS_DEFPUSHBUTTON)
    $btn_restart = GUICtrlCreateButton ("Restart-Build", 90, 430, 100, 25)
    GUISetState (@SW_SHOW)

    ; Run the GUI until the dialog is closed
    While 1
        $msg = GUIGetMsg()
        If $msg = $GUI_EVENT_CLOSE then ExitLoop
        If $msg = $close Then ExitLoop
        If $msg = $btn_restart then
            $bool_build_restart = "true"
            ExitLoop
        EndIf
    Wend
EndIf

exit_fli4l_build ()

Func exit_fli4l_build ()
    ; copy all log-Files to build dir
    ; check existence of build-dir create it
    if NOT FileExists($dir_build) then
        fli_DirCreate ($dir_build)
    EndIf
    FileCopy ($glob_dir_tmplog & "\*.log", $dir_build & "\",1)
    Cleanup ()
    If FileExists ($dir_build & "\mkfli4l_error.flg") then
        FileDelete ($dir_build & "\mkfli4l_error.flg")
    EndIf
    Select
        Case $bool_build_restart = "true"
            FileWrite ("flibuildrestart.flg", "RESTART BUILD" & @CRLF)
            exit (0)
        Case $bool_error = "true"
            ; flag-file erzeugen?
            FileWrite ($dir_build & "\mkfli4l_error.flg", "EXITCODE=ERROR" & @CRLF)
            exit (1)
        Case $bool_build_abort = "true"
            FileWrite ($dir_build & "\mkfli4l_error.flg", "EXITCODE=USER" & @CRLF)
            exit (2)
        Case Else
            exit (0)
    EndSelect
EndFunc
