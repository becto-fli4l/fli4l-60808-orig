#!/bin/sh
##-----------------------------------------------------------------------------
## /srv/www/admin/accounting.cgi - Frontend for httpd              __FLI4LVER__
##
## Creation:     05.06.2002  Michael Knipping <fli4l@knibo.de>
## Last Update:  $Id$
##
## Copyright (c) 2002-2009 - Michael Knipping <fli4l@knibo.de>
## Copyright (c) 2009-2016 - fli4l-Team <team@fli4l.de>
##-----------------------------------------------------------------------------

# get main helper functions
# set_debug="yes"
. /srv/www/include/cgi-helper
. /etc/boot.d/base-helper

# Security
check_rights "accounting" "view"

#------------------------------------------------------------------------------
# Config Datei einlesen, Abbruch wenn diese nicht existiert
#------------------------------------------------------------------------------
if [ -f "/etc/accounting.conf" ]; then
    . /etc/accounting.conf
else
    show_html_header "Accounting"
    show_error "Fehler" "<b>/etc/accounting.conf not found<br /></b>"
    show_html_footer
    exit 1
fi

#------------------------------------------------------------------------------
# Index Datei einlesen, Daten aktualisieren wenn diese nicht existiert
#------------------------------------------------------------------------------
if [ -f "$ACCOUNTING_DIR/index.acc" ]; then
    . $ACCOUNTING_DIR/index.acc
else
    FORM_action='aktualisieren'
fi

#------------------------------------------------------------------------------
# Variablen setzen
#------------------------------------------------------------------------------
myname=`basename $0`
month01='Januar'
month02='Februar'
month03='M&auml;rz'
month04='April'
month05='Mai'
month06='Juni'
month07='Juli'
month08='August'
month09='September'
month10='Oktober'
month11='November'
month12='Dezember'
monat=$(date +%m)
tag=$(date +%Y-%m-%d)
day=$(date +%d)
jahr=$(date +%Y)

#------------------------------------------------------------------------------
# Anzeigeeinstellungen
#------------------------------------------------------------------------------
if [ -f "$ACCOUNTING_DIR/view.conf" ]; then
    conf=`cat $ACCOUNTING_DIR/view.conf`
    set $conf
    while [ "$2" ]
    do
        eval $1=\$2
        shift 2
    done
fi

# Defaultwerte
: ${anzeigewert:="1048576"}
: ${anzeige:="MegaByte"}
: ${nachkomma:="1"}
: ${nulldata:="Nein"}
: ${yeardata:="Hosts"}
: ${namesolution:="DNS"}
: ${liverefresh:="5"}
: ${factorbw:="0.94"}

#------------------------------------------------------------------------------
# Standartaktion
#------------------------------------------------------------------------------
: ${FORM_action:=viewmonth} # aktueller Monat

#------------------------------------------------------------------------------
# fli4l Kopf, Men� und Maintabs
#------------------------------------------------------------------------------
case $FORM_action in
    livetraf)
        show_html_header "Accounting" "refresh=$liverefresh;url=$myname?action=livetraf"
    ;;
    *)
        show_html_header "Accounting" # "showmenu=no"
    ;;
esac

maintabs=""
case $FORM_action in
    viewyear|viewmonth|viewday|viewhost|viewint) maintabs="${maintabs}\"Daten\" \"no\" " ;;
    *) maintabs="${maintabs}\"Daten\" \"$myname\" " ;;
esac
case $FORM_action in
    viewperiod) maintabs="${maintabs}\"Zeitraum berechnen\" \"no\" " ;;
    *) maintabs="${maintabs}\"Zeitraum berechnen\" \"$myname?action=viewperiod\" " ;;
esac
case $FORM_action in
    livetraf|viewchains) maintabs="${maintabs}\"LiveTraf\" \"no\" " ;;
    *) maintabs="${maintabs}\"LiveTraf\" \"$myname?action=livetraf\" " ;;
esac
case $FORM_action in
    aktualisieren) maintabs="${maintabs}\"Aktualisieren\" \"no\" " ;;
    *) maintabs="${maintabs}\"Aktualisieren\" \"$myname?action=aktualisieren\" " ;;
esac
case $FORM_action in
    conf) maintabs="${maintabs}\"Einstellungen\" \"no\" " ;;
    *) maintabs="${maintabs}\"Einstellungen\" \"$myname?action=conf\" " ;;
esac
eval show_tab_header $maintabs

#------------------------------------------------------------------------------
# Funktionen
#------------------------------------------------------------------------------
cat_bc()
{
cat <<EOF | /usr/bin/bc
$1
EOF
}

cat_bc_format()
{
cat <<EOF | /usr/bin/bc | sed 's/\./,/; : loop; s/^\([0-9]\+\)\([0-9]\{3\}[,.]\?\)/\1.\2/; t loop'
$1
EOF
}

acc_tabs()
{
    yytabs=""
    for yy in $(ls -lA $ACCOUNTING_DIR/ | grep ^d | cut -c57-)
    do
        case $yy in
            $FORM_yy) yytabs="${yytabs}\"$yy\" \"no\" " ;;
            *) yytabs="${yytabs}\"$yy\" \"$myname?action=viewyear&amp;yy=$yy\" " ;;
        esac
    done
    eval show_tab_header $yytabs
    case $FORM_action in
        viewyear) mmtabs="\"Jahres&uuml;bersicht\" \"no\" " ;;
        *) mmtabs="\"Jahres&uuml;bersicht\" \"$myname?action=viewyear&amp;yy=$FORM_yy\" " ;;
    esac
    for mm in $(cd $ACCOUNTING_DIR/$FORM_yy/; ls $FORM_yy-??.acc | cut -d"." -f1 | cut -d"-" -f2)
    do
        case $mm in
            $FORM_mm)
                eval monthname=\$month$FORM_mm
                mmtabs="${mmtabs}\"$monthname\" \"no\" "
            ;;
            *) mmtabs="${mmtabs}\"$mm\" \"$myname?action=viewmonth&amp;yy=$FORM_yy&amp;mm=$mm\" " ;;
        esac
    done
    eval show_tab_header $mmtabs
}

