
.PHONY: build


build: $(TARGET)$(EXT)

ifeq ($(BUILD_3DSX), 1)
build: $(TARGET).3dsx $(TARGET).smdh
$(BUILD_DIR)/$(TARGET).3dsx: $(BUILD_DIR)/$(TARGET).elf
endif
ifeq ($(BUILD_3DS), 1)
build: $(TARGET).3ds
$(BUILD_DIR)/$(TARGET).3ds: $(BUILD_DIR)/$(TARGET).elf
endif

ifeq ($(BUILD_CIA), 1)
build: $(TARGET).cia
$(BUILD_DIR)/$(TARGET).cia: $(BUILD_DIR)/$(TARGET).elf
endif

%: $(BUILD_DIR)/%
	cp $< $@


$(BUILD_DIR)/$(TARGET)$(EXT): $(OBJS) $(MODULE) .lastbuild
	touch .lastbuild
	$(CXX) $(OBJS) -L$(dir $(MODULE)) -l:$(notdir $(MODULE)) $(LDFLAGS) $(LIBDIRS) $(LIBS) -Wall -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< $(CFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $< $(CXXFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $< $(ASFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $< $(ASFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

$(BUILD_DIR)/%.a:
	$(AR) -rc $@ $^

%.vert.inc: %.vert
	glslc -c -mfmt=c $< -o $@

%.frag.inc: %.frag
	glslc -c -mfmt=c $< -o $@

%.geom.inc: %.geom
	glslc -c -mfmt=c $< -o $@

%.vert.inc: %.slang
	glslc -c -mfmt=c -fshader-stage=vertex -DVERTEX_SHADER $< -o $@

%.frag.inc: %.slang
	glslc -c -mfmt=c -fshader-stage=fragment -DFRAGMENT_SHADER $< -o $@

%.geom.inc: %.slang
	glslc -c -mfmt=c -fshader-stage=geometry -DGEOMETRY_SHADER $< -o $@

ifeq ($(platform),3ds)
$(BUILD_DIR)/%.o: %.vsh %.gsh
	$(DEVKITPRO)/devkitARM/bin/picasso $^ -o $*.shbin
	$(DEVKITPRO)/devkitARM/bin/bin2s $*.shbin | $(PREFIX)as -o $@
	rm $*.shbin

$(BUILD_DIR)/%.o: %.vsh
	$(DEVKITPRO)/devkitARM/bin/picasso $^ -o $*.shbin
	$(DEVKITPRO)/devkitARM/bin/bin2s $*.shbin | $(PREFIX)as -o $@
	rm $*.shbin

$(BUILD_DIR)/$(TARGET).smdh: $(APP_ICON)
	smdhtool --create "$(APP_TITLE)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $(APP_ICON) $@

$(BUILD_DIR)/$(TARGET).3dsx: $(BUILD_DIR)/$(TARGET).elf
	3dsxtool $< $@

$(BUILD_DIR)/$(TARGET).bnr: $(APP_BANNER) $(APP_AUDIO)
	$(BANNERTOOL) makebanner -i "$(APP_BANNER)" -a "$(APP_AUDIO)" -o $@

$(BUILD_DIR)/$(TARGET).icn: $(APP_ICON)
	$(BANNERTOOL) makesmdh -s "$(APP_TITLE)" -l "$(APP_TITLE)" -p "$(APP_AUTHOR)" -i $(APP_ICON) -o $@

$(BUILD_DIR)/$(TARGET).3ds: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bnr $(BUILD_DIR)/$(TARGET).icn $(APP_RSF)
	$(MAKEROM) -f cci -o $@ $(MAKEROM_ARGS_COMMON) -DAPP_ENCRYPTED=true

$(BUILD_DIR)/$(TARGET).cia: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bnr $(BUILD_DIR)/$(TARGET).icn $(APP_RSF)
	$(MAKEROM) -f cia -o $@ $(MAKEROM_ARGS_COMMON) -DAPP_ENCRYPTED=false

endif

.lastbuild: ;

clean:
#	rm -rf objs
	rm -f $(OBJS) $(OBJS:.o=.depend)
	rm -f $(BUILD_DIR)/$(TARGET) $(TARGET) $(SPIRV_OBJS) .lastbuild


-include $(OBJS:.o=.depend)
