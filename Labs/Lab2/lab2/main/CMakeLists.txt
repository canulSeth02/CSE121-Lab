set(SOURCES "temp.c")
idf_component_register(SRCS ${SOURCES}
	INCLUDE_DIRS "." REQUIRES driver)
