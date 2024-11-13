#! /bin/sh

#
# here documents have to be indented by tabs, if you expect the 
# indentation to be removed by cat <<-EOF
#

if [ -n "$2" ]
then
  param2=$2
else
  param2=''
fi

ascutfcat ()
{
    [ $km -gt 16 ] &&
         cat | sed 's/î/Ã®/g;s/é/Ã©/g;s/è/Ã¨/g;s/à/Ã /g;s/ê/Ãª/g;s/ô/Ã´/g;s/û/Ã»/g;s/â/Ã¢/g'
    cat
}


km=`cat /etc/kernel-minor`
clrhome
case $1 in

    info)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Choisir le disque dur

		Appuyez sur entrée pour confirmer l'installation du disque dur proposé,
		vous pouvez aussi enregistrer un autre type de disque valide.

		Pour l'installation d'un autre disque dur, entrer l'un des paramètres
		suivants.
		  Primaire IDE, maître       :   hda
		  Primaire IDE, esclave      :   hdb
		  Secondaire IDE, maître     :   hdc
		  Secondaire IDE, esclave    :   hdd
		  Premier disque (S)ATA/SCSI :   sda

		Indiquez un nouveau paramètre ou appuyez sur X pour sortir du programme:
EOF
        ;;

    inv_partition_table)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		La table de partition est dans un état instable. Pour assurer une
		bonne installation, vous devez écrire une table de partition valide
		sur votre disque avant de continuer l'installation. L'écriture
		d'une nouvelle table de partition va détruire toutes les données 
		sur votre disque.

		Voulez-vous réécrire la table de partition maintenant ?

		*****************************************************************
		* C'est votre dernière chance avant la perte de vos données !   *
		* Toutes les données sur votre disque dur SERONT SUPPRIMEES de  *
		* manière irréversible !                                        *
		*****************************************************************

		Entrez OUI en majuscule pour continuer:
EOF
        ;;

    installation_type)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Choisir le type d'installation

		Type A: tous les fichiers nécessaires sont stockés sur une partition
		DOS. Au démarrage, tout est extrait et installé dans la mémoire RAM
		et tous les programmes fonctionneront sur la mémoire RAM du routeur.
		Le disque dur est seulement utilisé comme une disquette de grande capacité.

		Type B: utilise une partition ext3 en plus de la mémoire RAM.
		Les fichiers d'installation "boot" sont stockés sur la partition DOS.
		Les programmes inclus dans opt.img sont décompressés automatiquement
		et installés sur la partition ext3, ils seront démarrés sur cette partition.
		L'avantage de ce système, c'est qu'il utilise très peu de mémoire RAM.

		Choisissez S.V.P. le type d'installation, entrez A ou B en majuscule ou
		appuyez sur X pour sortir du programme:
EOF
        ;;

    dos_partition)
        if [ -z "$param2" ]
        then
          param2=2
        fi
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Création de la partition DOS

		Taille de la partition DOS ?

		La taille minimale est de $param2 méga-octets, la taille recommandée est
		entre 4 et 8 méga-octets. La taille maximale est de 128 méga-octets, si
		vous augmentez la taille elle sera réduite automatiquement à 128 méga-octets
		sur votre disque dur ou Compact-Flash.

		Vous pouvez écrire la taille désirée en méga-octets ou appuyez sur 1
		pour utiliser la taille maximale de la partition ou sur X pour sortir
		du programme:
EOF
        ;;

    opt_partition)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Création de la partition OPT

		Taille de la partition pour la décompression de l'opt.img ?

		La taille minimale est de 2 méga-octets, la taille recommandée est entre
		4 et 16 méga-octets. La taille maximale est de 512 méga-octets, si vous
		augmentez la taille elle sera réduite automatiquement à 512 méga-octets
		sur votre disque dur ou Compact-Flash.

		Vous pouvez écrire la taille désirée en méga-octets ou appuyez sur 1
		pour utiliser la taille maximale de la partition ou sur X pour sortir
		du programme:
