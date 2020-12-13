ifeq ($(OS),Windows_NT)
CHROMIUM	:= chromium.exe
SED			:= sed.exe -i
else
	ifeq ($(shell uname -s),Darwin)
# I use Edge, change this if using another Chromium browser -Pk11
CHROMIUM	:= /Applications/Microsoft\ Edge.app/Contents/MacOS/Microsoft\ Edge
# macOS sed is dumb apparently
SED			:= sed -i ''
	else
CHROMIUM	:= chromium
SED			:= sed -i
	endif
endif

BASEURL		:= https://ds-homebrew.github.io/twilight-manual/pages
PAGES		:= _howToUse.html chat.html gbarunner2.html nds-bootstrap.html settings.html theme1_dsi.html theme2_3ds.html theme3_r4.html theme4_acekard.html
LANGS		:= chinese_s chinese_t danish english french german hungarian italian japanese korean norwegian polish portuguese russian spanish swedish turkish ukrainian
NITROFILES	:= ../nitrofiles

INPUTS		:= $(foreach dir,$(LANGS),$(foreach file,$(PAGES),$(dir)/$(file)))
TARGETS		:= $(foreach file,$(INPUTS:.html=.gif),$(addprefix $(NITROFILES)/pages/, $(file)))

INIFILES	:= $(wildcard $(NITROFILES)/pages/english/*.ini)
INITARGETS	:= $(foreach file,$(INIFILES),$(foreach lang,$(filter-out english,$(LANGS)),$(subst english,$(lang),$(file))))

.PHONY: all clean $(INPUTS)

all:	inifiles $(TARGETS)

$(NITROFILES)/pages/%.gif	:	%.html
	@echo $$(basename $(@D))/$$(basename $<)
	@[ -d "$(@D)" ] || mkdir -p "$(@D)"
	$(CHROMIUM) --no-sandbox --hide-scrollbars --headless --disable-gpu --window-size=256,3000 1>/dev/null 2>&1 --screenshot$(SCREENSHOT) $(BASEURL)/$<
	@ffmpeg -i screenshot.png -vf "palettegen=max_colors=246" palette.png -y -loglevel error
	@ffmpeg -i screenshot.png -i palette.png -filter_complex "$$(ffmpeg -loop 1 -i screenshot.png -frames:v 3 -vf 'negate,cropdetect=0:2:0' -f null - 2>&1 | awk '/y2/ {print $$NF}' | tail -1),paletteuse" screenshot.gif -y -loglevel error
#	Remove if able to use ffmpeg 4.3 or newer again, old ffmpeg does the palettes backwards...
	@gifsicle screenshot.gif --colors=246 --optimize=200 > "$@"
	@rm -f screenshot.png screenshot.gif palette.png

inifiles	:	$(INITARGETS)

$(foreach lang,$(filter-out english,$(LANGS)),$(NITROFILES)/pages/$(lang)/%.ini)	:	$(NITROFILES)/pages/english/%.ini
	@echo $*.ini
	@$(foreach lang,$(filter-out english,$(LANGS)),[ -d "$(NITROFILES)/pages/$(lang)" ] || mkdir -p "$(NITROFILES)/pages/$(lang)";)
	@$(foreach lang,$(filter-out english,$(LANGS)),cp $< $(NITROFILES)/pages/$(lang)/$*.ini;)
	@$(foreach lang,$(LANGS),$(SED) "s/TITLE = .*/TITLE = $(shell curl -s $(BASEURL)/$(lang)/$*.html | xmllint --html --xpath "//head/title/text()" -)/" $(NITROFILES)/pages/$(lang)/$*.ini;)

clean:
	@echo clean ...
	@rm -rf $(TARGETS)
	@rm -rf $(INITARGETS)
