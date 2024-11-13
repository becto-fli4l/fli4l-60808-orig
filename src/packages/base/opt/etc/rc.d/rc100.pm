#!/bin/sh
begin_script PM "initializing power management subsystem ..."
case "$POWERMANAGEMENT" in
    apm_rm)
        do_modprobe_if_exists kernel/arch/x86/kernel apm realmode_power_off=1
        ;;
    apm)
        do_modprobe_if_exists kernel/arch/x86/kernel apm
        ;;
    acpi)
        # enable acpid if an ACPI button exists
        find /sys/devices/ -type f -name hid | while read hid
        do
            if grep -q "^LNXPWRBN$" $hid 2>/dev/null
            then
                > /var/run/start_acpid
                break
            fi
        done
        ;;
esac
end_script
