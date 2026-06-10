# GNU Make project for 'NoesisApp'

# Parallel builds can be enabled using the environment variable MAKEFLAGS (eg: MAKEFLAGS="-j 4")
# For parallel execution in Windows, Make 4.0 or better is required

CC = arm-linux-gnueabihf-gcc
CXX = arm-linux-gnueabihf-g++
AR = arm-linux-gnueabihf-ar

CFLAGS_COMMON = -g -pipe -Wall -Werror -Wfatal-errors -Wtype-limits -fvisibility=hidden -fmerge-all-constants
CXXFLAGS_COMMON = $(CFLAGS_COMMON) -std=c++11 -fno-exceptions -frtti -fvisibility-inlines-hidden -fno-threadsafe-statics
LDFLAGS_COMMON = 
SHADERFLAGS_COMMON = -I../../../Packages/Render -I../../../Tools/ShaderCompiler/Include -lang=glsl

SHELLTYPE = msdos
ifeq (, $(ComSpec)$(COMSPEC))
  SHELLTYPE = posix
endif
ifeq (/bin, $(findstring /bin, $(SHELL)))
  SHELLTYPE = posix
endif
ifeq (/bin, $(findstring /bin, $(MAKESHELL)))
  SHELLTYPE = posix
endif

ifeq (posix, $(SHELLTYPE))
  DEVNUL = /dev/null
  WHICH = which
  MKDIR = mkdir -p "$(1)"
  BIN2H = ../../../Tools/Bin2h/bin2h.py
  SHADERC = ../../../Tools/ShaderCompiler/Bin/Linux/ShaderCompiler
  CP = cp "$(1)" "$(2)"
  MV = mv "$(1)" "$(2)"
  RM = rm -f "$(1)"
  RMTREE = rm -rf "$(1)"
  FIXSLASHES = 
else
  DEVNUL = NUL
  WHICH = where
  MKDIR = mkdir "$(subst /,\,$(1))" 2> nul || exit 0
  BIN2H = ..\..\..\Tools\Bin2h\bin2h.exe
  SHADERC = ..\..\..\Tools\ShaderCompiler\Bin\Windows\ShaderCompiler.exe
  CP = copy /Y "$(subst /,\,$(1))" "$(subst /,\,$(2))" > nul
  MV = move /Y "$(subst /,\,$(1))" "$(subst /,\,$(2))" > nul
  RM = del /Q "$(subst /,\,$(1))" 2> nul || exit 0
  RMTREE = rmdir /S /Q "$(subst /,\,$(1))" 2> nul || exit 0
  SED = ..\..\..\Tools\sed\sed.exe
  FIXSLASHES = $(SED) -i "s/\(\S\)\\\(\S\)/\1\/\2/g" "$(1)"
endif

ifeq (, $(shell ${WHICH} $(CC) 2> $(DEVNUL) ))
$(error $(CC) not found in PATH)
endif

ifeq (, $(shell ${WHICH} $(CXX) 2> $(DEVNUL) ))
$(error $(CXX) not found in PATH)
endif

ifeq ($(V), 1)
  Q =
else
  Q = @
endif

ifndef CONFIG
  CONFIG = Release
endif

ifeq ($(CONFIG), Debug)
  TARGET_DIR = ../../../../Bin/linux_arm
  TARGET = ../../../../Bin/linux_arm/libNoesisApp.so
  INTERMEDIATE = build/Debug
  LIB_DIRS = 
  LIBS = -lNoesis -lGLESv2 -ldl -lX11 -lEGL -lpthread
  LIBS_DEPS = ../../../../Bin/linux_arm/libNoesis.so
  INCLUDES = -I../../../../Include -I../../../Packages/App/ApplicationLauncher/Include -I../../../Packages/App/Display/Include -I../../../Packages/App/DisplayLauncher/Include -I../../../Packages/App/Interactivity/Include -I../../../Packages/App/LangServer/Include -I../../../Packages/App/LangServerHelpers/Include -I../../../Packages/App/Launcher/Include -I../../../Packages/App/MediaElement/Include -I../../../Packages/App/Providers/Include -I../../../Packages/App/Shaders/Include -I../../../Packages/App/Theme/Include -I../../../Packages/Render/GLRenderDevice/Include -I../../../Packages/Render/RenderContext/Include -Ibuild/Debug
  DEFINES = -DNS_BUILD -DNS_CONFIG=Debug -DNS_VERSION=3.2.12 -DNS_APP_FRAMEWORK -DNS_LICENSE -DNS_DEBUG -DNS_PROFILE -DNS_MINIMUM_LOG_LEVEL=1 -DNS_APP_PROVIDERS_API=NS_DLL_EXPORT -DNS_RENDER_GLRENDERDEVICE_API=NS_DLL_EXPORT -DNS_RENDER_RENDERCONTEXT_API=NS_DLL_EXPORT -DNS_RENDER_GLRENDERCONTEXT_API=NS_DLL_EXPORT -DNS_APP_DISPLAY_API=NS_DLL_EXPORT -DNS_APP_XDISPLAY_API=NS_DLL_EXPORT -DNS_APP_LAUNCHER_API=NS_DLL_EXPORT -DNS_APP_SHADERS_API=NS_DLL_EXPORT -DNS_APP_DISPLAYLAUNCHER_API=NS_DLL_EXPORT -DNS_APP_THEME_API=NS_DLL_EXPORT -DNS_APP_LANGSERVER_API=NS_DLL_EXPORT -DNS_APP_LANGSERVERHELPERS_API=NS_DLL_EXPORT -DNS_APP_APPLICATIONLAUNCHER_API=NS_DLL_EXPORT -DNS_APP_MEDIAELEMENT_API=NS_DLL_EXPORT -DNS_APP_INTERACTIVITY_API=NS_DLL_EXPORT
  CFLAGS = $(CFLAGS_COMMON) -D_DEBUG
  CXXFLAGS = $(CXXFLAGS_COMMON) -D_DEBUG
  LDFLAGS = $(LDFLAGS_COMMON) $(CXXFLAGS)
  SHADERFLAGS = $(SHADERFLAGS_COMMON) -Od -g
