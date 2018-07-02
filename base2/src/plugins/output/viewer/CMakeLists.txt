# Create a linkable module

add_library(viewer-output MODULE
		viewer.c
		config.c
		config.h
		Reader.h
		Reader.c
)

install(
	TARGETS viewer-output
	LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/ipfixcol2/"
)