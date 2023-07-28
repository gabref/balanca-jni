e1_dll = E1_Balanca01.dll
output_dll = ECTSARA_BAL_ELGIN_DP30CK.dll
dir_class = com/ccibm/ect/perifericos
package = com.ccibm.ect.perifericos
class = BalancaPadraoSara
native = com_ccibm_ect_perifericos_$(class)
jni_include = "C:\Program Files\Java\jdk1.6.0_23\include"
jni_include_win = "C:\Program Files\Java\jdk1.6.0_23\include\win32" 
error_f = errors.c
# e1_dll = E1_Balanca01

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
	gcc jni/$(native).c jni/$(error_f) $(e1_dll) -I$(jni_include) -I$(jni_include_win) -shared -o $(output_dll)
	file $(output_dll)
	nm $(output_dll) | grep "Java" || true
	ldd $(output_dll)
		
# "make sig" to ask the user for a class name, then print the field and method signatures for that class
sig:
	@bash -c 'read -p "Fully-qualified class name (example: java.util.List) ? " CLASSNAME && javap -s $$CLASSNAME';
		
# "make signatures" to print the field and method signatures for the EasyD2XX class
signatures:
	javac com/ccibm/ect/perifericos/BalancaPadraoSara.java -d bin
	javap -s -p bin/BalancaPadraoSara.class
