; -------------------------------------------------------------------------
; Install U-Boot for sunxi                                     __FLI4LVER__
;
; $Id$
; -------------------------------------------------------------------------
#include-once
#RequireAdmin
#include "..\autoit\Constants.au3"

Func analyse_partition_table ($physdev)
    $hDisk = FileOpen ($physdev, 16)
    If $hDisk >= 0 Then
        FileRead ($hDisk, 446)
        $part_table = FileRead ($hDisk, 16 * 4)
        $signature = FileRead ($hDisk, 2)
        FileClose ($hDisk)

        If BinaryMid ($signature, 1, 1) <> 0x55 OR _
           BinaryMid ($signature, 2, 1) <> 0xAA Then
            MsgBox (4096 + 16, "Error", _
                    "mkhdinstall_sunxi: Partition table not found on physical device " _
                        & $physdev & "!")
            Return 4
        EndIf

        For $i = 0 To 3
            $active = BinaryMid ($part_table, $i * 16 + 1, 1)
            If $active <> 0 Then
                $start = Int (BinaryMid ($part_table, $i * 16 + 9, 4))
                If $start < 2048 Then
                    MsgBox (4096 + 16, "Error", _
                            "mkhdinstall_sunxi: Not enough space for boot loader on physical device " _
                                & $physdev & ": There are " & $start & _
                                " sectors between MBR and start of first partition (index " _
                                & $i + 1 & "), but we need at least 2048!")
                    Return 5
                Else
                    Return 0
                EndIf
            EndIf
        Next

        MsgBox (4096 + 16, "Error", _
                "mkhdinstall_sunxi: No partition found on physical device " & _
                    $physdev & "!")
        Return 6
    Else
        MsgBox (4096 + 16, "Error", _
                "mkhdinstall_sunxi: Cannot open device " & $physdev & "!")
        Return 3
    EndIf
EndFunc

Func install_uboot ($physdev)
    $val = RunWait ("windows\dd.exe of=" & $physdev & " bs=1024 count=1023 seek=1 if=/dev/zero", _
                    @WorkingDir, @SW_HIDE)
    If $val <> 0 Then
        MsgBox (4096 + 16, "Error", _
                "mkhdinstall_sunxi: Cannot clear boot loader area on physical device " & $physdev & "!")
        Return 7
    EndIf

    $val = RunWait ("windows\dd.exe of=" & $physdev & " bs=1024 seek=8 if=opt\img\u-boot-sunxi-with-spl.bin ", _
                    @WorkingDir, @SW_HIDE)
    If $val <> 0 Then
        MsgBox (4096 + 16, "Error", _
                "mkhdinstall_sunxi: Cannot install boot loader on physical device " & $physdev & "!")
        Return 8
    EndIf

    Return 0
EndFunc

$drive_hdinstall = $CmdLine[1]

$pid = Run("windows\log2physdev.exe " & $drive_hdinstall, @WorkingDir, _
           @SW_HIDE, $STDOUT_CHILD)
If ProcessWaitClose($pid) Then
    $result = @extended

    $output = StdoutRead($pid)
    $lines = StringSplit($output, @CRLF, 1) 
    StdioClose($pid)

    If $result <> 0 OR $lines[0] = 0 Then
        MsgBox (4096 + 16, "Error", _
                "mkhdinstall_sunxi: Cannot map logical volume " & _
                    $drive_hdinstall & " to underlying physical device!")
        Exit 2
    Else
        $physdev = $lines[1]
        $result = analyse_partition_table($physdev)
        If $result = 0 Then
            $result = install_uboot($physdev)
        EndIf
        Exit $result
    EndIf
Else
    MsgBox (4096 + 16, "Error", _
            "mkhdinstall_sunxi: Cannot run log2physdev.exe!")
    Exit 1
EndIf