else ifeq ($(CONFIG), Profile)
  TARGET_DIR = ../../../../Bin/linux_arm
  TARGET = ../../../../Bin/linux_arm/libNoesisApp.so
  INTERMEDIATE = build/Profile
  LIB_DIRS = 
  LIBS = -lNoesis -lGLESv2 -ldl -lX11 -lEGL -lpthread
  LIBS_DEPS = ../../../../Bin/linux_arm/libNoesis.so
  INCLUDES = -I../../../../Include -I../../../Packages/App/ApplicationLauncher/Include -I../../../Packages/App/Display/Include -I../../../Packages/App/DisplayLauncher/Include -I../../../Packages/App/Interactivity/Include -I../../../Packages/App/LangServer/Include -I../../../Packages/App/LangServerHelpers/Include -I../../../Packages/App/Launcher/Include -I../../../Packages/App/MediaElement/Include -I../../../Packages/App/Providers/Include -I../../../Packages/App/Shaders/Include -I../../../Packages/App/Theme/Include -I../../../Packages/Render/GLRenderDevice/Include -I../../../Packages/Render/RenderContext/Include -Ibuild/Profile
  DEFINES = -DNS_BUILD -DNS_CONFIG=Profile -DNS_VERSION=3.2.12 -DNS_APP_FRAMEWORK -DNS_LICENSE -DNS_PROFILE -DNS_MINIMUM_LOG_LEVEL=2 -DNS_APP_PROVIDERS_API=NS_DLL_EXPORT -DNS_RENDER_GLRENDERDEVICE_API=NS_DLL_EXPORT -DNS_RENDER_RENDERCONTEXT_API=NS_DLL_EXPORT -DNS_RENDER_GLRENDERCONTEXT_API=NS_DLL_EXPORT -DNS_APP_DISPLAY_API=NS_DLL_EXPORT -DNS_APP_XDISPLAY_API=NS_DLL_EXPORT -DNS_APP_LAUNCHER_API=NS_DLL_EXPORT -DNS_APP_SHADERS_API=NS_DLL_EXPORT -DNS_APP_DISPLAYLAUNCHER_API=NS_DLL_EXPORT -DNS_APP_THEME_API=NS_DLL_EXPORT -DNS_APP_LANGSERVER_API=NS_DLL_EXPORT -DNS_APP_LANGSERVERHELPERS_API=NS_DLL_EXPORT -DNS_APP_APPLICATIONLAUNCHER_API=NS_DLL_EXPORT -DNS_APP_MEDIAELEMENT_API=NS_DLL_EXPORT -DNS_APP_INTERACTIVITY_API=NS_DLL_EXPORT
  CFLAGS = $(CFLAGS_COMMON) -O3 -ffast-math -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -U_FORTIFY_SOURCE
  CXXFLAGS = $(CXXFLAGS_COMMON) -O3 -ffast-math -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -U_FORTIFY_SOURCE
  LDFLAGS = $(LDFLAGS_COMMON) $(CXXFLAGS) -fuse-ld=gold -Wl,-O3,--gc-sections,--icf=all
  SHADERFLAGS = $(SHADERFLAGS_COMMON)
