

set(COMPONENT_SRCS 
	"driver/lcd.c"
	"driver/io.c"
	"driver/sig_pwm.c"
	"driver/sensor.c"
	"service/pid.c"
	"service/alarm.c"
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