ip2name()  # usage: ip2name ip
{
    case $namesolution in
        keine)
            acch=""
        ;;
        DNS)
            acch=$(/usr/local/bin/gethostname $1 | cut -d"." -f1)
        ;;
        index)
            ACCOUNTING_HOST_NR=$(grep "'$1'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
            eval acch="\$ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_NAME"
        ;;
    esac
}

tab_open()
{
cat <<EOF
<table class="out">
EOF
}

tab_head()  # usage: tab_head name parm
{
cat <<EOF
<tr class="tabhead">
  <td class="col1">$1</th>
  <td class="col2" colspan="2">eingehende $anzeige</th>
  <td class="col3" colspan="2">ausgehende $anzeige</th>
  <td class="col4" colspan="2">$anzeige Gesamt</th>
</tr>
EOF
}

tab_line_graph()  # usage: tab_line_graph name in out
{
    case $sumi in
        0) graphii="0" ;;
        *) graphii=$(cat_bc "100 * $2 / $sumi") ;;
    esac
    case $sumo in
        0) graphoo="0" ;;
        *) graphoo=$(cat_bc "100 * $3 / $sumo") ;;
    esac
    case $summe in
        0)
            graphis="0"
            graphos="0"
            graphssi="0"
            graphsso="0"
        ;;
        *)
            graphis=$(cat_bc "100 * $2 / $summe")
            graphos=$(cat_bc "100 * $3 / $summe")
            graphssi=$(cat_bc "100 * $sumi / $summe")
            graphsso=$(cat_bc "100 * $sumo / $summe")
        ;;
    esac
    case $linecl in
        1)
            tablinecolor="tabline2"
            linecl="2"
        ;;
        *)
            tablinecolor="tabline1"
            linecl="1"
        ;;
    esac
cat <<EOF
<tr class="$tablinecolor">
  <td class="col1">$1</td>
  <td class="col2_1">
      <table class="tabgraph" cellspacing="0">
        <tr>
          <td class="trafin" width="$graphii%"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
  </td>
  <td class="col2_2">
      $(cat_bc_format "scale=$nachkomma; $2 / $anzeigewert")
  </td>
  <td class="col3_1">
      <table class="tabgraph" cellspacing="0">
        <tr>
          <td class="trafout" width="$graphoo%"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
  </td>
  <td class="col3_2">
      $(cat_bc_format "scale=$nachkomma; $3 / $anzeigewert")
  </td>
  <td class="col4_1">
      <table class="tabgraph" cellspacing="0">
        <tr>
          <td class="trafin" width="$graphis%"></td>
          <td class="trafout" width="$graphos%"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
  </td>
  <td class="col4_2">
      $(cat_bc_format "scale=$nachkomma; ($2 + $3) / $anzeigewert")
  </td>
</tr>
EOF
}

tab_line_sum()
{
cat <<EOF
<tr class="sum">
  <td class="col1">
    Summen
  </td>
  <td class="col2_1">
      <table class="tabgraph" cellspacing="0">
        <tr>
          <td class="trafin" width="100%"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
  </td>
  <td class="col2_2">
      $(cat_bc_format "scale=$nachkomma; $sumi / $anzeigewert")
  </td>
  <td class="col3_1">
      <table class="tabgraph" cellspacing="0">
        <tr>
          <td class="trafout" width="100%"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
  </td>
  <td class="col3_2">
      $(cat_bc_format "scale=$nachkomma; $sumo / $anzeigewert")
  </td>
  <td class="col4_1">
      <table class="tabgraph" cellspacing="0">
        <tr>
          <td class="trafin" width="$graphssi%"></td>
          <td class="trafout" width="$graphsso%"></td>
          <td>&nbsp;</td>
        </tr>
      </table>
  </td>
  <td class="col4_2">
      $(cat_bc_format "scale=$nachkomma; $summe / $anzeigewert")
  </td>
</tr>
EOF
}

tab_line()  # usage: tab_line name in out
{
    case $linecl in
        1)
            tablinecolor="tabline2"
            linecl="2"
        ;;
        *)
            tablinecolor="tabline1"
            linecl="1"
        ;;
    esac
cat <<EOF
<tr class="$tablinecolor">
  <td class="col1">
    $1
  </td>
  <td class="col2" colspan="2">
    $(cat_bc_format "scale=$nachkomma; $2 / $anzeigewert")
  </td>
  <td class="col3" colspan="2">
    $(cat_bc_format "scale=$nachkomma; $3 / $anzeigewert")
  </td>
  <td class="col4" colspan="2">
    $(cat_bc_format "scale=$nachkomma; ($2 + $3) / $anzeigewert")
  </td>
</tr>
EOF
}

