# invoke SourceDir generated makefile for release.pem4f
release.pem4f: .libraries,release.pem4f
.libraries,release.pem4f: package/cfg/release_pem4f.xdl
	$(MAKE) -f C:\Users\pedro\OneDrive\ESCRIT~1\GENERAL\TEC\SEMEST~3\ARQUI\UNIDAD~2\FOROSY~1\HVAC_un_hilo_obj\Aux_files/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\pedro\OneDrive\ESCRIT~1\GENERAL\TEC\SEMEST~3\ARQUI\UNIDAD~2\FOROSY~1\HVAC_un_hilo_obj\Aux_files/src/makefile.libs clean

