idf_component_register(SRCS "test.c" 
	#"main.c
	REQUIRES driver
	PRIV_REQUIRES hagl hagl_hal
	INCLUDE_DIRS ".")
