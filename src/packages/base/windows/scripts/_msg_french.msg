; -------------------------------------------------------------------------
; language constants (french)                                  __FLI4LVER__
;
; Creation: Bernard Charrier 2005-10-02
; $Id$
; -------------------------------------------------------------------------

[Global]
MSG_LINE===================================================================
MSG_BTN_close=Fermer
MSG_msgbox_error=Erreur
MSG_msgbox_warn=Avertissement
MSG_msgbox_info=Info
MSG_errorfilelog=Impossible d'ouvrir le fichier journal !
MSG_errorfilelogerror=Impossible d'ouvrir le fichier journal des erreurs !
MSG_finished=Terminer.
MSG_cancel=Annuler.
MSG_00_scpstart=Transfert des archives avec SCP sur le routeur ...
MSG_00_scpend=Le transfert des archives est terminé.
MSG_00_scpend2=Redémarrer maintenant votre routeur fli4l.
MSG_00_pass=Mot de passe pour le transfert avec SCP
MSG_00_usebasepass=Utiliser le mot de passe qui est dans 'base.txt' ?
MSG_00_scpenterpass=Entrer votre mot de passe pour le transfert des archives :
MSG_00_scpcancel=Le transfert des archives a été annulé.
MSG_00_scperror1=Une erreur est survenu pendant le transfert des archives !
MSG_00_scperror2=Voulez-vous vérifier tous les paramètres SSH et SSHD sur le routeur.
MSG_00_changesshkey=Sélectionner le fichier key-SSH
MSG_00_plinkaskreboot=Voulez-vous redémarrer le routeur maintenant ?
MSG_00_plinksentreboot=La commande de redémarrage a été envoyée au routeur.
MSG_00_askreboot=Le routeur fli4l - Redémarre

[mkfli4l]
MSG_01_build_abort=Le processus de construction a été arrêté par l'utilisateur.
MSG_01_build_error=Le processus de construction a été arrêté à cause d'une erreur !
MSG_01_no_boot_type=Valeur invalide dans la variable BOOT_TYPE !

[parse_cmd]
MSG_02_start=Processus de construction - __FLI4LVER__
MSG_02_noconfdir=Le fichier base.txt n'existe pas dans le répertoire de configuration fli4l !
MSG_02_useconfig=Utiliser la configuration à partir du répertoire '
MSG_02_undefboottype=La variable BOOT_TYPE n'est pas définie ! Utiliser S.V.P. un paramètre correct.
MSG_02_migrateconfig=Migration de mkfli4l.conf à mkfli4l.txt...

[parse_mkfli4l_conf]
MSG_03_readcheck_buildenv=Lecture et contrôle de l'environnement de construction '
MSG_03_nologfile=Impossible d'ouvrir le fichier log !

[mkopt]
MSG_04_start=Création des archives-fli4l ...
MSG_04_confdir=Utilisation de la configuration du répertoire '
MSG_04_builddir=Création des archives-fli4l dans le répertoire '
MSG_04_readcheckconf=Lecture et contrôle de la configuration, création du rootfs- et de l'archive-opt ...
MSG_04_generating_md5sums=Génération du md5 pour un contrôle total ...
MSG_04_finished=La création de la construction des archives-fli4l est terminée.
MSG_04_nologfile=Impossible d'ouvrir le fichier log !
MSG_04_msgboxerror=Erreur dans votre fichier de configuration fli4l !

