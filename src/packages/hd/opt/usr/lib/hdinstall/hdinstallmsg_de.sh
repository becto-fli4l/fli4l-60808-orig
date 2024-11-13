#! /bin/sh

#
# here documents have to be indented by tabs, if you expect the 
# indentation to be removed by cat <<-EOF
#

ascutfcat ()
{
    [ $km -gt 16 ] &&
         cat | sed 's/�/ö/g;s/�/Ö/g;s/�/ü/g;s/�/Ü/g;s/�/ä/g;s/�/Ä/g;s/�/ß/g'
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
		
		Bitte dr�cken Sie einfach Return, um mit der Installation auf die 
		vorgeschlagene Festplatte fortzufahren. 
		Sie k�nnen alternativ die Bezeichnung einer anderen Festplatte 
		eingeben, wobei die folgende Tabelle gilt:
		
		  prim�rer IDE Controller,   Master   hda
		  prim�rer IDE Controller,   Slave    hdb
		  sekund�rer IDE Controller, Master   hdc
		  sekund�rer IDE Controller, Slave    hdd
		  erste (S)ATA/SCSI-Festplatte        sda
		
		Um die Installation an dieser Stelle abzubrechen geben Sie X ein.
EOF
        ;;

    inv_partition_table)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash

		Die Partitionstabelle ist in einem inkonsistenten Zustand.
		Um eine ordnungsgem�sse Installation sicherzustellen, muss im
		ersten Schritt eine g�ltige Partitionstabelle geschrieben werden.
		Dabei gehen alle Informationen auf der Platte verloren.
		
		Soll die Partitionstabelle jetzt neu geschrieben werden?
		
		***************************************************************
		* Dies ist Ihre letzte Chance vor dem drohenden Datenverlust! *
		*   Alle Daten auf der Festplatte werden hiermit gel�scht!    *
		***************************************************************
		
		Bitte geben Sie JA in Gro�buchstaben ein, um weiterzumachen: 
EOF
        ;;
    installation_type)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Installationstyp ausw�hlen
		 
		Typ A: Betrieb komplett aus der Ramdisk
		    Alle ben�tigten Dateien werden auf einer DOS-Partition abgelegt. Beim Start
		    des Routers wird alles in eine Ramdisk entpackt und von dort aus gestartet.
		    Die Festplatte wird nur als gr��eres Diskettenlaufwerk benutzt. Falls keine
		    Daten-Partition genutzt wird, kann der Router auch ohne Shutdown 
		    abgeschaltet werden.
		
		Typ B: Auslagerung der sekund�ren Ramdisk auf die Festplatte
		    Die Dateien aus dem opt.img werden automatisch auf eine ext3-Partition
		    entpackt und von dort gestartet. Dies bringt Vorteile bei einem Router mit
		    sehr wenig RAM (8-12MB), kann aber Probleme mit dem Abschalten einer 
		    IDE-Platte mit OPT_HDSLEEP verursachen. Ein sauberer Shutdown vor dem 
		    Abschalten des Routers ist unbedingt notwendig.
		
		
		Bitte w�hlen Sie: A oder B (X bricht die Installation ab)
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
		   
		Wie gro� soll die DOS-Partition werden?
		
		Die Mindestgr��e ist $param2 MB, empfohlen werden 4 bis 8 MB.
		Die maximale Gr��e ist 128 MB, h�here Werte werden automatisch auf 128 MB
		verkleinert! Tippen Sie 1 ein, wenn Sie den gesamten Speicherplatz eines 
		Mediums mit weniger als 128 MB f�r die DOS-Partition nutzen wollen. 
		
		Bitte geben Sie die gew�nschte Gr��e in MB ein, 1 f�r die maximale 
		Partitionsgr��e bzw. den Rest der Platte oder X f�r exit: 
EOF
        ;;

    opt_partition)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Opt-Partition anlegen
		
		Wie gro� soll die Partition f�r das entpackte opt.img werden?
		
		Die Mindestgr��e sind 2 MB, empfohlen werden 4 bis 16 MB. Maximal m�glich 
		sind hier 512 MB. Diese Partition wird anstelle der sekund�ren Ramdisk 
		genutzt. Bitte �berlegen Sie nochmals ob der Installationstyp A ihre 
		Anforderungen nicht ebenfalls erf�llt.
		 
		Bitte geben Sie die gew�nschte Gr��e in MB ein, 1 f�r den Rest der 
		Platte bzw. die maximale Partitionsgr��e oder X f�r exit: 
