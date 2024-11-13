rrd_dbpath={dbpath}
rrd_watermark="fli4l - flexible internet router for linux - Host:`hostname -f`"
rrd_common_opts="--lazy --imgformat PNG --daemon unix:/var/run/rrdcached.sock"
rrd_width="--width 600"
rrd_height="--height 200"
rrd_default_graphtime=6h
rrd_graphtime_list="1h 6h 24h 7d 4w 1y all"
rrd_grid_background="--color CANVAS#E0E0E0"

# colors for graph LINE and AREA
rrd_col_red_light="#FF0000"
rrd_col_red="#770000"
rrd_col_red_dark="#7F0000"
rrd_col_green_light="#00FF00"
rrd_col_green="#007700"
rrd_col_green_dark="#007F00"
rrd_col_blue_light="#0000FF"
rrd_col_blue="#000077"
rrd_col_blue_dark="#00007F"
rrd_col_orange="#FF9900"
rrd_col_yellow_light="#FFFF80"
rrd_col_yellow="#FFFF00"
rrd_col_grey="#cbcbcb"

# recalc last_update every time
rrd_last_update="`date|sed -e 's/:/\\\\:/g'`"
