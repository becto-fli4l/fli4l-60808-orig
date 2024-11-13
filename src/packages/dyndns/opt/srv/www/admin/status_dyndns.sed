#
# save error codes from ZoneEdit from being removed as html tags
#
s/<ERROR /ERROR /g
s/<SUCCESS /SUCCESS /g

# 
# Format description
#
# Header (one per update attempt):
#   DynDNS-Provider:
#   DynDNS-Forced-Update:
#   DynDNS-Registered-IP:
#   DynDNS-Begin:
#   DynDNS-End:
#   DynDNS-Result:
# Update Protocol (one per method or attempt per method)
#   Subheader
#     DynDNS-Date: Wed Dec  3 09:47:40 MEZ 2008
#     DynDNS-Method: 'https'
#   Preformatted text
#     ----- start answer -----
#     text
#     ------ end answer ------
#     ----- start error -----
#     text
#     ------ end error ------
#   Closing comment (optional)
#     DynDNS-Comment:
# s/^# DynDNS-: \(.*\)//

#
# this line is inserted once at the beginning to open the table:
#
1 i\    <table width="100%">\

#
# this line will be appended once at the end in order to close the table:
#
$ a\    </table>

#
# these lines do the transformation from ascii log-file into html format:
#
1! {
/^# DynDNS-Provider:/i\      <tr><td colspan=2>\&nbsp;</td></tr>
}
/^# DynDNS-Provider:/ {
  s%^[^:]*: \(.*\)%      <tr><td colspan=2 class="HEAD">DYNDNS_general_info</td></tr>\
      <tr><td class="COMMENT">DYNDNS_provider: </td><td>\1</td></tr>%
}

s%^# DynDNS-Forced-Update: \(.*\)%      <tr><td class="COMMENT">DYNDNS_forced_update: </td><td>\1</td></tr>% 

s%^# DynDNS-Registered-IP: \(.*\)%      <tr><td class="COMMENT">DYNDNS_registered_ip: </td><td>\1</td></tr>% 

s%^# DynDNS-Begin: \(.*\)%      <tr><td class="COMMENT">DYNDNS_begin: </td><td>\1</td></tr>% 

s%^# DynDNS-End: \(.*\)%      <tr><td class="COMMENT">DYNDNS_end: </td><td>\1</td></tr>%

/^# DynDNS-Result:/ {
  s%^[^:]*: \(.*\)%      <tr><td class="COMMENT">DYNDNS_result: </td><td>\1</td></tr>%
}

/^# DynDNS-Method:/ {
  s%^[^:]*: \(.*\)%      <tr><td colspan=2 class="HEAD2">DYNDNS_attempt \1</td></tr>%
}

/^# DynDNS-Date:/ {
  s%^[^:]*: \(.*\)%      <tr><td class="COMMENT">DYNDNS_date: </td><td>\1</td></tr>%
}

/^# DynDNS-Cancel:/ {
  s%^[^:]*: \(.*\)%      <tr><td colspan=2 class="CANCEL">DYNDNS_canceled \1</td></tr>%
}

/^# DynDNS-Enable:/ {
  s%^[^:]*: \(.*\)%      <tr><td colspan=2 class="ENABLE">DYNDNS_enabled \1</td></tr>%
}

/^# DynDNS-Report:/ {
  s%^[^:]*: \(.*\)%      <tr><td colspan=2 class="REPORT">DYNDNS_report \1)</td></tr>%
}

/^----- start error.*/ {
c\      <tr>\
        <td colspan=2 class="ERROR_REPLY">\
          <p>DYNDNS_error:</p>\
            <pre>
}

/^----- start answer.*/ {
c\      <tr>\
        <td colspan=2 class="REPLY">\
          <p>DYNDNS_reply:</p>\
          <pre>
}

/^------ end[^-]*/ {
c\          </pre>\
        </td>\
      </tr>
}

s%^# DynDNS-Comment: None%%

s%^# DynDNS-Comment: \(.*\)%      <tr><td class="COMMENT">DYNDNS_comment:</td><td>\1</td></tr>%
