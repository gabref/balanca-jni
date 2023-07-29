# Makefile for compiling JNI code and generating DLLs for different scale models

# list of all scale models
ALL_MODELS := 0 1 2 3

# set the default scale model if not already defined
ifndef MODEL
MODEL = 3
endif

# define the dll name based on the scale model
ifeq ($(MODEL),0)
MODEL_NAME = DP3005
else ifeq ($(MODEL),1)
MODEL_NAME = SA100
else ifeq ($(MODEL),2)
MODEL_NAME = DPSC
else ifeq ($(MODEL),3)
MODEL_NAME = DP30CK
else
$(error Unsupported scale model: $(MODEL))
endif

# define the target dll name
output_dll = ECTSARA_BAL_ELGIN_$(MODEL_NAME).dll

output_dir_x86 = compile/x86
output_dir_x64 = compile/x64
output_dll_x86 = $(output_dir_x86)/$(output_dll)
output_dll_x64 = $(output_dir_x64)/$(output_dll)

e1_dll_x86 = e1_balanca/x86/E1_Balanca01.dll
e1_dll_x64 = e1_balanca/x64/E1_Balanca01.dll

e1_dll = E1_Balanca01.dll
dir_class = com/ccibm/ect/perifericos
package = com.ccibm.ect.perifericos
class = BalancaPadraoSara
native = com_ccibm_ect_perifericos_$(class)
jni_include = "C:\Program Files\Java\jdk1.6.0_23\include"
jni_include_win = "C:\Program Files\Java\jdk1.6.0_23\include\win32" 
error_f = errors.c
test = test

.PHONY: all compile run clean header dll_x86 dll_x64 dll_all sig signatures

compile:
	javac $(dir_class)/*.java

run:
	java $(package).Test

clean:
	rm $(dir_class)/*.class

# "make header" to generate the .h file
header:
	mkdir -p jni
	javac $(dir_class)/$(class).java
	javah $(package).$(class)
	mv $(native).h jni
	rm $(dir_class)/$(class).class

dll: jni/$(native).c
	gcc $< jni/$(error_f) $(e1_dll) -I$(jni_include) -I$(jni_include_win) -shared -o $(output_dll)
	file $(output_dll)
	nm $(output_dll) | grep "Java" || true
	ldd $(output_dll)

test: jni/$(test).c jni/$(test).h
	gcc $< jni/$(native).c jni/$(error_f) -I$(jni_include) -I$(jni_include_win) -o $(test).exe
	./$(test).exe
	rm $(test).exe

dll_x86: jni/$(native).c | $(output_dir_x86)
	i686-w64-mingw32-gcc -m32 $< jni/$(error_f) $(e1_dll_x86) -I$(jni_include) -I$(jni_include_win) -shared -o $(output_dll_x86)
	file $(output_dll_x86)
	nm $(output_dll_x86) | grep "Java" || true
	ldd $(output_dll_x86)

dll_x64: jni/$(native).c | $(output_dir_x64)
	gcc -m64 $< jni/$(error_f) $(e1_dll_x64) -I$(jni_include) -I$(jni_include_win) -shared -o $(output_dll_x64)
	file $(output_dll_x64)
	nm $(output_dll_x64) | grep "Java" || true
	ldd $(output_dll_x64)

dll_allv: dll_x86 dll_x64

$(output_dir_x86):
	mkdir -p $(output_dir_x86)
$(output_dir_x64):
	mkdir -p $(output_dir_x64)
		
dll_all: 
	$(foreach model,$(MODELS), \
		$(MAKE) MODEL=$(model) dll_x64 dll_x86; \
	)

# "make sig" to ask the user for a class name, then print the field and method signatures for that class
sig:
	@bash -c 'read -p "Fully-qualified class name (example: java.util.List) ? " CLASSNAME && javap -s $$CLASSNAME';
		
# "make signatures" to print the field and method signatures for the EasyD2XX class
signatures:
	javac com/ccibm/ect/perifericos/BalancaPadraoSara.java -d bin
	javap -s -p bin/BalancaPadraoSara.class
