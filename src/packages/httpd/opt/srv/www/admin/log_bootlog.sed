#
# keep in sync with bootlog.css in different skins
#

#
# escape html entities
#
s,&,\&amp;,g
s,<,\&lt;,g
s,>,\&gt;,g

#
# eat beeps
#
s,\\007,,g

#
# initial formatting
#
s,^,<tr>\
  <td class="date">,
s,$,</td>\
</tr>,
# switch year and time and create seperate columns for date and time
# 11:41:01 2008: -> ..2008</td><td>11:41:01</td>
s,\([0-9][0-9]:[0-9][0-9]:[0-9][0-9]\)[[:space:]]\+\([0-9]\+\): ,\2</td>\
  <td class="time">\1</td>\
  <td> ,

#
# color coding - known colors:
#
# ebegin
s,<td>  \^\[\[32;01m\*\^\[\[0m \^\[\[36;01m *\(\[.*\]\)\^\[\[0m ,<td class="beginscript"> \1 ,
# eend 0
s,<td> \^\[\[A\^\[\[[0-9]\+C  \^\[\[34;01m\[ \^\[\[32;01mOK\^\[\[34;01m \]\^\[\[0m,<td class="endscript" align="right"> \[ OK \],
# eend != 0 (warning only)
s,<td> \^\[\[A\^\[\[[0-9]\+C  \^\[\[34;01m\[ \^\[\[33;01m!!\^\[\[34;01m \]\^\[\[0m,<td class="endscriptwarning" align="right"> \[ !! \],
# eend != 0
s,<td> \^\[\[A\^\[\[[0-9]\+C  \^\[\[34;01m\[ \^\[\[31;01m!!\^\[\[34;01m \]\^\[\[0m,<td class="endscripterror" align="right"> \[ !! \],
# einfo
s,<td>  \^\[\[32;01m\*\^\[\[0m ,<td class="info"> ,
# ewarn
s,<td>  \^\[\[33;01m\*\^\[\[0m ,<td class="warning"> ,
# eerror
s,<td>  \^\[\[31;01m\*\^\[\[0m ,<td class="error"> ,
# 0 0 br    ^[[0m     ^[[49m    ^[[1m                         
s,<td> \^\[\[0m\^\[\[49m\^\[\[1m,<td class="unknown">(unknown color type) ,
# rd 0 br   ^[[31m    ^[[49m    ^[[1m                         
s,<td> \^\[\[31m\^\[\[49m\^\[\[1m,<td class="unknown">(unknown color type) ,
# br b br   ^[[33m    ^[[40m    ^[[1m                         
s,<td> \^\[\[33m\^\[\[40m\^\[\[1m,<td class="unknown">(unknown color type) ,
# br 0 br   ^[[33m    ^[[49m    ^[[1m                         
# br x br   ^[[33m    ^[[49m    ^[[1m                         
s,<td> \^\[\[33m\^\[\[49m\^\[\[1m,<td class="error"> ,
# bl x br   ^[[34m    ^[[49m    ^[[1m                         
s,<td> \^\[\[34m\^\[\[49m\^\[\[1m,<td class="unknown">(unknown color type) ,
# w b br    ^[[37m    ^[[40m    ^[[1m                         
s,<td> \^\[\[37m\^\[\[40m\^\[\[1m,<td class="unknown">(unknown color type) ,
# w n br    ^[[37m    ^[[49m    ^[[1m                         
s,<td> \^\[\[37m\^\[\[49m\^\[\[1m,<td class="unknown">(unknown color type) ,
# b gn      ^[[30m    ^[[42m                                  
s,<td> \^\[\[30m\^\[\[42m,<td class="unknown">(unknown color type) ,
# b rd      ^[[30m    ^[[41m                                  
s,<td> \^\[\[30m\^\[\[41m,<td class="unknown">(unknown color type) ,
# rd        ^[[31m    ^[[49m                                  
s,<td> \^\[\[31m\^\[\[49m,<td class="warning"> ,
# gn        ^[[32m    ^[[49m                                   
s,<td> \^\[\[32m\^\[\[49m,<td class="info"> ,
# br x      ^[[33m    ^[[49m                                  
s,<td> \^\[\[33m\^\[\[49m,<td class="unknown">(unknown color type) ,
# cy        ^[[36m    ^[[49m                                  
s,<td> \^\[\[36m\^\[\[49m,<td class="info-cyan"> ,

# set -x debug output
s,<td> + ,<td class="debug"> + ,

s,<td>,<td class="bootlog">,

# terminating "switch to std"
s,\^\[\[0m,,
