# Makefile
#	source:	entering source_code directory and compiling source code
#	install:mkdir directory for binary files and prepare package files
#	clean:	clean all binary files
#
# Wes Huang (Wes.Huang@moxa.com)

# source code name
MOXA_VERSION_CFG=moxa-configs/moxa-version.conf

PKG_FW_VERSION ?= develop
PKG_BUILDDATE ?= $(shell date +%y%m%d%H)

all: moxa-ver-cfg

moxa-ver-cfg:
	@echo "FW_VERSION=$(PKG_FW_VERSION)" > $(MOXA_VERSION_CFG)
	@echo "BUILDDATE=$(PKG_BUILDDATE)" >> $(MOXA_VERSION_CFG)

clean:
	rm -f $(MOXA_VERSION_CFG)