tab_line_diff()  # usage: tab_line_diff name in out
{
cat <<EOF
<tr class="diff">
  <td class="col1">$1</td>
  <td class="col2" colspan="2">
    $(cat_bc_format "scale=$nachkomma; ($2 - $sumi) / $anzeigewert")
  </td>
  <td class="col3" colspan="2">
    $(cat_bc_format "scale=$nachkomma; ($3 - $sumo) / $anzeigewert")
  </td>
  <td class="col4" colspan="2">
    $(cat_bc_format "scale=$nachkomma; ($2 + $3 - $sumi - $sumo) / $anzeigewert")
  </td>
</tr>
EOF
}

tab_line_blank()  # usage: tab_line_blank
{
cat <<EOF
<tr class="tabblank">
  <td colspan="7">&nbsp;</td>
</tr>
EOF
}

tab_close()
{
cat <<EOF
</table>
EOF
}

#------------------------------------------------------------------------------
# Jahres�bersicht
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewyear" ]; then
    : ${FORM_yy:=$jahr}
    acc_tabs
    yy=$FORM_yy
    sumi=0
    sumo=0
    for mm in $(cd $ACCOUNTING_DIR/$yy/; ls $yy-??.acc | cut -d"." -f1 | cut -d"-" -f2)
    do
        . $ACCOUNTING_DIR/$idx_y/$yy/$yy-$mm.acc
        case $yeardata in
            Interfaces)
                acci=0
                acco=0
                for acc_if in $ACCOUNTING_INT; do
                    ACCOUNTING_INT_NR=$(grep "'$acc_if'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
                    eval inti="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$yy"_M"$mm"_i"
                    eval into="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$yy"_M"$mm"_o"
                    acci=$(cat_bc "$acci + 0$inti")
                    acco=$(cat_bc "$acco + 0$into")
                done
            ;;
            *)
                eval acci="\$ACCOUNTING_Y"$yy"_M"$mm"_i"
                eval acco="\$ACCOUNTING_Y"$yy"_M"$mm"_o"
            ;;
        esac
        case $acci in
            ""|0) ;;
            *) sumi=$(cat_bc "$sumi + $acci") ;;
        esac
        case $acco in
            ""|0) ;;
            *) sumo=$(cat_bc "$sumo + $acco") ;;
        esac
    done
    summe=$(cat_bc "$sumi + $sumo")
    linecl="1"
    tab_open
    tab_head 'Monat'
    for mm in $(cd $ACCOUNTING_DIR/$yy/; ls $yy-??.acc | cut -d"." -f1 | cut -d"-" -f2)
    do
        case $yeardata in
            Interfaces)
                acci=0
                acco=0
                for acc_if in $ACCOUNTING_INT; do
                    ACCOUNTING_INT_NR=$(grep "'$acc_if'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
                    eval inti="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$yy"_M"$mm"_i"
                    eval into="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$yy"_M"$mm"_o"
                    acci=$(cat_bc "$acci + 0$inti")
                    acco=$(cat_bc "$acco + 0$into")
                done
            ;;
            *)
                eval acci="\$ACCOUNTING_Y"$yy"_M"$mm"_i"
                eval acco="\$ACCOUNTING_Y"$yy"_M"$mm"_o"
            ;;
        esac
        : ${acci:=0}
        : ${acco:=0}
        eval monthname=\$month$mm
        tab_line_graph '<a href="'$myname'?action=viewmonth&amp;yy='$yy'&amp;mm='$mm'">'$monthname'</a>' $acci $acco
    done
    tab_line_sum
    tab_close
    show_tab_footer
    show_tab_footer
fi

