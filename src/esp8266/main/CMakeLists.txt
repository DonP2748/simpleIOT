

set(COMPONENT_SRCS 
	"driver/lcd.c"
	"driver/io.c"
	"driver/sensor.c"
	"service/alarm.c"
	"service/button.c"
	"service/schedule.c"
	"service/device.c"
	"service/jsonparse.c"
	"service/process.c"
	"service/network.c"
	"main.c"
	)

set(COMPONENT_ADD_INCLUDEDIRS 	"." 
								"service"
								"driver"	
								)
								
register_component()
