
export OPENAPPS_OUTPATH := $(PWD)/output
export OPENAPPS_BASE := $(PWD)

-include $(OPENAPPS_OUTPATH)/package.cfg

-include $(OPENAPPS_OUTPATH)/Makefile

include tools/make.configure
