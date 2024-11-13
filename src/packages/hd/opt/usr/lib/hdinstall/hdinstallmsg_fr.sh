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
         cat | sed 's/�/î/g;s/�/é/g;s/�/è/g;s/�/à/g;s/�/ê/g;s/�/ô/g;s/�/û/g;s/�/â/g'
    cat
}


km=`cat /etc/kernel-minor`
clrhome
case $1 in

    info)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Choisir le disque dur

		Appuyez sur entr�e pour confirmer l'installation du disque dur propos�,
		vous pouvez aussi enregistrer un autre type de disque valide.

		Pour l'installation d'un autre disque dur, entrer l'un des param�tres
		suivants.
		  Primaire IDE, ma�tre       :   hda
		  Primaire IDE, esclave      :   hdb
		  Secondaire IDE, ma�tre     :   hdc
		  Secondaire IDE, esclave    :   hdd
		  Premier disque (S)ATA/SCSI :   sda

		Indiquez un nouveau param�tre ou appuyez sur X pour sortir du programme:
EOF
        ;;

    inv_partition_table)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		La table de partition est dans un �tat instable. Pour assurer une
		bonne installation, vous devez �crire une table de partition valide
		sur votre disque avant de continuer l'installation. L'�criture
		d'une nouvelle table de partition va d�truire toutes les donn�es 
		sur votre disque.

		Voulez-vous r��crire la table de partition maintenant ?

		*****************************************************************
		* C'est votre derni�re chance avant la perte de vos donn�es !   *
		* Toutes les donn�es sur votre disque dur SERONT SUPPRIMEES de  *
		* mani�re irr�versible !                                        *
		*****************************************************************

		Entrez OUI en majuscule pour continuer:
EOF
        ;;

    installation_type)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Choisir le type d'installation

		Type A: tous les fichiers n�cessaires sont stock�s sur une partition
		DOS. Au d�marrage, tout est extrait et install� dans la m�moire RAM
		et tous les programmes fonctionneront sur la m�moire RAM du routeur.
		Le disque dur est seulement utilis� comme une disquette de grande capacit�.

		Type B: utilise une partition ext3 en plus de la m�moire RAM.
		Les fichiers d'installation "boot" sont stock�s sur la partition DOS.
		Les programmes inclus dans opt.img sont d�compress�s automatiquement
		et install�s sur la partition ext3, ils seront d�marr�s sur cette partition.
		L'avantage de ce syst�me, c'est qu'il utilise tr�s peu de m�moire RAM.

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

		Cr�ation de la partition DOS

		Taille de la partition DOS ?

		La taille minimale est de $param2 m�ga-octets, la taille recommand�e est
		entre 4 et 8 m�ga-octets. La taille maximale est de 128 m�ga-octets, si
		vous augmentez la taille elle sera r�duite automatiquement � 128 m�ga-octets
		sur votre disque dur ou Compact-Flash.

		Vous pouvez �crire la taille d�sir�e en m�ga-octets ou appuyez sur 1
		pour utiliser la taille maximale de la partition ou sur X pour sortir
		du programme:
EOF
        ;;

    opt_partition)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Cr�ation de la partition OPT

		Taille de la partition pour la d�compression de l'opt.img ?

		La taille minimale est de 2 m�ga-octets, la taille recommand�e est entre
		4 et 16 m�ga-octets. La taille maximale est de 512 m�ga-octets, si vous
		augmentez la taille elle sera r�duite automatiquement � 512 m�ga-octets
		sur votre disque dur ou Compact-Flash.

		Vous pouvez �crire la taille d�sir�e en m�ga-octets ou appuyez sur 1
		pour utiliser la taille maximale de la partition ou sur X pour sortir
		du programme:
EOF
        ;;

    swap_partition)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Cr�ation de la partition SWAP

		Voulez-vous cr�er une partition swap ?

		En g�n�ral la partition swap n'est pas n�cessaire, vous devez la cr�er
		seulement si vous avez peu de m�moire RAM disponible. La partition swap
		doit �tre au moins �gale � la m�moire RAM install�e sur le routeur.
		La taille maximale est de 256 m�ga-octets, si vous augmentez la taille
		elle sera r�duite automatiquement � 256 m�ga-octets sur votre disque dur.

		Vous devrez cr�er une partition swap uniquement sur un disque dur.
		Si vous utilisez un Compact-Flash, vous ne devez pas cr�er de
		partition swap !

		Vous pouvez �crire la taille d�sir�e en m�ga-octets ou appuyez sur 1 pour
		utiliser la taille maximale de la partition ou appuyez sur entr�e si vous
		ne voulez pas cr�er de partition swap ou sur X pour sortir du programme:
