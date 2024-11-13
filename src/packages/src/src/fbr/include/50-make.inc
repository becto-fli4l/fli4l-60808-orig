#!/bin/bash

make_targets()
{
	make -C $br_wcdir -j $J \
		PARALLEL_JOBS=$J \
		BR2_CONFIG=$br_wc_config \
		BR2_DL_DIR=$fbr_dldir \
		FBR_SRCDIR=$br_dir \
		FBR_WCDIR=$br_wcdir \
		FBR_TIDY=$fbr_tidy \
		FLI4L_SRCDIR=$fbr_root/../fli4l \
		$@ 3>&1 1>>$fbr_log 2>&1 ||
			error "Build failed; see build log ($fbr_log) for details!" 90
}

make_targets_no_log()
{
	make -C $br_wcdir -j $J \
		PARALLEL_JOBS=$J \
		BR2_CONFIG=$br_wc_config \
		BR2_DL_DIR=$fbr_dldir \
		FBR_SRCDIR=$br_dir \
		FBR_WCDIR=$br_wcdir \
		FBR_TIDY=$fbr_tidy \
		FLI4L_SRCDIR=$fbr_root/../fli4l \
		$@ 3>/dev/null ||
			error "Build failed; see build log ($fbr_log) for details!" 90
}
