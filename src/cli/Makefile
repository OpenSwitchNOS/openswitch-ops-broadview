MODULE := broadview_cli

CC ?= gcc
AR ?= ar
OPENAPPS_OUTPATH ?= .

# NOOP - no-operation, used to suppress "Nothing to do for ..." messages.
NOOP  ?= @:

export OUT_BROADVIEW_CLI=$(OPENAPPS_OUTPATH)/cli
export LIBS_BROADVIEW_CLI=$(MODULE).a

searchdirs += .
CFLAGS += -I../../include -I$(BV_OVS_INCLUDE) 

export CPATH += $(searchdirs)

OBJECTS_BROADVIEW_CLI := $(notdir $(patsubst %.c,%.o,$(shell find $(searchdirs) -name "*.c")))

export VPATH += $(dir $(shell find . -name "*.c"))

$(OUT_BROADVIEW_CLI)/%.o :  $(searchdirs)/%.c
	@mkdir -p $(OUT_BROADVIEW_CLI)
	$(CC) $(CFLAGS) -c  $< -o $@

$(OUT_BROADVIEW_CLI)/$(LIBS_BROADVIEW_CLI): $(patsubst %,$(OUT_BROADVIEW_CLI)/%,$(subst :, ,$(OBJECTS_BROADVIEW_CLI)))
	@cd $(OUT_BROADVIEW_CLI) && $(AR) rvs $(MODULE).a $(OBJECTS_BROADVIEW_CLI)

#default target
$(MODULE) all: $(OUT_BROADVIEW_CLI)/$(LIBS_BROADVIEW_CLI)
	$(NOOP)

clean-$(MODULE) clean:
	rm -rf $(OUT_BROADVIEW_CLI)


