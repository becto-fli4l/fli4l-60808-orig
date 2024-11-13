; -------------------------------------------------------------------------
; language constants (german)                                  __FLI4LVER__
;
; Creation: lanspezi 2005-04-14
; $Id$
; -------------------------------------------------------------------------

[Global]
MSG_LINE===================================================================
MSG_BTN_close=Schließen
MSG_msgbox_error=Fehler
MSG_msgbox_warn=Warnung
MSG_msgbox_info=Info
MSG_errorfilelog=Die LOG-Datei kann nicht geöffnet werden!
MSG_errorfilelogerror=Die Fehler-LOG-Datei kann nicht geöffnet werden!
MSG_finished=abgeschlossen.
MSG_cancel=abgebrochen.
MSG_00_scpstart=Übertrage die Dateien per SCP zum Router ...
MSG_00_scpend=SCP-Übertragung abgeschlossen.
MSG_00_scpend2=Starten Sie nun Ihren fli4l-Router neu.
MSG_00_pass=Passwort für die SCP-Übertragung
MSG_00_usebasepass=Passwort aus 'base.txt' verwenden?
MSG_00_scpenterpass=Passwort für die SCP-Übertragung eingeben für den Host:
MSG_00_scpcancel=Die SCP-Übertragung wurde auf Benutzeranforderung abgebrochen.
MSG_00_scperror1=Bei der SCP-Übertragung ist ein Fehler aufgetreten!
MSG_00_scperror2=Bitte überprüfen Sie alle SSH-Parameter und den sshd auf dem Router.
MSG_00_changesshkey=SSH-Keyfile auswählen
MSG_00_plinkaskreboot=Möchten Sie den Router jetzt neu starten?
MSG_00_plinksentreboot=Der Befehl zum Neustart wurde an den Router gesendet.
MSG_00_askreboot=fli4l - Router Neustart

[mkfli4l]
MSG_01_build_abort=Der Build-Prozess wurde auf Benutzeranforderung abgebrochen.
MSG_01_build_error=Der Build-Prozess wurde aufgrund eines Fehlers abgebrochen!
MSG_01_no_boot_type=Ungültiger Wert für die Variable BOOT_TYPE!

[parse_cmd]
MSG_02_start=__FLI4LVER__ - Build Prozess
MSG_02_noconfdir=Im Verzeichnis mit der fli4l-Konfiguration existiert keine base.txt!
MSG_02_useconfig=Verwende die Konfiguration aus dem Verzeichnis '
MSG_02_undefboottype=Die Variable BOOT_TYPE ist nicht definiert! Bitte Konfigurations-Dateien im korrekten Format benutzen.
MSG_02_migrateconfig=Migriere mkfli4l.conf zu mkfli4l.txt...

[parse_mkfli4l_conf]
MSG_03_readcheck_buildenv=Einlesen und Prüfen des Build-Environments '
MSG_03_nologfile=Die mkfli4l_conf-Log-Datei kann nicht geöffnet werden!

[mkopt]
MSG_04_start=Erzeugen der fli4l-Dateien ...
MSG_04_confdir=Benutze die Konfiguration aus dem Verzeichnis '
MSG_04_builddir=Erzeuge die fli4l-Dateien im Verzeichnis '
MSG_04_readcheckconf=Einlesen und Prüfen der Konfiguration, Erzeugen des rootfs- und opt-Archives ...
MSG_04_generating_md5sums=md5-Prüfsummen generieren ...
MSG_04_finished=Erzeugung der fli4l-Dateien abgeschlossen.
MSG_04_nologfile=Die mkfli4l-Log-Datei kann nicht geöffnet werden!
MSG_04_msgboxerror=Fehler in den fli4l-Konfigurationsdateien!

