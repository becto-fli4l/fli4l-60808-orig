; -------------------------------------------------------------------------
; Boot loader specific code                                    __FLI4LVER__
;
; Last Update: $Id$
; -------------------------------------------------------------------------
#include-once

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; GENERIC BOILER-PLATE CODE ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Func boot_loader_post_build ($arch, $build_dir)
    Switch $arch
        Case "x86"
            Return boot_loader_post_build_x86 ($build_dir)
        Case "x86_64"
            Return boot_loader_post_build_x86_64 ($build_dir)
        Case "rpi"
            Return boot_loader_post_build_rpi ($build_dir)
        Case "sunxi"
            Return boot_loader_post_build_sunxi ($build_dir)
    EndSwitch
EndFunc

Func boot_loader_get_entries ($arch, $build_dir, ByRef $entries)
    Switch $arch
        Case "x86"
            boot_loader_get_entries_x86 ($build_dir, $entries)
        Case "x86_64"
            boot_loader_get_entries_x86_64 ($build_dir, $entries)
        Case "rpi"
            boot_loader_get_entries_rpi ($build_dir, $entries)
        Case "sunxi"
            boot_loader_get_entries_sunxi ($build_dir, $entries)
    EndSwitch
EndFunc

Func boot_loader_install ($arch, $boot_type, $part_dev, $blk_dev)
    Switch $arch
        Case "x86"
            Return boot_loader_install_x86 ($boot_type, $part_dev, $blk_dev)
        Case "x86_64"
            Return boot_loader_install_x86_64 ($boot_type, $part_dev, $blk_dev)
        Case "rpi"
            Return boot_loader_install_rpi ($boot_type, $part_dev, $blk_dev)
        Case "sunxi"
            Return boot_loader_install_sunxi ($boot_type, $part_dev, $blk_dev)
    EndSwitch
EndFunc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; ARCHITECTURE DEPENDANT CODE ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;
;;; x86/x86_64 ;;;
;;;;;;;;;;;;;;;;;;

Func boot_loader_post_build_x86 ($build_dir)
    Return boot_loader_post_build_x86_and_x86_64 ($build_dir)
EndFunc

Func boot_loader_get_entries_x86 ($build_dir, ByRef $entries)
    boot_loader_get_entries_x86_and_x86_64 ($build_dir, $entries)
EndFunc

Func boot_loader_install_x86 ($boot_type, $part_dev, $blk_dev)
    Return boot_loader_install_x86_and_x86_64 ($boot_type, $part_dev, $blk_dev)
EndFunc

Func boot_loader_post_build_x86_64 ($build_dir)
    Return boot_loader_post_build_x86_and_x86_64 ($build_dir)
EndFunc

Func boot_loader_get_entries_x86_64 ($build_dir, ByRef $entries)
    boot_loader_get_entries_x86_and_x86_64 ($build_dir, $entries)
EndFunc

Func boot_loader_install_x86_64 ($boot_type, $part_dev, $blk_dev)
    Return boot_loader_install_x86_and_x86_64 ($boot_type, $part_dev, $blk_dev)
EndFunc

Func boot_loader_post_build_x86_and_x86_64 ($build_dir)
    ; nothing to do
    Return 1
EndFunc

Func boot_loader_get_entries_x86_and_x86_64 ($build_dir, ByRef $entries)
    $num = UBound ($entries)
    ReDim $entries[$num + 1][3]
    $entries[$num][0] = $build_dir & "\syslinux.cfg"
    $entries[$num][1] = "syslinux.cfg"
    $entries[$num][2] = 1
EndFunc

Func boot_loader_install_x86_and_x86_64 ($boot_type, $part_dev, $blk_dev)
    Local $syslinux_option = ""

    If $boot_type = "hd" OR $boot_type = "integrated" then
        $syslinux_option = "-ma "
    endif
    If @OSType <> "WIN32_NT" Then
        $val = RunWait ("opt\img\syslinux\syslinux.com " & $syslinux_option & $part_dev, @WorkingDir, @SW_HIDE)
    Else
        $val = RunWait ("windows\autoit\autoit3.exe windows\scripts\mkhdinstall_syslinux.au3 " & $part_dev & " " & $syslinux_option, @WorkingDir)
    EndIf

    sleep (1000)

    Return FileExists ($part_dev & "\ldlinux.sys")
EndFunc

;;;;;;;;;;;
;;; rpi ;;;
;;;;;;;;;;;

