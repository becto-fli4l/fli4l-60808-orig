#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/user.inc
# Functions for working with users and groups.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

USER_FILE=/etc/passwd
GROUP_FILE=/etc/group
SHADOW_FILE=/etc/shadow

USER_NEXT_ID_FILE=/var/run/user.next
GROUP_NEXT_ID_FILE=/var/run/group.next

# [PRIVATE]
# Initializes the ID files.
user_group_init_id_files()
{
    # next available user ID/group ID
    echo "500" > "$USER_NEXT_ID_FILE"
    echo "500" > "$GROUP_NEXT_ID_FILE"
}

# [PRIVATE]
# Reads the next available ID from file and increments the ID in the file.
#
# Input:
#   $1 = ID file to use
#   $2 = caller
# Output:
#   $_result = next available ID
user_group_get_next_id()
{
    local file=$1 caller=$2
    sync_lock_resource $(basename "$file")-db "$caller"
    read _result < "$file"
    echo $((_result+1)) > "$file"
    sync_unlock_resource $(basename "$file")-db "$caller"
}

# [PUBLIC]
# Creates a user. If a user with the same name already exists, either the user
# IDs must be equal or the caller used an empty user ID. In both cases, the
# groups, the home directories, and the shells must coincide, and the password
# and the comment are left unchanged.
#
# If "-q" is passed as first parameter, errors are not logged.
#
# Input:
#   $1 = user name. Must be non-empty.
#   $2 = user ID. If empty, the next available ID is used.
#   $3 = group name or ID. Must already exist.
#   $4 = password. Must be already encrypted. If empty, "*" is used
#        (and the user cannot login).
#   $5 = home directory. If it exists, it must be a directory. If it does not
#        exist, it is created and chown'ed to specified user/group.
#   $6 = shell to use. Must be executable. If empty, /bin/true is used.
#   $7 = comment. May be empty.
# Output:
#   $_result = user ID
# Exit code:
#   0 if successful
#   1 if no user name passed
#   2 if group does not exist
#   3 if no home directory passed
#   4 if home exists, but is not a directory
#   5 if shell is not executable
#   6 if user already exists with incompatible properties
user_add()
{
    local quiet
    if [ "$1" = "-q" ]
    then
        quiet=-q
        shift
    fi

    local user=$1 userid=$2 group=$3 passwd=$4 home=$5 shell=$6 comment=$7
    _result=

    # default assignments
    : ${passwd:='*'}
    : ${shell:='/bin/true'}

    # validity checks
    if [ -z "$user" ]
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "user_add: no user name"
        return 1
    fi

    if ! group_name_to_id $quiet $group
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "user_add: group $group does not exist [user=$user]"
        return 2
    else
        group=$_result
    fi

    if [ -z "$home" ]
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "user_add: no home directpry [user=$user]"
        return 3
    fi

    if [ -e "$home" -a ! -d "$home" ]
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "user_add: home directpry $home is not a directory [user=$user]"
        return 4
    fi

    if [ ! -x "$shell" ]
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "user_add: shell $shell is not executable [user=$user]"
        return 5
    fi

    # check whether user already exists
    set -- $(sed -n "s/^$user:[^:]\+:\([^:]\+\):\([^:]\+\):[^:]*:\([^:]\+\):\([^:]\+\).*$/\1 \2 \3 \4/p" "$USER_FILE")
    local olduid=$1 oldgroup=$2 oldhome=$3 oldshell=$4
    if [ -n "$olduid" ]
    then
        if [ -z "$userid" -o "$userid" = "$olduid" ] &&
            [ "$oldgroup" = "$group" -a "$home" = "$oldhome" -a "$shell" = "$oldshell" ]
        then
            _result=$olduid
            return 0
        else
            [ -z "$quiet" ] &&
                logmsg "base-helper" "$LOG_FACILITY_BASE.err" "user_add: user already exists [$user]"
            return 6
        fi
    fi

    # everything OK, do it
    if [ -z "$userid" ]
    then
        user_group_get_next_id "$USER_NEXT_ID_FILE" user_add
        userid=$_result
    fi

    local date=$(($(date +%s) / 86400))
    echo "$user:x:$userid:$group:$comment:$home:$shell" >> "$USER_FILE"
    echo "$user:$passwd:$date::::::" >> "$SHADOW_FILE"
    sed -i "s/^\([^:]\+:[^:]*:$group:.\+\)$/\1,$user/" "$GROUP_FILE"
    sed -i "s/^\([^:]\+:[^:]*:$group:\)$/\1$user/" "$GROUP_FILE"

    if [ ! -e "$home" ]
    then
        mkdir -p "$home"
        chown $userid:$group "$home"
    fi

    _result=$userid
    return 0
}

