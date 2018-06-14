# Create a linkable module
add_library(viewer-output MODULE
	viewer.c
	config.c
	config.h
)

install(
	TARGETS viewer-output
	LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/ipfixcol2/"
)