else ifeq ($(CONFIG), Release)
  TARGET_DIR = ../../../../Bin/linux_arm
  TARGET = ../../../../Bin/linux_arm/libNoesisApp.so
  INTERMEDIATE = build/Release
  LIB_DIRS = 
  LIBS = -lNoesis -lGLESv2 -ldl -lX11 -lEGL -lpthread
  LIBS_DEPS = ../../../../Bin/linux_arm/libNoesis.so
  INCLUDES = -I../../../../Include -I../../../Packages/App/ApplicationLauncher/Include -I../../../Packages/App/Display/Include -I../../../Packages/App/DisplayLauncher/Include -I../../../Packages/App/Interactivity/Include -I../../../Packages/App/LangServer/Include -I../../../Packages/App/LangServerHelpers/Include -I../../../Packages/App/Launcher/Include -I../../../Packages/App/MediaElement/Include -I../../../Packages/App/Providers/Include -I../../../Packages/App/Shaders/Include -I../../../Packages/App/Theme/Include -I../../../Packages/Render/GLRenderDevice/Include -I../../../Packages/Render/RenderContext/Include -Ibuild/Release
  DEFINES = -DNS_BUILD -DNS_CONFIG=Release -DNS_VERSION=3.2.12 -DNS_APP_FRAMEWORK -DNS_LICENSE -DNS_APP_PROVIDERS_API=NS_DLL_EXPORT -DNS_RENDER_GLRENDERDEVICE_API=NS_DLL_EXPORT -DNS_RENDER_RENDERCONTEXT_API=NS_DLL_EXPORT -DNS_RENDER_GLRENDERCONTEXT_API=NS_DLL_EXPORT -DNS_APP_DISPLAY_API=NS_DLL_EXPORT -DNS_APP_XDISPLAY_API=NS_DLL_EXPORT -DNS_APP_LAUNCHER_API=NS_DLL_EXPORT -DNS_APP_SHADERS_API=NS_DLL_EXPORT -DNS_APP_DISPLAYLAUNCHER_API=NS_DLL_EXPORT -DNS_APP_THEME_API=NS_DLL_EXPORT -DNS_APP_LANGSERVER_API=NS_DLL_EXPORT -DNS_APP_LANGSERVERHELPERS_API=NS_DLL_EXPORT -DNS_APP_APPLICATIONLAUNCHER_API=NS_DLL_EXPORT -DNS_APP_MEDIAELEMENT_API=NS_DLL_EXPORT -DNS_APP_INTERACTIVITY_API=NS_DLL_EXPORT
  CFLAGS = $(CFLAGS_COMMON) -O3 -ffast-math -flto -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -U_FORTIFY_SOURCE -fomit-frame-pointer
  CXXFLAGS = $(CXXFLAGS_COMMON) -O3 -ffast-math -flto -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -U_FORTIFY_SOURCE -fomit-frame-pointer
  LDFLAGS = $(LDFLAGS_COMMON) $(CXXFLAGS) -fuse-ld=gold -Wl,-O3,--gc-sections,--icf=all
  SHADERFLAGS = $(SHADERFLAGS_COMMON)
else
  $(error Invalid configuration '$(CONFIG)')
endif

PCH = $(INTERMEDIATE)/pch.h.gch
PCHFLAGS = -include $(INTERMEDIATE)/pch.h
DATA = $(INTERMEDIATE)/Tint.bin.h $(INTERMEDIATE)/DirectionalBlur.bin.h $(INTERMEDIATE)/Pinch.bin.h $(INTERMEDIATE)/Pixelate.bin.h $(INTERMEDIATE)/Saturation.bin.h $(INTERMEDIATE)/Vignette.bin.h $(INTERMEDIATE)/Plasma.bin.h $(INTERMEDIATE)/ConicGradient.bin.h $(INTERMEDIATE)/Monochrome.bin.h $(INTERMEDIATE)/Waves.bin.h $(INTERMEDIATE)/CrossFade.bin.h $(INTERMEDIATE)/InnerShadow.bin.h $(INTERMEDIATE)/OuterShadow.bin.h $(INTERMEDIATE)/NoesisTheme.Styles.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Fonts.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Colors.Dark.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Colors.Light.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkRed.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightRed.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkGreen.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightGreen.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkBlue.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightBlue.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkOrange.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightOrange.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkEmerald.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightEmerald.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkPurple.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightPurple.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkCrimson.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightCrimson.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkLime.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightLime.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.DarkAqua.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.Brushes.LightAqua.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkRed.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightRed.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkGreen.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightGreen.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkBlue.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightBlue.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkOrange.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightOrange.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkEmerald.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightEmerald.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkPurple.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightPurple.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkCrimson.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightCrimson.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkLime.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightLime.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.DarkAqua.xaml.bin.h $(INTERMEDIATE)/NoesisTheme.LightAqua.xaml.bin.h $(INTERMEDIATE)/PT\ Root\ UI_Regular.otf.bin.h $(INTERMEDIATE)/PT\ Root\ UI_Bold.otf.bin.h $(INTERMEDIATE)/StatsOverlay.xaml.bin.h $(INTERMEDIATE)/MessageBox.xaml.bin.h $(INTERMEDIATE)/dripicons-v2.ttf.bin.h $(INTERMEDIATE)/ModeNine.ttf.bin.h $(INTERMEDIATE)/generic.xaml.bin.h

.SUFFIXES:
.DELETE_ON_ERROR:
.PHONY: all prebuild data clean

all: prebuild $(TARGET)

prebuild:
	@echo ==== [NoesisApp] Building '$(CONFIG)' configuration ====

data: $(DATA)

$(INTERMEDIATE):
	$(Q)$(call MKDIR,$(INTERMEDIATE))

$(TARGET_DIR):
	$(Q)$(call MKDIR,$(TARGET_DIR))

$(PCH): pch/pch.h | $(INTERMEDIATE)
	@echo [PCH] [NoesisApp] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -x c++-header -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)


$(INTERMEDIATE)/%.o: ../../../Packages/App/Providers/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/Providers] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Providers/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/Providers] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/Render/GLRenderDevice/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [Render/GLRenderDevice] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/Render/GLRenderDevice/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [Render/GLRenderDevice] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/Render/RenderContext/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [Render/RenderContext] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/Render/RenderContext/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [Render/RenderContext] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/Render/GLRenderContext/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [Render/GLRenderContext] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/Render/GLRenderContext/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [Render/GLRenderContext] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Display/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/Display] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Display/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/Display] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/XDisplay/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/XDisplay] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/XDisplay/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/XDisplay] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Launcher/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/Launcher] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Launcher/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/Launcher] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Shaders/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/Shaders] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Shaders/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/Shaders] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/Tint.bin.h: ../../../Packages/App/Shaders/Src/Tint.noesiseffect | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Tint.noesiseffect" -o "$(INTERMEDIATE)/Tint.bin.h"