#------------------------------------------------------------------------------
# Tagesdaten
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewday" ]; then
    : ${FORM_yy:=$jahr}
    : ${FORM_mm:=$monat}
    : ${FORM_dd:=$day}
    eval monthname=\$month$FORM_mm
    acc_tabs
    show_tab_header "$FORM_dd. $monthname $FORM_yy" "no" \
                    "zur Monatsansicht" "$myname?action=viewmonth&amp;yy=$FORM_yy&amp;mm=$FORM_mm"
    . $ACCOUNTING_DIR/$FORM_yy/$FORM_yy-$FORM_mm.acc
    sumi=0
    sumo=0
    [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
    do
        eval acci="\$ACCOUNTING_HOST_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_i"
        eval acco="\$ACCOUNTING_HOST_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_o"
        case $acci in
            ""|0) ;;
            *) sumi=$(cat_bc "$sumi + $acci") ;;
        esac
        case $acco in
            ""|0) ;;
            *) sumo=$(cat_bc "$sumo + $acco") ;;
        esac
    done
    summe=$(cat_bc "$sumi + $sumo")
    linecl="1"
    tab_open
    tab_head 'Host'
    [ -f $workdir/sorthosts ] && accseq=$(cat $workdir/sorthosts)
    : ${accseq:=`seq 1 $ACCOUNTING_HOST_n`}
    for idx in $accseq
    do
        eval accip="\$ACCOUNTING_HOST_"$idx"_IP"
        eval acci="\$ACCOUNTING_HOST_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_i"
        eval acco="\$ACCOUNTING_HOST_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_o"
        : ${acci:=0}
        : ${acco:=0}
        if [ "$acci" != "0" -o "$acco" != "0" -o "$nulldata" = "Ja" ]; then
            ip2name $accip
            tab_line_graph "$accip $acch" $acci $acco
        fi
    done
    tab_line_sum
    tab_line_blank
    allaccint_i="0"
    allaccint_o="0"
    linecl="2"
    for acc_if in $ACCOUNTING_INT; do
        ACCOUNTING_INT_NR=$(grep "'$acc_if'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
        eval acc_if_name="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_NAME"
        eval wertin="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_i"
        eval wertout="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_o"
        translate_net_if_reverse $acc_if_name acc_if_name
        : ${wertin:=0}
        : ${wertout:=0}
        allaccint_i=$(cat_bc "$allaccint_i + $wertin")
        allaccint_o=$(cat_bc "$allaccint_o + $wertout")
        tab_line "Interface $acc_if_name" $wertin $wertout
    done
    tab_line_diff "Differenz" $allaccint_i $allaccint_o
    tab_line_blank
    tab_head 'Interface'
    linecl="1"
    [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
    do
        eval intn="\$ACCOUNTING_INT_"$idx"_NAME"
        eval inti="\$ACCOUNTING_INT_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_i"
        eval into="\$ACCOUNTING_INT_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_D"$FORM_dd"_o"
        translate_net_if_reverse $intn intn
        : ${inti:=0}
        : ${into:=0}
        if [ "$inti" != "0" -o "$into" != "0" -o "$nulldata" = "Ja" ]; then
            tab_line $intn $inti $into
        fi
    done
    tab_close
    show_tab_footer
    show_tab_footer
    show_tab_footer
fi

#------------------------------------------------------------------------------
# Monatsdaten
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewmonth" ]; then
    : ${FORM_yy:=$jahr}
    : ${FORM_mm:=$monat}
    eval monthname=\$month$FORM_mm
    acc_tabs
    . $ACCOUNTING_DIR/$FORM_yy/$FORM_yy-$FORM_mm.acc
    eval sumi="\$ACCOUNTING_Y"$FORM_yy"_M"$FORM_mm"_i"
    eval sumo="\$ACCOUNTING_Y"$FORM_yy"_M"$FORM_mm"_o"
    summe=$(cat_bc "$sumi + $sumo")
    tab_open
    tab_head 'Host'
    linecl="1"
    [ -f $workdir/sorthosts ] && accseq=$(cat $workdir/sorthosts)
    : ${accseq:=`seq 1 $ACCOUNTING_HOST_n`}
    for idx in $accseq
    do
        eval accip="\$ACCOUNTING_HOST_"$idx"_IP"
        eval acci="\$ACCOUNTING_HOST_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_i"
        eval acco="\$ACCOUNTING_HOST_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_o"
        : ${acci:=0}
        : ${acco:=0}
        if [ "$acci" != "0" -o "$acco" != "0" -o "$nulldata" = "Ja" ]; then
            ip2name $accip
            tab_line_graph '<a href="'$myname'?action=viewhost&amp;yy='$FORM_yy'&amp;mm='$FORM_mm'&amp;hostn='$idx'">'$accip' '$acch'</a>' $acci $acco
        fi
    done
    tab_line_sum
    tab_line_blank
    allaccint_i="0"
    allaccint_o="0"
    linecl="2"
    for acc_if in $ACCOUNTING_INT; do
        ACCOUNTING_INT_NR=$(grep "'$acc_if'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
        eval acc_if_name="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_NAME"
        eval wertin="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$FORM_yy"_M"$FORM_mm"_i"
        eval wertout="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$FORM_yy"_M"$FORM_mm"_o"
        : ${wertin:=0}
        : ${wertout:=0}
        translate_net_if_reverse $acc_if_name acc_if_name
        allaccint_i=$(cat_bc "$allaccint_i + $wertin")
        allaccint_o=$(cat_bc "$allaccint_o + $wertout")
        tab_line '<a href="'$myname'?action=viewint&amp;yy='$FORM_yy'&amp;mm='$FORM_mm'&amp;intn='$ACCOUNTING_INT_NR'">Interface '$acc_if_name'</a>' $wertin $wertout
    done
    tab_line_diff "Differenz" $allaccint_i $allaccint_o
    tab_line_blank
    tab_head "Interface"
    linecl="1"
    [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
    do
        eval intn="\$ACCOUNTING_INT_"$idx"_NAME"
        eval inti="\$ACCOUNTING_INT_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_i"
        eval into="\$ACCOUNTING_INT_"$idx"_Y"$FORM_yy"_M"$FORM_mm"_o"
        : ${inti:=0}
        : ${into:=0}
        translate_net_if_reverse $intn intn
        if [ "$inti" != "0" -o "$into" != "0" -o "$nulldata" = "Ja" ]; then
            tab_line '<a href="'$myname'?action=viewint&amp;yy='$FORM_yy'&amp;mm='$FORM_mm'&amp;intn='$idx'">'$intn'</a>' $inti $into
        fi
    done
    tab_close
    show_tab_footer
    show_tab_footer
fi

#------------------------------------------------------------------------------
# Monatsansicht Hosts
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewhost" ]; then
    . $ACCOUNTING_DIR/$FORM_yy/$FORM_yy-$FORM_mm.acc
    eval days="\$ACCOUNTING_Y"$FORM_yy"_M"$FORM_mm"_DAYS"
    eval accip="\$ACCOUNTING_HOST_"$FORM_hostn"_IP"
    ip2name $accip
    eval sumi="\$ACCOUNTING_HOST_"$FORM_hostn"_Y"$FORM_yy"_M"$FORM_mm"_i"
    eval sumo="\$ACCOUNTING_HOST_"$FORM_hostn"_Y"$FORM_yy"_M"$FORM_mm"_o"
    : ${sumi:=0}
    : ${sumo:=0}
    summe=$(cat_bc "$sumi + $sumo")
    eval monthname=\$month$FORM_mm
    acc_tabs
    show_tab_header "$accip $acch - $monthname $FORM_yy" "no" \
                    "zur Monatsansicht" "$myname?action=viewmonth&amp;yy=$FORM_yy&amp;mm=$FORM_mm"
    tab_open
    tab_head 'Tag'
    linecl="1"
    [ 0$days -eq 0 ] || for idx in `seq 1 $days`
    do
        [ "$idx" -le "9" ] && dayn="0$idx" || dayn=$idx
        eval acci="\$ACCOUNTING_HOST_"$FORM_hostn"_Y"$FORM_yy"_M"$FORM_mm"_D"$dayn"_i"
        eval acco="\$ACCOUNTING_HOST_"$FORM_hostn"_Y"$FORM_yy"_M"$FORM_mm"_D"$dayn"_o"
        : ${acci:=0}
        : ${acco:=0}
        tab_line_graph '<a href="'$myname'?action=viewday&amp;yy='$FORM_yy'&amp;mm='$FORM_mm'&amp;dd='$dayn'">'$dayn'</a>' $acci $acco
    done
    tab_line_sum
    tab_close
    show_tab_footer
    show_tab_footer
    show_tab_footer
fi

#------------------------------------------------------------------------------
# Monatsansicht Interfaces
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewint" ]; then
    . $ACCOUNTING_DIR/$FORM_yy/$FORM_yy-$FORM_mm.acc
    eval days="\$ACCOUNTING_Y"$FORM_yy"_M"$FORM_mm"_DAYS"
    eval intn="\$ACCOUNTING_INT_"$FORM_intn"_NAME"
    eval sumi="\$ACCOUNTING_INT_"$FORM_intn"_Y"$FORM_yy"_M"$FORM_mm"_i"
    eval sumo="\$ACCOUNTING_INT_"$FORM_intn"_Y"$FORM_yy"_M"$FORM_mm"_o"
    translate_net_if_reverse $intn intn
    : ${sumi:=0}
    : ${sumo:=0}
    summe=$(cat_bc "$sumi + $sumo")
    eval monthname=\$month$FORM_mm
    acc_tabs
    show_tab_header "$intn - $monthname $FORM_yy" "no" \
                    "zur Monatsansicht" "$myname?action=viewmonth&amp;yy=$FORM_yy&amp;mm=$FORM_mm"
    tab_open
    tab_head 'Tag'
    linecl="1"
    [ 0$days -eq 0 ] || for idx in `seq 1 $days`
    do
        [ "$idx" -le "9" ] && dayn="0$idx" || dayn=$idx
        eval inti="\$ACCOUNTING_INT_"$FORM_intn"_Y"$FORM_yy"_M"$FORM_mm"_D"$dayn"_i"
        eval into="\$ACCOUNTING_INT_"$FORM_intn"_Y"$FORM_yy"_M"$FORM_mm"_D"$dayn"_o"
        : ${inti:=0}
        : ${into:=0}
        tab_line_graph '<a href="'$myname'?action=viewday&amp;yy='$FORM_yy'&amp;mm='$FORM_mm'&amp;dd='$dayn'">'$dayn'</a>' $inti $into
    done
    tab_line_sum
    tab_close
    show_tab_footer
    show_tab_footer
    show_tab_footer
fi

#------------------------------------------------------------------------------
# Zeitraum berechnen
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewperiod" ]; then
cat <<EOF
<form method="post" action="$myname"><input type="hidden" name="action" value="viewperiod">
Angabe im Format: dd.mm.yyyy<br />
von <input type="text" name="von" size="10" value="$FORM_von">
bis <input type="text" name="bis" size="10" value="$FORM_bis">
<input type="submit" value="  OK  "></form>
EOF
    case $FORM_von$FORM_bis in
        "") ;;
        *)
            expression="^[0-3][0-9]\.[0-1][0-9]\.[0-9][0-9][0-9][0-9]$"
            if ! echo "$FORM_von" | grep -q "${expression}" || ! echo "$FORM_bis" | grep -q "${expression}"; then
                show_error "Fehler" "<b>Falscher Syntax!<br />Angabe im Format: dd.mm.yyyy<br /></b>"
                show_tab_footer
            else
                [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
                do
                    eval accip="\$ACCOUNTING_HOST_"$idx"_IP"
                    ip2name $accip
                    eval accip$idx=$accip
                    eval acci$idx=0
                    eval acco$idx=0
                done
                [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
                do
                    eval intn="\$ACCOUNTING_INT_"$idx"_NAME"
                    translate_net_if_reverse $intn intn
                    eval intn$idx=$intn
                    eval inti$idx=0
                    eval into$idx=0
                done
                von_d=`expr $(echo $FORM_von | cut -d'.' -f1) + 1 - 1`
                von_m=`expr $(echo $FORM_von | cut -d'.' -f2) + 1 - 1`
                von_y=$(echo $FORM_von | cut -d'.' -f3)
                bis_d=`expr $(echo $FORM_bis | cut -d'.' -f1) + 1 - 1`
                bis_m=`expr $(echo $FORM_bis | cut -d'.' -f2) + 1 - 1`
                bis_y=$(echo $FORM_bis | cut -d'.' -f3)
                eval von_monthname=\$month$(echo $FORM_von | cut -d'.' -f2)
                eval bis_monthname=\$month$(echo $FORM_bis | cut -d'.' -f2)
                echo -n '<b>'
                for idx_y in `seq $von_y $bis_y`
                do
                    [ "$idx_y" = "$von_y" ] && start_m=$von_m || start_m="1"
                    [ "$idx_y" = "$bis_y" ] && end_m=$bis_m || end_m="12"
                    for idx_m in `seq $start_m $end_m`
                    do
                        [ "$idx_m" = "$von_m" -a "$idx_y" = "$von_y" ] && start_d=$von_d || start_d="1"
                        [ "$idx_m" = "$bis_m" -a "$idx_y" = "$bis_y" ] && end_d=$bis_d || end_d="31"
                        [ "$idx_m" -le "9" ] && idx_m="0$idx_m"
                        if [ -f "$ACCOUNTING_DIR/$idx_y/$idx_y-$idx_m.acc" ]; then
                            . $ACCOUNTING_DIR/$idx_y/$idx_y-$idx_m.acc
                            eval lastday="\$ACCOUNTING_Y"$idx_y"_M"$idx_m"_DAYS"
                            if [ "$start_d" = "1" -a "$end_d" -ge "$lastday" ]; then
                                [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
                                do
                                    eval tmpi="\$ACCOUNTING_HOST_"$idx"_Y"$idx_y"_M"$idx_m"_i"
                                    eval tmpo="\$ACCOUNTING_HOST_"$idx"_Y"$idx_y"_M"$idx_m"_o"
                                    case $tmpi in
                                        ""|0) ;;
                                        *)
                                            eval acci=\$acci$idx
                                            neui=$(cat_bc "$tmpi + $acci")
                                            eval acci$idx=$neui
                                        ;;
                                    esac
                                    case $tmpo in
                                        ""|0) ;;
                                        *)
                                            eval acco=\$acco$idx
                                            neuo=$(cat_bc "$tmpo + $acco")
                                            eval acco$idx=$neuo
                                        ;;
                                    esac
                                done
                                [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
                                do
                                    eval tmpi="\$ACCOUNTING_INT_"$idx"_Y"$idx_y"_M"$idx_m"_i"
                                    eval tmpo="\$ACCOUNTING_INT_"$idx"_Y"$idx_y"_M"$idx_m"_o"
                                    case $tmpi in
                                        ""|0) ;;
                                        *)
                                            eval inti=\$inti$idx
                                            neui=$(cat_bc "$tmpi + $inti")
                                            eval inti$idx=$neui
                                        ;;
                                    esac
                                    case $tmpo in
                                        ""|0) ;;
                                        *)
                                            eval into=\$into$idx
                                            neuo=$(cat_bc "$tmpo + $into")
                                            eval into$idx=$neuo
                                        ;;
                                    esac
                                done
                                echo -n ':'
                            else
                                for idx_d in `seq $start_d $end_d`
                                do
                                    [ "$idx_d" -le "9" ] && idx_d="0$idx_d"
                                    [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
                                    do
                                        eval tmpi="\$ACCOUNTING_HOST_"$idx"_Y"$idx_y"_M"$idx_m"_D"$idx_d"_i"
                                        eval tmpo="\$ACCOUNTING_HOST_"$idx"_Y"$idx_y"_M"$idx_m"_D"$idx_d"_o"
                                        case $tmpi in
                                            ""|0) ;;
                                            *)
                                                eval acci=\$acci$idx
                                                neui=$(cat_bc "$tmpi + $acci")
                                                eval acci$idx=$neui
                                            ;;
                                        esac
                                        case $tmpo in
                                            ""|0) ;;
                                            *)
                                                eval acco=\$acco$idx
                                                neuo=$(cat_bc "$tmpo + $acco")
                                                eval acco$idx=$neuo
                                            ;;
                                        esac
                                    done
                                    [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
                                    do
                                        eval tmpi="\$ACCOUNTING_INT_"$idx"_Y"$idx_y"_M"$idx_m"_D"$idx_d"_i"
                                        eval tmpo="\$ACCOUNTING_INT_"$idx"_Y"$idx_y"_M"$idx_m"_D"$idx_d"_o"
                                        case $tmpi in
                                            ""|0) ;;
                                            *)
                                                eval inti=\$inti$idx
                                                neui=$(cat_bc "$tmpi + $inti")
                                                eval inti$idx=$neui
                                            ;;
                                        esac
                                        case $tmpo in
                                            ""|0) ;;
                                            *)
                                                eval into=\$into$idx
                                                neuo=$(cat_bc "$tmpo + $into")
                                                eval into$idx=$neuo
                                            ;;
                                        esac
                                    done
                                    echo -n '.'
                                done
                            fi
                        else
                            echo "$ACCOUNTING_DIR/$idx_y/$idx_y-$idx_m.acc existiert nicht<br />"
                        fi
                    done
                done
                echo '</b>'
                show_tab_header "$von_d. $von_monthname $von_y - $bis_d. $bis_monthname $bis_y" no
                tab_open
                tab_head 'Host'
                sumi=0
                sumo=0
                [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
                do
                    eval acci=\$acci$idx
                    eval acco=\$acco$idx
                    sumi=$(cat_bc "$sumi + $acci")
                    sumo=$(cat_bc "$sumo + $acco")
                done
                summe=$(cat_bc "$sumi + $sumo")
                linecl="1"
                [ -f $workdir/sorthosts ] && accseq=$(cat $workdir/sorthosts)
                : ${accseq:=`seq 1 $ACCOUNTING_HOST_n`}
                for idx in $accseq
                do
                    eval accip=\$accip$idx
                    eval acci=\$acci$idx
                    eval acco=\$acco$idx
                    if [ "$acci" != "0" -o "$acco" != "0" -o "$nulldata" = "Ja" ]; then
                        ip2name $accip
                        tab_line_graph "$accip $acch" $acci $acco
                    fi
                done
                tab_line_sum
                tab_line_blank
                allaccint_i="0"
                allaccint_o="0"
                linecl="2"
                for acc_if in $ACCOUNTING_INT; do
                    ACCOUNTING_INT_NR=$(grep "'$acc_if'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
                    eval acc_if_name="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_NAME"
                    eval wertin=\$inti$ACCOUNTING_INT_NR
                    eval wertout=\$into$ACCOUNTING_INT_NR
                    : ${wertin:=0}
                    : ${wertout:=0}
                    translate_net_if_reverse $acc_if_name acc_if_name
                    allaccint_i=$(cat_bc "$allaccint_i + $wertin")
                    allaccint_o=$(cat_bc "$allaccint_o + $wertout")
                    tab_line "Interface $acc_if_name" $wertin $wertout
                done
                tab_line_diff "Differenz" $allaccint_i $allaccint_o
                tab_line_blank
                tab_head "Interface"
                [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
                do
                    eval intn=\$intn$idx
                    eval inti=\$inti$idx
                    eval into=\$into$idx
                    if [ "$inti" != "0" -o "$into" != "0" -o "$nulldata" = "Ja" ]; then
                        tab_line $intn $inti $into
                    fi
                done
                tab_close
                show_tab_footer
            fi
        ;;
    esac
fi

#------------------------------------------------------------------------------
# Live Traffic Anzeige
#------------------------------------------------------------------------------
if [ "$FORM_action" = "livetraf" ]; then
    /sbin/iptables -Z accoutlive
    /sbin/iptables -Z accinlive
    sleep 1
    /sbin/iptables -nvxL accinlive | grep 'RETURN' >$workdir/acclivein.tmp
    /sbin/iptables -nvxL accoutlive | grep 'RETURN' >$workdir/accliveout.tmp
    count=1
    while read line
    do
        set -f
        set $line
        case $9 in
            0.0.0.0/0) ;;
            *)
                eval "acci$count"=$2
                eval "accip$count"=$9
                count=`expr $count + 1`
            ;;
        esac
    done < $workdir/acclivein.tmp
    count=1
    while read line
    do
        set -f
        set $line
        case $8 in
            0.0.0.0/0) ;;
            *)
                eval "acco$count"=$2
                count=`expr $count + 1`
            ;;
        esac
    done < $workdir/accliveout.tmp
    count=`expr $count - 1`
    [ 0$count -eq 0 ] || for idx in `seq 1 $count`
    do
        eval accip=\$accip$idx
        eval acci=\$acci$idx
        eval acco=\$acco$idx
        case $acci$acco in
            00) ;;
            *)
                sumi=$(cat_bc "0$sumi + 0$acci")
                sumo=$(cat_bc "0$sumo + 0$acco")
                {
                echo -n "0000000000$(cat_bc "$acci + $acco")" | sed -e "s/0\{0,20\}\([0-9]\{10\}\)/\1/"
                echo " $acci $acco $accip"
                } >> $workdir/acclive.tmp
            ;;
        esac
    done
    summe=$(cat_bc "$sumi + $sumo")
    sort -r $workdir/acclive.tmp > $workdir/acclivesort.tmp
    show_tab_header "Bandbreite pro Client" "no" \
                    "Traffic seit letzter Aktualisierung" "$myname?action=viewchains" \
                    "Heute" "$myname?action=viewday"
    tab_open
    anzeige="KB/sek"
    anzeigewert=$(cat_bc "1024 / $factorbw")
    tab_head 'Host'
    linecl="1"
    while read line
    do
        set -f
        set $line
        ip2name $4
        tab_line_graph "$4 $acch" $2 $3
    done < $workdir/acclivesort.tmp
    tab_line_sum
    tab_close
    show_tab_footer

    # Aufr�umen
    rm -f $workdir/acclivein.tmp
    rm -f $workdir/accliveout.tmp
    rm -f $workdir/acclive.tmp
    rm -f $workdir/acclivesort.tmp
