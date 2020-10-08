# SPDX-License-Identifier: Apache-2.0
MOXA_VERSION_CFG=base-system/etc/moxa-configs/moxa-version.conf
PKG_BUILDDATE ?= $(shell date +%y%m%d%H)

all: moxa-ver-cfg

moxa-ver-cfg:
	@echo "BUILDDATE=$(PKG_BUILDDATE)" >> $(MOXA_VERSION_CFG)

clean:
	sed -i '2,$$d' $(MOXA_VERSION_CFG)
