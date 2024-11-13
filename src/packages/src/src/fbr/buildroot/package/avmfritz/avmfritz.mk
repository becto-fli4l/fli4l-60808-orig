AVMFRITZ_SOURCE       =
AVMFRITZ_DEPENDENCIES =

$(eval $(generic-package))

include $(sort $(wildcard package/avmfritz/km*/*.mk))
include $(sort $(wildcard package/avmfritz/f*/*.mk))