fi

#------------------------------------------------------------------------------
# LiveTraf Chains
#------------------------------------------------------------------------------
if [ "$FORM_action" = "viewchains" ]; then
    /sbin/iptables -nvxL accin | grep 'RETURN' >$workdir/acclivein.tmp
    /sbin/iptables -nvxL accout | grep 'RETURN' >$workdir/accliveout.tmp
    count=1
    while read line
    do
        set -f
        set $line
        case $9 in
            0.0.0.0/0) ;;
            *)
                eval "acci$count"=$2
                eval "accip$count"=$9
                count=`expr $count + 1`
            ;;
        esac
    done < $workdir/acclivein.tmp
    count=1
    while read line
    do
        set -f
        set $line
        case $8 in
            0.0.0.0/0) ;;
            *)
                eval "acco$count"=$2
                count=`expr $count + 1`
            ;;
        esac
    done < $workdir/accliveout.tmp
    count=`expr $count - 1`
    [ 0$count -eq 0 ] || for idx in `seq 1 $count`
    do
        eval accip=\$accip$idx
        eval acci=\$acci$idx
        eval acco=\$acco$idx
        case $acci$acco in
            00) ;;
            *)
                sumi=$(cat_bc "0$sumi + 0$acci")
                sumo=$(cat_bc "0$sumo + 0$acco")
                {
                echo -n "0000000000$(cat_bc "$acci + $acco")" | sed -e "s/0\{0,20\}\([0-9]\{10\}\)/\1/"
                echo " $acci $acco $accip"
                } >> $workdir/acclive.tmp
            ;;
        esac
    done
    summe=$(cat_bc "$sumi + $sumo")
    sort -r $workdir/acclive.tmp > $workdir/acclivesort.tmp
    show_tab_header "Bandbreite pro Client" "$myname?action=livetraf" \
                    "Traffic seit letzter Aktualisierung" "no" \
                    "Heute" "$myname?action=viewday"
    tab_open
    tab_head 'Host'
    linecl="1"
    while read line
    do
        set -f
        set $line
        ip2name $4
        tab_line_graph "$4 $acch" $2 $3
    done < $workdir/acclivesort.tmp
    tab_line_sum
    tab_close
    show_tab_footer

    # Aufr�umen
    rm -f $workdir/acclivein.tmp
    rm -f $workdir/accliveout.tmp
    rm -f $workdir/acclive.tmp
    rm -f $workdir/acclivesort.tmp
