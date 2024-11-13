; -------------------------------------------------------------------------
; creates a fli4l-pxeboot    e                                 __FLI4LVER__
;
; Creation: lanspezi 2005-06-11
; $Id$
; -------------------------------------------------------------------------
#include-once

Func mkpxeboot ()
    msg_lang_read ("mknetboot")
    fli4lbuild_errmsg ($MSG_07_mld1 & @CRLF & $MSG_07_mld2,1)
    $bool_error = "true"
    exit_fli4l_build ()
EndFunc