EOF
        ;;

    swap_partition)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Création de la partition SWAP

		Voulez-vous créer une partition swap ?

		En général la partition swap n'est pas nécessaire, vous devez la créer
		seulement si vous avez peu de mémoire RAM disponible. La partition swap
		doit être au moins égale à la mémoire RAM installée sur le routeur.
		La taille maximale est de 256 méga-octets, si vous augmentez la taille
		elle sera réduite automatiquement à 256 méga-octets sur votre disque dur.

		Vous devrez créer une partition swap uniquement sur un disque dur.
		Si vous utilisez un Compact-Flash, vous ne devez pas créer de
		partition swap !

		Vous pouvez écrire la taille désirée en méga-octets ou appuyez sur 1 pour
		utiliser la taille maximale de la partition ou appuyez sur entrée si vous
		ne voulez pas créer de partition swap ou sur X pour sortir du programme:
EOF
        ;;

    data_partition)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Création de la partition DATA

		Voulez-vous créer une partition supplémentaire ?

		Seulement nécessaire si vous utilisez un ou plusieurs programmes sur votre
		routeur pour stocker des fichiers, par exemple opt_vbox ou opt_samba_lpd.
		Alors, une partition ext3 peut être créée. La taille maximale est de
		2 téra-octets, si vous augmentez la taille elle sera réduite automatiquement
		à 2 téra-octets sur votre disque dur.

		Vous pouvez écrire la taille désirée en méga-octets ou appuyez sur 1 pour
		utiliser la taille maximale de la partition ou appuyez sur entrée si vous
		ne voulez pas créer de partition data ou sur X pour sortir du programme:
EOF
        ;;

    final_warning)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Dernière étape avant le partitionnement

		*****************************************************************
		* C'est votre dernière chance avant la perte de vos données !   *
		* Toutes les données sur votre disque dur SERONT SUPPRIMEES de  *
		* manière irréversible !                                        *
		*****************************************************************

		S'il vous plaît entrez OUI en majuscule pour créer les partitions définies
		ou sur X pour sortir du programme:
EOF
        ;;

    final_warning_1)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Contrôle de sécurité avant le partitionnement

		S'il vous plaît entrez OUI en majuscule pour créer les partitions définies:
EOF
        ;;

    recovery_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Impossible d'installer la version de secours "recovery". Vous devez
		partitionner à nouveau votre disque pour installer la version de secours.

		S'il vous plaît entrez OUI en majuscule pour continuer:
EOF
        ;;

    standard_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Impossible d'installation des fichiers de boot par défaut. Vous devez
		partitionner à nouveau votre disque pour installer la version des fichiers
		de boot sur le routeur.

		S'il vous plaît entrez OUI en majuscule pour continuer:
EOF
        ;;

    finish)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Que dois-je faire ensuite ?

		Le disque dur a été partitionné et formaté, il est maintenant accessible
		depuis le /boot. Vous pouvez modifier la configuration selon vos besoins
		en utilisant les paquetages Opt et transférez les fichiers créés avec Imonc
		ou avec fli4l-Build ou avec SCP dans le /boot du disque dur.

		Les archives suivantes sont nécessaires pour initialiser le disque dur:
		syslinux.cfg, kernel, rootfs.img, opt.img et rc.cfg

		ATTENTION: vous devez transférer les fichiers mentionnés ci-dessus sur
		le routeur (avant de le redémarrer), autrement le routeur ne pourra pas
		booter sur le disque dur !

		Après une nouvelle mise à jour à distance ou par disquette, retirez
		la disquette de votre routeur et redémarrez le routeur avec l'une des
		commandes suivantes reboot/halt/poweroff. Ne pas simplement redémarrer
		votre routeur avec le bouton reset, autrement, les dernières modifications
		ne seront pas prises en compte.
EOF
        ;;

    finish_repartitioning)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Que dois-je faire ensuite ?

		Le disque dur a été à nouveau partitionné et formaté, vous pouvez
		redémarrer votre routeur.

		Vous pouvez aussi mettre à jour le routeur avec l'option remote uptade
		du fli4l-Build si vous le souhaitez.

		Vous devez ensuite redémarrer le routeur avec l'une des commandes
		suivantes reboot/halt/poweroff. Ne vous contentez pas d'appuyer sur
		le bouton reset de votre routeur, autrement, les dernières modifications
		ne seront pas prises en compte.
EOF
        ;;
    *)
        echo "Unknown text message requested, please inform author."
	exit 1
        ;;
esac
exit 0
