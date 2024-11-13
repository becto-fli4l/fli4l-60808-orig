#!/bin/bash
# Merges two luarocks manifests.
#
# Input:
#   $1 = first manifest (and stores the result of the merge)
#   $2 = second manifest

# Transforms a manifest into "<start> <key> <end>" lines.
read_manifest()
{
	sed -n "/^[^[:space:]]\+ = {}$/{=;s/^\([^[:space:]]\+\).*/\1/p;=};/^[^[:space:]]\+ = {$/{=;s/^\([^[:space:]]\+\).*/\1/p};/^}$/=" "$1" | sed '{N;N;s/\n/ /g}'
}

# Reads the lines from read_manifest() and stores them in bash arrays.
# Input:
#   $1 = manifest file
#   $2 = name of array for keys
#   $3 = name of array for block info
analyze_manifest()
{
	declare -g -a $2
	declare -g -A $3
	[ -f "$1" ] || return
	local _begin _key _end
	while read _begin _key _end
	do
		eval $2+=\(\$_key\)
		eval $3\[\$_key\]=\"\$_begin \$_end\"
	done < <(read_manifest "$1")
}

merge_block() {
	local file=$1 start=$2 end=$3 ignore=$4
	if [ "$ignore" = "ignore-empty" ]
	then
		sed -n "${start},${end}p" "$file" | sed '/= {},\?$/d'
	else
		sed -n "${start},${end}p" "$file"
	fi
}

unmerge_block()
{
	local dstfile=$1 dststart=$2 dstend=$3 srcfile=$4 srcstart=$5 srcend=$6 efflen=$7
	local len=$(merge_block "$srcfile" $srcstart $srcend ignore-empty | grep -c "^")
	if [ $len -gt 2 ]
	then
		len=$((len-2))
		while read line
		do
			oldifs="$IFS"
			IFS=:
			set -- $line
			linenr=$1
			IFS=$oldifs

			if [ $linenr -gt $dststart -a $linenr -lt $dstend ]
			then
				remove+=("$linenr $((linenr+$len-1))")
				break
			fi
		done < <(grep -n -x -F "$(sed -n "$((srcstart+1)),${srcend}p" "$srcfile" | sed '/= {},\?$/d' | sed -n '1p')" "$dstfile")
	fi
}

# Checks whether an element is part of a list.
# Input:
#   $1 = element
#   $2... = list
# Return code:
#   0 if the element is part of the list, 1 otherwise
list_is_in()
{
    local e=$1 i
    shift
    for i
    do
        [ "$e" = "$i" ] && return 0
    done
    return 1
}

cmd="$1"
first="$2"
second="$3"

analyze_manifest "$first" keys1 manifest1
analyze_manifest "$second" keys2 manifest2
tmpresult=$(mktemp merge-luarocks-manifests.XXXXXXXXXX)

case $cmd in
merge)
	for key in ${keys1[@]}
	do
		range1=(${manifest1[$key]})
		if list_is_in $key ${keys2[@]}
		then
			range2=(${manifest2[$key]})
			if [ ${range1[0]} -eq ${range1[1]} -a ${range2[0]} -eq ${range2[1]} ]
			then
				merge_block "$first" ${range1[0]} ${range1[1]}
			elif [ ${range1[0]} -eq ${range1[1]} ]
			then
				merge_block "$second" ${range2[0]} ${range2[1]} ignore-empty
			elif [ ${range2[0]} -eq ${range2[1]} ]
			then
				merge_block "$first" ${range1[0]} ${range1[1]}
			else
				merge_block "$first" ${range1[0]} $((${range1[1]}-1)) | sed '$s/.*}$/&,/'
				merge_block "$second" $((${range2[0]}+1)) ${range2[1]} ignore-empty
			fi
		else
			merge_block "$first" ${range1[0]} ${range1[1]}
		fi
	done >> "$tmpresult"

	for key in ${keys2[@]}
	do
		if ! list_is_in $key ${keys1[@]}
		then
			range=(${manifest2[$key]})
			merge_block "$second" ${range[0]} ${range[1]} ignore-empty
		fi
	done >> "$tmpresult"

	echo "$(basename $0): $second merged with $first"
	;;
unmerge)
	declare -a remove
	for key in ${keys2[@]}
	do
		range2=(${manifest2[$key]})
		if list_is_in $key ${keys1[@]}
		then
			range1=(${manifest1[$key]})
			unmerge_block "$first" ${range1[0]} ${range1[1]} "$second" ${range2[0]} ${range2[1]}
		fi
	done

	{
		nexttoprint=1
		while read start end
		do
			if [ $nexttoprint -lt $start ]
			then
				sed -n "$nexttoprint,$((start-1))p" "$first"
			fi
			nexttoprint=$((end+1))
		done < <(printf "%s\n" "${remove[@]}" | sort -n)

		last=$(sed -n '$=' "$first")
		if [ $nexttoprint -le $last ]
		then
			sed -n "$nexttoprint,${last}p" "$first"
		fi
	} >> "$tmpresult"

	echo "$(basename $0): $second unmerged from $first"
	;;
esac

mv "$tmpresult" "$first"
exit 0