Func boot_loader_post_build_rpi ($build_dir)
    Local $syslinux_file = FileOpen($build_dir & "\syslinux.cfg")
    If $syslinux_file = -1 Then
        Return False
    EndIf

    Local $syslinux_file_new = FileOpen($build_dir & "\syslinux_new.cfg", 2)
    If $syslinux_file_new = -1 Then
        FileClose($syslinux_file)
        Return False
    EndIf

    While True
        $line = FileReadLine($syslinux_file)
        If @error = -1 Then
            ExitLoop
        ElseIf @error <> 0 Then
            Return False
        EndIf
        $line = StringRegExpReplace($line, "^([[:space:]]*APPEND )", "\1\${bootargs} ")
        FileWriteLine($syslinux_file_new, $line & @LF)
    WEnd
    FileClose($syslinux_file)
    FileClose($syslinux_file_new)
    FileMove($build_dir & "\syslinux_new.cfg", $build_dir & "\syslinux.cfg", 1)

    fli_DirCreate($build_dir & "\extlinux")

    Local $config_file = FileOpen($build_dir & "\config.txt", 2)
    If $config_file = -1 Then
        Return False
    else
        FileWriteLine($config_file, "kernel=u-boot.bin" & @LF)
        FileWriteLine($config_file, "device_tree_address=0x100" & @LF)
        FileWriteLine($config_file, "gpu_mem=16" & @LF)
    EndIf
    FileClose($config_file)

    ; the RPi firmware expects the DTB files to lie in the boot medium's root
    ; directory
    Local $dtb_file = ""
    Local $hSearch = FileFindFirstFile($build_dir & "\dtbs\*.dtb")
    If $hSearch <> -1 Then
        While True
            $dtb_file = FileFindNextFile($hSearch)
            If @error Then
                ExitLoop
            Else
                FileMove($build_dir & "\dtbs\" & $dtb_file, $build_dir & "\" & $dtb_file, 1)
            EndIf
        WEnd
        FileClose($hSearch)
    EndIf

    ; copy the RPi boot loader files
    FileCopy("opt\img\rpi-firmware\bootcode.bin", $build_dir, 1)
    FileCopy("opt\img\rpi-firmware\start.elf", $build_dir, 1)
    FileCopy("opt\img\rpi-firmware\fixup.dat", $build_dir, 1)
    FileCopy("opt\img\u-boot.bin", $build_dir, 1)

    Return 1
EndFunc

Func boot_loader_get_entries_rpi ($build_dir, ByRef $entries)
    Local $num_dtb = 0
    Local $dtb_file = ""
    Local $hSearch = FileFindFirstFile($build_dir & "\*.dtb")
    If $hSearch <> -1 Then
        While True
            $dtb_file = FileFindNextFile($hSearch)
            If @error Then
                ExitLoop
            Else
                $num_dtb += 1
            EndIf
        WEnd
        FileClose($hSearch)
    EndIf

    Local $num = UBound ($entries)
    ReDim $entries[$num + 6 + $num_dtb][3]
    $entries[$num][0] = $build_dir & "\syslinux.cfg"
    $entries[$num][1] = "extlinux\extlinux.conf"
    $entries[$num][2] = 1
    $num += 1
    $entries[$num][0] = $build_dir & "\config.txt"
    $entries[$num][1] = "config.txt"
    $entries[$num][2] = 1
    $num += 1
    $entries[$num][0] = $build_dir & "\bootcode.bin"
    $entries[$num][1] = "bootcode.bin"
    $entries[$num][2] = 1
    $num += 1
    $entries[$num][0] = $build_dir & "\start.elf"
    $entries[$num][1] = "start_cd.elf"
    $entries[$num][2] = 1
    $num += 1
    $entries[$num][0] = $build_dir & "\fixup.dat"
    $entries[$num][1] = "fixup_cd.dat"
    $entries[$num][2] = 1
    $num += 1
    $entries[$num][0] = $build_dir & "\u-boot.bin"
    $entries[$num][1] = "u-boot.bin"
    $entries[$num][2] = 1
    $num += 1

    If $num_dtb > 0 Then
        $hSearch = FileFindFirstFile($build_dir & "\*.dtb")
        If $hSearch <> -1 Then
            While True
                $dtb_file = FileFindNextFile($hSearch)
                If @error Then
                    ExitLoop
                Else
                    $entries[$num][0] = $build_dir & "\" & $dtb_file
                    $entries[$num][1] = $dtb_file
                    $entries[$num][2] = 1
                    $num += 1
                EndIf
            WEnd
            FileClose($hSearch)
        EndIf
    EndIf

    Return 1
EndFunc

Func boot_loader_install_rpi ($boot_type, $part_dev, $blk_dev)
    ; nothing to do
    Return 1
EndFunc

;;;;;;;;;;;;;
;;; sunxi ;;;
;;;;;;;;;;;;;

Func boot_loader_post_build_sunxi ($build_dir)
    fli_DirCreate ($build_dir & "\extlinux")
    Return 1
EndFunc

Func boot_loader_get_entries_sunxi ($build_dir, ByRef $entries)
    $num = UBound ($entries)
    ReDim $entries[$num + 1][3]
    $entries[$num][0] = $build_dir & "\syslinux.cfg"
    $entries[$num][1] = "extlinux\extlinux.conf"
    $entries[$num][2] = 1
EndFunc

Func boot_loader_install_sunxi ($boot_type, $part_dev, $blk_dev)
    $val = RunWait ("windows\autoit\autoit3.exe windows\scripts\mkhdinstall_sunxi.au3 " & $part_dev, @WorkingDir)
    If $val = 0 Then
        Return 1
    Else
        Return 0
    EndIf
EndFunc