EOF
        ;;

    data_partition)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Cr�ation de la partition DATA

		Voulez-vous cr�er une partition suppl�mentaire ?

		Seulement n�cessaire si vous utilisez un ou plusieurs programmes sur votre
		routeur pour stocker des fichiers, par exemple opt_vbox ou opt_samba_lpd.
		Alors, une partition ext3 peut �tre cr��e. La taille maximale est de
		2 t�ra-octets, si vous augmentez la taille elle sera r�duite automatiquement
		� 2 t�ra-octets sur votre disque dur.

		Vous pouvez �crire la taille d�sir�e en m�ga-octets ou appuyez sur 1 pour
		utiliser la taille maximale de la partition ou appuyez sur entr�e si vous
		ne voulez pas cr�er de partition data ou sur X pour sortir du programme:
EOF
        ;;

    final_warning)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Derni�re �tape avant le partitionnement

		*****************************************************************
		* C'est votre derni�re chance avant la perte de vos donn�es !   *
		* Toutes les donn�es sur votre disque dur SERONT SUPPRIMEES de  *
		* mani�re irr�versible !                                        *
		*****************************************************************

		S'il vous pla�t entrez OUI en majuscule pour cr�er les partitions d�finies
		ou sur X pour sortir du programme:
EOF
        ;;

    final_warning_1)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Contr�le de s�curit� avant le partitionnement

		S'il vous pla�t entrez OUI en majuscule pour cr�er les partitions d�finies:
EOF
        ;;

    recovery_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Impossible d'installer la version de secours "recovery". Vous devez
		partitionner � nouveau votre disque pour installer la version de secours.

		S'il vous pla�t entrez OUI en majuscule pour continuer:
EOF
        ;;

    standard_saving_failed)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Impossible d'installation des fichiers de boot par d�faut. Vous devez
		partitionner � nouveau votre disque pour installer la version des fichiers
		de boot sur le routeur.

		S'il vous pla�t entrez OUI en majuscule pour continuer:
EOF
        ;;

    finish)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Que dois-je faire ensuite ?

		Le disque dur a �t� partitionn� et format�, il est maintenant accessible
		depuis le /boot. Vous pouvez modifier la configuration selon vos besoins
		en utilisant les paquetages Opt et transf�rez les fichiers cr��s avec Imonc
		ou avec fli4l-Build ou avec SCP dans le /boot du disque dur.

		Les archives suivantes sont n�cessaires pour initialiser le disque dur:
		syslinux.cfg, kernel, rootfs.img, opt.img et rc.cfg

		ATTENTION: vous devez transf�rer les fichiers mentionn�s ci-dessus sur
		le routeur (avant de le red�marrer), autrement le routeur ne pourra pas
		booter sur le disque dur !

		Apr�s une nouvelle mise � jour � distance ou par disquette, retirez
		la disquette de votre routeur et red�marrez le routeur avec l'une des
		commandes suivantes reboot/halt/poweroff. Ne pas simplement red�marrer
		votre routeur avec le bouton reset, autrement, les derni�res modifications
		ne seront pas prises en compte.
EOF
        ;;

    finish_repartitioning)
        ascutfcat <<-EOF
		fli4l - Installation d'un disque dur / Compact-Flash

		Que dois-je faire ensuite ?

		Le disque dur a �t� � nouveau partitionn� et format�, vous pouvez
		red�marrer votre routeur.

		Vous pouvez aussi mettre � jour le routeur avec l'option remote uptade
		du fli4l-Build si vous le souhaitez.

		Vous devez ensuite red�marrer le routeur avec l'une des commandes
		suivantes reboot/halt/poweroff. Ne vous contentez pas d'appuyer sur
		le bouton reset de votre routeur, autrement, les derni�res modifications
		ne seront pas prises en compte.
EOF
        ;;
    *)
        echo "Unknown text message requested, please inform author."
	exit 1
        ;;
esac
exit 0
