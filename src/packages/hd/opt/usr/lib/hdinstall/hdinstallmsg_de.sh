#! /bin/sh

#
# here documents have to be indented by tabs, if you expect the 
# indentation to be removed by cat <<-EOF
#

ascutfcat ()
{
    [ $km -gt 16 ] &&
         cat | sed 's/ö/Ã¶/g;s/Ö/Ã–/g;s/ü/Ã¼/g;s/Ü/Ãœ/g;s/ä/Ã¤/g;s/Ä/Ã„/g;s/ß/ÃŸ/g'
    cat
}

if [ -n "$2" ]
then
  param2=$2
else
  param2=''
fi

km=`cat /etc/kernel-minor`
clrhome
case $1 in

    info)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Installationswunsch abfragen
		
		Bitte drücken Sie einfach Return, um mit der Installation auf die 
		vorgeschlagene Festplatte fortzufahren. 
		Sie können alternativ die Bezeichnung einer anderen Festplatte 
		eingeben, wobei die folgende Tabelle gilt:
		
		  primärer IDE Controller,   Master   hda
		  primärer IDE Controller,   Slave    hdb
		  sekundärer IDE Controller, Master   hdc
		  sekundärer IDE Controller, Slave    hdd
		  erste (S)ATA/SCSI-Festplatte        sda
		
		Um die Installation an dieser Stelle abzubrechen geben Sie X ein.
EOF
        ;;

    inv_partition_table)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash

		Die Partitionstabelle ist in einem inkonsistenten Zustand.
		Um eine ordnungsgemässe Installation sicherzustellen, muss im
		ersten Schritt eine gültige Partitionstabelle geschrieben werden.
		Dabei gehen alle Informationen auf der Platte verloren.
		
		Soll die Partitionstabelle jetzt neu geschrieben werden?
		
		***************************************************************
		* Dies ist Ihre letzte Chance vor dem drohenden Datenverlust! *
		*   Alle Daten auf der Festplatte werden hiermit gelöscht!    *
		***************************************************************
		
		Bitte geben Sie JA in Großbuchstaben ein, um weiterzumachen: 
EOF
        ;;
    installation_type)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Installationstyp auswählen
		 
		Typ A: Betrieb komplett aus der Ramdisk
		    Alle benötigten Dateien werden auf einer DOS-Partition abgelegt. Beim Start
		    des Routers wird alles in eine Ramdisk entpackt und von dort aus gestartet.
		    Die Festplatte wird nur als größeres Diskettenlaufwerk benutzt. Falls keine
		    Daten-Partition genutzt wird, kann der Router auch ohne Shutdown 
		    abgeschaltet werden.
		
		Typ B: Auslagerung der sekundären Ramdisk auf die Festplatte
		    Die Dateien aus dem opt.img werden automatisch auf eine ext3-Partition
		    entpackt und von dort gestartet. Dies bringt Vorteile bei einem Router mit
		    sehr wenig RAM (8-12MB), kann aber Probleme mit dem Abschalten einer 
		    IDE-Platte mit OPT_HDSLEEP verursachen. Ein sauberer Shutdown vor dem 
		    Abschalten des Routers ist unbedingt notwendig.
		
		
		Bitte wählen Sie: A oder B (X bricht die Installation ab)
EOF
        ;;

    dos_partition)
        if [ -z "$param2" ]
        then
          param2=2
        fi
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		  
		DOS-Partition anlegen
		   
		Wie groß soll die DOS-Partition werden?
		
		Die Mindestgröße ist $param2 MB, empfohlen werden 4 bis 8 MB.
		Die maximale Größe ist 128 MB, höhere Werte werden automatisch auf 128 MB
		verkleinert! Tippen Sie 1 ein, wenn Sie den gesamten Speicherplatz eines 
		Mediums mit weniger als 128 MB für die DOS-Partition nutzen wollen. 
		
		Bitte geben Sie die gewünschte Größe in MB ein, 1 für die maximale 
		Partitionsgröße bzw. den Rest der Platte oder X für exit: 
EOF
        ;;

    opt_partition)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Opt-Partition anlegen
		
		Wie groß soll die Partition für das entpackte opt.img werden?
		
		Die Mindestgröße sind 2 MB, empfohlen werden 4 bis 16 MB. Maximal möglich 
		sind hier 512 MB. Diese Partition wird anstelle der sekundären Ramdisk 
		genutzt. Bitte überlegen Sie nochmals ob der Installationstyp A ihre 
		Anforderungen nicht ebenfalls erfüllt.
		 
		Bitte geben Sie die gewünschte Größe in MB ein, 1 für den Rest der 
		Platte bzw. die maximale Partitionsgröße oder X für exit: 
