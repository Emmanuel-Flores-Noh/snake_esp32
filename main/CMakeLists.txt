idf_component_register(
	# SRCS "selector.c"
	# SRCS "test.c"
	SRCS "main.c"
	REQUIRES driver
	PRIV_REQUIRES hagl hagl_hal
	INCLUDE_DIRS ".")
