#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/locking.inc
# Functions for locking/unlocking global resources.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$locking_api" != yes ]
then

. /etc/boot.d/process.inc
. /etc/boot.d/forking.inc
. /etc/boot.d/exittrap.inc

# lock sleep interval in microseconds
sync_lock_sleep_interval=100000

# Determines the PID of the running shell. The reason for this behaviour is
# that $$ does not change within subshells. Sometimes we need to use the PID of
# the main shell ($$), sometimes we need to use the PID of the subshell,
# especially if we are not allowed to reuse the locks of the parent shell (e.g.
# if the subshell is spawned in parallel).
sync_set_subshell_pid()
{
    local rest
    read -r PID rest < /proc/self/stat
}

# Determines the locker of a resource (if any).
#
# $1 = resource
# $2 = name of variable receiving the PID of the locker or nothing if the
#      resource is not locked
sync_get_locker()
{
    local lockfile="/var/lock/fli4l-$1.lock"
    if [ -f "$lockfile" ]
    then
        local link=$(readlink "$lockfile")
        eval $2=\${link#/var/lock/fli4l-\$1.}
    else
        eval $2=
    fi
}

# Tries to lock a resource.
#
# $1 = resource to be locked
# $2 = name of caller
# Returns: 0 on success and non-zero on failure
# locker = (on non-zero exit) PID of locking process
sync_try_lock_resource()
{
    local traps=$(trap) pidfile="/var/lock/fli4l-$1.$PID" count=0
    # we have to be atomic
    trap "" 0 1 2 3 4 6 8 10 11 12 13 14 15

    if ! ln -s "$pidfile" "/var/lock/fli4l-$1.lock" 2>/dev/null
    then
        local locker
        sync_get_locker "$1" locker
        if [ -n "$locker" ] && [ $locker -eq $PID ]
        then
            read count < "$pidfile"
        else
            trap - 0 1 2 3 4 6 8 10 11 12 13 14 15
            eval "$traps"
            return 1
        fi
    fi

    echo $((count+1)) > "$pidfile"
    sync_locked_resources="$sync_locked_resources$1 "
    trap - 0 1 2 3 4 6 8 10 11 12 13 14 15
    eval "$traps"
    return 0
}

# (internal) Displays a lock error.
#
# $1 = resource to be locked
# $2 = name of caller
# $3 = locker
sync_show_lock_error()
{
    local caller=$(map_pid_to_process $PID)
    if [ -n "$3" ]
    then
        local owner=$(map_pid_to_process $3)
        log_error "$caller/$2[$PID]: failed to lock resource $1: already locked by $owner[$3]"
    else
        log_error "$caller/$2[$PID]: failed to lock resource $1 for unknown reasons"
    fi
}

# (internal) Displays an unlock error.
#
# $1 = resource to be unlocked
# $2 = name of caller
# $3 = locker (if resource is currently locked)
sync_show_unlock_error()
{
    local caller=$(map_pid_to_process $PID)
    if [ -n "$3" ]
    then
        local owner=$(map_pid_to_process $3)
        log_error "$caller/$2[$PID]: failed to unlock resource $1: already locked by $owner[$3]"
    else
        log_error "$caller/$2[$PID]: failed to unlock resource $1 as it is not locked at all"
    fi
}

# Locks a resource.
#
# $1 = resource to be locked
# $2 = name of caller
# Returns: 0 on success and non-zero on failure
sync_lock_resource()
{
    while ! sync_try_lock_resource "$1" "$2"
    do
        usleep $sync_lock_sleep_interval
    done
    return 0
}

# Unlocks a resource.
#
# $1 = resource to be unlocked
# #2 = name of caller
# Returns: always 0
sync_unlock_resource()
{
    local traps=$(trap) pidfile="/var/lock/fli4l-$1.$PID" locker count
    # we have to be atomic
    trap "" 0 1 2 3 4 6 8 10 11 12 13 14 15

    sync_get_locker "$1" locker
    if [ -n "$locker" ] && [ $locker -eq $PID ]
    then
        read count < "$pidfile"
        count=$((count-1))
        if [ $count -eq 0 ]
        then
            rm -f "/var/lock/fli4l-$1.lock" "$pidfile"
        else
            echo $count > "$pidfile"
        fi
        sync_locked_resources=${sync_locked_resources/ $1 / }
        trap - 0 1 2 3 4 6 8 10 11 12 13 14 15
        eval "$traps"
        return 0
    else
        sync_show_unlock_error "$1" "$2" $locker
        trap - 0 1 2 3 4 6 8 10 11 12 13 14 15
        eval "$traps"
        return 1
    fi
}

# Unlocks all resources currently held. Used at shell exit in order to avoid
# deadlocks due to scripts terminating in an unexpected way.
sync_unlock_all_resources()
{
    local r
    while [ "$sync_locked_resources" != " " ]
    do
        for r in $sync_locked_resources
        do
            sync_unlock_resource $r sync_unlock_all_resources
        done
    done
    while [ "$sync_write_locked_resources" != " " ]
    do
        for r in $sync_write_locked_resources
        do
            sync_unlock_resource_for_writing $r sync_unlock_all_resources
        done
    done
    while [ "$sync_read_locked_resources" != " " ]
    do
        for r in $sync_read_locked_resources
        do
            sync_unlock_resource_for_reading $r sync_unlock_all_resources
        done
    done
}

#######################
# READER/WRITER LOCKS #
#######################
# Reader/writer locks offer fine-grained locking depending on the type of
# operation to be performed on the protected data. Read locks are shared and
# allow an unlimited number of processes to access the protected data in
# parallel for reading. Write locks are exclusive and allow exactly one process
# to access the protected data for writing while all other processes have to
# wait. Writers are preferred, i.e. if a writer wants to acquire a write lock,
# subsequent readers are forced to wait until the writer has acquired (and
# later released) the lock. This avoids writer starvation but it relies on the
# fact that read operations are frequent and write operations are infrequent.
# If a process has acquired a read lock, it can later try to acquire the write
# lock. This only works if all other readers have released their read lock and
# is called lock promotion. If two readers holding a read lock want to promote
# their lock to a write lock, a deadlock occurs. To prevent this, a
# reader/writer lock must always be guarded by another exclusive lock which is
# acquired and released around the code that may want to promote the lock. This
# additional exclusive lock ensures that a lock promotion is only possible if
# at most one process holds a read lock.

# directory holding read/write lock information, created by boot.d/rc000.base
sync_rwlock_dir=/var/lock/rwlock

# Locks a resource for reading.
#
# $1 = resource to be locked
# $2 = name of caller
# Returns: 0 on success and non-zero on failure
sync_lock_resource_for_reading()
{
    local readers_active="$sync_rwlock_dir/$1:readers-active" count=

    sync_lock_resource "$1:mutex" "$2"
    if [ -f "$readers_active.$PID" ]
    then
        read count < "$readers_active.$PID"
    fi

    if [ -z "$count" ]
    then
        local writer_active="$sync_rwlock_dir/$1:writer-active" \
            writers_waiting="$sync_rwlock_dir/$1:writers-waiting"
        while [ -s "$writer_active" -o -s "$writers_waiting" ]
        do
            sync_unlock_resource "$1:mutex" "$2"
            usleep $sync_lock_sleep_interval
            sync_lock_resource "$1:mutex" "$2"
        done
        echo 1 > "$readers_active.$PID"

        if [ -f "$readers_active" ]
        then
            read count < "$readers_active"
            echo $((count+1)) > "$readers_active"
        else
            echo 1 > "$readers_active"
        fi
    else
        echo $((count+1)) > "$readers_active.$PID"
    fi

    sync_read_locked_resources="$sync_read_locked_resources$1 "
    sync_unlock_resource "$1:mutex" "$2"
    return 0
}

# Unlocks a resource locked for reading.
#
# $1 = resource to be unlocked
# #2 = name of caller
# Returns: 0 on success and non-zero on failure
sync_unlock_resource_for_reading()
{
    local readers_active="$sync_rwlock_dir/$1:readers-active" count

    sync_lock_resource "$1:mutex" "$2"
    if [ -f "$readers_active.$PID" ]
    then
        read count < "$readers_active.$PID"
    fi

    if [ -n "$count" ]
    then
        count=$((count-1))
        if [ $count -eq 0 ]
        then
            rm "$readers_active.$PID"

            read count < "$readers_active"
            count=$((count-1))
            if [ $count -eq 0 ]
            then
                > "$readers_active"
            else
                echo $count > "$readers_active"
            fi
        else
            echo $count > "$readers_active.$PID"
        fi

        sync_read_locked_resources=${sync_read_locked_resources/ $1 / }
        sync_unlock_resource "$1:mutex" "$2"
        return 0
    else
        local caller=$(map_pid_to_process $PID)
        log_error "$caller/$2[$PID]: failed to unlock resource $1 supposed to be locked for reading as it is not locked at all"
        sync_unlock_resource "$1:mutex" "$2"
        return 1
    fi
}

# Checks if a write lock can be taken. This is the case if no other process has
# already acquired the write lock, and no other process (except us) has already
# acquired a read lock.
#
# NOTE: For performance reasons, this function does NOT check whether the write
# lock has already been taken by some other process. This check has to be done
# by the caller BEFORE calling this function.
#
# $1 = resource to be locked
sync_can_take_write_lock()
{
    local readers_active="$sync_rwlock_dir/$1:readers-active"
    if [ -s "$readers_active" ]
    then
        [ -f "$readers_active.$PID" ] || return 1
        local num_readers
        read num_readers < "$readers_active"
        [ $num_readers -eq 1 ]
    else
        return 0
    fi
}

# Locks a resource for writing.
#
# $1 = resource to be locked
# $2 = name of caller
# Returns: 0 on success and non-zero on failure
sync_lock_resource_for_writing()
{
    local writer_active="$sync_rwlock_dir/$1:writer-active" \
        writers_waiting="$sync_rwlock_dir/$1:writers-waiting"

    sync_lock_resource "$1:mutex" "$2"
    if [ -s "$writer_active" ]
    then
        local lockpid=0 lockcount=0
        read lockpid lockcount < "$writer_active"
        if [ $PID -eq $lockpid ]
        then
            echo "$PID $((lockcount+1))" > "$writer_active"
            sync_write_locked_resources="$sync_write_locked_resources$1 "
            sync_unlock_resource "$1:mutex" "$2"
            return 0
        fi
    elif sync_can_take_write_lock "$1"
    then
        echo "$PID 1" > "$writer_active"
        sync_write_locked_resources="$sync_write_locked_resources$1 "
        sync_unlock_resource "$1:mutex" "$2"
        return 0
    fi

    local num_writers_waiting=0
    [ -s "$writers_waiting" ] && read num_writers_waiting < "$writers_waiting"
    echo $((num_writers_waiting+1)) > "$writers_waiting"

    while true
    do
        sync_unlock_resource "$1:mutex" "$2"
        usleep $sync_lock_sleep_interval
        sync_lock_resource "$1:mutex" "$2"

        [ -s "$writer_active" ] && continue

        if sync_can_take_write_lock "$1"
        then
            echo "$PID 1" > "$writer_active"
            read num_writers_waiting < "$writers_waiting"
            num_writers_waiting=$((num_writers_waiting-1))
            if [ $num_writers_waiting -eq 0 ]
            then
                > "$writers_waiting"
            else
                echo $num_writers_waiting > "$writers_waiting"
            fi
            sync_write_locked_resources="$sync_write_locked_resources$1 "
            sync_unlock_resource "$1:mutex" "$2"
            return 0
        fi
    done
}

# Unlocks a resource locked for writing.
#
# $1 = resource to be unlocked
# #2 = name of caller
# Returns: 0 on success and non-zero on failure
sync_unlock_resource_for_writing()
{
    local writer_active="$sync_rwlock_dir/$1:writer-active"

    sync_lock_resource "$1:mutex" "$2"
    if [ -s "$writer_active" ]
    then
        local lockpid=0 lockcount=0
        read lockpid lockcount < "$writer_active"
        if [ $PID -eq $lockpid ]
        then
            lockcount=$((lockcount-1))
            if [ $lockcount -eq 0 ]
            then
                > "$writer_active"
            else
                echo "$PID $lockcount" > "$writer_active"
            fi
            sync_write_locked_resources=${sync_write_locked_resources/ $1 / }
            sync_unlock_resource "$1:mutex" "$2"
            return 0
        else
            local caller=$(map_pid_to_process $PID)
            local owner=$(map_pid_to_process $lockpid)
            log_error "$caller/$2[$PID]: failed to unlock resource $1 supposed to be locked for writing: already locked by $owner[$lockpid]"
            sync_unlock_resource "$1:mutex" "$2"
        fi
    else
        local caller=$(map_pid_to_process $PID)
        log_error "$caller/$2[$PID]: failed to unlock resource $1 supposed to be locked for writing as it is not locked at all"
        sync_unlock_resource "$1:mutex" "$2"
    fi

    return 1
}

# Makes the synchronization layer usable by the current process.
sync_init()
{
    # List of locked resources hold by the current process. Access to this list
    # is not synchronized in any way as it is local to the running script and
    # shell scripts are not multi-threaded.
    sync_locked_resources=" "
    # resources read-locked by this process
    sync_read_locked_resources=" "
    # resources write-locked by this process
    sync_write_locked_resources=" "

    # Determine real PID needed for proper locking.
    sync_set_subshell_pid

    # Be nice and release all locks at shell exit.
    #
    # NOTE: You should nevertheless release all locks explicitly! This is only a
    # safety belt for scripts that are terminated in an unexpected way (e.g. by
    # Ctrl+C).
    exit_trap_add sync_unlock_all_resources
}

locking_api='yes'

fork_add_handler sync_init
sync_init

fi # $locking_api != yes