[mkfloppy]
MSG_05_start=Création de la/les disquette(s) ...
MSG_05_insertfloppy=S.V.P. insérer une disquette formatée.
MSG_05_bootdisk=(cette disquette est votre BOOT_DISK)
MSG_05_startcopy=Cliquez sur OK ou pressez ENTER pour démarrer la copie des fichiers.
MSG_05_copysyslinux=Copie syslinux.cfg ...
MSG_05_writesystem=Écriture des fichiers systèmes ...
MSG_05_copykernel=Copie kernel ...
MSG_05_copyrootfstgz=Copie rootfs.img ...
MSG_05_copyrootfsbz2=Copie rootfs.bz2 ...
MSG_05_copyrccfg=Copie rc.cfg ...
MSG_05_copyopttarbz2=Copie opt.img ...
MSG_05_bootdisk2= (BOOT_DISK)
MSG_05_optdisk= (OPT_DISK)
MSG_05_secfloppy1=Enlever la premier disquette (BOOT_DISK).
MSG_05_secfloppy2=Insérer une disquette formatée et pressez ENTER ou cliquez sur OK.
MSG_05_secfloppy3=(cette disquette est votre OPT_DISK)
MSG_05_floppynowrite1=Lors de l'écriture des fichiers '
MSG_05_floppynowrite2=une erreur a été rencontrée !
MSG_05_floppynospace=Pas assez d'espace sur la disquette pour enregistrer les archives.
MSG_05_bootflnospace=Pas assez d'espace sur la disquette BOOT pour enregistrer les archives.
MSG_05_cancel=L'écriture de la/les disquette(s) a été arrêtée par l'utilisateur.
MSG_05_error=Erreur d'écriture sur la disquette !
MSG_05_finish1=La création de la/les disquette(s) fli4l est terminée.
MSG_05_finish2=- Vous pouvez installer la/les disquette(s) dans votre routeur et démarrer celui-ci.
MSG_05_filesonfloppy=La disquette contient des fichiers. Les supprimer ?
MSG_05_usefloppy=Utiliser quand même cette disquette ?
MSG_05_nofloppy=Aucune disquette trouvée !
MSG_05_nofloppyW9x=Aucune disquette formatée trouvée !
MSG_05_calculatespace=Calcul de l'espace requit ...
MSG_05_requiredspace1=Espace requit :
MSG_05_requiredspace2=Octet.
MSG_05_requiredspace3=Espace disponible :
MSG_05_floppyseemsboot=Attention : cette disquette semble être une BOOT_DISK !
MSG_05_exchangefloppy=Installer une autre disquette et cliquez sur OK pour continuer
MSG_05_floppynotformatted1=La disquette n'est pas formatée !
MSG_05_floppynotformatted2=voulez-vous la formater ?
MSG_05_formatfloppy=Formatage de la disquette ...

[mkiso]
MSG_06_createisoimage=Création d'une image-iso-fli4l ...
MSG_06_buildisoimage=Construction de l'image-iso '
MSG_06_finish1=La création de l'image-iso-fli4l est terminée
MSG_06_finish2=- Après avoir gravé l'image, vous pouvez installer le CD dans votre routeur et démarrer celui-ci.

[mknetboot]
MSG_07_mld1=La création de l'image pour le démarrage par le réseau n'est pas
MSG_07_mld2=encore possible avec toutes les versions de Microsoft Windows !

