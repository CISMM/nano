



main:	main.obj
	link /DEBUG \
		main.obj \
		quat.lib vrpn.lib vrpn_phantom.lib ghost31.lib \
		wsock32.lib \
		/LIBPATH:/hudson/quat/pc_win32_MTd \
		/LIBPATH:/hudson/vrpn/pc_win32_MTd \
		/LIBPATH:/hudson/vrpn/server_src/pc_win32_MTd \
		/LIBPATH:/hudson/external/pc_win32/lib

main.obj:	main.c
	cl -c main.c /TP \
		-I/hudson/vrpn -I/hudson/vrpn/server_src \
		-I/hudson/external/pc_win32/include/ghost3.1 \
		-I/hudson/external/pc_win32/include/ghost-stl