$(INTERMEDIATE)/DirectionalBlur.bin.h: ../../../Packages/App/Shaders/Src/DirectionalBlur.noesiseffect | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/DirectionalBlur.noesiseffect" -o "$(INTERMEDIATE)/DirectionalBlur.bin.h"

$(INTERMEDIATE)/Pinch.bin.h: ../../../Packages/App/Shaders/Src/Pinch.noesiseffect | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Pinch.noesiseffect" -o "$(INTERMEDIATE)/Pinch.bin.h"

$(INTERMEDIATE)/Pixelate.bin.h: ../../../Packages/App/Shaders/Src/Pixelate.noesiseffect | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Pixelate.noesiseffect" -o "$(INTERMEDIATE)/Pixelate.bin.h"

$(INTERMEDIATE)/Saturation.bin.h: ../../../Packages/App/Shaders/Src/Saturation.noesiseffect | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Saturation.noesiseffect" -o "$(INTERMEDIATE)/Saturation.bin.h"

$(INTERMEDIATE)/Vignette.bin.h: ../../../Packages/App/Shaders/Src/Vignette.noesiseffect | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Vignette.noesiseffect" -o "$(INTERMEDIATE)/Vignette.bin.h"

$(INTERMEDIATE)/Plasma.bin.h: ../../../Packages/App/Shaders/Src/Plasma.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Plasma.noesisbrush" -o "$(INTERMEDIATE)/Plasma.bin.h"

$(INTERMEDIATE)/ConicGradient.bin.h: ../../../Packages/App/Shaders/Src/ConicGradient.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/ConicGradient.noesisbrush" -o "$(INTERMEDIATE)/ConicGradient.bin.h"

$(INTERMEDIATE)/Monochrome.bin.h: ../../../Packages/App/Shaders/Src/Monochrome.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Monochrome.noesisbrush" -o "$(INTERMEDIATE)/Monochrome.bin.h"

$(INTERMEDIATE)/Waves.bin.h: ../../../Packages/App/Shaders/Src/Waves.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/Waves.noesisbrush" -o "$(INTERMEDIATE)/Waves.bin.h"

$(INTERMEDIATE)/CrossFade.bin.h: ../../../Packages/App/Shaders/Src/CrossFade.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/CrossFade.noesisbrush" -o "$(INTERMEDIATE)/CrossFade.bin.h"

$(INTERMEDIATE)/InnerShadow.bin.h: ../../../Packages/App/Shaders/Src/InnerShadow.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/InnerShadow.noesisbrush" -o "$(INTERMEDIATE)/InnerShadow.bin.h"

$(INTERMEDIATE)/OuterShadow.bin.h: ../../../Packages/App/Shaders/Src/OuterShadow.noesisbrush | $(INTERMEDIATE)
	@echo [SHADER] [App/Shaders] $(notdir $<)
	$(Q)$(SHADERC) $(SHADERFLAGS) "../../../Packages/App/Shaders/Src/OuterShadow.noesisbrush" -o "$(INTERMEDIATE)/OuterShadow.bin.h"

$(INTERMEDIATE)/%.o: ../../../Packages/App/DisplayLauncher/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/DisplayLauncher] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/DisplayLauncher/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/DisplayLauncher] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Theme/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/Theme] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Theme/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/Theme] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/NoesisTheme.Styles.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Styles.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Styles.xaml" > "$(INTERMEDIATE)/NoesisTheme.Styles.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Fonts.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Fonts.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Fonts.xaml" > "$(INTERMEDIATE)/NoesisTheme.Fonts.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Colors.Dark.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Colors.Dark.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Colors.Dark.xaml" > "$(INTERMEDIATE)/NoesisTheme.Colors.Dark.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Colors.Light.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Colors.Light.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Colors.Light.xaml" > "$(INTERMEDIATE)/NoesisTheme.Colors.Light.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkRed.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkRed.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkRed.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkRed.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightRed.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightRed.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightRed.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightRed.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkGreen.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkGreen.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkGreen.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkGreen.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightGreen.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightGreen.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightGreen.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightGreen.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkBlue.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkBlue.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkBlue.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkBlue.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightBlue.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightBlue.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightBlue.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightBlue.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkOrange.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkOrange.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkOrange.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkOrange.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightOrange.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightOrange.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightOrange.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightOrange.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkEmerald.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkEmerald.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkEmerald.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkEmerald.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightEmerald.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightEmerald.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightEmerald.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightEmerald.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkPurple.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkPurple.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkPurple.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkPurple.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightPurple.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightPurple.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightPurple.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightPurple.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkCrimson.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkCrimson.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkCrimson.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkCrimson.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightCrimson.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightCrimson.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightCrimson.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightCrimson.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkLime.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkLime.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkLime.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkLime.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightLime.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightLime.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightLime.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightLime.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.DarkAqua.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkAqua.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.DarkAqua.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.DarkAqua.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.Brushes.LightAqua.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightAqua.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.Brushes.LightAqua.xaml" > "$(INTERMEDIATE)/NoesisTheme.Brushes.LightAqua.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkRed.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkRed.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkRed.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkRed.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightRed.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightRed.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightRed.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightRed.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkGreen.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkGreen.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkGreen.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkGreen.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightGreen.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightGreen.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightGreen.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightGreen.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkBlue.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkBlue.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkBlue.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkBlue.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightBlue.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightBlue.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightBlue.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightBlue.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkOrange.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkOrange.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkOrange.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkOrange.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightOrange.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightOrange.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightOrange.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightOrange.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkEmerald.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkEmerald.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkEmerald.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkEmerald.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightEmerald.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightEmerald.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightEmerald.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightEmerald.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkPurple.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkPurple.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkPurple.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkPurple.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightPurple.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightPurple.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightPurple.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightPurple.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkCrimson.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkCrimson.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkCrimson.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkCrimson.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightCrimson.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightCrimson.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightCrimson.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightCrimson.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkLime.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkLime.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkLime.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkLime.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightLime.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightLime.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightLime.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightLime.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.DarkAqua.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkAqua.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.DarkAqua.xaml" > "$(INTERMEDIATE)/NoesisTheme.DarkAqua.xaml.bin.h"

