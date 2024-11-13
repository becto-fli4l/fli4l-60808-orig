# Creation:     20.05.2020 chfritsch

hwmon_generic_acpi_coretemp_frequency ()
{
  # var1=`cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`
    var1=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq`
    var1=$((var1 / 1000))

    show_tab_header ${_HWSUPP_RT_FREQUENCY} no

    echo "<table class='normtable'>"
    echo "<tr>"
    echo "<th>${_HWSUPP_HWMON_SENSOR}</th>"
    echo "<th>${_HWSUPP_RL_FREQUENCY}</th>"
    echo "</tr>"
    echo "<tr align='center'>"
    echo "<td>${_HWSUPP_RC_CPU} 0</td>"
    echo "<td>${var1}</td>"
    echo "</tr>"
    echo "</table>"

    show_tab_footer
}


hwmon_generic_acpi_coretemp_coretemp ()
{
    var_coretemp=`sensors -u k10temp-pci-00c3 | grep temp1_input | cut -d : -f 2`
    var_coretemp=`echo ${var_coretemp}| sed 's/[^0-9]*\([0-9]*\)\.\([0-9]\{1\}\).*/\1.\2/'`
    show_tab_header Core-${_HWSUPP_TEMP} no
    echo "<table class='normtable'>"

    echo "<tr>"
    echo "<th>${_HWSUPP_HWMON_SENSOR}</th>"
    echo "<th>$Core-${_HWSUPP_RL_TEMP}</th>"
    echo "</tr>"

    echo "<tr align='center'>"
    echo "<td>K10</td>"
    echo "<td>${var_coretemp}</td>"
    echo "</tr>"

    echo "</table>"
    show_tab_footer
}

hwmon_generic_acpi_coretemp ()
{
    hwmon_generic_acpi_coretemp_frequency
    hwmon_generic_acpi_coretemp_coretemp
}

# _oOo_