# [PUBLIC]
# Creates a group. If the group already exists, either the group IDs must be
# equal or the caller used an empty group ID.
#
# If "-q" is passed as first parameter, errors are not logged.
#
# Input:
#   $1 = group name. Must be non-empty.
#   $2 = group ID. If empty, the next available ID is used.
# Output:
#   $_result = user ID
# Exit code:
#   0 if successful
#   1 if no group name passed
#   2 if group already exists with incompatible properties
group_add()
{
    local quiet
    if [ "$1" = "-q" ]
    then
        quiet=-q
        shift
    fi

    local group=$1 groupid=$2
    _result=

    # validity checks
    if [ -z "$group" ]
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "group_add: no group name"
        return 1
    fi

    # check whether group already exists
    local oldgid=$(sed -n "s/^$group:x:\([^:]\+\):.*$/\1/p" "$GROUP_FILE")
    if [ -n "$oldgid" ]
    then
        if [ -z "$groupid" -o "$groupid" = "$oldgid" ]
        then
            _result=$oldgid
            return 0
        else
            [ -z "$quiet" ] &&
                logmsg "base-helper" "$LOG_FACILITY_BASE.err" "group_add: group already exists [$group]"
            return 2
        fi
    fi

    # everything OK, do it
    if [ -z "$groupid" ]
    then
        user_group_get_next_id "$GROUP_NEXT_ID_FILE" group_add
        groupid=$_result
    fi

    echo "$group:x:$groupid:" >> "$GROUP_FILE"
    _result=$groupid
    return 0
}

# [PUBLIC]
# Maps a group name to its ID. If a valid group ID is passed, the function
# returns success (and leaves the ID unchanged).
#
# If "-q" is passed as first parameter, errors are not logged.
#
# Input:
#   $1 = group name
# Output:
#   $_result = group ID
# Exit code:
#   0 if successful
#   1 if group does not exist
group_name_to_id()
{
    local quiet
    if [ "$1" = "-q" ]
    then
        quiet=1
        shift
    fi

    local group=$1
    if echo "$group" | grep -q "^[0-9]\+$"
    then
        if ! grep -q "^[^:]\+:[^:]*:$group:" "$GROUP_FILE"
        then
            [ -z "$quiet" ] &&
                logmsg "base-helper" "$LOG_FACILITY_BASE.err" "group_name_to_id: group $group does not exist"
            return 1
        else
            _result=$group
            return 0
        fi
    else
        local groupid=$(sed -n "s,^$group:[^:]*:\([^:]\+\):.*$,\1,p" "$GROUP_FILE")
        if [ -z "$groupid" ]
        then
            [ -z "$quiet" ] &&
                logmsg "base-helper" "$LOG_FACILITY_BASE.err" "group_name_to_id: group $group does not exist"
            return 1
        else
            _result=$groupid
            return 0
        fi
    fi
}

# [PUBLIC]
# Maps a group ID to its name.
#
# If "-q" is passed as first parameter, errors are not logged.
#
# Input:
#   $1 = group ID
# Output:
#   $_result = group name
# Exit code:
#   0 if successful
#   1 if group ID does not exist
group_id_to_name()
{
    local quiet
    if [ "$1" = "-q" ]
    then
        quiet=1
        shift
    fi

    local groupid=$1
    local group=$(sed -n "s,^\([^:]\+\):[^:]*:$groupid:.*$,\1,p" "$GROUP_FILE")
    if [ -z "$group" ]
    then
        [ -z "$quiet" ] &&
            logmsg "base-helper" "$LOG_FACILITY_BASE.err" "group_id_to_name: group ID $groupid does not exist"
        return 1
    else
        _result=$group
        return 0
    fi
}