$(INTERMEDIATE)/NoesisTheme.LightAqua.xaml.bin.h: ../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightAqua.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/NoesisTheme.LightAqua.xaml" > "$(INTERMEDIATE)/NoesisTheme.LightAqua.xaml.bin.h"

$(INTERMEDIATE)/PT\ Root\ UI_Regular.otf.bin.h: ../../../Packages/App/Theme/Data/Theme/Fonts/PT\ Root\ UI_Regular.otf | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/Fonts/PT Root UI_Regular.otf" > "$(INTERMEDIATE)/PT Root UI_Regular.otf.bin.h"

$(INTERMEDIATE)/PT\ Root\ UI_Bold.otf.bin.h: ../../../Packages/App/Theme/Data/Theme/Fonts/PT\ Root\ UI_Bold.otf | $(INTERMEDIATE)
	@echo [BIN] [App/Theme] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/Theme/Data/Theme/Fonts/PT Root UI_Bold.otf" > "$(INTERMEDIATE)/PT Root UI_Bold.otf.bin.h"

$(INTERMEDIATE)/%.o: ../../../Packages/App/LangServer/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/LangServer] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/LangServer/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/LangServer] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/LangServerHelpers/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/LangServerHelpers] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/LangServerHelpers/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/LangServerHelpers] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/ApplicationLauncher/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/ApplicationLauncher/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/StatsOverlay.xaml.bin.h: ../../../Packages/App/ApplicationLauncher/Src/StatsOverlay.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/ApplicationLauncher/Src/StatsOverlay.xaml" > "$(INTERMEDIATE)/StatsOverlay.xaml.bin.h"

$(INTERMEDIATE)/MessageBox.xaml.bin.h: ../../../Packages/App/ApplicationLauncher/Src/MessageBox.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/ApplicationLauncher/Src/MessageBox.xaml" > "$(INTERMEDIATE)/MessageBox.xaml.bin.h"

$(INTERMEDIATE)/dripicons-v2.ttf.bin.h: ../../../Packages/App/ApplicationLauncher/Src/dripicons-v2.ttf | $(INTERMEDIATE)
	@echo [BIN] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/ApplicationLauncher/Src/dripicons-v2.ttf" > "$(INTERMEDIATE)/dripicons-v2.ttf.bin.h"

$(INTERMEDIATE)/ModeNine.ttf.bin.h: ../../../Packages/App/ApplicationLauncher/Src/ModeNine.ttf | $(INTERMEDIATE)
	@echo [BIN] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/ApplicationLauncher/Src/ModeNine.ttf" > "$(INTERMEDIATE)/ModeNine.ttf.bin.h"

$(INTERMEDIATE)/generic.xaml.bin.h: ../../../Packages/App/ApplicationLauncher/Src/generic.xaml | $(INTERMEDIATE)
	@echo [BIN] [App/ApplicationLauncher] $(notdir $<)
	$(Q)$(BIN2H) "../../../Packages/App/ApplicationLauncher/Src/generic.xaml" > "$(INTERMEDIATE)/generic.xaml.bin.h"

$(INTERMEDIATE)/%.o: ../../../Packages/App/MediaElement/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/MediaElement] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/MediaElement/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/MediaElement] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Interactivity/Src/%.cpp $(PCH) | $(INTERMEDIATE) data
	@echo [CXX] [App/Interactivity] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/%.o: ../../../Packages/App/Interactivity/Src/%.c | $(INTERMEDIATE) data
	@echo [CC] [App/Interactivity] $(notdir $<)
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)

	$(Q)$(call FIXSLASHES,$(@:.o=.d))

