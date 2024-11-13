# $Id$

extra_dmidecode_format ()
{
   local var=`echo ${1} | tr -d '\n' | sed -e 's/^ //g' -e 's/ $//g'`
   if [ -z "${var}" ]
      then
          var="N/A"
      fi
   echo ${var}
}

extra_dmidecode_bios ()
{
    var1=`/usr/sbin/dmidecode -s bios-vendor`
    var2=`/usr/sbin/dmidecode -s bios-version`
    var3=`/usr/sbin/dmidecode -s bios-release-date`
    var1=$(extra_dmidecode_format "${var1}")
    var2=$(extra_dmidecode_format "${var2}")
    var3=$(extra_dmidecode_format "${var3}")

    show_tab_header ${_HWSUPP_DMI_BIOS} no

    echo "<table class='normtable'>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VENDOR}</td>"
    echo "<td>${var1}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VERSION}</td>"
    echo "<td>${var2}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_DATE}</td>"
    echo "<td>${var3}</td>"
    echo "</tr>"

    echo "</table>"

    show_tab_footer
}

extra_dmidecode_system ()
{
    var1=`/usr/sbin/dmidecode -s system-manufacturer`
    var2=`/usr/sbin/dmidecode -s system-product-name`
    var3=`/usr/sbin/dmidecode -s system-version`
    var4=`/usr/sbin/dmidecode -s system-serial-number`
    var1=$(extra_dmidecode_format "${var1}")
    var2=$(extra_dmidecode_format "${var2}")
    var3=$(extra_dmidecode_format "${var3}")
    var4=$(extra_dmidecode_format "${var4}")

    show_tab_header ${_HWSUPP_DMI_SYSTEM} no
    
    echo "<table class='normtable'>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VENDOR}</td>"
    echo "<td>${var1}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_PRODUCTNAME}</td>"
    echo "<td>${var2}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VERSION}</td>"
    echo "<td>${var3}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_SERIAL}</td>"
    echo "<td>${var4}</td>"
    echo "  </tr>"

    echo "</table>"

    show_tab_footer
}

extra_dmidecode_baseboard ()
{
    var1=`/usr/sbin/dmidecode -s baseboard-manufacturer`
    var2=`/usr/sbin/dmidecode -s baseboard-product-name`
    var3=`/usr/sbin/dmidecode -s baseboard-version`
    var4=`/usr/sbin/dmidecode -s baseboard-serial-number`
    var1=$(extra_dmidecode_format "${var1}")
    var2=$(extra_dmidecode_format "${var2}")
    var3=$(extra_dmidecode_format "${var3}")
    var4=$(extra_dmidecode_format "${var4}")

    show_tab_header ${_HWSUPP_DMI_BASEBOARD} no

    echo "<table class='normtable'>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VENDOR}</td>"
    echo "<td>${var1}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_PRODUCTNAME}</td>"
    echo "<td>${var2}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VERSION}</td>"
    echo "<td>${var3}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_SERIAL}</td>"
    echo "<td>${var4}</td>"
    echo "  </tr>"

    echo "</table>"

    show_tab_footer
}

extra_dmidecode_chassis ()
{
    var1=`/usr/sbin/dmidecode -s chassis-manufacturer`
    var2=`/usr/sbin/dmidecode -s chassis-type`
    var3=`/usr/sbin/dmidecode -s chassis-version`
    var4=`/usr/sbin/dmidecode -s chassis-serial-number`
    var1=$(extra_dmidecode_format "${var1}")
    var2=$(extra_dmidecode_format "${var2}")
    var3=$(extra_dmidecode_format "${var3}")
    var4=$(extra_dmidecode_format "${var4}")
    
    show_tab_header ${_HWSUPP_DMI_CHASSIS} no

    echo "<table class='normtable'>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VENDOR}</td>"
    echo "<td>${var1}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_TYPE}</td>"
    echo "<td>${var2}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VERSION}</td>"
    echo "<td>${var3}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_SERIAL}</td>"
    echo "<td>${var4}</td>"
    echo "  </tr>"

    echo "</table>"

    show_tab_footer
}

extra_dmidecode_processor ()
{
    var1=`/usr/sbin/dmidecode -s processor-manufacturer`
    var2=`/usr/sbin/dmidecode -s processor-family`
    var3=`/usr/sbin/dmidecode -s processor-version`
    var4=`/usr/sbin/dmidecode -s processor-frequency`
    var1=$(extra_dmidecode_format "${var1}")
    var2=$(extra_dmidecode_format "${var2}")
    var3=$(extra_dmidecode_format "${var3}")
    var4=$(extra_dmidecode_format "${var4}")

    show_tab_header ${_HWSUPP_DMI_PROCESSOR} no

    echo "<table class='normtable'>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VENDOR}</td>"
    echo "<td>${var1}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_FAMILY}</td>"
    echo "<td>${var2}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_VERSION}</td>"
    echo "<td>${var3}</td>"
    echo "</tr>"

    echo "<tr align='left'>"
    echo "<td>${_HWSUPP_DMI_FREQUENCY}</td>"
    echo "<td>${var4}</td>"
    echo "  </tr>"

    echo "</table>"

    show_tab_footer
}

extra_dmidecode ()
{
    extra_dmidecode_bios
    extra_dmidecode_system
    extra_dmidecode_baseboard
    extra_dmidecode_chassis
    extra_dmidecode_processor
}
# _oOo_