fi

#------------------------------------------------------------------------------
# accounting.sh aufrufen
#------------------------------------------------------------------------------
if [ "$FORM_action" = "aktualisieren" ]; then
    echo '<pre>'
    /usr/local/bin/accounting.sh
    echo '</pre><p>&nbsp;</p>'
fi

#------------------------------------------------------------------------------
# Anzeigeeinstellungen �ndern
#------------------------------------------------------------------------------
if [ "$FORM_action" = "conf" ]; then
    case $FORM_anzeige in
        "") ;;
        *)
            case $FORM_anzeige in
                Byte) anzeigewert="1" ;;
                KiloByte) anzeigewert="1024" ;;
                MegaByte) anzeigewert="1048576" ;;
                GigaByte) anzeigewert="1073741824" ;;
            esac
            {
            echo 'anzeigewert '$anzeigewert
            echo 'anzeige '$FORM_anzeige
            echo 'nachkomma '$FORM_nachkomma
            echo 'nulldata '$FORM_nulldata
            echo 'yeardata '$FORM_yeardata
            echo 'namesolution '$FORM_namesolution
            echo 'liverefresh '$FORM_liverefresh
            echo 'factorbw '$FORM_factorbw
            } > $ACCOUNTING_DIR/view.conf
            echo 'Anzeigeeinstellungen ge&auml;ndert<br />'
            anzeige=$FORM_anzeige
            nachkomma=$FORM_nachkomma
            nulldata=$FORM_nulldata
            yeardata=$FORM_yeardata
            namesolution=$FORM_namesolution
            liverefresh=$FORM_liverefresh
            factorbw=$FORM_factorbw
        ;;
    esac