$(INTERMEDIATE)/register.o: register.cpp $(PCH) | $(INTERMEDIATE)
	@echo [CXX] [NoesisApp] $(notdir $<)
	$(Q)$(CXX) $(CXXFLAGS) $(PCHFLAGS) $(INCLUDES) $(DEFINES) -fPIC -c -MMD -MF $(basename $@).d.tmp -MP -o $@ $< && $(call MV,$(basename $@).d.tmp,$(basename $@).d)


$(TARGET): $(LIBS_DEPS) $(INTERMEDIATE)/register.o $(INTERMEDIATE)/EmbeddedXamlProvider.o $(INTERMEDIATE)/LocalXamlProvider.o $(INTERMEDIATE)/FastLZ.o $(INTERMEDIATE)/Find.o $(INTERMEDIATE)/App.Providers.o $(INTERMEDIATE)/EmbeddedTextureProvider.o $(INTERMEDIATE)/FileTextureProvider.o $(INTERMEDIATE)/LocalFontProvider.o $(INTERMEDIATE)/EmbeddedFontProvider.o $(INTERMEDIATE)/FileSystemWatcher.o $(INTERMEDIATE)/LocalTextureProvider.o $(INTERMEDIATE)/Render.GLRenderDevice.o $(INTERMEDIATE)/GLRenderDevice.o $(INTERMEDIATE)/Image.o $(INTERMEDIATE)/RenderContext.o $(INTERMEDIATE)/Render.RenderContext.o $(INTERMEDIATE)/GLRenderContext.mm.o $(INTERMEDIATE)/Render.GLRenderContext.o $(INTERMEDIATE)/Display.o $(INTERMEDIATE)/Audio.mm.o $(INTERMEDIATE)/App.Display.o $(INTERMEDIATE)/XDisplay.o $(INTERMEDIATE)/Gui.XDisplay.o $(INTERMEDIATE)/CommandLine.o $(INTERMEDIATE)/App.Launcher.o $(INTERMEDIATE)/Launcher.o $(INTERMEDIATE)/SaturationEffect.o $(INTERMEDIATE)/OuterShadowBrush.o $(INTERMEDIATE)/ConicGradientBrush.o $(INTERMEDIATE)/InnerShadowBrush.o $(INTERMEDIATE)/PinchEffect.o $(INTERMEDIATE)/VignetteEffect.o $(INTERMEDIATE)/DirectionalBlurEffect.o $(INTERMEDIATE)/BoxShadow.o $(INTERMEDIATE)/CrossFadeBrush.o $(INTERMEDIATE)/App.Effects.o $(INTERMEDIATE)/WavesBrush.o $(INTERMEDIATE)/MonochromeBrush.o $(INTERMEDIATE)/PlasmaBrush.o $(INTERMEDIATE)/PixelateEffect.o $(INTERMEDIATE)/TintEffect.o $(INTERMEDIATE)/DisplayLauncher.o $(INTERMEDIATE)/App.DisplayLauncher.o $(INTERMEDIATE)/ThemeProviders.o $(INTERMEDIATE)/App.Theme.o $(INTERMEDIATE)/LangServerReflectionHelper.o $(INTERMEDIATE)/MessageWriter.o $(INTERMEDIATE)/DocumentHelper.o $(INTERMEDIATE)/App.LangServer.o $(INTERMEDIATE)/MessageBuffer.o $(INTERMEDIATE)/JsonObject.o $(INTERMEDIATE)/CapabilityCompletion.o $(INTERMEDIATE)/CapabilityDiagnostics.o $(INTERMEDIATE)/LangServerSocket.o $(INTERMEDIATE)/CapabilityColor.o $(INTERMEDIATE)/MessageReader.o $(INTERMEDIATE)/LenientXamlParser.o $(INTERMEDIATE)/Workspace.o $(INTERMEDIATE)/LangServer.o $(INTERMEDIATE)/JsonBuilder.o $(INTERMEDIATE)/PreviewRenderer.o $(INTERMEDIATE)/App.LangServerHelpers.o $(INTERMEDIATE)/App.ApplicationLauncher.o $(INTERMEDIATE)/MessageBox.o $(INTERMEDIATE)/Window.o $(INTERMEDIATE)/DelegateCommand.o $(INTERMEDIATE)/LocExtension.o $(INTERMEDIATE)/ApplicationLauncher.o $(INTERMEDIATE)/RichText.o $(INTERMEDIATE)/NotifyPropertyChangedBase.o $(INTERMEDIATE)/Application.o $(INTERMEDIATE)/MediaElement.o $(INTERMEDIATE)/App.MediaElement.o $(INTERMEDIATE)/MediaPlayer.o $(INTERMEDIATE)/SelectAllAction.o $(INTERMEDIATE)/TriggerActionCollection.o $(INTERMEDIATE)/ConditionBehavior.o $(INTERMEDIATE)/CollectionSortBehavior.o $(INTERMEDIATE)/EventTrigger.Interactivity.o $(INTERMEDIATE)/TriggerAction.Interactivity.o $(INTERMEDIATE)/AttachableObject.o $(INTERMEDIATE)/Interaction.o $(INTERMEDIATE)/TimerTrigger.o $(INTERMEDIATE)/FilterPredicate.o $(INTERMEDIATE)/CollectionFilterBehavior.o $(INTERMEDIATE)/Behavior.o $(INTERMEDIATE)/ConditionalExpression.o $(INTERMEDIATE)/PlaySoundAction.o $(INTERMEDIATE)/SelectAction.o $(INTERMEDIATE)/KeyTrigger.o $(INTERMEDIATE)/MoveFocusAction.o $(INTERMEDIATE)/StoryboardAction.o $(INTERMEDIATE)/StringFilterPredicate.o $(INTERMEDIATE)/DataBindingHelper.o $(INTERMEDIATE)/MediaActions.o $(INTERMEDIATE)/ChangePropertyAction.o $(INTERMEDIATE)/MouseDragElementBehavior.o $(INTERMEDIATE)/EventTriggerBase.o $(INTERMEDIATE)/RemoveElementAction.o $(INTERMEDIATE)/TranslateZoomRotateBehavior.o $(INTERMEDIATE)/LaunchUriOrFileAction.o $(INTERMEDIATE)/TargetedTriggerAction.o $(INTERMEDIATE)/SortComparer.o $(INTERMEDIATE)/StoryboardTrigger.o $(INTERMEDIATE)/GamepadTrigger.o $(INTERMEDIATE)/DataEventTrigger.o $(INTERMEDIATE)/ControlStoryboardAction.o $(INTERMEDIATE)/BehaviorCollection.o $(INTERMEDIATE)/BackgroundEffectBehavior.o $(INTERMEDIATE)/App.Interactivity.o $(INTERMEDIATE)/InvokeCommandAction.o $(INTERMEDIATE)/StoryboardCompletedTrigger.o $(INTERMEDIATE)/ComparisonLogic.o $(INTERMEDIATE)/GoToStateAction.o $(INTERMEDIATE)/StringSortComparer.o $(INTERMEDIATE)/TriggerBase.Interactivity.o $(INTERMEDIATE)/TriggerCollection.o $(INTERMEDIATE)/StyleInteraction.o $(INTERMEDIATE)/SetFocusAction.o $(INTERMEDIATE)/DataTrigger.Interactivity.o $(INTERMEDIATE)/LoadContentAction.o $(INTERMEDIATE)/ComparisonCondition.o $(INTERMEDIATE)/PropertyChangedTrigger.o $(INTERMEDIATE)/LineDecorationBehavior.o | $(TARGET_DIR)
	@echo [LD] $(notdir $@)
	+$(Q)$(CXX) $(LDFLAGS) -shared -Wl,--no-undefined -Wl,-soname,libNoesisApp.so -Wl,-rpath,\$$ORIGIN -Wl,-rpath,../../../../Bin/linux_arm -L../../../../Bin/linux_arm $(LIB_DIRS) -o $(TARGET) @$(CONFIG).rsp $(LIBS)
	@echo [STRIP] $(notdir $@)
	$(Q)arm-linux-gnueabihf-objcopy --only-keep-debug $(TARGET) $(TARGET).debug
	$(Q)arm-linux-gnueabihf-strip --strip-unneeded $(TARGET)
	$(Q)arm-linux-gnueabihf-objcopy --add-gnu-debuglink=$(TARGET).debug $(TARGET)