[dlg_mkfli4l_conf]
MSG_08_btnabort=Arrêter la construction fli4l
MSG_08_btnbuild=Démarrer la construction fli4l
MSG_08_nobasetxt=Le fichier 'base.txt' n'existe pas dans le répertoire
MSG_08_changedirconfig=Répertoire des fichiers de configuration fli4l
MSG_08_changedconfigdir1=Changer le répertoire de configuration
MSG_08_changedconfigdir2=Le répertoire choisi doit-il être utilisé par défaut ?
MSG_08_changedirbuild=Création du répertoire "build" pour les archives-fli4l
MSG_08_dirconfiginfo1=Les modifications des options du 'Config Dir' peuvent être sauvegardées dans le fichier 'mkfli4l.txt'.
MSG_08_dirconfiginfo2=Vous pouvez aussi sauvegarder les options par ligne de commande.
MSG_08_savemkfli4lconf=Sauvegarder la configuration dans mkfli4l.txt
MSG_08_savemkfli4ldone=Votre configuration a été sauvegardée dans mkfli4l.txt
MSG_08_savemkfli4lerror=Erreur d'écriture dans le fichier mkfli4l.txt
MSG_08TT_btnconfigdir=Changer le répertoire des fichiers de configuration
MSG_08TT_btnbuilddir=Changer le répertoire pour la construction des archives-fli4l
MSG_08TT_verbose=Activer pour détailler le processus de construction
MSG_08TT_filesonly=Activer pour seulement créer les archives-fli4l
MSG_08TT_drive=Sélectionner le lecteur de disquettes
MSG_08TT_floppy_normal=Sélectionner le mode normal du lecteur de disquettes
MSG_08TT_floppy_safe=Sélectionner le mode lent du lecteur de disquettes
MSG_08TT_remoteupdate=Activer pour transférer les archives-fli4l
MSG_08TT_remotehostname=Nom d'hôte ou adresse-IP pour le transfert des archives
MSG_08TT_remoteusername=Nom d'utilisateur pour le transfert des archives
MSG_08TT_remotepathname=Répertoire de destination pour le transfert des archives
MSG_08TT_remoteport=Port pour le transfert des archives
MSG_08TT_btn_sshfile=Rechercher le fichier key-SSH
MSG_08TT_btn_sshfileclear=Suppression de l'utilisation de la key-SSH -> Nom d'utilisateur/mot de passe

[mkhdinstall]
MSG_09_start=Sélectionner le disque dur pour l'installation
MSG_09_copysyslinux=Copie syslinux.cfg ...
MSG_09_copykernel=Copie kernel ...
MSG_09_copyrootfstgz=Copie rootfs.img ...
MSG_09_copyrootfsbz2=Copie rootfs.bz2 ...
MSG_09_copyrccfg=Copie rc.cfg ...
MSG_09_copyopttarbz2=Copie opt.img ...
MSG_09_writesystem=Écriture des fichiers systèmes ...
MSG_09_finish1=La création pour la pré-installation du HD est terminée
MSG_09_finish2=- Vous pouvez installer le disque dur dans votre routeur et démarrer celui-ci.
MSG_09_cancel=L'écriture de la pré-installation du HD a été arrêtée par l'utilisateur
MSG_09_error=Erreur d'écriture des archives sur la pré-installation du HD !
MSG_09_admin1=Vous n'avez pas les Droits -Admin- ils sont nécessaires pour faire un disque amorçable,
MSG_09_admin2=voulez-vous exécuter 'syslinux' en tant qu'administrateur ?
MSG_09_ask_pass=Mot de passe administrateur
MSG_09_pass=Mot de passe :
MSG_09_wrongfs1=Le lecteur
MSG_09_wrongfs2=n'a pas de fichier système FAT !
MSG_09_errorcopy=Erreur lors de la copie des archives

[mkls120]
MSG_10_start=Sélectionner le lecteur LS120 pour l'installation
MSG_10_copysyslinux=Copie syslinux.cfg ...
MSG_10_copykernel=Copie kernel ...
MSG_10_copyrootfstgz=Copie rootfs.img ...
MSG_10_copyrootfsbz2=Copie rootfs.bz2 ...
MSG_10_copyrccfg=Copie rc.cfg ...
MSG_10_copyopttarbz2=Copie opt.img ...
MSG_10_writesystem=Écriture des fichiers systèmes ...
MSG_10_finish1=La création du lecteur LS120 fli4l est terminée
MSG_10_finish2=- Vous pouvez installer le LS120 dans votre routeur et démarrer celui-ci.
MSG_10_cancel=L'écriture du lecteur LS120 a été arrêtée par l'utilisateur.
MSG_10_error=Erreur d'écriture sur le lecteur LS120 !
MSG_10_wrongfs1=Le lecteur
MSG_10_wrongfs2=n'a pas de fichier système FAT !
MSG_10_errorcopy=Erreur lors de la copie des archives