[mkfloppy]
MSG_05_start=Erstellen der fli4l-Diskette(n) ...
MSG_05_insertfloppy=Bitte formatierte Diskette einlegen.
MSG_05_bootdisk=(diese wird Ihre BOOT_DISK)
MSG_05_startcopy=Bitte auf OK klicken oder RETURN drücken um das Kopieren der Dateien auf die Diskette zu starten.
MSG_05_copysyslinux=Kopiere syslinux.cfg ...
MSG_05_writesystem=Schreibe System-Dateien ...
MSG_05_copykernel=Kopiere kernel ...
MSG_05_copyrootfstgz=Kopiere rootfs.img ...
MSG_05_copyrootfsbz2=Kopiere rootfs.bz2 ...
MSG_05_copyrccfg=Kopiere rc.cfg ...
MSG_05_copyopttarbz2=Kopiere opt.img ...
MSG_05_bootdisk2= (BOOT_DISK)
MSG_05_optdisk= (OPT_DISK)
MSG_05_secfloppy1=Bitte die erste Diskette entnehmen (BOOT_DISK).
MSG_05_secfloppy2=Bitte formatierte Diskette einlegen und RETURN drücken oder OK klicken.
MSG_05_secfloppy3=(das ist die OPT_DISK)
MSG_05_floppynowrite1=Beim Schreiben der Datei '
MSG_05_floppynowrite2=ist ein Fehler aufgetreten!
MSG_05_floppynospace=Auf der Diskette ist nicht genügend Speicher für die Build Dateien.
MSG_05_bootflnospace=Auf der BOOT-Diskette ist nicht genügend Speicher für die Build Dateien.
MSG_05_cancel=Erstellung der Diskette(n) auf Benutzeranforderung abgebrochen.
MSG_05_error=Fehler beim Erstellen der Diskette(n)!
MSG_05_finish1=Erstellung der Diskette(n) abgeschlossen.
MSG_05_finish2=Starten Sie nun Ihren fli4l-Router mit der/den neuen Diskette(n).
MSG_05_filesonfloppy=Auf der Diskette befinden sich Dateien. Löschen?
MSG_05_usefloppy=Soll diese Diskette trotzdem benutzt werden?
MSG_05_nofloppy=Keine Diskette im Laufwerk!
MSG_05_nofloppyW9x=Keine Diskette im Laufwerk oder Diskette ist nicht formatiert!
MSG_05_calculatespace=Benötigten Platz berechnen ...
MSG_05_requiredspace1=Benötigter Platz:
MSG_05_requiredspace2=Bytes.
MSG_05_requiredspace3=Verfügbarer Platz:
MSG_05_floppyseemsboot=Achtung: diese Diskette könnte die BOOT_DISK sein!
MSG_05_exchangefloppy=Diskette auswechseln und OK klicken um weiter zu gehen
MSG_05_floppynotformatted1=Diskette ist nicht formatiert!
MSG_05_floppynotformatted2=Formatieren?
MSG_05_formatfloppy=Diskette formatieren ...

[mkiso]
MSG_06_createisoimage=Erzeugen des fli4l-ISO-Images ...
MSG_06_buildisoimage=Erzeuge das ISO-Image '
MSG_06_finish1=Erzeugung des fli4l-ISO-Images abgeschlossen.
MSG_06_finish2=Nach dem Brennen des Images auf eine CD, können Sie Ihren fli4l-Router mit dieser starten.

[mknetboot]
MSG_07_mld1=Das Erzeugen eines Netzwerkboot-Images ist im Moment
MSG_07_mld2=mit allen Versionen von Microsoft-Windows nicht möglich!

