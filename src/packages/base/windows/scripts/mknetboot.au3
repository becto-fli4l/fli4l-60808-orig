; -------------------------------------------------------------------------
; creates a fli4l-netbootimage                                 __FLI4LVER__
;
; Creation: lanspezi 2005-04-04
; $Id$
; -------------------------------------------------------------------------
#include-once

Func mknetboot ()
    msg_lang_read ("mknetboot")
    fli4lbuild_errmsg ($MSG_07_mld1 & @CRLF & $MSG_07_mld2,1)
    $bool_error = "true"
    exit_fli4l_build ()
EndFunc