EOF
        ;;

    swap_partition)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Swap-Partition anlegen
		 
		M�chten Sie eine Swap-Partition anlegen?
		
		Dies ist normalerweise nur notwendig, wenn sie wenig Hauptspeicher zur
		Verf�gung haben. Die Swap-Partition sollte mindestens so gro� wie der real 
		eingebaute Hauptspeicher (RAM) sein. Die maximale Gr��e sind 256 MB, Werte 
		dar�ber werden automatisch auf 256 MB verringert.
		
		Eine Swap-Partition ist nur bei Verwendung einer Festplatte sinnvoll. Wenn
		Sie fli4l auf ein Flash-Medium installieren, sollten Sie kein Swap anlegen, 
		weil Sie sonst einen fr�hzeitigen Defekt des Mediums riskieren!
		
		Bitte geben Sie die gew�nschte Gr��e in MB ein, 1 f�r die maximale Gr��e bzw.
		den Rest der Platte oder dr�cken Sie einfach Enter, wenn Sie keine 
		Swap-Partition anlegen wollen:
EOF
        ;;

    data_partition)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Daten-Partition anlegen
		 
		M�chten Sie eine zus�tzliche Daten-Partition anlegen?
		
		Dies ist nur notwendig, wenn Sie auf dem Router etwas Platz f�r Dateiablage
		z.B. f�r opt_vbox oder opt_samba_lpd ben�tigen.
		Es wird hierf�r eine ext3-Partition angelegt.
		Die maximale Gr��e ist 2 TB, gr�ssere Werte werden automatisch verkleinert!
		
		Bitte geben Sie die gew�nschte Gr��e in MB ein, 1 f�r die maximale Gr��e bzw.
		den Rest der Platte oder dr�cken Sie einfach Enter, wenn Sie keine 
		Daten-Partition anlegen wollen: 
EOF
        ;;

    final_warning)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Sicherheitsabfrage vor der Neupartitionierung
		
		***************************************************************
		* Dies ist Ihre letzte Chance vor dem drohenden Datenverlust! *
		*   Alle Daten auf der Festplatte werden hiermit gel�scht!    *
		***************************************************************
		
		Bitte geben Sie JA in Gro�buchstaben ein, um die Platte wie 
		folgt zu partitionieren:
EOF
        ;;

    final_warning_1)
	ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Sicherheitsabfrage vor der Neupartitionierung
		
		Bitte geben Sie JA in Gro�buchstaben ein, um die Platte wie 
		folgt zu partitionieren:
EOF
	;;

    standard_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Der Versuch, die Standard-Bootdateien zu sichern, schlug fehl.
		Nach einer Neupartitionierung muss eine aktuelle Version mittels
		remote update auf den Router kopiert werden.
		
		Bitte geben Sie JA in Gro�buchstaben ein, um fortzufahren:

EOF
	;;

    recovery_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		
		Der Versuch, die Recovery-Bootdateien zu sichern, schlug fehl.
		Nach einer Neupartitionierung muss die Recovery-Version neu erstellt
		werden.
		
		Bitte geben Sie JA in Gro�buchstaben ein, um fortzufahren:

EOF
	;;
    finish)
        ascutfcat <<-EOF
		fli4l - Installation auf Festplatte / Compact-Flash
		 
		Wie geht es weiter?
		  
		Die Festplatte wurde partitioniert, formatiert und ist jetzt unter /boot
		erreichbar. Sie k�nnen jetzt alle gew�nschten Opt-Pakete aktivieren und 
		per imonc, fli4l-Build oder scp auf den Router nach /boot �bertragen.
		
		Der Router ben�tigt folgende Dateien um von der Festplatte zu starten:
		syslinux.cfg, kernel, rootfs.img, rc.cfg und opt.img
		
		WICHTIG: Sie m�ssen die aufgef�hrten Dateien jetzt per remote update
		auf den Router �bertragen, andernfalls ist der Router nicht in der
		Lage, von HD zu booten!
		
		Entfernen Sie nach dem remote update die Diskette aus dem Router, 
		fahren Sie ihn herunter (unter Verwendung von halt/reboot/poweroff)
		und starten Sie neu.
		Dr�cken Sie nicht einfach nur Reset, dabei k�nnen die beim
		Remote-Update vorgenommenen �nderungen verloren gehen.
		
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
		Dr�cken Sie nicht einfach nur Reset, dabei k�nnen die
		beim Remote-Update bzw. bei der Re-Partitionierung
		vorgenommenen �nderungen verloren gehen.
		
EOF
        ;;
    *)
        echo "Unknown text message requested, please inform author."
	exit 1
	;;
esac
exit 0