-include $(INTERMEDIATE)/register.d $(INTERMEDIATE)/pch.h.d $(INTERMEDIATE)/EmbeddedXamlProvider.d $(INTERMEDIATE)/LocalXamlProvider.d $(INTERMEDIATE)/FastLZ.d $(INTERMEDIATE)/Find.d $(INTERMEDIATE)/App.Providers.d $(INTERMEDIATE)/EmbeddedTextureProvider.d $(INTERMEDIATE)/FileTextureProvider.d $(INTERMEDIATE)/LocalFontProvider.d $(INTERMEDIATE)/EmbeddedFontProvider.d $(INTERMEDIATE)/FileSystemWatcher.d $(INTERMEDIATE)/LocalTextureProvider.d $(INTERMEDIATE)/Render.GLRenderDevice.d $(INTERMEDIATE)/GLRenderDevice.d $(INTERMEDIATE)/Image.d $(INTERMEDIATE)/RenderContext.d $(INTERMEDIATE)/Render.RenderContext.d $(INTERMEDIATE)/GLRenderContext.mm.d $(INTERMEDIATE)/Render.GLRenderContext.d $(INTERMEDIATE)/Display.d $(INTERMEDIATE)/Audio.mm.d $(INTERMEDIATE)/App.Display.d $(INTERMEDIATE)/XDisplay.d $(INTERMEDIATE)/Gui.XDisplay.d $(INTERMEDIATE)/CommandLine.d $(INTERMEDIATE)/App.Launcher.d $(INTERMEDIATE)/Launcher.d $(INTERMEDIATE)/SaturationEffect.d $(INTERMEDIATE)/OuterShadowBrush.d $(INTERMEDIATE)/ConicGradientBrush.d $(INTERMEDIATE)/InnerShadowBrush.d $(INTERMEDIATE)/PinchEffect.d $(INTERMEDIATE)/VignetteEffect.d $(INTERMEDIATE)/DirectionalBlurEffect.d $(INTERMEDIATE)/BoxShadow.d $(INTERMEDIATE)/CrossFadeBrush.d $(INTERMEDIATE)/App.Effects.d $(INTERMEDIATE)/WavesBrush.d $(INTERMEDIATE)/MonochromeBrush.d $(INTERMEDIATE)/PlasmaBrush.d $(INTERMEDIATE)/PixelateEffect.d $(INTERMEDIATE)/TintEffect.d $(INTERMEDIATE)/DisplayLauncher.d $(INTERMEDIATE)/App.DisplayLauncher.d $(INTERMEDIATE)/ThemeProviders.d $(INTERMEDIATE)/App.Theme.d $(INTERMEDIATE)/LangServerReflectionHelper.d $(INTERMEDIATE)/MessageWriter.d $(INTERMEDIATE)/DocumentHelper.d $(INTERMEDIATE)/App.LangServer.d $(INTERMEDIATE)/MessageBuffer.d $(INTERMEDIATE)/JsonObject.d $(INTERMEDIATE)/CapabilityCompletion.d $(INTERMEDIATE)/CapabilityDiagnostics.d $(INTERMEDIATE)/LangServerSocket.d $(INTERMEDIATE)/CapabilityColor.d $(INTERMEDIATE)/MessageReader.d $(INTERMEDIATE)/LenientXamlParser.d $(INTERMEDIATE)/Workspace.d $(INTERMEDIATE)/LangServer.d $(INTERMEDIATE)/JsonBuilder.d $(INTERMEDIATE)/PreviewRenderer.d $(INTERMEDIATE)/App.LangServerHelpers.d $(INTERMEDIATE)/App.ApplicationLauncher.d $(INTERMEDIATE)/MessageBox.d $(INTERMEDIATE)/Window.d $(INTERMEDIATE)/DelegateCommand.d $(INTERMEDIATE)/LocExtension.d $(INTERMEDIATE)/ApplicationLauncher.d $(INTERMEDIATE)/RichText.d $(INTERMEDIATE)/NotifyPropertyChangedBase.d $(INTERMEDIATE)/Application.d $(INTERMEDIATE)/MediaElement.d $(INTERMEDIATE)/App.MediaElement.d $(INTERMEDIATE)/MediaPlayer.d $(INTERMEDIATE)/SelectAllAction.d $(INTERMEDIATE)/TriggerActionCollection.d $(INTERMEDIATE)/ConditionBehavior.d $(INTERMEDIATE)/CollectionSortBehavior.d $(INTERMEDIATE)/EventTrigger.Interactivity.d $(INTERMEDIATE)/TriggerAction.Interactivity.d $(INTERMEDIATE)/AttachableObject.d $(INTERMEDIATE)/Interaction.d $(INTERMEDIATE)/TimerTrigger.d $(INTERMEDIATE)/FilterPredicate.d $(INTERMEDIATE)/CollectionFilterBehavior.d $(INTERMEDIATE)/Behavior.d $(INTERMEDIATE)/ConditionalExpression.d $(INTERMEDIATE)/PlaySoundAction.d $(INTERMEDIATE)/SelectAction.d $(INTERMEDIATE)/KeyTrigger.d $(INTERMEDIATE)/MoveFocusAction.d $(INTERMEDIATE)/StoryboardAction.d $(INTERMEDIATE)/StringFilterPredicate.d $(INTERMEDIATE)/DataBindingHelper.d $(INTERMEDIATE)/MediaActions.d $(INTERMEDIATE)/ChangePropertyAction.d $(INTERMEDIATE)/MouseDragElementBehavior.d $(INTERMEDIATE)/EventTriggerBase.d $(INTERMEDIATE)/RemoveElementAction.d $(INTERMEDIATE)/TranslateZoomRotateBehavior.d $(INTERMEDIATE)/LaunchUriOrFileAction.d $(INTERMEDIATE)/TargetedTriggerAction.d $(INTERMEDIATE)/SortComparer.d $(INTERMEDIATE)/StoryboardTrigger.d $(INTERMEDIATE)/GamepadTrigger.d $(INTERMEDIATE)/DataEventTrigger.d $(INTERMEDIATE)/ControlStoryboardAction.d $(INTERMEDIATE)/BehaviorCollection.d $(INTERMEDIATE)/BackgroundEffectBehavior.d $(INTERMEDIATE)/App.Interactivity.d $(INTERMEDIATE)/InvokeCommandAction.d $(INTERMEDIATE)/StoryboardCompletedTrigger.d $(INTERMEDIATE)/ComparisonLogic.d $(INTERMEDIATE)/GoToStateAction.d $(INTERMEDIATE)/StringSortComparer.d $(INTERMEDIATE)/TriggerBase.Interactivity.d $(INTERMEDIATE)/TriggerCollection.d $(INTERMEDIATE)/StyleInteraction.d $(INTERMEDIATE)/SetFocusAction.d $(INTERMEDIATE)/DataTrigger.Interactivity.d $(INTERMEDIATE)/LoadContentAction.d $(INTERMEDIATE)/ComparisonCondition.d $(INTERMEDIATE)/PropertyChangedTrigger.d $(INTERMEDIATE)/LineDecorationBehavior.d

clean:
	@echo ==== [NoesisApp] Cleaning '$(CONFIG)' configuration ====
	$(Q)$(call RMTREE,$(INTERMEDIATE))
	$(Q)$(call RM,$(TARGET))
	$(Q)$(call RM,$(TARGET).debug)
