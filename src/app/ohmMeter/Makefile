USES_LIBS := tclLinkVar nmAux nmMScope nmBase nmMP vrpn quat

include ../Makefile.share

ifeq ($(MAKELEVEL),0)
.PHONY: client
client: $(OBJECT_DIR)/client

$(OBJECT_DIR)/client:
	$(MAKE) APP_NAME=client \
            MAKECMDGOALS="" \
            EXCLUDE_SRC="server.C"

.PHONY: server
server: $(OBJECT_DIR)/server

$(OBJECT_DIR)/server:
	$(MAKE) APP_NAME=server \
            MAKECMDGOALS="" \
            EXCLUDE_SRC="client.C"
endif