cat <<EOF
<form method="post" action="$myname">
  <input type="hidden" name="action" value="conf">
  <table>
    <tr>
      <td>Anzeige in:</td>
      <td>
        <select name="anzeige" size="1">
          <option $([ $anzeige = "Byte" ] && echo -n ' selected')>Byte</option>
          <option $([ $anzeige = "KiloByte" ] && echo -n ' selected')>KiloByte</option>
          <option $([ $anzeige = "MegaByte" ] && echo -n ' selected')>MegaByte</option>
          <option $([ $anzeige = "GigaByte" ] && echo -n ' selected')>GigaByte</option>
        </select>
      </td>
    </tr>
    <tr>
      <td>Nachkommastellen:</td>
      <td>
        <select name="nachkomma" size="1">
          <option $([ $nachkomma = "0" ] && echo -n ' selected')>0</option>
          <option $([ $nachkomma = "1" ] && echo -n ' selected')>1</option>
          <option $([ $nachkomma = "2" ] && echo -n ' selected')>2</option>
          <option $([ $nachkomma = "3" ] && echo -n ' selected')>3</option>
        </select>
      </td>
    </tr>
    <tr>
      <td>Nulldaten anzeigen:</td>
      <td>
        <select name="nulldata" size="1">
          <option $([ $nulldata = "Ja" ] && echo -n ' selected')>Ja</option>
          <option $([ $nulldata = "Nein" ] && echo -n ' selected')>Nein</option>
        </select>
      </td>
    </tr>
    <tr>
      <td>Daten der &Uuml;bersicht:</td>
      <td>
        <select name="yeardata" size="1">
          <option $([ $yeardata = "Hosts" ] && echo -n ' selected')>Hosts</option>
          <option $([ $yeardata = "Interfaces" ] && echo -n ' selected')>Interfaces</option>
        </select>
      </td>
    </tr>
    <tr>
      <td>Namesaufl&ouml;sung:</td>
      <td>
        <select name="namesolution" size="1">
          <option $([ $namesolution = "keine" ] && echo -n ' selected')>keine</option>
          <option $([ $namesolution = "DNS" ] && echo -n ' selected')>DNS</option>
          <option $([ $namesolution = "index" ] && echo -n ' selected')>index</option>
        </select>
      </td>
    </tr>
    <tr>
      <td>LiveTraf Refresh (Sek.):</td>
      <td>
        <input type="text" name="liverefresh" size="12" value="$liverefresh">
      </td>
    </tr>
    <tr>
      <td>Faktor Bandbreite:</td>
      <td>
        <input type="text" name="factorbw" size="12" value="$factorbw">
        <input type="submit" value="  OK  ">
      </td>
    </tr>
  </table>
</form>
EOF
fi

show_tab_footer
show_html_footer