[dlg_mkfli4l_conf]
MSG_08_btnabort=Abbruch Build-Prozess
MSG_08_btnbuild=Starte Build-Prozess
MSG_08_nobasetxt=Im neuen Verzeichnis existiert keine 'base.txt'
MSG_08_changedirconfig=Verzeichnis mit den fli4l Konfigurationsdateien
MSG_08_changedconfigdir1=Änderung des Config-Verzeichnisses
MSG_08_changedconfigdir2=Soll das gewählte Verzeichnis zukünftig als Standard verwendet werden?
MSG_08_changedirbuild=Verzeichnis für die erzeugten fli4l Dateien
MSG_08_dirconfiginfo1=Eine Änderung des 'Config-Dir' setzt alle Optionen auf die Werte, die in der dortigen 'mkfli4l.txt'
MSG_08_dirconfiginfo2=gesetzt wurden, bzw. als Kommandozeilenparameter übergeben wurden.
MSG_08_savemkfli4lconf=Aktuelle Einstellungen in mkfli4l.txt speichern
MSG_08_savemkfli4ldone=Die Einstellungen wurden in der mkfli4l.txt gespeichert
MSG_08_savemkfli4lerror=Fehler beim Schreiben der mkfli4l.txt
MSG_08TT_btnconfigdir=Ändern des aktuellen Verezichnisses mit den Config-Dateien
MSG_08TT_btnbuilddir=Ändern des Verzeichnisses für die erzeugten fli4l-Dateien
MSG_08TT_verbose=Ausführliche Ausgabe einschalten
MSG_08TT_filesonly=Nur Dateien erzeugen
MSG_08TT_drive=Diskettenlaufwerk für die fli4l-Diskette
MSG_08TT_floppy_normal=Normalen Modus für die Diskette
MSG_08TT_floppy_safe=Sicheren Modus für die Diskette
MSG_08TT_remoteupdate=Remotupdate-Funktion aktivieren
MSG_08TT_remotehostname=Hostname oder IP-Adresse für das Remoteupdate
MSG_08TT_remoteusername=Benutzername für das Remoteupdate
MSG_08TT_remotepathname=Zielverzeichniss für das Remoteupdate
MSG_08TT_remoteport=Port für das Remoteupdate
MSG_08TT_btn_sshfile=Datei mit dem ssh-Key
MSG_08TT_btn_sshfileclear=Löschen der Benutzung vom SSH-Key -> Username/Passwort

[mkhdinstall]
MSG_09_start=HD-Install auf dem gewählten Laufwerk
MSG_09_copysyslinux=Kopiere syslinux.cfg ...
MSG_09_copykernel=Kopiere kernel ...
MSG_09_copyrootfstgz=Kopiere rootfs.img ...
MSG_09_copyrootfsbz2=Kopiere rootfs.bz2 ...
MSG_09_copyrccfg=Kopiere rc.cfg ...
MSG_09_copyopttarbz2=Kopiere opt.img ...
MSG_09_writesystem=Schreibe System-Dateien ...
MSG_09_finish1=Abschluß des HD-pre-installs
MSG_09_finish2=- nach den Wechsel der Disk in Ihren Router, können Sie diesen starten
MSG_09_cancel=Abbruch der HD-pre-installation durch Benutzer
MSG_09_error=Fehler beim Schreiben von Dateien (HD-pre-Installation)!

MSG_09_admin1=Sie haben keine Admin-Rechte - diese werden zum erstellen des bootbaren Datenträgers benötigt
MSG_09_admin2=Möchten Sie 'syslinux' als Administrator ausführen?
MSG_09_ask_pass=Passwort für Administrator
MSG_09_pass=Passwort:
MSG_09_wrongfs1=Das Laufwerk
MSG_09_wrongfs2=hat kein FAT-Filessystem!
MSG_09_errorcopy=Fehler beim kopieren der Datei

[mkls120]
MSG_10_start=Install LS120-Konfiguration auf dem gewählten Laufwerk
MSG_10_copysyslinux=Kopiere syslinux.cfg ...
MSG_10_copykernel=Kopiere kernel ...
MSG_10_copyrootfstgz=Kopiere rootfs.img ...
MSG_10_copyrootfsbz2=Kopiere rootfs.bz2 ...
MSG_10_copyrccfg=Kopiere rc.cfg ...
MSG_10_copyopttarbz2=Kopiere opt.img ...
MSG_10_writesystem=Schreibe System-Dateien ...
MSG_10_finish1=Abschluß der Installation auf LS120
MSG_10_finish2=- nach den Wechsel der LS120 in Ihren Router, können Sie diesen starten
MSG_10_cancel=Abbruch der Installation auf LS120 durch Benutzer
MSG_10_error=Fehler beim Schreiben von Dateien!
MSG_10_wrongfs1=Das Laufwerk
MSG_10_wrongfs2=hat kein FAT-Filessystem!
MSG_10_errorcopy=Fehler beim kopieren der Datei
