USES_LIBS := nmSEM nmImageViewer nmBase vrpn quat

USES_EXTERNAL_LIBS := tcl tk

EXTRA_CFLAGS := -DVIRTUAL_SEM

include ../Makefile.share

ifeq ($(MAKELEVEL),0)
.PHONY: client
client: $(OBJECT_DIR)/client

$(OBJECT_DIR)/client:
	$(MAKE) APP_NAME=client \
            MAKECMDGOALS="" \
            EXCLUDE_SRC="edax_server.C"

.PHONY: server
server: $(OBJECT_DIR)/server

$(OBJECT_DIR)/server:
	$(MAKE) APP_NAME=server \
            MAKECMDGOALS="" \
            EXCLUDE_SRC="sem_client.C"
endif