EOF
        ;;

    swap_partition)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Swap-Partition anlegen
		 
		Möchten Sie eine Swap-Partition anlegen?
		
		Dies ist normalerweise nur notwendig, wenn sie wenig Hauptspeicher zur
		Verfügung haben. Die Swap-Partition sollte mindestens so groß wie der real 
		eingebaute Hauptspeicher (RAM) sein. Die maximale Größe sind 256 MB, Werte 
		darüber werden automatisch auf 256 MB verringert.
		
		Eine Swap-Partition ist nur bei Verwendung einer Festplatte sinnvoll. Wenn
		Sie fli4l auf ein Flash-Medium installieren, sollten Sie kein Swap anlegen, 
		weil Sie sonst einen frühzeitigen Defekt des Mediums riskieren!
		
		Bitte geben Sie die gewünschte Größe in MB ein, 1 für die maximale Größe bzw.
		den Rest der Platte oder drücken Sie einfach Enter, wenn Sie keine 
		Swap-Partition anlegen wollen:
EOF
        ;;

    data_partition)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Daten-Partition anlegen
		 
		Möchten Sie eine zusätzliche Daten-Partition anlegen?
		
		Dies ist nur notwendig, wenn Sie auf dem Router etwas Platz für Dateiablage
		z.B. für opt_vbox oder opt_samba_lpd benötigen.
		Es wird hierfür eine ext3-Partition angelegt.
		Die maximale Größe ist 2 TB, grössere Werte werden automatisch verkleinert!
		
		Bitte geben Sie die gewünschte Größe in MB ein, 1 für die maximale Größe bzw.
		den Rest der Platte oder drücken Sie einfach Enter, wenn Sie keine 
		Daten-Partition anlegen wollen: 
EOF
        ;;

    final_warning)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Sicherheitsabfrage vor der Neupartitionierung
		
		***************************************************************
		* Dies ist Ihre letzte Chance vor dem drohenden Datenverlust! *
		*   Alle Daten auf der Festplatte werden hiermit gelöscht!    *
		***************************************************************
		
		Bitte geben Sie JA in Großbuchstaben ein, um die Platte wie 
		folgt zu partitionieren:
EOF
        ;;

    final_warning_1)
	ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Sicherheitsabfrage vor der Neupartitionierung
		
		Bitte geben Sie JA in Großbuchstaben ein, um die Platte wie 
		folgt zu partitionieren:
EOF
	;;

    standard_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Der Versuch, die Standard-Bootdateien zu sichern, schlug fehl.
		Nach einer Neupartitionierung muss eine aktuelle Version mittels
		remote update auf den Router kopiert werden.
		
		Bitte geben Sie JA in Großbuchstaben ein, um fortzufahren:

EOF
	;;

    recovery_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Der Versuch, die Recovery-Bootdateien zu sichern, schlug fehl.
		Nach einer Neupartitionierung muss die Recovery-Version neu erstellt
		werden.
		
		Bitte geben Sie JA in Großbuchstaben ein, um fortzufahren:

EOF
	;;
    finish)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Wie geht es weiter?
		  
		Die Festplatte wurde partitioniert, formatiert und ist jetzt unter /boot
		erreichbar. Sie können jetzt alle gewünschten Opt-Pakete aktivieren und 
		per imonc, fli4l-Build oder scp auf den Router nach /boot übertragen.
		
		Der Router benötigt folgende Dateien um von der Festplatte zu starten:
		syslinux.cfg, kernel, rootfs.img, rc.cfg und opt.img
		
		WICHTIG: Sie müssen die aufgeführten Dateien jetzt per remote update
		auf den Router übertragen, andernfalls ist der Router nicht in der
		Lage, von HD zu booten!
		
		Entfernen Sie nach dem remote update die Diskette aus dem Router, 
		fahren Sie ihn herunter (unter Verwendung von halt/reboot/poweroff)
		und starten Sie neu.
		Drücken Sie nicht einfach nur Reset, dabei können die beim
		Remote-Update vorgenommenen Änderungen verloren gehen.
		
EOF
        ;;
    finish_repartitioning)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Wie geht es weiter?
		  
		Die Festplatte wurde neu partitioniert und formatiert, der Router kann 
		jetzt neu gebootet werden. 
		
		Wollen Sie neben der Neupartitionierung auch gleich Updates am Router
		vornehmen, koennen Sie das jetzt per remote update tun.
		
		Fahren Sie dann den Router herunter (unter Verwendung
		von halt/reboot/poweroff) und starten Sie neu.
		Drücken Sie nicht einfach nur Reset, dabei können die
		beim Remote-Update bzw. bei der Re-Partitionierung
		vorgenommenen Änderungen verloren gehen.
		
EOF
        ;;
    *)
        echo "Unknown text message requested, please inform author."
	exit 1
	;;
esac
exit 0
