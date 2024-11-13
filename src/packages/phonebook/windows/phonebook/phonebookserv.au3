;---------------------------------------------------------------------------- ;
; AutoIt Version: 3.1.0
; Author:         Henry H.
;
; Script Function:
;	Stellt ein Server her der auf dem Port 5002 Lauscht.
; Jeder darf das script auch ändern... wie ihm es beliebt
; Ich übernehme keinerlei Hafutng für dieses Script und was es auch für ein
; Schaden verursachen könnte. Nutzung auf eigene Gefahr
----------------------------------------------------------------------------

Global $MainSocket = -1
Global $ConnectedSocket = -1
Local $MaxConnection = 1; Maximum Amount Of Concurrent Connections
Local $MaxLength = 512; Maximum Length Of String
Local $Port = IniRead ( "phonebookserv.ini", "Host", "Port", "5002" ); Port Number
Local $Server = IniRead ( "phonebookserv.ini", "Host", "Ip", @IPAddress1 ); Server IpAddress

TCPStartup()
$MainSocket = TCPListen($Server, $Port , 5000)
If $MainSocket = -1 Then Exit MsgBox(16, "Error", "Unable to intialize socket.")
While 1
    $Data = TCPRecv($ConnectedSocket, $MaxLength)
    If $Data == "exit" & @LF Then
      TCPCloseSocket( $ConnectedSocket )
      $ConnectedSocket = -1
      ElseIf $Data <> "" Then
      $newdata = StringReplace($Data, "=", " => ")
      TrayTip("Anruf", $newdata , 5)
      TCPCloseSocket( $ConnectedSocket )
      $ConnectedSocket = -1
   EndIf


    If $ConnectedSocket = -1 Then
        $ConnectedSocket = TCPAccept($MainSocket)
        If $ConnectedSocket <> -1 Then
; Someone Connected
            TCPSend($ConnectedSocket, "FLI4L 3.0.1 - OPT_Phonebook_Server                     erstellt am 2006-03-29"  & @CRLF )
            TCPSend($ConnectedSocket, "============================================================================="  & @CRLF )
        EndIf
    EndIf
WEnd
Func OnAutoItExit()
    If $ConnectedSocket <> - 1 Then
        TCPSend($ConnectedSocket, "~bye")
        TCPCloseSocket($ConnectedSocket)
    EndIf
    If $MainSocket <> -1 Then TCPCloseSocket($MainSocket)
    TCPShutdown()
EndFunc;==>OnAutoItExit
