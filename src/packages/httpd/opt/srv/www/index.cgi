#!/bin/sh
#------------------------------------------------------------------------------
# index.cgi - startfile for the webserver
#
# Creation:     12.08.2005  hh
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

cat <<EOF > /tmp/httpdmenu
t 100 - $_MT_admin
e 100 admin/index.cgi $_MP_lgin
EOF
show_html_header "$_MN_tit" "menufile=/tmp/httpdmenu"
case `date +%m%d` in
    122[56]) _welc="$_MN_xmas";;
       0101) _welc="$_MN_newy";;
          *) _welc="$_MN_welc";;
esac
cat <<EOF
       <h2 style="margin: 0px;">$_welc</h2>
       <h3>$_MN_lgin</h3>
EOF
show_html_footer
rm /tmp/httpdmenu
